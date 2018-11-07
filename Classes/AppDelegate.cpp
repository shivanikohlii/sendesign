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

typedef unsigned char uchar;

#define STB_IMAGE_IMPLEMENTATION
#include "libraries/stb_image.h"

#define max_val(a, b) (((a) > (b)) ? (a) : (b))
#define min_val(a, b) (((a) < (b)) ? (a) : (b))
#define array_size(a) ((sizeof(a))/(sizeof((a)[0])))

#include "modules/dynamic_array.cpp"
#include "modules/math.cpp"
#include "modules/random.cpp"
#include "modules/hash_table.cpp"
#include "modules/string.cpp"

// Note: This will put the cocos2d library into the global namespace:      USING_NS_CC;

#include "audio/include/AudioEngine.h"
#include "input.cpp"

//
// Game "Engine" Globals:
//
cocos2d::Size window_resolution = cocos2d::Size(1366, 768);
bool fullscreen = false;
float dt = 0.0f;
float total_time_elapsed = 0.0f;

cocos2d::Scene *main_scene = NULL;
cocos2d::Layer *game_layer = NULL;
cocos2d::Layer *screen_layer = NULL;

Rect view = {0.0f, 0.0f, 300.0f, 300.0f/1.778645833f};
//@FIX: This calculation is incorrect:
inline V2 screen_to_world(V2 v, Rect view1 = view) {
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

int make_texture(uchar *data, int width, int height) {
  cocos2d::Texture2D *texture = new cocos2d::Texture2D();
  texture->initWithData(data, 4*width*height, cocos2d::Texture2D::PixelFormat::RGBA8888, width, height, cocos2d::Size(width, height));
  if (is_power_of_2(width) && is_power_of_2(height)) texture->generateMipmap(); // NOTE: This call will cause a crash if it's called with an image that isn't a power of 2 in both dimensions...
                                                                        // TODO: (We may want to print an error message in that case)
  textures.add(texture);
  return textures.length - 1;
}
int make_texture(char *name) {
  char filename[256]; //@MEMORY
  sprintf(filename, "bitmaps/%s", name);
  int pixel_width = 0, pixel_height = 0, bitdepth = 0;
  uchar *data = stbi_load(filename, &pixel_width, &pixel_height, &bitdepth, STBI_rgb_alpha);
  int texture_index = make_texture(data, pixel_width, pixel_height);
  STBI_FREE(data);
  return texture_index;
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

struct Font {
  cocos2d::TTFConfig *ttf_config;
  float line_height;
  float l_width;
};
Dynamic_Array<Font> fonts;

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
inline void set_draw_color(Color4B color) {
  draw_settings.draw_color.r = color.r;
  draw_settings.draw_color.g = color.g;
  draw_settings.draw_color.b = color.b;
  draw_settings.draw_color_opacity = color.a;
}
inline void set_draw_color(Color4 color) {
  set_draw_color(color4b(color));
}
inline void set_draw_color_bytes(u8 red, u8 green, u8 blue, u8 alpha) {
  draw_settings.draw_color.r = red;
  draw_settings.draw_color.g = green;
  draw_settings.draw_color.b = blue;
  draw_settings.draw_color_opacity = alpha;
}
inline void enable_screen_draw() {draw_settings.screen_draw = true;}
inline void disable_screen_draw() {draw_settings.screen_draw = false;}

void draw_rect(int texture, float x, float y, float w, float h, int z_order = 0) {
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
  float tw = textures[texture]->getPixelsWide();
  float th = textures[texture]->getPixelsHigh();
  item->sprite->setTexture(textures[texture]);
  cocos2d::Rect texture_rect = {0.0f, 0.0f, tw, th};
  item->sprite->setTextureRect(texture_rect);

  item->sprite->setLocalZOrder(z_order);
  item->sprite->setColor(draw_settings.draw_color);
  item->sprite->setOpacity(draw_settings.draw_color_opacity);
  item->sprite->setScaleX(w/tw);
  item->sprite->setScaleY(h/th);
  item->sprite->setPosition(cocos2d::Vec2(x, y) - cocos2d::Vec2((tw - w)/2.0f, (th - h)/2.0f)); //@CHECK: This might be incorrect
  item->sprite->setVisible(true);
  items->next_item++;
}
inline void draw_solid_rect(float x, float y, float w, float h, int z_order = 0) {
  draw_rect(0, x, y, w, h, z_order);
}

struct Text_Information {
  int num_lines;
  float line_height;
};

Rect get_text_rect(char *text, float x, float y, int font_index, Text_Information *info = NULL) {
  assert(font_index >= 0);
  assert(font_index < fonts.length);
  Graphics_Items *items = NULL;
  if (draw_settings.screen_draw) items = &screen_label_graphics_items;
  else items = &game_label_graphics_items;

  if (items->next_item >= items->items.length) {
    Graphics_Item item = {};
    item.label = cocos2d::Label::createWithTTF(*(fonts[font_index].ttf_config), text);
    item.label->setIgnoreAnchorPointForPosition(true);
	item.label->setUserData((void *)font_index); // Save the font id that this label is using

    if (draw_settings.screen_draw) screen_layer->addChild(item.label, 0);
    else game_layer->addChild(item.label, 0);
    items->items.add(item);
    item.label->setVisible(false);
  }
  Graphics_Item *item = items->items.data + items->next_item;

  // Check the id of the font that the label is using, only change the font if the ids don't match:
  if ((int)item->label->getUserData() != font_index) {
	  item->label->setTTFConfig(*(fonts[font_index].ttf_config));
	  item->label->setUserData((void *)font_index);
  }
  if (!match(item->label->getString(), text)) item->label->setString(text);

  y -= (item->label->getStringNumLines() - 1)*item->label->getLineHeight(); //@HACK: Figure out a better way to ensure that multi-line text moves down from y as the number of lines increases
  if (info) {
    info->num_lines = item->label->getStringNumLines();
    info->line_height = item->label->getLineHeight();
  }
  
  cocos2d::Size size = item->label->getContentSize();
  return {x, y, size.width, size.height};
}

Rect draw_text(char *text, float x, float y, int font_index, int z_order = 0) {
  assert(font_index >= 0);
  assert(font_index < fonts.length);
  Graphics_Items *items = NULL;
  if (draw_settings.screen_draw) items = &screen_label_graphics_items;
  else items = &game_label_graphics_items;

  if (items->next_item >= items->items.length) {
    Graphics_Item item = {};
    item.label = cocos2d::Label::createWithTTF(*(fonts[font_index].ttf_config), text);
    item.label->setIgnoreAnchorPointForPosition(true);
	item.label->setUserData((void *)font_index); // Save the font id that this label is using

    if (draw_settings.screen_draw) screen_layer->addChild(item.label, 0);
    else game_layer->addChild(item.label, 0);
    items->items.add(item);
  }
  Graphics_Item *item = items->items.data + items->next_item;

  // Check the id of the font that the label is using, only change the font if the ids don't match:
  if ((int)item->label->getUserData() != font_index) {
	  item->label->setTTFConfig(*(fonts[font_index].ttf_config));
	  item->label->setUserData((void *)font_index);
  }
  if (!match(item->label->getString(), text)) item->label->setString(text);

  item->label->setLocalZOrder(z_order);

  item->label->setColor(draw_settings.draw_color);
  item->label->setOpacity(draw_settings.draw_color_opacity);
  y -= (item->label->getStringNumLines() - 1)*item->label->getLineHeight(); //@HACK: Figure out a better way to ensure that multi-line text moves down from y as the number of lines increases
  item->label->setPosition(cocos2d::Vec2(x, y)); //@FIX: This is probably incorrect
  item->label->setVisible(true);
  items->next_item++;
  cocos2d::Size size = item->label->getContentSize();
  return {x, y, size.width, size.height};
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

//
// Fonts:
//

int add_font(char *font_filename, int font_size) {
  Font f = {};
  cocos2d::TTFConfig *ttf_config = new cocos2d::TTFConfig();
  ttf_config->fontFilePath = "fonts/";
  ttf_config->fontFilePath += font_filename;
  ttf_config->fontSize = font_size;
  ttf_config->glyphs = cocos2d::GlyphCollection::DYNAMIC;
  ttf_config->outlineSize = 0;
  ttf_config->customGlyphs = NULL;
  ttf_config->distanceFieldEnabled = false;
  f.ttf_config = ttf_config;
  
  fonts.add(f);
  int font_index = fonts.length - 1;

  bool screen_draw = draw_settings.screen_draw;
  if (!screen_draw) enable_screen_draw();
  fonts[font_index].l_width = get_text_rect("L", 0.0f, 0.0f, font_index).w;
  cocos2d::Label *label = screen_label_graphics_items.items[screen_label_graphics_items.next_item].label;
  fonts[font_index].line_height = label->getLineHeight();
  if (!screen_draw) disable_screen_draw();
  
  return font_index;
}

#include "ui.cpp"
#include "CSP.cpp"

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
  void update(float delta_time) {
    void draw_immediate_mode_graphics();

    dt = delta_time;
    total_time_elapsed += dt;
    ui_begin_frame();
    main_loop();
    ui_end_frame();
    reset_inputs();
    draw_immediate_mode_graphics();
  }
  CREATE_FUNC(Main_Scene);
};

AppDelegate::AppDelegate() {}
AppDelegate::~AppDelegate() {}

void AppDelegate::initGLContextAttrs() {
  // NOTE: OpenGL context attributes: red,green,blue,alpha,depth,stencil,multisamplesCount
  GLContextAttrs gl_context_attributes = {8, 8, 8, 8, 24, 8, 0};
  cocos2d::GLView::setGLContextAttrs(gl_context_attributes);
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
