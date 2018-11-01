#include "AppDelegate.h"

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

#define STB_IMAGE_IMPLEMENTATION
#include "libraries/stb_image.h"

#include "modules/dynamic_array.cpp"
#include "modules/random.cpp"
#include "modules/hash_table.cpp"

// Note: This will put the cocos2d library into the global namespace:      USING_NS_CC;

#define KeyCode cocos2d::EventKeyboard::KeyCode
#include "audio/include/AudioEngine.h"

struct Input_Button {
  bool is_down;
  bool just_pressed;
};
struct Mouse {
  Input_Button left;
  Input_Button right;
  float x;
  float y;
  float scroll;
} mouse;

//
// Game "Engine" Globals:
//
cocos2d::Size window_resolution = cocos2d::Size(1366, 768);
bool fullscreen = false;

Input_Button key[256] = {};
Dynamic_Array<unsigned char> keys_just_pressed;
cocos2d::Scene *main_scene = NULL;
cocos2d::Layer *game_layer = NULL;
cocos2d::Layer *screen_layer = NULL;

//
// Math Stuff (Might Remove):
//

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
inline bool point_in_rect(V2 point, MRect rect) {
  return (point.x > rect.x && point.x < (rect.x + rect.w) &&
	  point.y > rect.y && point.y < (rect.y + rect.h));
}
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

//
// String Stuff:
//

int first_index_of(char *str, char to_find) {
  for (int i = 0; str[i] != 0; i++) {
    if (str[i] == to_find) return i;
  }
  return -1;
}
int last_index_of(char *str, char to_find) {
  int last_index = -1;
  for (int i = 0; str[i] != 0; i++) {
    if (str[i] == to_find) last_index = i;
  }
  return last_index;
}
inline char *file_extension(char *filename) {
  int last_index = last_index_of(filename, '.');
  if (last_index < 0) return "";
  else return filename + last_index + 1;
}

//
// Getting Files From Directory:
//

#ifdef _WIN32
void get_filenames_in_directory(char *directory, Dynamic_Array<char *> *result) {
  char dir_str[256]; //@MEMORY: We can only have 256 character directories from this.
  snprintf(dir_str, sizeof(dir_str), "%s/*", directory);
  WIN32_FIND_DATAA file_info = {};
  HANDLE handle = FindFirstFileA(dir_str, &file_info);
  if (handle == INVALID_HANDLE_VALUE) {
    printf("Error opening when getting filenames in directory: %s.\n", directory);
    FindClose(handle);
    return;
  }
  for (int i = 0;; i++) {
    if (file_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      if (!FindNextFileA(handle, &file_info)) break;
      continue;
    }
    char *str = (char *)malloc(strlen(file_info.cFileName) + 1);
    strcpy(str, file_info.cFileName);
    result->add(str);
    if (!FindNextFileA(handle, &file_info)) break;
  }
  FindClose(handle);
}
#else // Not Windows, This is for Linux:
#if 0
#include <ctime>
#include <dirent.h>
#include <sys/stat.h>
#endif
//NOTE: This Code Has Not Been Tested:
void get_filenames_in_directory(char *directory, Dynamic_Array<char *> *result) {
  dirent *ent = 0;
  struct stat st;
  DIR *dir = opendir(directory);
  char full_filename[512]; //@MEMORY: Full Filenames Can Only Have a Max Size of 512 Characters
  while ((ent = readdir(dir)) != 0) {
    char *filename = ent->d_name;
    if (filename[0] == '.') continue;
    snprintf(full_filename, sizeof(full_filename), "%s/%s", directory, filename);
    if (stat(full_filename, &st) == -1) continue;
    bool is_dir = (st.st_mode & S_IFDIR) != 0;
    if (is_dir) continue;
    char *str = (char *)malloc(strlen(filename) + 1);
    strcpy(str, filename);
    result->add(str);
  }
  closedir(dir);
}
#endif

//
// Texture System:
//

Dynamic_Array<cocos2d::Texture2D *> textures;

int make_texture(unsigned char *data, int width, int height) {
  cocos2d::Texture2D *texture = new cocos2d::Texture2D();
  texture->initWithData(data, 4*width*height, cocos2d::Texture2D::PixelFormat::RGBA8888, width, height, cocos2d::Size(width, height));
  if ((width % 2) == 0 && (height % 2) == 0) texture->generateMipmap(); // NOTE: This call will cause a crash if it's called with an image that isn't a multiple of 2 in both dimensions...
                                                                        // TODO: (We may want to print an error message in that case)
  textures.add(texture);
  return textures.length - 1;
}
int make_texture(char *name) {
  char filename[256]; //@MEMORY
  sprintf(filename, "bitmaps/%s", name);
  int pixel_width = 0, pixel_height = 0, bitdepth = 0;
  unsigned char *data = stbi_load(filename, &pixel_width, &pixel_height, &bitdepth, STBI_rgb_alpha);
  assert(bitdepth == 4); // We're assuming an RGBA image
  int texture_index = make_texture(data, pixel_width, pixel_height);
  STBI_FREE(data);
  return texture_index;
}

