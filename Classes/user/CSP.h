#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cassert>
#include <cstdio>
#include <cmath>
#include <cfloat>

#include "CSP/math.cpp"
#include "CSP/Dynamic_Array.cpp"
#include "CSP/string.cpp"

#include "CSP/shared.cpp"
#include "entity.cpp"

enum Editor_Mode {
  SELECT_MODE = 0, PLACEMENT_MODE = 1, NUM_EDITOR_MODES = 2
};
struct Editor {
  int mode = SELECT_MODE;
  bool enabled = true;
  bool draw_grid = false;
  bool lock_to_grid = false;
  float grid_size = 100.0f;
  Entity clipboard_entity = get_default_entity();
  V2 selected_entity_mouse_dpos = v2(0.0f, 0.0f);
  Entity *selected_entity = NULL;  
} editor;

#include "CSP/level.cpp"

__draw_rect_fun_type __draw_rect = NULL;
__get_text_rect_fun_type __get_text_rect = NULL;
__draw_text_fun_type __draw_text = NULL;
__load_texture_from_data_fun_type load_texture_from_data = NULL;
__load_texture_from_file_fun_type load_texture_from_file = NULL;
__load_font_fun_type load_font = NULL;
__load_music_fun_type load_music = NULL;
__load_sound_fun_type load_sound = NULL;
__play_music_fun_type __play_music = NULL;
__play_sound_fun_type __play_sound = NULL;
__add_action_binding_fun_type add_action_binding = NULL;
__action_button_fun_type action_button = NULL;
__ui_begin_fun_type ui_begin = NULL;
__button_fun_type button = NULL;
__text_fun_type text = NULL;
__text_field_fun_type __text_field = NULL;
__int_edit_fun_type __int_edit = NULL;
__unsigned_char_edit_fun_type __unsigned_char_edit = NULL;
__color_edit_fun_type color_edit = NULL;
__float_edit_fun_type __float_edit = NULL;
__checkbox_fun_type checkbox = NULL;
__spacing_fun_type __spacing = NULL;

inline void draw_rect(int texture, float x, float y, float w, float h, int z_order = 0, float rotation = 0.0f) {
  __draw_rect(texture, x, y, w, h, z_order, rotation);
}
inline void draw_solid_rect(float x, float y, float w, float h, int z_order = 0, float rotation = 0.0f) {
  __draw_rect(0, x, y, w, h, z_order, rotation);
}
inline Rect get_text_rect(char *text, float x, float y, int font_index, Text_Information *info = NULL) {
  return __get_text_rect(text, x, y, font_index, info);
}
inline Rect draw_text(char *text, float x, float y, int font_index, int z_order = 0) {
  return __draw_text(text, x, y, font_index, z_order);
}
inline int load_texture(unsigned char *data, int width, int height) {
  return load_texture_from_data(data, width, height);
}
inline int load_texture(char *name) {
  return load_texture_from_file(name);
}
inline void play_music(int music_index, bool loop = true) {
  __play_music(music_index, loop);
}
inline void play_sound(int sound_index, bool loop = false) {
  __play_sound(sound_index, loop);
}
inline bool text_field(char *name, String *str, bool multiline = false, char *hash_string = NULL) {
  return __text_field(name, str, multiline, hash_string);
}
inline bool int_edit(char *name, int *value, int min_val = -INT_MAX, int max_val = INT_MAX, char *hash_string = NULL) {
  return __int_edit(name, value, min_val, max_val, hash_string);
}
inline bool unsigned_char_edit(char *name, unsigned char *value, unsigned char min_val = 0, unsigned char max_val = UCHAR_MAX, char *hash_string = NULL) {
  return __unsigned_char_edit(name, value, min_val, max_val, hash_string);
}
inline bool float_edit(char *name, float *value, float min_value = -FLT_MAX, float max_value = FLT_MAX) {
  return __float_edit(name, value, min_value, max_value);
}
inline void spacing(float amount, bool use_default_color = true) {
  __spacing(amount, use_default_color);
}

void main_loop(void);
void initialize(void);
void on_reload(void);

