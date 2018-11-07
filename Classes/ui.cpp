bool handle_text_input(String *buffer, int *caret, bool multiline = true, bool *caret_changed = NULL) {
  int prev_caret = *caret;
  bool changed = false;
  if (characters_typed.length > 0) {
    for (int i = 0; i < characters_typed.length; i++) {
      uchar c = characters_typed[i];
      if (c == '\n' && !multiline) continue;
      if (add_char(buffer, *caret, (char)c)) {
	(*caret)++;
	changed = true;
      }
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

  if (multiline) {
    if (key[KEY_UP].just_pressed || key[KEY_DOWN].just_pressed) {
      int start_line_index = *caret;
      for (; start_line_index > 0; start_line_index--) {
	if (buffer->str[start_line_index] == '\n') {
	  start_line_index++;
	  break;
	}
      }
      int caret_line_index = *caret - start_line_index;
      if (key[KEY_UP].just_pressed) *caret = move_up_a_line(*buffer, *caret);
      if (key[KEY_DOWN].just_pressed) *caret = move_down_a_line(*buffer, *caret);
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
    } else {
      (*caret)--;
    }
  }
  if (key[KEY_RIGHT].just_pressed && *caret < buffer->length /*@CHECK*/) {
    if (key[KEY_CTRL].is_down) {
      *caret = move_right_to_next_whitespace(buffer, *caret);
    } else {
      (*caret)++;
    }
  }

  if (multiline) {
    if (key[KEY_HOME].just_pressed) {
      *caret = get_start_line_index(*buffer, *caret);
    }
    if (key[KEY_END].just_pressed) {
      *caret = get_end_line_index(*buffer, *caret);
    }
  } else {
    if (key[KEY_HOME].just_pressed && (*caret) != 0) {
      *caret = 0;
    }
    if (key[KEY_END].just_pressed && (*caret) != (buffer->length) /*@CHECK*/) {
      *caret = buffer->length; //@CHECK
    }
  }
  
  if (caret_changed) *caret_changed = *caret != prev_caret;
  terminate_with_null(buffer);
  
  return changed;
}

//
// Actual UI Library:
//

#define UI_PAD_X 0.15f
#define UI_FONT 0

struct UI_State {
  float x = 0.0f;
  float y = 1.0f;
  float width = 0.2f;
  float height = 0.035f;
  bool prev_screen_draw = false;
  bool has_keyboard_input = false;
  int caret_index = 0;
  char item_with_keyboard_input[256] = {};
  float time_since_text_field_change = 0.0f;
  bool just_changed_ui_input_focus = false;
  bool mouse_hovering_ui = false;
  bool prev_mouse_hovering_ui = false;
} ui_state;

inline void ui_begin(float x, float y) {
  ui_state.x = x*window_resolution.width;
  ui_state.y = y*window_resolution.height - ui_state.height*window_resolution.height;
}
inline void ui_end() {}

inline void ui_begin_frame() {
  
}
inline void ui_end_frame() {
  ui_state.mouse_hovering_ui = ui_state.prev_mouse_hovering_ui;
  ui_state.prev_mouse_hovering_ui = false;
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
  Rect r = {ui_state.x, ui_state.y, ui_state.width*window_resolution.width, ui_state.height*window_resolution.height};
  ui_state.prev_screen_draw = draw_settings.screen_draw;
  enable_screen_draw();
  return r;
}
inline void ui_end_element(float h = 0.0f) {
  if (h == 0.0f) h = ui_state.height*window_resolution.height;
  ui_state.y -= h;
  if (!ui_state.prev_screen_draw) disable_screen_draw();
}
bool button(char *name) {
  Rect r = ui_begin_element();
  bool in_rect = point_in_rect(v2(mouse.x, mouse.y), r);
  if (in_rect) ui_state.prev_mouse_hovering_ui = true;
  if (in_rect) {
    if (mouse.left.is_down) set_draw_color_bytes(247, 220, 80, 255); 
    else set_draw_color_bytes(245, 175, 50, 255);
  } else {
    set_draw_color_bytes(210, 130, 20, 255);
  }
  draw_solid_rect(ui_state.x, ui_state.y, r.w, r.h);
  set_draw_color_bytes(255, 255, 255, 255);
  float start_x = ui_state.x + ui_state.height*window_resolution.height*UI_PAD_X;
  draw_text(name, start_x, ui_state.y, UI_FONT, 1);
  ui_end_element();
  return point_in_rect(v2(mouse.x, mouse.y), r) && mouse.left.just_pressed;
}

void text(char *fmt, ...) {
  va_list arg_list;
  va_start(arg_list, fmt);
  Rect r = ui_begin_element();
  if (point_in_rect(v2(mouse.x, mouse.y), r)) ui_state.prev_mouse_hovering_ui = true;
  set_draw_color_bytes(210, 130, 20, 255);
  draw_solid_rect(ui_state.x, ui_state.y, r.w, r.h);
  set_draw_color_bytes(255, 255, 255, 255);
  char buffer[1024]; //@MEMORY
  vsprintf(buffer, fmt, arg_list);
  float start_x = ui_state.x + ui_state.height*window_resolution.height*UI_PAD_X;
  draw_text(buffer, start_x, ui_state.y, UI_FONT, 1);
  va_end(arg_list);

  ui_end_element();
}

bool text_field(char *name, String *str, bool multiline = false, char *hash_string = NULL) {
  hash_string = hash_string ? hash_string : name;
  Rect r = ui_begin_element();

  Text_Information text_info;
  Rect str_text_rect = get_text_rect(str->str, 0.0f, 0.0f, UI_FONT, &text_info);
  r.y += str_text_rect.y;
  r.h += (text_info.num_lines - 1)*text_info.line_height;
  bool mouse_hovering = point_in_rect(v2(mouse.x, mouse.y), r);
  if (mouse_hovering) ui_state.prev_mouse_hovering_ui = true;

  if (mouse_hovering) set_draw_color_bytes(245, 175, 50, 255);
  else set_draw_color_bytes(210, 130, 20, 255);
  draw_solid_rect(ui_state.x, r.y, r.w, r.h);
  set_draw_color_bytes(255, 255, 255, 255);

  float l_width = fonts[UI_FONT].l_width;
  float start_x = ui_state.x + ui_state.height*window_resolution.height*UI_PAD_X;
  Rect name_text_rect = draw_text(name, start_x, ui_state.y, UI_FONT, 1);
  str_text_rect = draw_text(str->str, name_text_rect.x + name_text_rect.w + l_width*2.0f, ui_state.y, UI_FONT, 1);
  float x = str_text_rect.x;
  float y = ui_state.y;

  bool selected = strcmp(ui_state.item_with_keyboard_input, hash_string) == 0;
  if (mouse.left.just_pressed) {
    if (point_in_rect(v2(mouse.x, mouse.y), r)) {
      selected = true;
      strncpy(ui_state.item_with_keyboard_input, hash_string, sizeof(ui_state.item_with_keyboard_input));
      ui_state.just_changed_ui_input_focus = true;
      ui_state.has_keyboard_input = true;
      
      ui_state.caret_index = (mouse.x - x)/l_width; //@HACK: Assumes monospace font
      if (ui_state.caret_index < 0) ui_state.caret_index = 0;

      if (multiline) {
	int line_num = text_info.num_lines - (int)((mouse.y - r.y)/text_info.line_height);
	if (line_num > text_info.num_lines) line_num = text_info.num_lines;
	if (line_num < 1) line_num = 1;
	int line_start = get_start_index_of_line(*str, line_num);
	int line_end = get_end_line_index(*str, line_start);
	int line_length = line_end - line_start;
	if (ui_state.caret_index > line_length) ui_state.caret_index = line_length;
	ui_state.caret_index += line_start;
      } else {
	if (ui_state.caret_index > str->length) ui_state.caret_index = str->length;
      }
    }
  }
  
  bool changed = false;

  if (selected) {
    ui_state.time_since_text_field_change += dt;
    bool caret_changed = false;
    changed = handle_text_input(str, &ui_state.caret_index, multiline, &caret_changed);
    if (changed || caret_changed) ui_state.time_since_text_field_change = 0.0f;
    if (!multiline && key['\n'].just_pressed) {
      ui_state.has_keyboard_input = false;
      ui_state.item_with_keyboard_input[0] = 0;
    }
    if (key[KEY_ESCAPE].just_pressed) {
      ui_state.has_keyboard_input = false;
      ui_state.item_with_keyboard_input[0] = 0;
    }
    
    // Getting the position of the caret and drawing it:
    int last_line_index = 0;
    int num_newlines = 0;
    if (multiline) {
      for (int i = 0; i < ui_state.caret_index; i++) {
	if (str->str[i] == '\n') {
	  last_line_index = i + 1;
	  num_newlines++;
	}
      }
    }

    String str_before_caret = substring(*str, last_line_index, ui_state.caret_index);
    
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
    float font_size = fonts[UI_FONT].ttf_config->fontSize;

    //@REVISE: This won't work for multiline:
    float caret_x = r2.x + r2.w;
    float caret_y = ui_state.y + font_size*0.05f;
    if (multiline) {
      caret_y -= num_newlines*fonts[UI_FONT].line_height;
    }
    set_draw_color_bytes(10, 215, 30, (u8)255.0f*(0.45f + 0.3f*cos(ui_state.time_since_text_field_change*6.0f)));
    draw_solid_rect(caret_x, caret_y, l_width, font_size, 2);
  }
  
  ui_end_element(r.h);
  return changed;
}

inline int compare_names(char *str1, char *str2) { return strcmp(str1, str2); }
Hash_Table<char *, String> ui_str_buffer_storage(djb2_hash, compare_names);

bool int_edit(char *name, int *value, int min_val = -INT_MAX, int max_val = INT_MAX, char *hash_string = NULL) {
  hash_string = hash_string ? hash_string : name;
  String *str_edit_buffer = ui_str_buffer_storage.retrieve(hash_string);
  if (!str_edit_buffer) {
    String str;
    str.memory_size = 256;
    str.length = 0;
    str.str = (char *)malloc(str.memory_size);
    str_edit_buffer = ui_str_buffer_storage.add(hash_string, str);
  }

  bool selected = strcmp(ui_state.item_with_keyboard_input, hash_string) == 0;

  if (!selected) {
    //@OPTIMIZE: Save the integer and only convert to a string when necessary
    snprintf(str_edit_buffer->str, str_edit_buffer->memory_size, "%i", *value);
    str_edit_buffer->length = strlen(str_edit_buffer->str);
  }
  
  if (text_field(name, str_edit_buffer, false, hash_string)) {
    char *parsed_to = NULL;
    long int val = strtol(str_edit_buffer->str, &parsed_to, 10);
    *value = (int)val;
    if (min_val > max_val) {
      int tmp = min_val;
      min_val = max_val;
      max_val = tmp;
    }
    if (*value > max_val) *value = max_val;
    if (*value < min_val) *value = min_val;
    return true;
  }
  return false;
}
bool u8_edit(char *name, u8 *value, u8 min_val = 0, u8 max_val = UCHAR_MAX, char *hash_string = NULL) {
  int int_val = *value;
  bool changed = int_edit(name, &int_val, min_val, max_val, hash_string);
  *value = (u8)int_val;
  return changed;
}
bool color_edit(char *name, Color4B *color) {
  char name_buffer[256];
  String name_str = fixed_str(name_buffer);
  append(&name_str, name);
  int name_length = name_str.length;
  bool changed = false;
  float prev_ui_state_x = ui_state.x;
  float prev_ui_state_width = ui_state.width;
  float ui_height = ui_state.height*window_resolution.height;
  float w = ui_height*0.9f;
  float x = ui_state.x + ui_state.width*window_resolution.width - w - ui_height*UI_PAD_X;
  float y = ui_state.y + (ui_height - w)/2.0f;
  text("%s:", name);
  bool prev_screen_draw = draw_settings.screen_draw;
  enable_screen_draw();
  set_draw_color(*color);
  draw_solid_rect(x, y, w, w, 1);
  if (!prev_screen_draw) disable_screen_draw();
  
  ui_state.width *= 0.25f;
  
  append(&name_str, "__R");
  if (u8_edit("R", &color->r, 0, UCHAR_MAX, name_str.str)) changed = true;
  ui_state.y += ui_state.height*window_resolution.height;
  ui_state.x += ui_state.width*window_resolution.width;
  
  name_str.length = name_length;
  append(&name_str, "__G");
  if (u8_edit("G", &color->g, 0, UCHAR_MAX, name_str.str)) changed = true;
  ui_state.y += ui_state.height*window_resolution.height;
  ui_state.x += ui_state.width*window_resolution.width;
  
  name_str.length = name_length;
  append(&name_str, "__B");
  if (u8_edit("B", &color->b, 0, UCHAR_MAX, name_str.str)) changed = true;
  ui_state.y += ui_state.height*window_resolution.height;
  ui_state.x += ui_state.width*window_resolution.width;
    
  name_str.length = name_length;
  append(&name_str, "__A");
  if (u8_edit("A", &color->a, 0, UCHAR_MAX, name_str.str)) changed = true;
  ui_state.x = prev_ui_state_x;
  ui_state.width = prev_ui_state_width;

  return changed;
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
  if (mouse_hovering) ui_state.prev_mouse_hovering_ui = true;
  bool changed = mouse.left.just_pressed && mouse_hovering;
  if (changed) *value = !(*value);

  if (mouse_hovering) set_draw_color_bytes(245, 175, 50, 255);
  else set_draw_color_bytes(210, 130, 20, 255);
  draw_solid_rect(ui_state.x, ui_state.y, r.w, r.h);
  set_draw_color_bytes(255, 255, 255, 255);
  float start_x = ui_state.x + ui_state.height*window_resolution.height*UI_PAD_X;
  Rect text_rect = draw_text(name, start_x, ui_state.y, UI_FONT, 1);

  float ui_height = ui_state.height*window_resolution.height;
  float w1 = ui_height*0.9f;
  float w2 = w1*0.72f;
  float x = r.x + r.w - w1 - ui_height*UI_PAD_X;
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