//
// Fonts:
//

Dynamic_Array<cocos2d::TTFConfig *> fonts;

int add_font(char *font_filename, int font_size) {
  cocos2d::TTFConfig *ttf_config = new cocos2d::TTFConfig();
  ttf_config->fontFilePath = "fonts/";
  ttf_config->fontFilePath += font_filename;
  ttf_config->fontSize = font_size;
  ttf_config->glyphs = cocos2d::GlyphCollection::DYNAMIC;
  ttf_config->outlineSize = 0;
  ttf_config->customGlyphs = NULL;
  ttf_config->distanceFieldEnabled = false;
  fonts.add(ttf_config);
  return fonts.length - 1;
}

//
// Sound System:
//

struct Sound {
  char *filename;
  char *name;
};
struct Song {
  char *filename;
  char *name;
};
Dynamic_Array<Sound> sounds;
Dynamic_Array<Song> songs;
int load_music(char *name) {
  std::string filename = "music/";
  filename.append(name);
  cocos2d::experimental::AudioEngine::preload(filename);

  char *c_filename = (char *)malloc(filename.length() + 1);
  strcpy(c_filename, filename.c_str());
  Song song;
  song.filename = c_filename;

  int folder_index = last_index_of(c_filename, '/');
  if (folder_index < 0) song.name = c_filename;
  else song.name = c_filename + folder_index + 1;
  
  songs.add(song);
  return songs.length - 1;
}
int load_sound(char *name) {
  std::string filename = "sounds/";
  filename.append(name);
  cocos2d::experimental::AudioEngine::preload(filename);
  char *c_filename = (char *)malloc(filename.length() + 1);
  strcpy(c_filename, filename.c_str());

  Sound sound;
  sound.filename = c_filename;

  int folder_index = last_index_of(c_filename, '/');
  if (folder_index < 0) sound.name = c_filename;
  else sound.name = c_filename + folder_index + 1;
  
  sounds.add(sound);
  return sounds.length - 1;
}
void play_music(int music_index, bool loop = true) {
  assert(music_index >= 0);
  assert(music_index < songs.length);
  cocos2d::experimental::AudioEngine::play2d(songs[music_index].filename, loop);
}
void play_sound(int sound_index, bool loop = false) {
  assert(sound_index >= 0);
  assert(sound_index < sounds.length);
  cocos2d::experimental::AudioEngine::play2d(sounds[sound_index].filename, loop);
}

//
// Immediate Mode Graphics:
//

union Graphics_Item {
  cocos2d::Sprite *sprite;
  cocos2d::Label *label;
};
struct Graphics_Items {
  Dynamic_Array<Graphics_Item> items;
  int next_item;
};

Graphics_Items game_sprite_graphics_items;
Graphics_Items screen_sprite_graphics_items;
Graphics_Items game_label_graphics_items;
Graphics_Items screen_label_graphics_items;

struct Draw_Settings {
  cocos2d::Color3B draw_color = cocos2d::Color3B(255, 255, 255);
  u8 draw_color_opacity = 255;
  bool screen_draw = false;
} draw_settings;

inline void set_draw_color(float red, float green, float blue, float alpha) {
  draw_settings.draw_color.r = (u8)(red*255.0f);
  draw_settings.draw_color.g = (u8)(green*255.0f);
  draw_settings.draw_color.b = (u8)(blue*255.0f);
  draw_settings.draw_color_opacity = (u8)(alpha*255.0f);
}
inline void set_draw_color_bytes(u8 red, u8 green, u8 blue, u8 alpha) {
  draw_settings.draw_color.r = red;
  draw_settings.draw_color.g = green;
  draw_settings.draw_color.b = blue;
  draw_settings.draw_color_opacity = alpha;
}
inline void enable_screen_draw() {draw_settings.screen_draw = true;}
inline void disable_screen_draw() {draw_settings.screen_draw = false;}