extern "C" __declspec(dllexport) int __load_csp_lib_functions(CSP_Library_Load to_load) {
  void **pointers = (void **)&to_load;
  for (int i = 0; i < sizeof(CSP_Library_Load)/sizeof(void *); i++) {
    assert(pointers[i] != NULL);
  }
  csp = to_load.csp;
  __draw_rect = to_load.draw_rect;
  __get_text_rect = to_load.get_text_rect;
  __draw_text = to_load.draw_text;
  load_texture_from_data = to_load.load_texture_from_data;
  load_texture_from_file = to_load.load_texture_from_file;
  load_font = to_load.load_font;
  load_music = to_load.load_music;
  load_sound = to_load.load_sound;
  __play_music = to_load.play_music;
  __play_sound = to_load.play_sound;
  add_action_binding = to_load.add_action_binding;
  action_button = to_load.action_button;
  ui_begin = to_load.ui_begin;
  button = to_load.button;
  text = to_load.text;
  __text_field = to_load.text_field;
  __int_edit = to_load.int_edit;
  __unsigned_char_edit = to_load.unsigned_char_edit;
  color_edit = to_load.color_edit;
  __float_edit = to_load.float_edit;
  checkbox = to_load.checkbox;
  __spacing = to_load.spacing;
  on_reload();
  return 0;
}
extern "C" __declspec(dllexport)  void __initialize() {
#ifndef _DEBUG
  csp->hotloading_enabled = false;
#endif
  initialize();
}

inline float editor_lock_dim_to_grid(float v, float grid_size) {
  if (grid_size == 0.0f) return v;
  float res = (float)((int)(v / grid_size));
  if (v < 0.0f) res -= 1.0f;
  res *= grid_size;
  return res;
}


