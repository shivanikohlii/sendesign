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

void add_char(String *str, int index, char c) {
  str->length++;
  for (int i = str->length; i > index; i--) str->str[i] = str->str[i - 1];
  str->str[index] = c;
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
  int max_index = str->length - 1; //@CHECK
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
  int max_index = str->length - 1; //@CHECK
  int i = move_right_to_next_whitespace(str, index);
  if (i != index) {
    if (i != max_index || is_whitespace(str->str[i])) i--;
    return remove_character_range(str, index, i);
  }
  return 0;
}
inline int remove_left_characters_up_to_whitespace(String *str, int index) {
  int i = move_left_to_next_whitespace(str, index);
  if (i != index) {
    i++;
    return remove_character_range(str, i, index);
  }
  return 0;
}

bool handle_text_input(String *buffer, int *caret, bool allow_newlines = true) {
  bool changed = false;
  if (characters_typed.length > 0) {
    for (int i = 0; i < characters_typed.length; i++) {
      uchar c = characters_typed[i];
      if (c == '\n' && !allow_newlines) continue;
      add_char(buffer, *caret, (char)c);
      (*caret)++;
      changed = true;
    }
  }
  if (key[KEY_BACKSPACE].just_pressed) {
    if (*caret > 0) {
      if (key[KEY_CTRL].is_down) {
	int num_chars_removed = remove_left_characters_up_to_whitespace(buffer, *caret);
	*caret -= num_chars_removed;
	changed = true;
      } else {
	remove_char(buffer, (*caret) - 1);
	(*caret)--;
	changed = true;
      }
    }
  }

  // @NOTE: We'll remove all of the @CHECKs here after we've tested
  // the text input system further.
  
  if (key[KEY_DELETE].just_pressed && *caret < buffer->length /*@CHECK*/) {
    if (key[KEY_CTRL].is_down) {
      int num_chars_removed = remove_right_characters_up_to_whitespace(buffer, *caret);
      changed = true;
    } else {
      remove_char(buffer, *caret);
      changed = true;
    }
  }

  if (key[KEY_LEFT].just_pressed && *caret > 0) {
    if (key[KEY_CTRL].is_down) {
      *caret = move_left_to_next_whitespace(buffer, *caret);
      changed = true;
    } else {
      (*caret)--;
      changed = true;
    }
  }
  if (key[KEY_RIGHT].just_pressed && *caret < buffer->length /*@CHECK*/) {
    if (key[KEY_CTRL].is_down) {
      *caret = move_right_to_next_whitespace(buffer, *caret);
      changed = true;
    } else {
      (*caret)++;
      changed = true;
    }
  }

  if (key[KEY_HOME].just_pressed && (*caret) != 0) {
    *caret = 0;
    changed = true;
  }
  if (key[KEY_END].just_pressed && (*caret) != (buffer->length) /*@CHECK*/) {
    *caret = buffer->length; //@CHECK
    changed = true;
  }

  terminate_with_null(buffer);
  
  return changed;
}

//
// Actual UI Library:
//

#define UI_WIDTH 0.2f
#define UI_HEIGHT 0.035f
#define UI_FONT 0

struct UI_State {
  float x = 0.0f;
  float y = 1.0f;
  bool prev_screen_draw = false;
  bool has_keyboard_input = false;
  int caret_index = 0;
  char item_with_keyboard_input[256] = {};
  float time_since_text_field_change = 0.0f;
  bool just_changed_ui_input_focus = false;
} ui_state;

inline void ui_begin(float x, float y) {
  ui_state.x = x*window_resolution.width;
  ui_state.y = y*window_resolution.height - UI_HEIGHT*window_resolution.height;
}
inline void ui_end() {}

inline void ui_begin_frame() {
  
}
inline void ui_end_frame() {
  if (!ui_state.just_changed_ui_input_focus) {
    if (mouse.left.just_pressed) {
      ui_state.item_with_keyboard_input[0] = 0;
      ui_state.has_keyboard_input = false;
    }
  } else {
    ui_state.just_changed_ui_input_focus = false;
  }
}