void draw_rect(int texture, float x, float y, float w, float h) {
  assert(texture >= 0);
  assert(texture < textures.length);
  Graphics_Items *items = NULL;
  if (draw_settings.screen_draw) items = &screen_sprite_graphics_items;
  else items = &game_sprite_graphics_items;
  
  if (items->next_item >= items->items.length) {
    Graphics_Item item = {};
    item.sprite = cocos2d::Sprite::createWithTexture(textures[texture]);
    item.sprite->setIgnoreAnchorPointForPosition(true);
    
    if (draw_settings.screen_draw) screen_layer->addChild(item.sprite, 0);
    else game_layer->addChild(item.sprite, 0);
    items->items.add(item);
  }
  Graphics_Item *item = items->items.data + items->next_item;
  item->sprite->setTexture(textures[texture]);
  float tw = textures[texture]->getPixelsWide();
  float th = textures[texture]->getPixelsHigh();
  item->sprite->setColor(draw_settings.draw_color);
  item->sprite->setOpacity(draw_settings.draw_color_opacity);
  item->sprite->setScaleX(w/tw);
  item->sprite->setScaleY(h/th);
  item->sprite->setPosition(cocos2d::Vec2(x, y) - cocos2d::Vec2((tw - w)/2.0f, (th - h)/2.0f)); //@CHECK: This might be incorrect
  item->sprite->setVisible(true);
  items->next_item++;
}
inline void draw_solid_rect(float x, float y, float w, float h) {
  draw_rect(0, x, y, w, h);
}

void draw_text(char *text, float x, float y, int font_index) {
  assert(font_index >= 0);
  assert(font_index < fonts.length);
  Graphics_Items *items = NULL;
  if (draw_settings.screen_draw) items = &screen_label_graphics_items;
  else items = &game_label_graphics_items;

  if (items->next_item >= items->items.length) {
    Graphics_Item item = {};
    item.label = cocos2d::Label::createWithTTF(*fonts[font_index], text);
    item.label->setIgnoreAnchorPointForPosition(true);

    if (draw_settings.screen_draw) screen_layer->addChild(item.label, 0);
    else game_layer->addChild(item.label, 0);
    items->items.add(item);
  }
  Graphics_Item *item = items->items.data + items->next_item;
  item->sprite->setColor(draw_settings.draw_color);
  item->sprite->setOpacity(draw_settings.draw_color_opacity);
  item->label->setPosition(cocos2d::Vec2(x, y)); //@FIX: This is probably incorrect
  item->label->setVisible(true);
  items->next_item++;
}

void draw_immediate_mode_graphics() {
  for (int graphics_items_index = 0; graphics_items_index < 4; graphics_items_index++) {
    Graphics_Items *items = NULL;
    if (graphics_items_index == 0) items = &game_sprite_graphics_items;
    else if (graphics_items_index == 1) items = &screen_sprite_graphics_items;
    else if (graphics_items_index == 2) items = &game_label_graphics_items;
    else if (graphics_items_index == 3) items = &screen_label_graphics_items;
    
    for (int i = items->next_item; i < items->items.length; i++) {
      Graphics_Item *g = items->items.data + i;
      if (graphics_items_index > 1) g->label->setVisible(false);
      else g->sprite->setVisible(false);
    }
    items->next_item = 0;
  }
}

#include "ui.cpp"
#include "CSP.cpp"

