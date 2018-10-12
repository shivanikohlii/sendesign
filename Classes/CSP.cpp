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

  auto label = cocos2d::Label::createWithTTF("Hello Sailor!", "fonts/Marker Felt.ttf", 24);
  if (label == NULL) {
    problem_loading("'fonts/Marker Felt.ttf'");
  } else {
    label->setPosition(cocos2d::Vec2(origin.x + visibleSize.width/2,
				     origin.y + visibleSize.height - label->getContentSize().height));

    main_scene->addChild(label, 1);
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
inline V2 screen_to_world(V2 v, MRect view1 = view) {
  return v2(view1.x + v.x/window_resolution.width*view1.w,
	    view1.y + v.y/window_resolution.height*view1.h);
}

void main_loop(float dt) {
  static float time_elapsed = 0.0f;
  time_elapsed += dt;
  
  { // Update View:
	view.h = view.w * ((float)window_resolution.height / (float)window_resolution.width);
    float scale_x = window_resolution.width / view.w;
    float scale_y = window_resolution.height / view.h;
    main_scene->setScaleX(scale_x);
    main_scene->setScaleY(scale_y);
    main_scene->setPosition(-view.x*scale_x, -view.y*scale_y);
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
