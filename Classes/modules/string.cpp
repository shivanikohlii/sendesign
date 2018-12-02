#ifndef CSP_STRING_CPP

//
// Text Input String Operations:
//

inline bool is_whitespace(char c) {
  return (c == ' ' || c == '\n' || c == '\r' || c == '\t');
}

struct String {
  char *str;
  int length;
  int memory_size;
};

inline int string_compare(char *str1, char *str2) { return strcmp(str1, str2); }
inline String make_string(char *str_buffer, int length, int memory_size) {
  String str;
  str.str = str_buffer;
  str.length = length;
  str.memory_size = memory_size;
  return str;
}
inline String make_string(char *str, int length) {
  return make_string(str, length, length);
}

#define _S(str) (make_string(str, sizeof(str) - 1, sizeof(str)))
#define fixed_str(str) (make_string(str, 0, sizeof(str)))

inline bool terminate_with_null(String *str) {
  if (str->length < str->memory_size) {
    str->str[str->length] = 0;
    return true;
  }
  return false;
}

bool append(String *str, char *to_append) {
  int i = str->length;
  int j = 0;
  for (; to_append[j] != 0; j++, i++) {
    if (i >= str->memory_size) return false;
    str->str[i] = to_append[j];
  }
  str->length += j;
  return terminate_with_null(str);
}
String substring(String str, int start_index, int end_index) {
  str.str += start_index;
  str.length = end_index - start_index;
  str.memory_size -= start_index;
  return str;
}
String substring(String str, int start_index) {
  str.str += start_index;
  str.length -= start_index;
  str.memory_size -= start_index;
  return str;
}

bool add_char(String *str, int index, char c) {
  if (str->length >= (str->memory_size - 1)) return false;
  str->length++;
  for (int i = str->length; i > index; i--) str->str[i] = str->str[i - 1];
  str->str[index] = c;
  return true;
}
void remove_char(String *str, int index) {
  for (int i = index; i <= str->length; i++) str->str[i] = str->str[i + 1];
  str->length--;
}
int remove_character_range(String *str, int start_index, int end_index) {
  int max_str_index = str->length; //@CHECK: Might be str->length + 1
  int num_chars = end_index - start_index + 1;
  for (int i = start_index; i < max_str_index - num_chars; i++) {
    int char_index = i + num_chars;
    str->str[i] = str->str[char_index];
  }
  str->length -= num_chars;
  return num_chars;
}
int move_right_to_next_whitespace(String *str, int index) {
  int max_index = str->length; 
  bool hit_non_whitespace = false;
  int i1 = index;
  for (; i1 <= max_index; i1++) {
    char c = str->str[i1];
    if (hit_non_whitespace) {
      if (is_whitespace(c)) break;
    } else {
      if (!is_whitespace(c) && c != 0) hit_non_whitespace = true;
    }
  }
  if (i1 > max_index) i1 = max_index;
  return i1;
}
int move_left_to_next_whitespace(String *str, int index) {
  bool hit_non_whitespace = false;
  int i1 = index - 1;
  for (; i1 >= 0; i1--) {
    char c = str->str[i1];
    if (hit_non_whitespace) {
      if (is_whitespace(c)) break;
    } else {
      if (!is_whitespace(c) && c != 0) hit_non_whitespace = true;
    }
  }
  i1++;
  if (i1 > index) i1 = index;
  return i1;
}
inline int remove_right_characters_up_to_whitespace(String *str, int index) {
  int max_index = str->length; 
  int i = move_right_to_next_whitespace(str, index);
  if (i != index) {
    if (i != max_index && is_whitespace(str->str[i])) {
      i--;
    } else if (i == max_index) {
      index++; //@HACK
    }
    return remove_character_range(str, index, i);
  }
  return 0;
}
inline int remove_left_characters_up_to_whitespace(String *str, int index) {
  int i = move_left_to_next_whitespace(str, index);
  if (i != index) {
    return remove_character_range(str, i, index - 1);
  }
  return 0;
}

int get_start_index_of_line(String str, int line) {
  if (line == 1) return 0;
  int current_line = 1;
  for (int i = 0; i < str.length; i++) {
    if (str.str[i] == '\n') {
      current_line++;
      if (current_line == line) {
	int v0 = i + 1;
	int v1 = str.length - 1;
	return (v0 > v1) ? v1 : v0;
      }
    }
  }
  return 0;
}
int get_start_line_index(String str, int index) {
  int start_line_index = index > 0 ? (index - 1) : index;
  for (; start_line_index > 0; start_line_index--) {
    if (str.str[start_line_index] == '\n') {
      start_line_index++;
      break;
    }
  }
  return start_line_index;
}
int get_end_line_index(String str, int index) {
  int end_line_index = index;
  for (; end_line_index < str.length; end_line_index++) {
    if (str.str[end_line_index] == '\n') break;
  }
  return end_line_index;
}
int move_up_a_line(String str, int index) {
  int start_line_index = get_start_line_index(str, index);
  if (start_line_index == 0) return index;
  int upper_line_end_index = start_line_index - 1;
  if (upper_line_end_index < 0) upper_line_end_index = 0;
  int upper_line_start_index = get_start_line_index(str, upper_line_end_index);
  int upper_line_length = (upper_line_end_index - upper_line_start_index);
  int caret_line_index = index - start_line_index;
  int new_caret_line_index = caret_line_index;
  if (new_caret_line_index > upper_line_length) new_caret_line_index = upper_line_length;
  index = upper_line_start_index + new_caret_line_index;
  assert(index >= 0);
  assert(index <= str.length);
  return index;
}
int move_down_a_line(String str, int index) {
  int start_line_index = get_start_line_index(str, index);
  int end_line_index = get_end_line_index(str, index);
  if (end_line_index == str.length) return index;
  int lower_line_end_index = get_end_line_index(str, end_line_index + 1);
  int lower_line_length = (lower_line_end_index - end_line_index);
  int caret_line_index = index - start_line_index;
  int new_caret_line_index = caret_line_index;
  if (new_caret_line_index > (lower_line_length - 1)) new_caret_line_index = lower_line_length - 1;
  index = end_line_index + 1 + new_caret_line_index;
  assert(index >= 0);
  assert(index <= str.length);
  return index;
}

#define CSP_STRING_CPP
#endif
