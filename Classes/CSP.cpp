//
// Entity System:
//

struct Entity {
  cocos2d::Sprite *sprite;
  int texture;
  int manager_index;
  union {
    Rect rect;
    struct {
      union {
	V2 pos;
	struct {float x, y;};
      };
      union {
	V2 dims;
	struct {float w, h;};
      };
    };
  };
  float theta;
  Color4B color;
  int z_order;
  bool invisible;
};
Dynamic_Array<Entity *> entity_manager;

Entity *create_entity(Entity *e) {
  Entity *n = (Entity *)malloc(sizeof(Entity));
  *n = *e;
  if (n->texture < 0 || n->texture >= textures.length) n->texture = 0;
  n->sprite = cocos2d::Sprite::createWithTexture(textures[n->texture]);
  n->sprite->setIgnoreAnchorPointForPosition(true);
  float tw = textures[n->texture]->getPixelsWide();
  float th = textures[n->texture]->getPixelsHigh();
  n->sprite->setScaleX(n->w/tw);
  n->sprite->setScaleY(n->h/th);
  n->sprite->setPosition(cocos2d::Vec2(n->x, n->y) - cocos2d::Vec2((tw - n->w)/2.0f,
								   (th - n->h)/2.0f));
  main_scene->addChild(n->sprite, 0);

  n->manager_index = entity_manager.length;
  entity_manager.add(n);
  return n;
}
void delete_entity(Entity *e) {
  entity_manager.remove_at(e->manager_index);
  if (e->manager_index < entity_manager.length)
    entity_manager[e->manager_index]->manager_index = e->manager_index;
  main_scene->removeChild(e->sprite, true); //@CHECK
  free(e);
}

void ordered_delete_entity(Entity *e) { // Call this if you don't want the entity manager to reorder the entities after deleting one
  entity_manager.ordered_remove_at(e->manager_index);
  for (int i = e->manager_index; i < entity_manager.length; i++)
    entity_manager[i]->manager_index = i;
  main_scene->removeChild(e->sprite, true); //@CHECK
  free(e);
}

inline Entity get_default_entity() {
  Entity e = {};
  e.w = 100.0f;
  e.h = 100.0f;
  e.color = Color4B::WHITE;
  return e;
}

inline bool point_in_entity(V2 point, Entity *e) {
  return point_in_rect(point, e->rect, e->theta*(180.0f/PI));
}

//
// Binding Code:
//

//@TODO: Axis bindings?
//@TODO: Controller support?

struct Binding {
  Dynamic_Array<Input_Button *> *button_bindings;
  Input_Button button_state;
};

Hash_Table<char *, Binding *> button_binding_table(djb2_hash, string_compare);
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
  add_button_binding(action, &key[key_code]);
}
inline void bind_mouse_button(char *action, int mouse_button_code) {
  assert(mouse_button_code >= 0);
  assert(mouse_button_code < NUM_MOUSE_BUTTONS);
  add_button_binding(action, &mouse.mouse_buttons[mouse_button_code]);
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

  add_font("Inconsolata-Regular.ttf", 16);
  add_font("AndBasR.ttf", 24);

  { // Make Blank White Texture:
    const int W = 4;
    const int H = 4;
    unsigned char data[4*W*H];
    for (int i = 0; i < sizeof(data); i++) data[i] = 255;
    make_texture(data, W, H);
  }
  
  //@TODO: Support other image formats? (jpg, gif, bmp, etc.)
  Dynamic_Array<char *> bitmap_filenames;
  get_filenames_in_directory("bitmaps", &bitmap_filenames);
  for (int i = 0; i < bitmap_filenames.length; i++) {
    if (strcmp(file_extension(bitmap_filenames[i]), "png") == 0) {
      make_texture(bitmap_filenames[i]);
    }
    free(bitmap_filenames[i]);
  }

  // NOTE: For some reason, we can't load .wav files. Not sure if they're supported, but they're a much better format to use for sound effects.
  // TODO: Figure out if they're supported, if not we may want to consider supporting it ourselves if we have time...

  // Load All Sounds in the sounds directory:
  Dynamic_Array<char *> sound_filenames;
  get_filenames_in_directory("sounds", &sound_filenames);
  for (int i = 0; i < sound_filenames.length; i++) {
    if (strcmp(file_extension(sound_filenames[i]), "mp3") == 0) load_sound(sound_filenames[i]);
    free(sound_filenames[i]);
  }
  load_music("night.mp3");
  play_music(0);
  
  Entity e = {};
  e.texture = 1;
  e.w = e.h = 100.0f;
  e.color = Color4B::WHITE;
  create_entity(&e);
  extern Entity *selected_entity;
  selected_entity = create_entity(&e);
  
  main_scene->scheduleUpdate();
  return true;
}