inline Rect ui_begin_element() {
  Rect r = {ui_state.x, ui_state.y, UI_WIDTH*window_resolution.width, UI_HEIGHT*window_resolution.height};
  ui_state.prev_screen_draw = draw_settings.screen_draw;
  enable_screen_draw();
  return r;
}
inline void ui_end_element() {
  ui_state.y -= UI_HEIGHT*window_resolution.height;
  if (!ui_state.prev_screen_draw) disable_screen_draw();
}
bool button(char *name) {
  Rect r = ui_begin_element();
  bool in_rect = point_in_rect(v2(mouse.x, mouse.y), r);
  if (in_rect) set_draw_color_bytes(245, 175, 50, 255);
  else set_draw_color_bytes(210, 130, 20, 255);
  draw_solid_rect(ui_state.x, ui_state.y, r.w, r.h);
  set_draw_color_bytes(255, 255, 255, 255);
  draw_text(name, ui_state.x, ui_state.y, UI_FONT, 1);
  ui_end_element();
  return point_in_rect(v2(mouse.x, mouse.y), r) && mouse.left.just_pressed;
}

bool text_field(char *name, String *str) {
  Rect r = ui_begin_element();
  set_draw_color_bytes(210, 130, 20, 255);
  draw_solid_rect(ui_state.x, ui_state.y, r.w, r.h);
  set_draw_color_bytes(255, 255, 255, 255);
  
  float l_width = get_text_rect("L", ui_state.x, ui_state.y, UI_FONT).w;
  Rect name_text_rect = draw_text(name, ui_state.x, ui_state.y, UI_FONT, 1);
  Rect str_text_rect = draw_text(str->str, name_text_rect.x + name_text_rect.w + l_width*2.0f, ui_state.y, UI_FONT, 1);
  float x = str_text_rect.x;
  float y = ui_state.y;

  bool selected = strcmp(ui_state.item_with_keyboard_input, name) == 0;
  if (mouse.left.just_pressed) {
    if (point_in_rect(v2(mouse.x, mouse.y), r)) {
      selected = true;
      strncpy(ui_state.item_with_keyboard_input, name, sizeof(ui_state.item_with_keyboard_input));
      ui_state.just_changed_ui_input_focus = true;
      ui_state.has_keyboard_input = true;
      
      ui_state.caret_index = (mouse.x - x)/l_width; //@HACK: Assumes monospace font
      if (ui_state.caret_index > str->length) ui_state.caret_index = str->length;
      if (ui_state.caret_index < 0) ui_state.caret_index = 0;
    }
  }
  
  bool changed = false;

  if (selected) {
    ui_state.time_since_text_field_change += dt;
    changed = handle_text_input(str, &ui_state.caret_index, false);
    if (changed) ui_state.time_since_text_field_change = 0.0f;
    if (key['\n'].just_pressed) {
      ui_state.has_keyboard_input = false;
      ui_state.item_with_keyboard_input[0] = 0;
    }
    
    // Getting the position of the caret and drawing it:
    String str_before_caret = substring(*str, 0, ui_state.caret_index);
    char last_char_before_caret = str_before_caret.str[str_before_caret.length];
    char second_to_last_char = str_before_caret.str[str_before_caret.length - 1];
    if (is_whitespace(second_to_last_char))
      str_before_caret.str[str_before_caret.length - 1] = 'L'; //@HACK: This makes sure that ending spaces count toward the cursors position.
    terminate_with_null(&str_before_caret);
    Rect r2 = get_text_rect(str_before_caret.str, x, y, UI_FONT);
    str_before_caret.str[str_before_caret.length] = last_char_before_caret;
    str_before_caret.str[str_before_caret.length - 1] = second_to_last_char;
    char last_char_str[2];
    last_char_str[0] = last_char_before_caret;
    last_char_str[1] = 0;
    Rect char_rect = get_text_rect(last_char_str, x, y, UI_FONT);
    float font_size = fonts[UI_FONT]->fontSize;

    //@REVISE: This won't work for multiline:
    float caret_x = r2.x + r2.w;
    float caret_y = ui_state.y + font_size*0.05f;
    set_draw_color_bytes(10, 215, 30, (u8)255.0f*(0.45f + 0.3f*cos(ui_state.time_since_text_field_change*6.0f)));
    draw_solid_rect(caret_x, caret_y, l_width, font_size, 2);
  }
  
  ui_end_element();
  return changed;
}

