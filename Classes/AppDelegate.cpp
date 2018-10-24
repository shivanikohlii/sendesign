#include "AppDelegate.h"

typedef uint64_t u64;

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
GLuint _glProgram = 0;
GLuint _vbo1 = 0, _vbo2 = 0;
GLuint _vao = 0;
USING_NS_CC;

void init_opengl() {
  auto program = GLProgram::createWithFilenames("shaders/basic_shader_vert.glsl", "shaders/basic_shader_frag.glsl");

  auto glProgramState = GLProgramState::create(program);
  main_scene->setGLProgramState(glProgramState);

  CHECK_GL_ERROR_DEBUG();

  auto size = Director::getInstance()->getWinSize();

  GLfloat vertices[] = {

    90,30,
    size.width - 90, 30,
    size.width / 2, size.width - 180

  };
	
  GLfloat colors[] = {

    1, 0, 0, 1,
    0, 1, 0, 1,
    0, 0, 1, 1

  };

  glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
  glEnable(GL_POLYGON_SMOOTH);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glGenVertexArrays(1, &_vao);

  glGenBuffers(1, &_vbo1);
  glGenBuffers(1, &_vbo2);


  glBindVertexArray(_vao);

  glBindBuffer(GL_ARRAY_BUFFER, _vbo1);

  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
  glEnableVertexAttribArray(GLProgram::VERTEX_ATTRIB_POSITION);

  glBindBuffer(GL_ARRAY_BUFFER, _vbo2);

  glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
  glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
  glEnableVertexAttribArray(GLProgram::VERTEX_ATTRIB_COLOR);

  glBindVertexArray(0);


  CHECK_GL_ERROR_DEBUG();
}
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

    init_opengl();
    
    return initialize();
  }
  void update(float dt) {
    void reset_ui();
    void cleanup_unused_ui_elements();
    
    reset_ui();
    main_loop(dt);
    reset_inputs();
    cleanup_unused_ui_elements();
  }
  void draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t parent_flags) {
    //cocos2d::DrawPrimitives::drawRect(cocos2d::Vec2(0.0f, 0.0f), cocos2d::Vec2(100.0f, 100.0f));
  }
#if 0
  void draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t parentFlags) {
    Layer::draw(renderer, transform, parentFlags);
    CustomCommand *_customCommand = new CustomCommand();
    _customCommand->init(_globalZOrder, transform, parentFlags);
    _customCommand->func = CC_CALLBACK_0(HelloWorld::onDraw, this, transform, parentFlags);
    Director::getInstance()->getRenderer()->addCommand(_customCommand);
  }

  void onDraw(const Mat4 &transform, uint32_t flags) {
    getGLProgramState()->applyGLProgram(transform);
    glBindVertexArray(_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    CC_INCREMENT_GL_DRAWN_BATCHES_AND_VERTICES(1, 3);
    glBindVertexArray(0);
    CHECK_GL_ERROR_DEBUG();
  }
#endif
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