//
// Main Loop (This Gets Called Every Frame (~16 MS)):
//

enum Editor_Mode {
  SELECT_MODE = 0, PLACEMENT_MODE = 1, NUM_EDITOR_MODES = 2
};
int editor_mode = SELECT_MODE;
bool editor = true;
bool paused = true;
bool draw_grid = false;
bool lock_to_grid = false;
float grid_size = 100.0f;
Entity clipboard_entity = get_default_entity();
V2 selected_entity_mouse_dpos = v2(0.0f, 0.0f);
Entity *selected_entity = NULL;
Color4B background_color = color4b(200, 200, 200, 255);
int background_texture = 0;

inline float lock_dim_to_grid(float v, float grid_size) {
  if (grid_size == 0.0f) return v;
  float res = (float)((int)(v / grid_size));
  if (v < 0.0f) res -= 1.0f;
  res *= grid_size;
  return res;
}

void main_loop() {
  { // Update View:
    view.h = view.w * ((float)window_resolution.height / (float)window_resolution.width);
    float scale_x = window_resolution.width / view.w;
    float scale_y = window_resolution.height / view.h;
    main_scene->setScaleX(scale_x);
    main_scene->setScaleY(scale_y);
    
    main_scene->setPosition(-view.x*scale_x, -view.y*scale_y);
    screen_layer->setAdditionalTransform(main_scene->getWorldToNodeTransform());
  }

  if (!ui_state.has_keyboard_input && key[KEY_CTRL].is_down && key['o'].just_pressed) editor = !editor;
  if (!ui_state.has_keyboard_input && key[KEY_ESCAPE].just_pressed) paused = !paused;

  set_draw_color(background_color);
  draw_rect(background_texture, view.x, view.y, view.w, view.h, -1000); //@HACK @INCORRECT
  
  if (editor) {
    if (!ui_state.has_keyboard_input && key[KEY_CTRL].is_down && key['p'].just_pressed) {
      if (editor_mode == SELECT_MODE) editor_mode = PLACEMENT_MODE;
      else editor_mode = SELECT_MODE;
    }

    V2 world_mouse = screen_to_world(v2(mouse.x, mouse.y));
    V2 placement_position = world_mouse;
    if (lock_to_grid) {
      placement_position.x = lock_dim_to_grid(world_mouse.x, grid_size);
      placement_position.y = lock_dim_to_grid(world_mouse.y, grid_size);
    }
  
    if (!ui_state.has_keyboard_input) { // Camera Control:
      int dx = 0, dy = 0;
      if (key['w'].is_down) dy++;
      if (key['s'].is_down) dy--;
      if (key['a'].is_down) dx--;
      if (key['d'].is_down) dx++;
      if (dx != 0 || dy != 0) {
	float m = 300.0f;
	if (key[KEY_SHIFT].is_down) m *= 3.0f;
	V2 dpos = m*dt*normalize(v2(dx, dy));
	view.x += dpos.x;
	view.y += dpos.y;
      }
    }

    if (mouse.scroll != 0.0f) { // Zoom:
      V2 prev_dims = view.dims;
      float aspect = view.w / view.h;
    
      float m = 50.0f;
      if (key[KEY_SHIFT].is_down) m *= 3.0f;
      view.w += mouse.scroll*m;
      if (view.w > 0.0f) {
	view.h = view.w / aspect;
	view.pos -= (view.dims - prev_dims)/2.0f; 
      } else {
	view.w -= mouse.scroll*50.0f;
      }
    }

    ui_begin(0.8f, 1.0f);

    text("Editor Panel:");
    
    static bool show_view = false;
    if (button(show_view ? "Hide View" : "Show View")) show_view = !show_view;
    if (show_view) {
      float_edit("View X", &view.x);
      float_edit("View Y", &view.y);
      float_edit("View W", &view.w);
      float_edit("View H", &view.h);
      spacing(0.3f);
    }

    static bool show_options = false;
    if (button(show_options ? "Hide Options" : "Show Options")) show_options = !show_options;

    if (show_options) {
      float_edit("Grid Size", &grid_size, 0.0f);
      checkbox("Draw Grid", &draw_grid);
      checkbox("Lock to Grid", &lock_to_grid);
      checkbox("Paused", &paused);
      bool placement_mode = editor_mode == PLACEMENT_MODE;
      if (checkbox("Placement Mode", &placement_mode)) {
	if (placement_mode) editor_mode = PLACEMENT_MODE;
	else editor_mode = SELECT_MODE;
      }
      color_edit("Background Color", &background_color);
      int_edit("Background Texture", &background_texture);
    }
    spacing(0.5f);

    text("Entity Panel:");
    if (selected_entity) {
      Entity *s = selected_entity;
      text("Entity Manager Index: %i", s->manager_index);
      int_edit("Texture", &s->texture, 0, textures.length - 1);
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

    ui_end();

    if (key[KEY_CTRL].is_down && key['g'].just_pressed) lock_to_grid = !lock_to_grid;
	if (key[KEY_CTRL].is_down && key['h'].just_pressed) draw_grid = !draw_grid;

    if (!ui_state.has_keyboard_input && key[KEY_CTRL].is_down && key['v'].just_pressed) {
      Entity *e = create_entity(&clipboard_entity);
      e->x = placement_position.x;
      e->y = placement_position.y;
    }
    if (!ui_state.has_keyboard_input && key[KEY_CTRL].is_down && key['c'].just_pressed && selected_entity) {
      clipboard_entity = *selected_entity;
    }

    if (!ui_state.mouse_hovering_ui && mouse.right.just_pressed) { // Remove Entity:
      Entity *to_delete = NULL;
      float dist_s;
      for (int i = 0; i < entity_manager.length; i++) {
	Entity *e = entity_manager[i];
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
	ordered_delete_entity(to_delete);
      }
    }

    if (draw_grid && (view.w/grid_size) < 400.0f) {
      float view_width = view.w*2.0f; //@HACK @INCORRECT
      float view_height = view.h*2.0f; //@HACK @INCORRECT
      float start_x = lock_dim_to_grid(view.x, grid_size);
      float end_x = lock_dim_to_grid(view.x + view_width, grid_size);
      float start_y = lock_dim_to_grid(view.y, grid_size);
      float end_y = lock_dim_to_grid(view.y + view_height, grid_size);
      const float LW = 5.0f;
      set_draw_color_bytes(255, 255, 255, 100);
      for (float x = start_x; x <= end_x; x += grid_size) {
	draw_rect(0, x - LW/2.0f, view.y, LW, view_height, 0);
      }
      for (float y = start_y; y <= end_y; y += grid_size) {
	draw_rect(0, view.x, y - LW/2.0f, view_width, LW, 0);
      }
    }
    
    if (editor_mode == PLACEMENT_MODE) {
      if (!ui_state.mouse_hovering_ui) {
	Color4B c = clipboard_entity.color;
	c.a /= 4;
	set_draw_color(c);
	draw_rect(clipboard_entity.texture, placement_position.x, placement_position.y,
		  clipboard_entity.w, clipboard_entity.h, clipboard_entity.z_order,
		  clipboard_entity.theta);
      }
      
      if (!ui_state.mouse_hovering_ui && mouse.left.just_pressed) { // Place Entity:
	Entity *e = create_entity(&clipboard_entity);
	e->x = placement_position.x;
	e->y = placement_position.y;
      }
    } else if (editor_mode == SELECT_MODE) {
      if (!ui_state.mouse_hovering_ui) {
	if (mouse.left.just_pressed) {
	  selected_entity = NULL;
	  float dist_s;
	  for (int i = 0; i < entity_manager.length; i++) {
	    Entity *e = entity_manager[i];
	    if (point_in_entity(world_mouse, e)) {
	      V2 c = v2(e->x + e->w/2.0f, e->y + e->h/2.0f);
	      V2 dpos = world_mouse - c;
	      float d = dpos.x*dpos.x + dpos.y*dpos.y;
	      if (!selected_entity || (dist_s > d)) {
		selected_entity = e;
		dist_s = d;
	      }
	    }
	  }
	  if (selected_entity) selected_entity_mouse_dpos = world_mouse - v2(selected_entity->x, selected_entity->y);
	}

	if (selected_entity && mouse.left.is_down) { // Move Entity:
	  if (lock_to_grid) {
	    selected_entity->x = placement_position.x;
	    selected_entity->y = placement_position.y;
	  } else {
	    V2 mdpos = world_mouse - v2(selected_entity->x, selected_entity->y) - selected_entity_mouse_dpos;
	    if ((mdpos.x*mdpos.x + mdpos.y*mdpos.y) > 2.0f) { // Don't move for small position changes
	      selected_entity->x += mdpos.x;
	      selected_entity->y += mdpos.y;
	    }
	  }
	}
      }
    }
  
    if (selected_entity) {
      set_draw_color_bytes(222, 222, 222, 30);
      float w = selected_entity->w*1.1f;
      float h = selected_entity->h*1.1f;
      float dw = w - selected_entity->w;
      float dh = h - selected_entity->h;
      draw_solid_rect(selected_entity->x - dw/2.0f, selected_entity->y - dh/2.0f, w, h, 3, selected_entity->theta);
    }
  
    set_draw_color_bytes(255, 255, 255, 255);
  }
  
  for (int i = 0; i < entity_manager.length; i++) { // Update Entities:
    Entity *e = entity_manager[i];

    if (!paused) {
      e->x += (30.0f)*sin(total_time_elapsed + i*4.73f)*dt;
      e->y += (30.0f)*cos(total_time_elapsed + i*4.73f)*dt;
      e->theta += dt*64.0f;
    }
    
    cocos2d::Sprite *s = e->sprite;
    float tw = textures[e->texture]->getPixelsWide();
    float th = textures[e->texture]->getPixelsHigh();
    e->sprite->setVisible(!e->invisible);
    e->sprite->setPosition(cocos2d::Vec2(e->x, e->y) - cocos2d::Vec2((tw - e->w)/2.0f,
								     (th - e->h)/2.0f));
    cocos2d::Color3B c = {e->color.r, e->color.g, e->color.b};
    e->sprite->setColor(c);
    e->sprite->setOpacity(e->color.a);
    e->sprite->setRotation(e->theta);
    e->sprite->setLocalZOrder(e->z_order);
    if (e->texture >= 0 && e->texture < textures.length) {
      s->setScaleX(e->w/tw);
      s->setScaleY(e->h/th);
      s->setTexture(textures[e->texture]);
      cocos2d::Rect texture_rect = {0.0f, 0.0f, tw, th};
      e->sprite->setTextureRect(texture_rect);
    }
  }
}
