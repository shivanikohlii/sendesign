//
// Binding Code:
//

//@TODO: Axis bindings?
//@TODO: Controller support?

struct Binding {
  Dynamic_Array<Input_Button *> *button_bindings;
  Input_Button button_state;
};

String_Hash_Table<Binding *> button_binding_table;
Dynamic_Array<Binding *> button_binding_list;

void add_button_binding(char *action, Input_Button *button_binding) {
  Binding *binding = NULL;
  bool in_table = button_binding_table.retrieve(action, &binding);
  if (!in_table) {
    binding = (Binding *)malloc(sizeof(Binding));
    binding->button_state = {};
    binding->button_bindings = (Dynamic_Array<Input_Button *> *)malloc(sizeof(Dynamic_Array<Input_Button *>));
    binding->button_bindings->initialize(2);
    button_binding_table.add(action, binding);
    button_binding_list.add(binding);
  }
  binding->button_bindings->add(button_binding);
}

inline void bind_keyboard_button(char *action, int key_code) {
  assert(key_code >= 0);
  assert(key_code <= 255);
  add_button_binding(action, &csp->key[key_code]);
}
inline void bind_mouse_button(char *action, int mouse_button_code) {
  assert(mouse_button_code >= 0);
  assert(mouse_button_code < NUM_MOUSE_BUTTONS);
  add_button_binding(action, &csp->mouse.mouse_buttons[mouse_button_code]);
}
inline Input_Button action_button(char *action_name) {
  Binding *binding = NULL;
  bool action_bound = button_binding_table.retrieve(action_name, &binding);
  assert(action_bound);
  return binding->button_state;
}

void update_bindings() {
  for (int i = 0; i < button_binding_list.length; i++) {
    Binding *binding = button_binding_list[i];
    binding->button_state.just_pressed = false;
    binding->button_state.is_down = false;
    for (int j = 0; j < binding->button_bindings->length; j++) {
      Input_Button *b = binding->button_bindings->data[j];
      if (b->just_pressed) binding->button_state.just_pressed = true;
      if (b->is_down) binding->button_state.is_down = true;
    }
  }
}

#if 0
// Example Code:
void init() {
  bind_keyboard_button("sound", 'e');
}

void main_loop() {
  if (action_button("sound").just_pressed) {
    play_sound(0);
  }
}
#endif

//
// Game Startup Code:
//

void problem_loading(char *msg) {
#if DEBUG
  char buffer[300];
  sprintf(buffer, "Problem Loading: %s\n", msg);
  MessageBox(buffer, "Load Failure");
  assert(false);
#endif
}

bool initialize() {
  auto visibleSize = cocos2d::Director::getInstance()->getVisibleSize();
  cocos2d::Vec2 origin = cocos2d::Director::getInstance()->getVisibleOrigin();

  screen_layer = cocos2d::Layer::create();
  
  main_scene->addChild(screen_layer, 1000);

  load_font("Inconsolata-Regular.ttf", 23);
  load_font("AndBasR.ttf", 24);

  { // Make Blank White Texture:
    const int W = 4;
    const int H = 4;
    unsigned char data[4*W*H];
    for (int i = 0; i < sizeof(data); i++) data[i] = 255;
    load_texture_from_data(data, W, H);
  }

  char *supported_formats[] = {"png", "jpeg", "jpg", "bmp", "tga", "gif", "psd"};
  Dynamic_Array<char *> bitmap_filenames;
  get_filenames_in_directory("data/bitmaps", &bitmap_filenames);
  for (int i = 0; i < bitmap_filenames.length; i++) {
    char *ext = file_extension(bitmap_filenames[i]);
    bool supported_ext = false;
    for (int j = 0; j < array_size(supported_formats); j++) {
      if (strcmp(ext, supported_formats[j]) == 0) {
	load_texture_from_file(bitmap_filenames[i]);
	break;
      }
    }

    free(bitmap_filenames[i]);
  }

  // NOTE: For some reason, we can't load .wav files. Not sure if they're supported, but they're a much better format to use for sound effects.
  // TODO: Figure out if they're supported, if not we may want to consider supporting it ourselves if we have time...

  // Load All Sounds in the sounds directory:
  Dynamic_Array<char *> sound_filenames;
  get_filenames_in_directory("data/sounds", &sound_filenames);
  for (int i = 0; i < sound_filenames.length; i++) {
    if (strcmp(file_extension(sound_filenames[i]), "mp3") == 0) load_sound(sound_filenames[i]);
    free(sound_filenames[i]);
  }
  load_music("night.mp3");
  play_music(0);
  
  main_scene->scheduleUpdate();
  return true;
}

//
// Main Loop (This Gets Called Every Frame (~16 MS)):
//

void main_loop() {
  { // Update View:
    csp->view.h = csp->view.w * ((float)csp->window_height / (float)csp->window_width);
    float scale_x = csp->window_width / csp->view.w;
    float scale_y = csp->window_height / csp->view.h;
    main_scene->setScaleX(scale_x);
    main_scene->setScaleY(scale_y);
    
    main_scene->setPosition(-csp->view.x*scale_x, -csp->view.y*scale_y);
    screen_layer->setAdditionalTransform(main_scene->getWorldToNodeTransform());
  }

  set_draw_color(csp->background_color);
  draw_rect(csp->background_texture, csp->view.x, csp->view.y, csp->view.w, csp->view.h, -1000, 0.0f);
}
