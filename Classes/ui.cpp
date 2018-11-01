#define UI_WIDTH 0.2f
#define UI_HEIGHT 0.065f
#define UI_FONT 0

struct UI_State {
  float x = 0.0f;
  float y = 1.0f;
  bool prev_screen_draw = false;
} ui_state;

inline void ui_begin(float x, float y) {
  ui_state.x = x*window_resolution.width;
  ui_state.y = y*window_resolution.height - UI_HEIGHT*window_resolution.height;
}
inline void ui_end() {
}
inline MRect ui_begin_element() {
  MRect r = {ui_state.x, ui_state.y, UI_WIDTH*window_resolution.width, UI_HEIGHT*window_resolution.height};
  ui_state.prev_screen_draw = draw_settings.screen_draw;
  enable_screen_draw();
  return r;
}
inline void ui_end_element() {
  ui_state.y -= UI_HEIGHT*window_resolution.height;
  if (!ui_state.prev_screen_draw) disable_screen_draw();
}
bool button(char *name) {
  MRect r = ui_begin_element();
  bool in_rect = point_in_rect(v2(mouse.x, mouse.y), r);
  if (in_rect) set_draw_color_bytes(245, 175, 50, 255);
  else set_draw_color_bytes(210, 130, 20, 255);
  draw_solid_rect(ui_state.x, ui_state.y, r.w, r.h);
  set_draw_color_bytes(255, 255, 255, 255);
  draw_text(name, ui_state.x, ui_state.y, UI_FONT);
  ui_end_element();
  return point_in_rect(v2(mouse.x, mouse.y), r) && mouse.left.just_pressed;
}
