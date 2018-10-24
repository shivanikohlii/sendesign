struct Entity {
	cocos2d::Sprite *sprite;
	int texture;
	int manager_index;
	float x;
	float y;
	float w;
	float h;
};
Dynamic_Array<Entity *> entity_manager;
Dynamic_Array<cocos2d::Texture2D *> textures;

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

cocos2d::Texture2D *make_texture(char *name) {
  char filename[256];
  sprintf(filename, "bitmaps/%s", name);
  cocos2d::Texture2D *texture = new cocos2d::Texture2D();
  int pixel_width = 0, pixel_height = 0, bitdepth = 0;
  unsigned char *data = stbi_load(filename, &pixel_width, &pixel_height, &bitdepth, STBI_rgb_alpha);
  assert(bitdepth == 4);
  texture->initWithData(data, bitdepth*pixel_width*pixel_height, cocos2d::Texture2D::PixelFormat::RGBA8888, pixel_width, pixel_height, cocos2d::Size(pixel_width, pixel_height));
  if ((pixel_width % 2) == 0 && (pixel_height % 2) == 0) texture->generateMipmap(); // NOTE: This call will cause a crash if it's called with an image that isn't a multiple of 2 in both dimensions...
                                                                                    // TODO: (We may want to print an error message in that case)
  STBI_FREE(data);
  textures.add(texture);
  return texture;
}


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

  auto label = cocos2d::Label::createWithTTF("Hello Sailor!", "fonts/Marker Felt.ttf", 24);
  if (label == NULL) {
    problem_loading("'fonts/Marker Felt.ttf'");
  } else {
    label->setPosition(cocos2d::Vec2(origin.x + visibleSize.width/2,
				     origin.y + visibleSize.height - label->getContentSize().height));

    screen_layer->addChild(label, 1);
  }

  make_texture("sun.png");

  cocos2d::experimental::AudioEngine::preload("music/night.mp3");

  

  cocos2d::experimental::AudioEngine::play2d("music/night.mp3", true);
  cocos2d::experimental::AudioEngine::preload("sounds/place.mp3"); // NOTE: For some reason, this won't work for .wav files. Not sure if they're supported, but they're a much better format to use for sound effects.
                                                                   // TODO: Figure out if they're supported, if not we may want to consider supporting it ourselves...
  
  Entity e = {};
  e.w = e.h = 100.0f;
  create_entity(&e);
  create_entity(&e);
  
  main_scene->scheduleUpdate();
  return true;
}

struct V2 {float x, y;};
struct MRect {
  union {
    V2 pos;
    struct {float x, y;};
  };
  union {
    V2 dims;
    struct {float w, h;};
  };
};
inline V2 v2(float x, float y) {return {x, y};}
inline V2 operator+(V2 a, V2 b) {return v2(a.x + b.x, a.y + b.y);}
inline V2 operator-(V2 a, V2 b) {return v2(a.x - b.x, a.y - b.y);}
inline V2 operator*(V2 a, float b) {return v2(a.x*b, a.y*b);}
inline V2 operator*(float a, V2 b) {return v2(b.x*a, b.y*a);}
inline V2 operator*(V2 a, V2 b) {return v2(a.x*b.x, a.y*b.y);}
inline V2 operator-(V2 a) {return v2(-a.x, -a.y);}
inline V2 operator/(V2 a, float b) {return v2(a.x/b, a.y/b);}
inline V2 operator/(V2 a, V2 b) {return v2(a.x/b.x, a.y/b.y);}
inline V2 &operator*=(V2 &a, float b) {return (a = a*b);}
inline V2 &operator/=(V2 &a, float b) {return (a = a/b);}
inline V2 &operator+=(V2 &a, V2 b) {return (a = a + b);}
inline V2 &operator-=(V2 &a, V2 b) {return (a = a - b);}
inline bool operator==(V2 a, V2 b) {return a.x == b.x && a.y == b.y;}
inline bool operator!=(V2 a, V2 b) {return !(a == b);}
inline float mag(V2 v) {return sqrt(v.x*v.x + v.y*v.y);}
inline V2 normalize(V2 v) {
  float m = mag(v);
  if (m == 0.0f) return v2(0.0f, 0.0f);
  return v / m;
}

static MRect view = {0.0f, 0.0f, 300.0f, 300.0f/1.778645833f};
//@FIX: This calculation is incorrect:
inline V2 screen_to_world(V2 v, MRect view1 = view) {
  float scale_x = window_resolution.width / view1.w;
  float scale_y = window_resolution.height / view1.h;
  return v2(view1.x + view.w/2.0f*scale_x + v.x/scale_x,
	    view1.y + view.h/2.0f*scale_y + v.y/scale_y);
}

