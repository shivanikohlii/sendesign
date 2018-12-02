enum Noncharacter_Key_Code {
  KEY_SPACE = ' ', KEY_ENTER = '\n', KEY_SHIFT = 128, KEY_ALT = 129,
  KEY_CTRL = 130, KEY_BACKSPACE = 131,
  KEY_INSERT = 132, KEY_HOME = 133, KEY_PAGE_UP = 134, KEY_DELETE = 135,
  KEY_END = 136, KEY_PG_DOWN = 137, KEY_RIGHT = 138, KEY_UP = 139,
  KEY_LEFT = 140, KEY_DOWN = 141, KEY_PRINT_SCREEN = 142, KEY_SCROLL_LOCK = 143,
  KEY_PAUSE = 144, KEY_F1 = 145, KEY_F2 = 146, KEY_F3 = 147, KEY_F4 = 148, KEY_F5 = 149,
  KEY_F6 = 150, KEY_F7 = 151, KEY_F8 = 152, KEY_F9 = 153, KEY_F10 = 154, KEY_F11 = 155,
  KEY_F12 = 156, KEY_CAPS_LOCK = 157, KEY_ESCAPE = 158, KEY_LEFT_SHIFT = 159,
  KEY_RIGHT_SHIFT = 160, KEY_LEFT_CTRL = 161, KEY_RIGHT_CTRL = 162,
  KEY_LEFT_ALT = 163, KEY_RIGHT_ALT = 164,
};
enum Mouse_Button_Code {
  MOUSE_LEFT = 0, MOUSE_RIGHT = 1, NUM_MOUSE_BUTTONS = 2
};

struct Input_Button {
  bool is_down;
  bool just_pressed;
};
struct Mouse {
  union {
    struct {Input_Button left, right;};
    Input_Button mouse_buttons[NUM_MOUSE_BUTTONS];
  };
  float x;
  float y;
  float scroll;
};

struct Draw_Settings {
  Color3B draw_color = color3b(255, 255, 255);
  unsigned char draw_color_opacity = 255;
  bool screen_draw = false;
} __temp_draw_settings;
struct Text_Information {
  int num_lines;
  float line_height;
};
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
};

struct Texture {
  void *cocos_texture;
  int width;
  int height;
};

struct Entity;

struct CSP {
  bool hotloading_enabled;
  bool fullscreen = false;
  float dt = 0.0f;
  int windowed_mode_width = 1366;
  int windowed_mode_height = 768;
  int window_width = 1366;
  int window_height = 768;
  Rect view = {0.0f, 0.0f, 300.0f, 300.0f/1.778645833f};
  
  Dynamic_Array<Entity *> entity_manager = Dynamic_Array<Entity *>(32, false);
  Mouse mouse = {};
  Input_Button key[256] = {};
  Dynamic_Array<unsigned char> keys_just_pressed = Dynamic_Array<unsigned char>(8, false);
  Dynamic_Array<unsigned char> characters_typed = Dynamic_Array<unsigned char>(8, false);
  UI_State ui_state;
  Draw_Settings draw_settings;
  Dynamic_Array<Texture> textures = Dynamic_Array<Texture>(8, false);

  Color4B background_color = color4b(200, 200, 200, 255);
  int background_texture = 0;
};

CSP *csp = NULL;

typedef void (*__draw_rect_fun_type)(int, float, float, float, float, int, float);
typedef Rect (*__get_text_rect_fun_type)(char *, float, float, int, Text_Information *);
typedef Rect (*__draw_text_fun_type)(char *, float, float, int, int);
typedef int (*__load_texture_from_data_fun_type)(unsigned char *, int, int);
typedef int (*__load_texture_from_file_fun_type)(char *);
typedef int (*__load_font_fun_type)(char *, int);
typedef int (*__load_music_fun_type)(char *);
typedef int (*__load_sound_fun_type)(char *);
typedef void (*__play_music_fun_type)(int, bool);
typedef void (*__play_sound_fun_type)(int, bool);
typedef void (*__add_action_binding_fun_type)(char *, Input_Button *);
typedef Input_Button (*__action_button_fun_type)(char *);
typedef void (*__ui_begin_fun_type)(float, float);
typedef bool (*__button_fun_type)(char *);
typedef void (*__text_fun_type)(char *, ...);
typedef bool (*__text_field_fun_type)(char *, String *, bool, char *);
typedef bool (*__int_edit_fun_type)(char *, int *, int, int, char *);
typedef bool (*__unsigned_char_edit_fun_type)(char *, unsigned char *, unsigned char, unsigned char, char *);
typedef bool (*__color_edit_fun_type)(char *, Color4B *);
typedef bool (*__float_edit_fun_type)(char *, float *, float, float);
typedef bool (*__checkbox_fun_type)(char *, bool *);
typedef void (*__spacing_fun_type)(float, bool);

