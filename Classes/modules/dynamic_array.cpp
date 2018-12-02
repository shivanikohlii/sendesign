#ifndef CSP_DYNAMIC_ARRAY_CPP
#define CSP_DYNAMIC_ARRAY_START_SIZE 32
inline void _dynamic_array_allocate(void **data_ret, int *items_ret,
                          int start_items, int item_size) {
  int bytes = start_items * item_size;
  void *data = malloc(bytes);
  *data_ret = data;
  *items_ret = start_items;
}
inline void _dynamic_array_expand(void **data_ret, int *items_ret,
                        int item_size) {
  int new_items = 2 * (*items_ret);
  int new_bytes = new_items * item_size;
  *data_ret = realloc(*data_ret, new_bytes);
  assert(*data_ret != 0);
  *items_ret = new_items;
}
inline void _dynamic_array_resize(void **data_ret, int new_num_items, int item_size) {
  *data_ret = realloc(*data_ret, new_num_items*item_size);
  assert(*data_ret != 0);
}
inline void _dynamic_array_deallocate(void **data_ret) {
  if (*data_ret) free(*data_ret);
  *data_ret = 0;
}

template <class T> struct Dynamic_Array {
public:
  Dynamic_Array(int start_size = CSP_DYNAMIC_ARRAY_START_SIZE, bool deallocate = true) {
    initialize(start_size, deallocate);
  }
  Dynamic_Array(T *initial_items, int num_items) {
    initialize();
    for (int i = 0; i < num_items; i++) add(initial_items[i]);
  }
  ~Dynamic_Array() {
    if (auto_deallocate) _dynamic_array_deallocate((void **)&data);
  }

  int length;
  int allocated_items;
  bool auto_deallocate;
  T *data;

  // NOTE: Call this if the constructor doesn't get called:
  inline void initialize(int start_size = CSP_DYNAMIC_ARRAY_START_SIZE, bool deallocate = true) {
    length = 0;
    _dynamic_array_allocate((void **)&data, &allocated_items,
			    start_size, sizeof(T));
    allocated_items = start_size;
    auto_deallocate = deallocate;
  }

  inline void free() {
    _dynamic_array_deallocate((void **)&data);
  }

  inline void zero_mem(int start_item = 0) {
    zero_memory(data[start_item], size_of(T)*length);
  }
  inline bool resize(int new_length) {
    if (new_length > allocated_items) {
      _dynamic_array_resize((void **)&data, new_length, sizeof(T));
      allocated_items = new_length;
      return true;
    }
    return false;
  }
  inline bool set_length(int new_length) {
    if (length == new_length) return false;
    resize(new_length);
    length = new_length;
    return true;
  }
  inline void add(T item) {
    if (length == allocated_items)
      _dynamic_array_expand((void **)&data, &allocated_items, sizeof(T));

    data[length++] = item;
  }
  inline bool _set_length_with_size(int new_length, int item_size) {
    if (length == new_length) return false;
    length = new_length;
    if (new_length > allocated_items) {
      _dynamic_array_resize((void **)&data, new_length, item_size);
      allocated_items = new_length;
    }
    return true;
  }
  void add_at(int index, T item) {
    assert(index <= length);

    if (length == allocated_items)
      _dynamic_array_expand((void **)&data, &allocated_items, sizeof(T));

    int i;
    for (i = length; i > index; i--) data[i] = data[i - 1];
    data[index] = item;
    length++;
  }
  bool remove(T item) {
    int i;
    for (i = 0; i < length; i++) {
      if (data[i] == item) {
	data[i] = data[length - 1];
	length--;
	return true;
      }
    }

    return false;
  }
  inline T top() {
    assert(length > 0);
    return data[length - 1];
  }
  
  inline T remove_at(int index) {
    T result = data[index];
    data[index] = data[length - 1];
    length--;

    return result;
  }
  inline T remove_top() {
    length--;
    return data[length];
  }
  T ordered_remove_at(int index) {
    T result = data[index];
    length--;
    for (int i = index; i < length; i++) data[i] = data[i + 1];
    return result;
  }
  bool contains(T element) {
    for (int i = 0; i < length; i++) {
      if (data[i] == element) return true;
    }
    return false;
  }

  T &operator [](const int i) const {
    assert(i >= 0);
    assert(i < length);

    return data[i];
  }
  int quick_sort_partition(int lo, int hi, int (*compare_function)(T, T)) {
    T p = data[lo];
    int i = lo - 1;
    int j = hi + 1;
    for (;;) {
      i++;
      while (compare_function(data[i], p) < 0) i++;
      j--;
      while (compare_function(data[j], p) > 0) j--;
      if (i >= j) return j;
      T tmp = data[i];
      data[i] = data[j];
      data[j] = tmp;
    }
  }
  int quick_sort_partition(int lo, int hi, float (*compare_function)(T, T)) {
    T p = data[lo];
    int i = lo - 1;
    int j = hi + 1;
    for (;;) {
      i++;
      while (compare_function(data[i], p) < 0) i++;
      j--;
      while (compare_function(data[j], p) > 0) j--;
      if (i >= j) return j;
      T tmp = data[i];
      data[i] = data[j];
      data[j] = tmp;
    }
  }
  void quick_sort(int lo, int hi, int (*compare_function)(T, T)) {
    if (lo < hi) {
      int p = quick_sort_partition(lo, hi, compare_function);
      quick_sort(lo, p, compare_function);
      quick_sort(p + 1, hi, compare_function);
    }
  }
  void quick_sort(int lo, int hi, float (*compare_function)(T, T)) {
    if (lo < hi) {
      int p = quick_sort_partition(lo, hi, compare_function);
      quick_sort(lo, p, compare_function);
      quick_sort(p + 1, hi, compare_function);
    }
  }
  inline void quick_sort(int (*compare_function)(T, T)) {
    quick_sort(0, length - 1, compare_function);
  }
  inline void quick_sort(float (*compare_function)(T, T)) {
    quick_sort(0, length - 1, compare_function);
  }
  void insertion_sort(int lo, int hi, int (*compare_function)(T, T)) {
    for (int i = lo; i <= hi; i++) {
      for (int j = i; j > lo && compare_function(data[j], data[j - 1]) < 0; j--) {
	T tmp = data[j];
	data[j] = data[j - 1];
	data[j - 1] = tmp;
      }
    }
  }
  void insertion_sort(int lo, int hi, float (*compare_function)(T, T)) {
    for (int i = lo; i <= hi; i++) {
      for (int j = i; j > lo && compare_function(data[j], data[j - 1]) < 0; j--) {
	T tmp = data[j];
	data[j] = data[j - 1];
	data[j - 1] = tmp;
      }
    }
  }
  inline void insertion_sort(int (*compare_function)(T, T)) {
    insertion_sort(0, length - 1, compare_function);
  }
  inline void insertion_sort(float (*compare_function)(T, T)) {
    insertion_sort(0, length - 1, compare_function);
  }
};

#define CSP_DYNAMIC_ARRAY_CPP
#endif