enum UI_Type {
  UI_BUTTON = 0, UI_TEXTFIELD = 1, NUM_UI_TYPES = 2
};
struct UI_Item {
  union {
    cocos2d::ui::Button *button;
    cocos2d::ui::EditBox *textfield;
    cocos2d::ui::Widget *widget; //@CHECK
  };
  int ui_type;
  char *name;
};
struct UI_Element_Data {
  bool element_changed;
  bool should_remove;
};
inline void *to_user_data(UI_Element_Data d) {
  return *(void **)&d;
}
inline UI_Element_Data to_ui_element_data(void *d) {
  return *(UI_Element_Data *)&d;
}

u64 button_hash_function(char *c) { return djb2_hash((unsigned char *)c); }
int button_compare_strings(char *c1, char *c2) { return strcmp(c1, c2); }
Hash_Table<char *, UI_Item> ui_item_table(button_hash_function, button_compare_strings);
Dynamic_Array<UI_Item> ui_items_list;

void reset_ui() {
  for (int i = 0; i < ui_items_list.length; i++) {
    cocos2d::ui::Widget *w = ui_items_list[i].widget;
    UI_Element_Data data = to_ui_element_data(w->getUserData());
    data.should_remove = true;
    w->setUserData(to_user_data(data));
  }
}
void cleanup_unused_ui_elements() {
  for (int i = 0; i < ui_items_list.length; i++) {
    cocos2d::ui::Widget *w = ui_items_list[i].widget;
    UI_Element_Data data = to_ui_element_data(w->getUserData());
    if (data.should_remove) {
      ui_item_table.remove(ui_items_list[i].name);
      free(ui_items_list[i].name);
      ui_items_list.remove_at(i);
      screen_layer->removeChild(w, true); //@CHECK
      OutputDebugStringA("REMOVED!\n");
      i--;
    }
  }
}

void button_listener(cocos2d::Ref *sender, cocos2d::ui::Widget::TouchEventType type) {
  cocos2d::ui::Button *b = (cocos2d::ui::Button *)sender;
  UI_Element_Data data = to_ui_element_data(b->getUserData());
  switch (type) {
  case cocos2d::ui::Widget::TouchEventType::ENDED:
    data.element_changed = true;
    b->setUserData(to_user_data(data));
    break;
  }
}

#define UI_ITEM_SIZE 0.1f
struct UI_State {
  float y = 1.0f - UI_ITEM_SIZE/2.0f;
} ui_state;

void ui_begin(float y) {
  ui_state.y = y - UI_ITEM_SIZE/2.0f;
}
void ui_end() {
}
cocos2d::Vec2 get_content_size(UI_Item item) {
  switch (item.ui_type) {
  case UI_BUTTON: return item.button->getContentSize();
  case UI_TEXTFIELD: return item.textfield->getContentSize();
  }
  return cocos2d::Vec2(1.0f, 1.0f);
}
void add_ui_item(char *name, UI_Item *item) {
  cocos2d::Vec2 content_size = get_content_size(*item);
  float height = UI_ITEM_SIZE;
  float width = height*(content_size.x / content_size.y);
  cocos2d::ui::Widget *w = item->widget;
  w->setScaleX(width*window_resolution.height/content_size.x);
  w->setScaleY(height*window_resolution.height/content_size.y);
  w->setPosition(cocos2d::Vec2(width*window_resolution.height/2.0f, ui_state.y*window_resolution.height));
    
  w->setUserData(NULL);

  screen_layer->addChild(w);

  char *name_to_use = (char *)malloc(strlen(name) + 1);
  strcpy(name_to_use, name);
  item->name = name_to_use;

  ui_items_list.add(*item);
  ui_item_table.add_to_hash_table(name, *item);
  OutputDebugStringA("BUTTON Created!\n");
}
void update_ui_item(UI_Item *item) {
  cocos2d::Vec2 content_size = get_content_size(*item);
  float height = UI_ITEM_SIZE;
  float width = height*(content_size.x / content_size.y);
  item->widget->setPositionY(ui_state.y*window_resolution.height);
  UI_Element_Data data = to_ui_element_data(item->widget->getUserData());
  data.should_remove = false;
  item->widget->setUserData(to_user_data(data));
}

bool button(char *name) {
  UI_Item button;
  bool exists = ui_item_table.retrieve_from_hash_table(name, &button);
  if (!exists) {
    cocos2d::ui::Button *b = cocos2d::ui::Button::create("ui/button_normal.png", "ui/button_selected.png", "ui/button_disabled.png");
    b->setTitleText(name);
    b->setTitleColor(cocos2d::ccColor3B(0, 0, 0));
    b->setTouchEnabled(true);
    b->addTouchEventListener(button_listener);
	b->setTitleFontName("Marker Felt");
    //b->setTitleFontName("fonts/arial.ttf");
    b->setTitleFontSize(UI_ITEM_SIZE);
    b->setColor(cocos2d::ccColor3B(214, 90, 50));
    
    button.button = b;
    button.ui_type = UI_BUTTON;
    add_ui_item(name, &button);
  }

  update_ui_item(&button);
  ui_state.y -= UI_ITEM_SIZE;

  UI_Element_Data data = to_ui_element_data(button.button->getUserData());
  if (data.element_changed) {
    OutputDebugStringA("PRESSED!\n");
    data.element_changed = false;
    button.button->setUserData(to_user_data(data));
    return true;
  }

  return false;
}

