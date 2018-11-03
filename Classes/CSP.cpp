//
// Entity System:
//

struct Entity {
  cocos2d::Sprite *sprite;
  int texture;
  int manager_index;
  float x;
  float y;
  float w;
  float h;
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
  game_layer->addChild(n->sprite, 0);

  n->manager_index = entity_manager.length;
  entity_manager.add(n);
  return n;
}
void delete_entity(Entity *e) {
  entity_manager.remove_at(e->manager_index);
  if (e->manager_index < entity_manager.length)
    entity_manager[e->manager_index]->manager_index = e->manager_index;
  game_layer->removeChild(e->sprite, true); //@CHECK
  free(e);
}
void ordered_delete_entity(Entity *e) { // Call this if you don't want the entity manager to reorder the entities after deleting one
  entity_manager.ordered_remove_at(e->manager_index);
  for (int i = e->manager_index; i < entity_manager.length; i++)
    entity_manager[i]->manager_index = i;
  game_layer->removeChild(e->sprite, true); //@CHECK
  free(e);
}

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

  game_layer = cocos2d::Layer::create();
  screen_layer = cocos2d::Layer::create();
  main_scene->addChild(game_layer, 0);
  main_scene->addChild(screen_layer, 1);

  add_font("Marker Felt.ttf", 24);

  { // Make Blank White Texture:
    const int W = 4;
    const int H = 4;
    unsigned char data[4*W*H];
    for (int i = 0; i < sizeof(data); i++) data[i] = 255;
    make_texture(data, W, H);
  }
  make_texture("sun.png");

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
  create_entity(&e);
  create_entity(&e);
  
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

void main_loop() {
  { // Update View:
    view.h = view.w * ((float)window_resolution.height / (float)window_resolution.width);
    float scale_x = window_resolution.width / view.w;
    float scale_y = window_resolution.height / view.h;
    //@FIX: This calculation is incorrect:
    game_layer->setScaleX(scale_x);
    game_layer->setScaleY(scale_y);
    game_layer->setPosition(-view.x*scale_x - scale_x*view.w/2.0f, -view.y*scale_y - scale_y*view.h/2.0f);
  }

  if (key[KEY_CTRL].is_down && key['p'].just_pressed) {
    if (editor_mode == SELECT_MODE) editor_mode = PLACEMENT_MODE;
    else editor_mode = SELECT_MODE;
  }

  V2 world_mouse = screen_to_world(v2(mouse.x, mouse.y));
  if (editor_mode == PLACEMENT_MODE && entity_manager.length > 0) {
    entity_manager.top()->invisible = false;
    entity_manager.top()->x = world_mouse.x - entity_manager.top()->w/2.0f;
    entity_manager.top()->y = world_mouse.y - entity_manager.top()->h/2.0f;
  } else {
    entity_manager.top()->invisible = true;
  }
  
  { // Camera Control:
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

  ui_begin(0.0f, 1.0f);
  if (button("Test Button")) {
    OutputDebugStringA("TEST!\n");
  }
  if (button("Test Button 2")) {
    OutputDebugStringA("TEST 2!\n");
  }
  static char buffer[256];
  static String str = {};
  if (!str.str) { // On Initialization
    str = fixed_str(buffer);
    append(&str, "Type Stuff...");
  }
  
  if (text_field(&str)) {
    // Stuff Changed
  }
  
  ui_end();

  if (key[KEY_SHIFT].is_down) {
    for (int i = 0; i < 10; i++) {
      draw_rect(1, 100.0f + 200.0f*sin(total_time_elapsed*0.25f*(i + 1)), 100.0f*i, 100.0f, 100.0f);
    }
  }

  {
    char text[256];
    for (int i = 0; i < 10; i++) {
      snprintf(text, sizeof(text), "TEST TEXT %i", i);
      set_draw_color(i/10.0f, 0.7f, 0.0f, 1.0f);
    
      draw_text(text, 500.0f + 100.0f*sin(total_time_elapsed*3.0f), 500.0f + i*100.0f, 0);
    }
  }
  set_draw_color_bytes(255, 255, 255, 255);

  if (editor_mode == PLACEMENT_MODE) {
    if (mouse.left.just_pressed) { // Place Entity:
      Entity e = {};
      e.texture = 1;
      e.w = e.h = rand_float()*130.0f;
      e.x = world_mouse.x - e.w/2.0f;
      e.y = world_mouse.y - e.h/2.0f;
      //play_sound(0);
      create_entity(&e);
    }
    if (mouse.right.just_pressed) { // Remove Entity:
      Entity *to_delete = NULL;
      float dist_s;
      for (int i = 0; i < (entity_manager.length - 1); i++) {
	Entity *e = entity_manager[i];
	if (world_mouse.x > e->x && world_mouse.x < (e->x + e->w) &&
	    world_mouse.y > e->y && world_mouse.y < (e->y + e->h)) {
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
  }
  
  for (int i = 0; i < entity_manager.length; i++) { // Update Entities:
    Entity *e = entity_manager[i];

    e->x += (30.0f)*sin(total_time_elapsed + i*4.73f)*dt;
    e->y += (30.0f)*cos(total_time_elapsed + i*4.73f)*dt;
    
    cocos2d::Sprite *s = e->sprite;
    float tw = textures[e->texture]->getPixelsWide();
    float th = textures[e->texture]->getPixelsHigh();
    e->sprite->setVisible(!e->invisible);
    e->sprite->setPosition(cocos2d::Vec2(e->x, e->y) - cocos2d::Vec2((tw - e->w)/2.0f,
								     (th - e->h)/2.0f));
    if (e->texture >= 0 && e->texture < textures.length) {
      s->setScaleX(e->w/tw);
      s->setScaleY(e->h/th);
      s->setTexture(textures[e->texture]);
    }
  }
}