extern "C" __declspec(dllexport)  void __main_loop() {
  if (!csp->ui_state.has_keyboard_input && csp->key[KEY_CTRL].is_down && csp->key['o'].just_pressed) editor.enabled = !editor.enabled;

  if (editor.enabled) {
    if (!csp->ui_state.has_keyboard_input && csp->key[KEY_CTRL].is_down && csp->key['p'].just_pressed) {
      if (editor.mode == SELECT_MODE) editor.mode = PLACEMENT_MODE;
      else editor.mode = SELECT_MODE;
    }

    V2 world_mouse = screen_to_world(v2(csp->mouse.x, csp->mouse.y));
    V2 placement_position = world_mouse;
    if (editor.lock_to_grid) {
      placement_position.x = editor_lock_dim_to_grid(world_mouse.x, editor.grid_size);
      placement_position.y = editor_lock_dim_to_grid(world_mouse.y, editor.grid_size);
    }
  
    if (!csp->ui_state.has_keyboard_input && !csp->key[KEY_CTRL].is_down) { // Camera Control:
      int dx = 0, dy = 0;
      if (csp->key['w'].is_down) dy++;
      if (csp->key['s'].is_down) dy--;
      if (csp->key['a'].is_down) dx--;
      if (csp->key['d'].is_down) dx++;
      if (dx != 0 || dy != 0) {
	float m = 300.0f;
	if (csp->key[KEY_SHIFT].is_down) m *= 3.0f;
	V2 dpos = m*csp->dt*normalize(v2((float)dx, (float)dy));
	csp->view.x += dpos.x;
	csp->view.y += dpos.y;
      }
    }

    if (csp->mouse.scroll != 0.0f) { // Zoom:
      V2 prev_dims = csp->view.dims;
      float aspect = csp->view.w / csp->view.h;
    
      float m = 50.0f;
      if (csp->key[KEY_SHIFT].is_down) m *= 3.0f;
      csp->view.w += csp->mouse.scroll*m;
      if (csp->view.w > 0.0f) {
	csp->view.h = csp->view.w / aspect;
	csp->view.pos -= (csp->view.dims - prev_dims)/2.0f; 
      } else {
	csp->view.w -= csp->mouse.scroll*m;
      }
    }
    
    ui_begin(0.8f, 1.0f);

    text("Editor Panel:");
    
    static bool show_view = false;
    if (button(show_view ? "Hide View" : "Show View")) show_view = !show_view;
    if (show_view) {
      float_edit("View X", &csp->view.x);
      float_edit("View Y", &csp->view.y);
      float_edit("View W", &csp->view.w);
      float_edit("View H", &csp->view.h);
      spacing(0.3f);
    }

    static bool show_options = false;
    if (button(show_options ? "Hide Options" : "Show Options")) show_options = !show_options;

    if (show_options) {
      float_edit("Grid Size", &editor.grid_size, 0.0f);
      checkbox("Draw Grid", &editor.draw_grid);
      checkbox("Lock to Grid", &editor.lock_to_grid);
      bool placement_mode = editor.mode == PLACEMENT_MODE;
      if (checkbox("Placement Mode", &placement_mode)) {
	if (placement_mode) editor.mode = PLACEMENT_MODE;
	else editor.mode = SELECT_MODE;
      }
      color_edit("Background Color", &csp->background_color);
      int_edit("Background Texture", &csp->background_texture);
    }
    static bool show_level_options = false;
    if (button(show_level_options ? "Hide Level Options" : "Show Level Options")) show_level_options = !show_level_options;
    if (show_level_options) {
      static char level_name_buffer[256];
      static String level_name = fixed_str(level_name_buffer);
      text_field("Level Name", &level_name);
      if (button("Save Level")) save_level(level_name.str);
      if (button("Load Level")) load_level(level_name.str);
    }
    
    spacing(0.5f);

    text("Entity Panel:");
    if (editor.selected_entity) {
      Entity *s = editor.selected_entity;
      text("Entity Manager Index: %i", s->manager_index);
      int_edit("Type", &s->type, 0, NUM_ENTITY_TYPES - 1);
      text("Type Name: %s", entity_type_names[s->type]);
      int_edit("Texture", &s->texture, 0, csp->textures.length - 1);
      float_edit("X", &s->x);
      float_edit("Y", &s->y);
      float_edit("W", &s->w);
      float_edit("H", &s->h);
      float_edit("Theta", &s->theta);
      color_edit("Color", &s->color);
      int_edit("Z", &s->z_order);
      checkbox("Invisible", &s->invisible);
    } else {
      text("No Selected Entity...");
    }
    static char str_buffer[300];
    static String str = fixed_str(str_buffer);
    text_field("Multiline", &str, true);

    if (csp->key[KEY_CTRL].is_down) {
      if (!csp->ui_state.has_keyboard_input) {
	if (csp->key['v'].just_pressed) {
	  Entity *e = create_entity(&editor.clipboard_entity);
	  e->x = placement_position.x;
	  e->y = placement_position.y;
	}
	if (csp->key['c'].just_pressed && editor.selected_entity) {
	  editor.clipboard_entity = *editor.selected_entity;
	}
      }
      if (csp->key['g'].just_pressed) editor.lock_to_grid = !editor.lock_to_grid;
      if (csp->key['h'].just_pressed) editor.draw_grid = !editor.draw_grid;

      if (csp->key['s'].just_pressed) save_level(current_level_name);
      if (csp->key['r'].just_pressed) load_level(current_level_name);
    }

    if (!csp->ui_state.mouse_hovering_ui && csp->mouse.right.just_pressed) { // Remove Entity:
      Entity *to_delete = NULL;
      float dist_s;
      for (int i = 0; i < csp->entity_manager.length; i++) {
	Entity *e = csp->entity_manager[i];
	if (point_in_entity(world_mouse, e)) {
	  V2 c = v2(e->x + e->w/2.0f, e->y + e->h/2.0f);
	  V2 dpos = world_mouse - c;
	  float d = dpos.x*dpos.x + dpos.y*dpos.y;
	  if (!to_delete || (dist_s > d)) {
	    to_delete = e;
	    dist_s = d;
	  }
	}
      }
      if (to_delete) {
	if (to_delete == editor.selected_entity) editor.selected_entity = NULL;
	ordered_delete_entity(to_delete);
      }
    }

    if (editor.draw_grid && (csp->view.w/editor.grid_size) < 400.0f) {
      float view_width = csp->view.w;
      float view_height = csp->view.h;
      float start_x = editor_lock_dim_to_grid(csp->view.x, editor.grid_size);
      float end_x = editor_lock_dim_to_grid(csp->view.x + view_width, editor.grid_size);
      float start_y = editor_lock_dim_to_grid(csp->view.y, editor.grid_size);
      float end_y = editor_lock_dim_to_grid(csp->view.y + view_height, editor.grid_size);
      const float LW = 5.0f;
      set_draw_color_bytes(255, 255, 255, 100);
      for (float x = start_x; x <= end_x; x += editor.grid_size) {
	draw_rect(0, x - LW/2.0f, csp->view.y, LW, view_height, 0, 0.0f);
      }
      for (float y = start_y; y <= end_y; y += editor.grid_size) {
	draw_rect(0, csp->view.x, y - LW/2.0f, view_width, LW, 0, 0.0f);
      }
    }
    
    if (editor.mode == PLACEMENT_MODE) {
      if (!csp->ui_state.mouse_hovering_ui) {
	Color4B c = editor.clipboard_entity.color;
	c.a /= 4;
	set_draw_color(c);
	draw_rect(editor.clipboard_entity.texture, placement_position.x,
		  placement_position.y, editor.clipboard_entity.w,
		  editor.clipboard_entity.h, editor.clipboard_entity.z_order,
		  editor.clipboard_entity.theta);
      }
      
      if (!csp->ui_state.mouse_hovering_ui && csp->mouse.left.just_pressed) { // Place Entity:
	Entity *e = create_entity(&editor.clipboard_entity);
	e->x = placement_position.x;
	e->y = placement_position.y;
      }
    } else if (editor.mode == SELECT_MODE) {
      if (!csp->ui_state.mouse_hovering_ui) {
	if (csp->mouse.left.just_pressed) {
	  editor.selected_entity = NULL;
	  float dist_s;
	  for (int i = 0; i < csp->entity_manager.length; i++) {
	    Entity *e = csp->entity_manager[i];
	    if (point_in_entity(world_mouse, e)) {
	      V2 c = v2(e->x + e->w/2.0f, e->y + e->h/2.0f);
	      V2 dpos = world_mouse - c;
	      float d = dpos.x*dpos.x + dpos.y*dpos.y;
	      if (!editor.selected_entity || (dist_s > d)) {
		editor.selected_entity = e;
		dist_s = d;
	      }
	    }
	  }
	  if (editor.selected_entity) editor.selected_entity_mouse_dpos = world_mouse - v2(editor.selected_entity->x, editor.selected_entity->y);
	}

	if (editor.selected_entity && csp->mouse.left.is_down) { // Move Entity:
	  if (editor.lock_to_grid) {
	    editor.selected_entity->x = placement_position.x;
	    editor.selected_entity->y = placement_position.y;
	  } else {
	    V2 mdpos = world_mouse - v2(editor.selected_entity->x, editor.selected_entity->y) - editor.selected_entity_mouse_dpos;
	    if ((mdpos.x*mdpos.x + mdpos.y*mdpos.y) > 2.0f) { // Don't move for small position changes
	      editor.selected_entity->x += mdpos.x;
	      editor.selected_entity->y += mdpos.y;
	    }
	  }
	}
      }
    }
  
    if (editor.selected_entity) {
      set_draw_color_bytes(222, 222, 222, 30);
      float w = editor.selected_entity->w*1.1f;
      float h = editor.selected_entity->h*1.1f;
      float dw = w - editor.selected_entity->w;
      float dh = h - editor.selected_entity->h;
      draw_solid_rect(editor.selected_entity->x - dw/2.0f, editor.selected_entity->y - dh/2.0f, w, h, 3, editor.selected_entity->theta);
    }
  
    set_draw_color_bytes(255, 255, 255, 255);
  }
  
  main_loop();
  for (int i = 0; i < csp->entity_manager.length; i++) {
    Entity *e = csp->entity_manager[i];
    if (!e->invisible) {
      set_draw_color(e->color);
      draw_rect(e->texture, e->x, e->y, e->w, e->h, e->z_order, e->theta);
    }
  }
}