bool text_field(char *name) {
  UI_Item item;
  bool exists = ui_item_table.retrieve_from_hash_table(name, &item);
  if (!exists) {
    OutputDebugStringA("Created TEXT FIELD!\n");
    cocos2d::ui::Scale9Sprite *s = cocos2d::ui::Scale9Sprite::create("ui/textbox.png", cocos2d::Rect(2.0f, 2.0f, 252.0f, 124.0f));
    cocos2d::ui::EditBox *t = cocos2d::ui::EditBox::create(cocos2d::Size(UI_ITEM_SIZE*2.0f, UI_ITEM_SIZE), s); //@CHANGE
    t->setInputMode(cocos2d::ui::EditBox::InputMode::SINGLE_LINE);
	t->setFontColor(cocos2d::Color3B(0, 0, 0));
	//t->setFontSize(UI_ITEM_SIZE);
    
    item.textfield = t;
    item.ui_type = UI_TEXTFIELD;
    
    add_ui_item(name, &item);
  }

  update_ui_item(&item);
  ui_state.y -= UI_ITEM_SIZE;
  
  return false;
}

void main_loop(float dt) {
  static float time_elapsed = 0.0f;
  time_elapsed += dt;
  
  { // Update View:
    view.h = view.w * ((float)window_resolution.height / (float)window_resolution.width);
    float scale_x = window_resolution.width / view.w;
    float scale_y = window_resolution.height / view.h;
    //@FIX: This calculation is incorrect:
    game_layer->setScaleX(scale_x);
    game_layer->setScaleY(scale_y);
    game_layer->setPosition(-view.x*scale_x - scale_x*view.w/2.0f, -view.y*scale_y - scale_y*view.h/2.0f);
  }

  V2 world_mouse = screen_to_world(v2(mouse.x, mouse.y));
  if (entity_manager.length > 0) {
    entity_manager.top()->x = world_mouse.x - entity_manager.top()->w/2.0f;
    entity_manager.top()->y = world_mouse.y - entity_manager.top()->h/2.0f;
  }

  { // Camera Control:
    int dx = 0, dy = 0;
    if (key['w'].is_down) dy++;
    if (key['s'].is_down) dy--;
    if (key['a'].is_down) dx--;
    if (key['d'].is_down) dx++;
    if (dx != 0 || dy != 0) {
      float m = 300.0f;
      if (key[(int)KeyCode::KEY_SHIFT].is_down) m *= 3.0f;
      V2 dpos = m*dt*normalize(v2(dx, dy));
      view.x += dpos.x;
      view.y += dpos.y;
    }
  }

  if (mouse.scroll != 0.0f) { // Zoom:
    V2 prev_dims = view.dims;
    float aspect = view.w / view.h;
    
    float m = 50.0f;
    if (key[(int)KeyCode::KEY_SHIFT].is_down) m *= 3.0f;
    view.w += mouse.scroll*m;
    if (view.w > 0.0f) {
      view.h = view.w / aspect;
      view.pos -= (view.dims - prev_dims)/2.0f; 
    } else {
      view.w -= mouse.scroll*50.0f;
    }
  }

  ui_begin(1.0f);
  if (key[(int)KeyCode::KEY_SPACE].is_down) {
    if (button("Hello Sailor")) {
      OutputDebugStringA("Hello Sailor!\n");
    }
  }
  char buffer[128] = {};
  for (int i = 0; i < 4; i++) {
    sprintf(buffer, "YO %i", i);
    if (button(buffer)) {
      OutputDebugStringA(buffer);
    }
  }
  if (button("button1")) {
	  //...
  }

  text_field("Sup");
  ui_end();
  
  if (mouse.left.just_pressed) { // Place Entity:
    Entity e = {};
    e.w = e.h = rand_float()*130.0f;
    e.x = world_mouse.x - e.w/2.0f;
    e.y = world_mouse.y - e.h/2.0f;
    cocos2d::experimental::AudioEngine::play2d("sounds/place.mp3");
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
  
  for (int i = 0; i < entity_manager.length; i++) { // Update Entities:
    Entity *e = entity_manager[i];

    e->x += (30.0f)*sin(time_elapsed + i*4.73f)*dt;
    e->y += (30.0f)*cos(time_elapsed + i*4.73f)*dt;
    
    cocos2d::Sprite *s = e->sprite;
    float tw = textures[e->texture]->getPixelsWide();
    float th = textures[e->texture]->getPixelsHigh();
    e->sprite->setPosition(cocos2d::Vec2(e->x, e->y) - cocos2d::Vec2((tw - e->w)/2.0f,
								     (th - e->h)/2.0f));
    if (e->texture >= 0 && e->texture < textures.length) {
      s->setScaleX(e->w/tw);
      s->setScaleY(e->h/th);
      s->setTexture(textures[e->texture]);
    }
  }
}