//
// Input Code:
//
void reset_inputs() {
  mouse.scroll = 0.0f;
  mouse.left.just_pressed = false;
  mouse.right.just_pressed = false;
  for (int i = 0; i < keys_just_pressed.length; i++) {
    unsigned char c = keys_just_pressed[i];
    key[c].just_pressed = false;
  }
  keys_just_pressed.length = 0;
}
void on_mouse_down(cocos2d::Event *event) {
  cocos2d::EventMouse *mouse_event = (cocos2d::EventMouse *)event;
  if ((int)mouse_event->getMouseButton() == 1) {
    mouse.right.is_down = mouse.right.just_pressed = true;
  } else if ((int)mouse_event->getMouseButton() == 0) {
    mouse.left.is_down = mouse.left.just_pressed = true;
  }
}
void on_mouse_up(cocos2d::Event *event) {
  cocos2d::EventMouse *mouse_event = (cocos2d::EventMouse *)event;
  if ((int)mouse_event->getMouseButton() == 1) mouse.right.is_down = false;
  else if ((int)mouse_event->getMouseButton() == 0) mouse.left.is_down = false;
}
void on_mouse_move(cocos2d::Event *event) {
  cocos2d::EventMouse *mouse_event = (cocos2d::EventMouse *)event;
  mouse.x = mouse_event->getCursorX();
  mouse.y = mouse_event->getCursorY();
}
void on_mouse_scroll(cocos2d::Event *event) {
  cocos2d::EventMouse *mouse_event = (cocos2d::EventMouse *)event;
  mouse.scroll = mouse_event->getScrollY();
}
// Given a key code, returns whether the key should be registered as an input as well as its corresponding index in the keys table
// The reason why this exists is so you can do things like:
// if (keys['a'].is_down) ...
// rather than having to do something like:
// if (keys[(unsigned char)KeyCode::KEY_A].is_down) ...
bool process_key(KeyCode key_code, unsigned char *out) { 
  unsigned int i = (unsigned int)key_code;
  if (i < 256) {
    unsigned char c = (unsigned char)key_code;
    bool add_key = true;
    if (c >= 124 && c <= 149) c = (c - 124) + 'a';
    else if (c >= 76 && c <= 85) c = (c - 76) + '0';
    else if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) add_key = false;
    
    if (add_key) {
      *out = c;
      return true;
    }
    return false;
  }
  return false;
}
void on_key_pressed(KeyCode key_code, cocos2d::Event *event) {
  unsigned char c = 0;
  if (process_key(key_code, &c)) {
    keys_just_pressed.add(c);
    key[c].just_pressed = true;
    key[c].is_down = true;
  }
}
void on_key_released(KeyCode key_code, cocos2d::Event *event) {
  unsigned char c = 0;
  if (process_key(key_code, &c)) key[c].is_down = false;
}

//
// Main Scene, Necessary to Interface With Cocos:
//

struct Main_Scene : cocos2d::Scene {
  static cocos2d::Scene* createScene() {
    return Main_Scene::create();
  }
  virtual bool init() {
    main_scene = this;
    auto listener = cocos2d::EventListenerMouse::create();

    listener->onMouseDown = on_mouse_down;
    listener->onMouseMove = on_mouse_move;
    listener->onMouseScroll = on_mouse_scroll;
    listener->onMouseUp = on_mouse_up;
    _eventDispatcher->addEventListenerWithFixedPriority(listener, 1);

    auto keyboard_listener = cocos2d::EventListenerKeyboard::create();
    keyboard_listener->onKeyPressed = on_key_pressed;
    keyboard_listener->onKeyReleased = on_key_released;
    _eventDispatcher->addEventListenerWithFixedPriority(keyboard_listener, 1);

    return initialize();
  }
  void update(float dt) {
    void reset_ui();
    void cleanup_unused_ui_elements();
    void draw_immediate_mode_graphics();
    
    reset_ui();
    main_loop(dt);
    reset_inputs();
    cleanup_unused_ui_elements();
    draw_immediate_mode_graphics();
  }
  CREATE_FUNC(Main_Scene);
};

AppDelegate::AppDelegate() {}
AppDelegate::~AppDelegate() {}

void AppDelegate::initGLContextAttrs() {
  // NOTE: OpenGL context attributes: red,green,blue,alpha,depth,stencil,multisamplesCount
  GLContextAttrs glContextAttrs = {8, 8, 8, 8, 24, 8, 0};
  cocos2d::GLView::setGLContextAttrs(glContextAttrs);
}

static int register_all_packages() {return 0;}

bool AppDelegate::applicationDidFinishLaunching() {
  auto director = cocos2d::Director::getInstance();
  auto glview = director->getOpenGLView();
  if (!glview) {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) || (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
    if (fullscreen) glview = cocos2d::GLViewImpl::createWithFullScreen("CSP");
    else glview = cocos2d::GLViewImpl::createWithRect("CSP", cocos2d::Rect(0, 0, window_resolution.width, window_resolution.height));
#else
    glview = GLViewImpl::create("CSP");
#endif
    director->setOpenGLView(glview);
  }

#if 1
  director->setDisplayStats(true);
#endif
  director->setAnimationInterval(1.0f / 60);
  glview->setDesignResolutionSize(window_resolution.width, window_resolution.height, ResolutionPolicy::NO_BORDER);

  register_all_packages();

  main_scene = Main_Scene::create();
  director->runWithScene(main_scene);

  return true;
}

void AppDelegate::applicationDidEnterBackground() {
  cocos2d::Director::getInstance()->stopAnimation();
  cocos2d::experimental::AudioEngine::pauseAll();
}

void AppDelegate::applicationWillEnterForeground() {
  cocos2d::Director::getInstance()->startAnimation();
  cocos2d::experimental::AudioEngine::resumeAll();
}

void AppDelegate::applicationScreenSizeChanged(int new_width, int new_height) {
  window_resolution.width = new_width;
  window_resolution.height = new_height;
}
