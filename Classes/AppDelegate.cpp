#include "AppDelegate.h"

#define STB_IMAGE_IMPLEMENTATION
#include "libraries/stb_image.h"

#define max_val(a, b) (((a) > (b)) ? (a) : (b))
#define min_val(a, b) (((a) < (b)) ? (a) : (b))
#define array_size(a) ((sizeof(a))/(sizeof((a)[0])))

#include "modules/basic.cpp"
#include "modules/dynamic_array.cpp"
#include "modules/math.cpp"

bool is_power_of_2(int n) {
  while ((n % 2) == 0) n /= 2;
  if (n > 1) return false;
  return true;
}

#include "modules/random.cpp"
#include "modules/string_hash_table.cpp"
#include "modules/string.cpp"

// NOTE: This is supposed to be a fast std::string to cstring comparison which
// guarantees there will be no cstring to std::string conversions or
// vice versa as well as any other std C++ nonsense.
inline bool match(const std::string &str1, const char *str2) {
  int str1_length = str1.length();
  return strncmp(str1.data(), str2, str1_length) == 0 && str2[str1_length] == 0;
}

#include "shared.cpp"

// Note: This will put the cocos2d library into the global namespace:      USING_NS_CC;

#include "audio/include/AudioEngine.h"
#include "input.cpp"

//
// Game "Engine" Globals:
//

float total_time_elapsed = 0.0f;

#define PROGRAM_NAME "CSP"

cocos2d::Scene *main_scene = NULL;
cocos2d::Layer *screen_layer = NULL;

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