struct CSP_Library_Load {
  CSP *csp;
  __draw_rect_fun_type draw_rect;
  __get_text_rect_fun_type get_text_rect;
  __draw_text_fun_type draw_text;
  __load_texture_from_data_fun_type load_texture_from_data;
  __load_texture_from_file_fun_type load_texture_from_file;
  __load_font_fun_type load_font;
  __load_music_fun_type load_music;
  __load_sound_fun_type load_sound;
  __play_music_fun_type play_music;
  __play_sound_fun_type play_sound;
  __add_action_binding_fun_type add_action_binding;
  __action_button_fun_type action_button;
  __ui_begin_fun_type ui_begin;
  __button_fun_type button;
  __text_fun_type text;
  __text_field_fun_type text_field;
  __int_edit_fun_type int_edit;
  __unsigned_char_edit_fun_type unsigned_char_edit;
  __color_edit_fun_type color_edit;
  __float_edit_fun_type float_edit;
  __checkbox_fun_type checkbox;
  __spacing_fun_type spacing;
};

inline void set_draw_color(float red, float green, float blue, float alpha) {
  csp->draw_settings.draw_color.r = (unsigned char)(red*255.0f);
  csp->draw_settings.draw_color.g = (unsigned char)(green*255.0f);
  csp->draw_settings.draw_color.b = (unsigned char)(blue*255.0f);
  csp->draw_settings.draw_color_opacity = (unsigned char)(alpha*255.0f);
}
inline void set_draw_color(Color4B color) {
  csp->draw_settings.draw_color.r = color.r;
  csp->draw_settings.draw_color.g = color.g;
  csp->draw_settings.draw_color.b = color.b;
  csp->draw_settings.draw_color_opacity = color.a;
}
inline void set_draw_color(Color4 color) {
  set_draw_color(color4b(color));
}
inline void set_draw_color_bytes(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha) {
  csp->draw_settings.draw_color.r = red;
  csp->draw_settings.draw_color.g = green;
  csp->draw_settings.draw_color.b = blue;
  csp->draw_settings.draw_color_opacity = alpha;
}
inline void enable_screen_draw() {csp->draw_settings.screen_draw = true;}
inline void disable_screen_draw() {csp->draw_settings.screen_draw = false;}

inline V2 screen_to_world(V2 v, Rect view1 = csp->view) {
  return v2(view1.x + v.x/csp->window_width*view1.w,
	    view1.y + v.y/csp->window_height*view1.h);
}
inline Rect screen_to_world(Rect r1, Rect view1 = csp->view) {
  return make_rect(view1.x + r1.x/csp->window_width*view1.w,
		   view1.y + r1.y/csp->window_height*view1.h,
		   r1.w/csp->window_width*view1.w,
		   r1.h/csp->window_height*view1.h);
}
inline V2 world_to_screen(V2 v, Rect view1 = csp->view) {
  return v2((csp->window_width*(v.x - view1.x))/view1.w,
	    (csp->window_height*(v.y - view1.y))/view1.h);
}
inline Rect world_to_screen(Rect r1, Rect view1 = csp->view) {
  return make_rect((csp->window_width*(r1.x - view1.x))/view1.w,
		   (csp->window_height*(r1.y - view1.y))/view1.h,
		   (r1.w*csp->window_width)/view1.w,
		   (r1.h*csp->window_height)/view1.h);
}
