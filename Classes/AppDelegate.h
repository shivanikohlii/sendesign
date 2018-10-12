#ifndef APP_DELEGATE_H_
#define APP_DELEGATE_H_
#include "cocos2d.h"

struct AppDelegate : private cocos2d::Application {
    AppDelegate();
    virtual ~AppDelegate();

    virtual void initGLContextAttrs();
    virtual bool applicationDidFinishLaunching();
    virtual void applicationDidEnterBackground();
    virtual void applicationWillEnterForeground();
    virtual void applicationScreenSizeChanged(int new_width, int new_height);
};
#endif