inline int compare_names(char *str1, char *str2) { return strcmp(str1, str2); }
Hash_Table<char *, String> ui_str_buffer_storage(djb2_hash, compare_names);

bool int_edit(char *name, int *value) {
  String *str_edit_buffer = ui_str_buffer_storage.retrieve(name);
  if (!str_edit_buffer) {
    String str;
    str.memory_size = 256;
    str.length = 0;
    str.str = (char *)malloc(str.memory_size);
    str_edit_buffer = ui_str_buffer_storage.add(name, str);
  }

  bool selected = strcmp(ui_state.item_with_keyboard_input, name) == 0;

  if (!selected) {
    //@OPTIMIZE: Save the integer and only convert to a string when necessary
    snprintf(str_edit_buffer->str, str_edit_buffer->memory_size, "%i", *value);
    str_edit_buffer->length = strlen(str_edit_buffer->str);
  }
  
  if (text_field(name, str_edit_buffer)) {
    char *parsed_to = NULL;
    long int val = strtol(str_edit_buffer->str, &parsed_to, 10);
    *value = (int)val;
    return true;
  }
  return false;
}

bool float_edit(char *name, float *value) {
  String *str_edit_buffer = ui_str_buffer_storage.retrieve(name);
  if (!str_edit_buffer) {
    String str;
    str.memory_size = 256;
    str.length = 0;
    str.str = (char *)malloc(str.memory_size);
    str_edit_buffer = ui_str_buffer_storage.add(name, str);
  }

  bool selected = strcmp(ui_state.item_with_keyboard_input, name) == 0;

  if (!selected) {
    //@OPTIMIZE: Save the float and only convert to a string when necessary
    snprintf(str_edit_buffer->str, str_edit_buffer->memory_size, "%g", *value);
    str_edit_buffer->length = strlen(str_edit_buffer->str);
  }
  
  if (text_field(name, str_edit_buffer)) {
    char *parsed_to = NULL;
    float val = strtof(str_edit_buffer->str, &parsed_to);
    *value = val;
    return true;
  }
  return false;
}

bool checkbox(char *name, bool *value) {
  Rect r = ui_begin_element();
  bool mouse_hovering = point_in_rect(v2(mouse.x, mouse.y), r);
  bool changed = mouse.left.just_pressed && mouse_hovering;
  if (changed) *value = !(*value);

  if (mouse_hovering) set_draw_color_bytes(245, 175, 50, 255);
  else set_draw_color_bytes(210, 130, 20, 255);
  draw_solid_rect(ui_state.x, ui_state.y, r.w, r.h);
  set_draw_color_bytes(255, 255, 255, 255);
  Rect text_rect = draw_text(name, ui_state.x, ui_state.y, UI_FONT, 1);

  float ui_height = UI_HEIGHT*window_resolution.height;
  float w1 = ui_height*0.9f;
  float w2 = w1*0.72f;
  float x = r.x + r.w - w1 - ui_height*0.1f;
  float y = ui_state.y + (ui_height - w1) / 2.0f;
  set_draw_color_bytes(210, 210, 210, 255);
  draw_solid_rect(x, y, w1, w1, 1);
  if (*value) {
    set_draw_color_bytes(125, 125, 125, 255);
    draw_solid_rect(x + (w1 - w2)/2.0f, y + (w1 - w2)/2.0f, w2, w2, 2);
  }
  
  ui_end_element();
  return changed;
}