int load_texture_from_data(uchar *data, int width, int height) {
  cocos2d::Texture2D *texture = new cocos2d::Texture2D();
  texture->initWithData(data, 4*width*height, cocos2d::Texture2D::PixelFormat::RGBA8888, width, height, cocos2d::Size(width, height));
  if (is_power_of_2(width) && is_power_of_2(height)) texture->generateMipmap(); // NOTE: This call will cause a crash if it's called with an image that isn't a power of 2 in both dimensions...
                                                                        // TODO: (We may want to print an error message in that case)
  Texture t;
  t.cocos_texture = texture;
  t.width = width;
  t.height = height;
  csp->textures.add(t);
  return csp->textures.length - 1;
}
int load_texture_from_file(char *name) {
  char filename[256]; //@MEMORY
  sprintf(filename, "data/bitmaps/%s", name);
  int pixel_width = 0, pixel_height = 0, bitdepth = 0;
  uchar *data = stbi_load(filename, &pixel_width, &pixel_height, &bitdepth, STBI_rgb_alpha);
  if (!data) return -1;
  int texture_index = load_texture_from_data(data, pixel_width, pixel_height);
  STBI_FREE(data);
  return texture_index;
}
inline int load_texture(uchar *data, int width, int height) {
  return load_texture_from_data(data, width, height);
}
inline int load_texture(char *name) {
  return load_texture_from_file(name);
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
  std::string filename = "data/music/";
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
  std::string filename = "data/sounds/";
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
inline int __play_music(int music_index, bool loop) {
  assert(music_index >= 0);
  assert(music_index < songs.length);
  return cocos2d::experimental::AudioEngine::play2d(songs[music_index].filename, loop);
}
inline int __play_sound(int sound_index, bool loop) {
  assert(sound_index >= 0);
  assert(sound_index < sounds.length);
  return cocos2d::experimental::AudioEngine::play2d(sounds[sound_index].filename, loop);
}
inline int play_music(int music_index, bool loop = true) {
  return __play_music(music_index, loop);
}
inline int play_sound(int sound_index, bool loop = false) {
  return __play_sound(sound_index, loop);
}
inline void pause_audio(int audio_id) {
  cocos2d::experimental::AudioEngine::pause(audio_id);
}
inline void stop_audio(int audio_id) {
  cocos2d::experimental::AudioEngine::stop(audio_id);
}
inline void resume_audio(int audio_id) {
  cocos2d::experimental::AudioEngine::resume(audio_id);
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

void __draw_rect(int texture, float x, float y, float w, float h, int z_order, float rotation) {
  assert(texture >= 0);
  assert(texture < csp->textures.length);
  Graphics_Items *items = NULL;
  if (csp->draw_settings.screen_draw) items = &screen_sprite_graphics_items;
  else items = &game_sprite_graphics_items;
  
  if (items->next_item >= items->items.length) {
    Graphics_Item item = {};
    item.sprite = cocos2d::Sprite::createWithTexture((cocos2d::Texture2D *)csp->textures[texture].cocos_texture);
    item.sprite->setIgnoreAnchorPointForPosition(true);
    
    if (csp->draw_settings.screen_draw) screen_layer->addChild(item.sprite, 0);
    else main_scene->addChild(item.sprite, 0);
    items->items.add(item);
  }
  Graphics_Item *item = items->items.data + items->next_item;
  float tw = csp->textures[texture].width;
  float th = csp->textures[texture].height;
  item->sprite->setTexture((cocos2d::Texture2D *)csp->textures[texture].cocos_texture);
  cocos2d::Rect texture_rect = {0.0f, 0.0f, tw, th};
  item->sprite->setTextureRect(texture_rect);
  item->sprite->setRotation(rotation);

  item->sprite->setLocalZOrder(z_order);
  item->sprite->setColor(cocos2d::Color3B(csp->draw_settings.draw_color.r,
					  csp->draw_settings.draw_color.g,
					  csp->draw_settings.draw_color.b));
  item->sprite->setOpacity(csp->draw_settings.draw_color_opacity);
  
  item->sprite->setScaleX(w/tw);
  item->sprite->setScaleY(h/th);
  item->sprite->setPosition(cocos2d::Vec2(x, y) - cocos2d::Vec2((tw - w)/2.0f, (th - h)/2.0f));
  item->sprite->setVisible(true);
  items->next_item++;
}
inline void draw_rect(int texture, float x, float y, float w, float h, int z_order = 0, float rotation = 0.0f) {
  __draw_rect(texture, x, y, w, h, z_order, rotation);
}
inline void draw_solid_rect(float x, float y, float w, float h, int z_order = 0, float rotation = 0.0f) {
  draw_rect(0, x, y, w, h, z_order, rotation);
}

Rect __get_text_rect(char *text, float x, float y, int font_index, Text_Information *info) {
  assert(font_index >= 0);
  assert(font_index < fonts.length);
  Graphics_Items *items = NULL;
  if (csp->draw_settings.screen_draw) items = &screen_label_graphics_items;
  else items = &game_label_graphics_items;

  if (items->next_item >= items->items.length) {
    Graphics_Item item = {};
    item.label = cocos2d::Label::createWithTTF(*(fonts[font_index].ttf_config), text);
    item.label->setIgnoreAnchorPointForPosition(true);
	item.label->setUserData((void *)font_index); // Save the font id that this label is using

    if (csp->draw_settings.screen_draw) screen_layer->addChild(item.label, 0);
    else main_scene->addChild(item.label, 0);
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
inline Rect get_text_rect(char *text, float x, float y, int font_index, Text_Information *info = NULL) {
  return __get_text_rect(text, x, y, font_index, info);
}

Rect __draw_text(char *text, float x, float y, int font_index, int z_order) {
  assert(font_index >= 0);
  assert(font_index < fonts.length);
  Graphics_Items *items = NULL;
  if (csp->draw_settings.screen_draw) items = &screen_label_graphics_items;
  else items = &game_label_graphics_items;

  if (items->next_item >= items->items.length) {
    Graphics_Item item = {};
    item.label = cocos2d::Label::createWithTTF(*(fonts[font_index].ttf_config), text);
    item.label->setIgnoreAnchorPointForPosition(true);
	item.label->setUserData((void *)font_index); // Save the font id that this label is using

    if (csp->draw_settings.screen_draw) screen_layer->addChild(item.label, 0);
    else main_scene->addChild(item.label, 0);
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

  item->label->setColor(cocos2d::Color3B(csp->draw_settings.draw_color.r,
					 csp->draw_settings.draw_color.g,
					 csp->draw_settings.draw_color.b));
  item->label->setOpacity(csp->draw_settings.draw_color_opacity);
  y -= (item->label->getStringNumLines() - 1)*item->label->getLineHeight(); //@HACK: Figure out a better way to ensure that multi-line text moves down from y as the number of lines increases
  item->label->setPosition(cocos2d::Vec2(x, y));
  item->label->setVisible(true);
  items->next_item++;
  cocos2d::Size size = item->label->getContentSize();
  return {x, y, size.width, size.height};
}
inline Rect draw_text(char *text, float x, float y, int font_index, int z_order = 0) {
	return __draw_text(text, x, y, font_index, z_order);
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

int load_font(char *font_filename, int font_size) {
  Font f = {};
  cocos2d::TTFConfig *ttf_config = new cocos2d::TTFConfig();
  ttf_config->fontFilePath = "data/fonts/";
  ttf_config->fontFilePath += font_filename;
  ttf_config->fontSize = font_size;
  ttf_config->glyphs = cocos2d::GlyphCollection::DYNAMIC;
  ttf_config->outlineSize = 0;
  ttf_config->customGlyphs = NULL;
  ttf_config->distanceFieldEnabled = false;
  f.ttf_config = ttf_config;
  
  fonts.add(f);
  int font_index = fonts.length - 1;

  bool screen_draw = csp->draw_settings.screen_draw;
  if (!screen_draw) enable_screen_draw();
  fonts[font_index].l_width = get_text_rect("L", 0.0f, 0.0f, font_index).w;
  cocos2d::Label *label = screen_label_graphics_items.items[screen_label_graphics_items.next_item].label;
  fonts[font_index].line_height = label->getLineHeight();
  if (!screen_draw) disable_screen_draw();
  
  return font_index;
}

void (*user_main_loop)(void) = NULL;
void (*user_initialize)(void) = NULL;

#include "ui.cpp"
#include "CSP.cpp"

#ifdef _WIN32
inline u64 get_file_last_modified_time(char *file) {
  FILETIME filetime;
  HANDLE file_handle = CreateFileA(file, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
  if (file_handle == INVALID_HANDLE_VALUE) return 0;
  bool result = GetFileTime(file_handle, 0, 0, &filetime);
  u64 time = (u64)filetime.dwLowDateTime | (((u64)filetime.dwHighDateTime) << 32);
  CloseHandle(file_handle);
  return time;
}
#else
inline u64 get_file_last_modified_date(char *file) {
  stat st;
  if (stat(file, &st)) {
    printf("Failed to get info for file '%s'.\n", file);
    return 0;
  } else {
    time_t time = st.st_mtime;
    tm zero_time_tm = {};
    zero_time_tm.tm_mday = 1;
    time_t zero_time = mktime(&zero_time_tm);
    double time_elapsed = difftime(time, zero_time);
    time_elapsed *= 1000.0f; //@REVISE: Possibly make this larger.
    //assert(time_elapsed < (double)U64_MAX);
    return (u64)(time_elapsed + 0.5f);
  }
}
#endif

void hotload_code() {
#ifdef _WIN32
  static HMODULE game_code_library = NULL;
  static u64 time_since_code_load = 0;
  u64 time = get_file_last_modified_time("game.dll");
  if (time_since_code_load != time) {
    time_since_code_load = time;
    if (game_code_library) FreeLibrary(game_code_library);
    CopyFileA("game.dll", "game_temp.dll", FALSE);
    game_code_library = LoadLibraryA("game_temp.dll");
    if (game_code_library) {
      user_main_loop = (void (*)(void))GetProcAddress(game_code_library, "__main_loop");
      if (!user_main_loop) printf("ERROR: Couldn't find '__main_loop()' in game.dll\n");
      int (*load_csp_lib_functions)(CSP_Library_Load) = (int(*)(CSP_Library_Load))GetProcAddress(game_code_library, "__load_csp_lib_functions");
      if (load_csp_lib_functions) {
	CSP_Library_Load lib_load = {};
	lib_load.csp = csp;
	lib_load.draw_rect = __draw_rect;
	lib_load.get_text_rect = __get_text_rect;
	lib_load.draw_text = __draw_text;
	lib_load.load_texture_from_data = load_texture_from_data;
	lib_load.load_texture_from_file = load_texture_from_file;
	lib_load.load_font = load_font;
	lib_load.load_music = load_music;
	lib_load.load_sound = load_sound;
	lib_load.play_music = __play_music;
	lib_load.play_sound = __play_sound;
	lib_load.pause_audio = pause_audio;
	lib_load.stop_audio = stop_audio;
	lib_load.resume_audio = resume_audio;
	lib_load.add_action_binding = add_button_binding;
	lib_load.action_button = action_button;
	lib_load.ui_begin = ui_begin;
	lib_load.button = button;
	lib_load.text = text;
	lib_load.text_field = __text_field;
	lib_load.int_edit = __int_edit;
	lib_load.unsigned_char_edit = __unsigned_char_edit;
	lib_load.color_edit = color_edit;
	lib_load.float_edit = __float_edit;
	lib_load.checkbox = checkbox;
	lib_load.spacing = __spacing;
	
	int res = load_csp_lib_functions(lib_load);
	if (res != 0) printf("ERROR: Error loading csp library functions!\n");
      }
      else {
	printf("ERROR: Couldn't find '__load_csp_lib_functions' in game.dll\n");
      }
      user_initialize = (void(*)(void))GetProcAddress(game_code_library, "__initialize");
      if (!user_initialize) printf("ERROR: Couldn't find '__initialize' in game.dll\n");
    }
    else {
      printf("ERROR: Couldn't load 'game.dll' (ERROR Code: %x)\n", GetLastError());
    }
  }
#endif
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
    
    auto mouse_listener = cocos2d::EventListenerMouse::create();
    mouse_listener->onMouseDown = on_mouse_down;
    mouse_listener->onMouseMove = on_mouse_move;
    mouse_listener->onMouseScroll = on_mouse_scroll;
    mouse_listener->onMouseUp = on_mouse_up;
    _eventDispatcher->addEventListenerWithFixedPriority(mouse_listener, 1);

    auto keyboard_listener = cocos2d::EventListenerKeyboard::create();
    keyboard_listener->onKeyPressed = on_key_pressed;
    keyboard_listener->onKeyReleased = on_key_released;
    _eventDispatcher->addEventListenerWithFixedPriority(keyboard_listener, 1);
    
    //NOTE: Did this in hopes this would get around the keycode to character code translation problem
    // but of course this didn't work...
#if 0
    cocos2d::Director *director = cocos2d::Director::getInstance();
    cocos2d::GLViewImpl *glview = (cocos2d::GLViewImpl *)director->getOpenGLView();
    GLFWwindow *window = glview->getWindow();
    glfwSetCharCallback(window, character_callback);
#endif

    bool res = initialize();
    hotload_code();
    if (user_initialize) user_initialize();

    return res;
  }

  void update(float delta_time) {
    void draw_immediate_mode_graphics();

    csp->dt = delta_time;
    total_time_elapsed += csp->dt;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) || (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
    if (csp->key[KEY_ALT].is_down && csp->key['\n'].just_pressed) {
      csp->fullscreen = !csp->fullscreen;
      cocos2d::Director *director = cocos2d::Director::getInstance();
      cocos2d::GLViewImpl *glview = (cocos2d::GLViewImpl *)director->getOpenGLView();
      if (csp->fullscreen) glview->setFullscreen();
      else glview->setWindowed(csp->windowed_mode_width, csp->windowed_mode_height);
      cocos2d::Size window_resolution = glview->getFrameSize();
      csp->window_width = window_resolution.width;
      csp->window_height = window_resolution.height;
      glview->setDesignResolutionSize(csp->window_width, csp->window_height, ResolutionPolicy::NO_BORDER);
    }
#endif

    if (csp->hotloading_enabled) hotload_code();
    update_bindings();
    ui_begin_frame();
    main_loop();
    if (user_main_loop) user_main_loop();
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

enum Config_Parameter_Type {
  CONFIG_PARAM_FLOAT = 0, CONFIG_PARAM_INT = 1,
  CONFIG_PARAM_BOOL = 2, CONFIG_PARAM_STRING = 3,
  NUM_CONFIG_PARAM_TYPES = 4
};
struct Config_Parameter {
  int type;
  void *var;
};
Dynamic_Array<Config_Parameter> config_parameters(8);
inline void add_config_param(float *param) {
  Config_Parameter c = {CONFIG_PARAM_FLOAT, param};
  config_parameters.add(c);
}
inline void add_config_param(int *param) {
  Config_Parameter c = {CONFIG_PARAM_INT, param};
  config_parameters.add(c);
}
inline void add_config_param(bool *param) {
  Config_Parameter c = {CONFIG_PARAM_BOOL, param};
  config_parameters.add(c);
}
inline void add_config_param(char *param) {
  Config_Parameter c = {CONFIG_PARAM_STRING, param};
  config_parameters.add(c);
}


struct Test_Struct {
	int a;
};
void change_int(Test_Struct &a) {
	a.a = 5;
}

bool AppDelegate::applicationDidFinishLaunching() {
#ifdef _DEBUG
#ifdef _WIN32
  AllocConsole();
  FILE *console;
  freopen_s(&console, "CONOUT$", "wb", stdout);
#endif
#endif

  csp = new CSP;

  add_config_param(&csp->windowed_mode_width);
  add_config_param(&csp->windowed_mode_height);
  add_config_param(&csp->fullscreen);
  
  FILE *config_file = fopen("data/config.txt", "r");
  if (!config_file) {
    printf("ERROR: Couldn't find config file, using default settings.\n");
  } else {
    bool error = false;
    for (int i = 0; i < config_parameters.length; i++) {
      Config_Parameter c = config_parameters[i];
      for (;;) {
	int res = 0;
	switch (c.type) {
	case CONFIG_PARAM_FLOAT:
	  res = fscanf(config_file, "%f", (float *)c.var);
	  break;
	case CONFIG_PARAM_INT:
	  res = fscanf(config_file, "%i", (int *)c.var);
	  break;
	case CONFIG_PARAM_BOOL:
	  {
	    int tmp = 0;
	    res = fscanf(config_file, "%i", &tmp);
	    if (res > 0) *(bool *)c.var = (tmp != 0);
	  }
	  break;
	case CONFIG_PARAM_STRING:
	  res = fscanf(config_file, "%s", (char *)c.var);
	  break;
	default:
	  assert(!"ERROR: Invalid config parameter type.\n");
	  break;
	}
	if (res > 0) break;
	if (res == EOF) {
	  error = true;
	  break;
	}
	res = fscanf(config_file, "%*s");
	if (res == EOF) {
	  error = true;
	  break;
	}
      }
      if (error) break;
    }
    if (error) printf("ERROR: There was an error parsing the config file. It has an invalid format.\n");
    fclose(config_file);
  }
  
  auto director = cocos2d::Director::getInstance();
  auto glview = director->getOpenGLView();
  if (!glview) {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) || (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
    if (csp->fullscreen) glview = cocos2d::GLViewImpl::createWithFullScreen(PROGRAM_NAME);
    else glview = cocos2d::GLViewImpl::createWithRect(PROGRAM_NAME, cocos2d::Rect(0, 0, csp->windowed_mode_width, csp->windowed_mode_height));
#else
    glview = GLViewImpl::create(PROGRAM_NAME);
#endif
    director->setOpenGLView(glview);
  }

#if _DEBUG
  director->setDisplayStats(true);
#endif

  director->setAnimationInterval(1.0f / 60);
  cocos2d::Size window_resolution = glview->getFrameSize();
  csp->window_width = window_resolution.width;
  csp->window_height = window_resolution.height;
  glview->setDesignResolutionSize(csp->window_width, csp->window_height, ResolutionPolicy::NO_BORDER);

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
  csp->window_width = new_width;
  csp->window_height = new_height;
}
