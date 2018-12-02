#ifndef CSP_HASH_TABLE_CPP
#define CSP_DEFAULT_HASH_TABLE_SIZE 128
#define CSP_MAX_HASH_TABLE_KEY_SIZE 128

unsigned long long djb2_hash(unsigned char *str) {
  unsigned long long hash = 5381;
  int c;
  while (c = *str++) hash = ((hash << 5) + hash) + c;
  return hash;
}
inline unsigned long long djb2_hash(char *str) { return djb2_hash((unsigned char *)str); }

template <class Data_Type> struct String_Hash_Record {
  char key[CSP_MAX_HASH_TABLE_KEY_SIZE];
  Data_Type data;
};
template <class Data_Type> struct String_Hash_Slot {
  Dynamic_Array<String_Hash_Record<Data_Type>> *entries;
};
template <class Data_Type> struct String_Hash_Table {
  Dynamic_Array<String_Hash_Slot<Data_Type>> slots;

  bool remove(char *key) {
    if (slots.length == 0) return false;
    unsigned long long slot_index = djb2_hash(key) % slots.length;
    String_Hash_Slot<Data_Type> slot = slots[slot_index];
    if (!slot.entries) return false;
    for (int i = 0; i < slot.entries->length; i++) {
      if (key_compare(slot.entries->data[i].key, key) == 0) {
	slot.entries->remove_at(i);
	return true;
      }
    }
    return false;
  }
  Data_Type *add(char *key, Data_Type data) {
    if (slots.length == 0) {
      slots.set_length(CSP_DEFAULT_HASH_TABLE_SIZE);
      memset(slots.data, 0, sizeof(String_Hash_Slot<Data_Type>)*CSP_DEFAULT_HASH_TABLE_SIZE);
    }
    unsigned long long slot_index = djb2_hash(key) % slots.length;
    String_Hash_Record<Data_Type> record;
    strncpy(record.key, key, CSP_MAX_HASH_TABLE_KEY_SIZE);
    record.data = data;
    if (!slots[slot_index].entries) {
      slots[slot_index].entries = (Dynamic_Array<String_Hash_Record<Data_Type>> *)malloc(sizeof(Dynamic_Array<String_Hash_Record<Data_Type>>));
      slots[slot_index].entries->initialize(4); // 4 item slots to start
    }
    Dynamic_Array<String_Hash_Record<Data_Type>> *entries = slots[slot_index].entries;
    entries->add(record);
    return &entries->data[entries->length - 1].data;
  }

  bool retrieve(char *key, Data_Type *out) {
    if (slots.length == 0) return false;
    unsigned long long slot_index = djb2_hash(key) % slots.length;
    String_Hash_Slot<Data_Type> slot = slots[slot_index];
    if (slot.entries) {
      for (int i = 0; i < slot.entries->length; i++) {
	if (strncmp(slot.entries->data[i].key, key, CSP_MAX_HASH_TABLE_KEY_SIZE) == 0) {
	  *out = slot.entries->data[i].data;
	  return true;
	}
      }
    }
    return false;
  }
  // @Warning: No guarantees how long this pointer will last if you decide
  // to keep it. It may become invalid if the dynamic array holding
  // this pointer has to resize, which can happen whenever you add something to
  // the hash table.
  Data_Type *retrieve(char *key) {
    if (slots.length == 0) return NULL;
    unsigned long long slot_index = djb2_hash(key) % slots.length;
    String_Hash_Slot<Data_Type> slot = slots[slot_index];
    if (slot.entries) {
      for (int i = 0; i < slot.entries->length; i++) {
	if (strncmp(slot.entries->data[i].key, key, CSP_MAX_HASH_TABLE_KEY_SIZE) == 0) {
	  return &slot.entries->data[i].data;
	}
      }
    }
    return NULL;
  }

  inline Dynamic_Array<String_Hash_Record<Data_Type>> *retrieve_records(unsigned long long key) {
    if (slots.length == 0) return NULL;
    return slots[key % slots.length].entries;
  }
};

#define CSP_HASH_TABLE_CPP
#endif
