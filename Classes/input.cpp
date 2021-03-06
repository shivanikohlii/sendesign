// TODOS:
// -Caps Lock Support
// -Key Repeat

#define KeyCode cocos2d::EventKeyboard::KeyCode

// NOTE: These are our own "custom" codes for every keycode that doesn't have a direct
// character. Every other keycode is represented by its corresponding character.
// For example:
// The keycode for "a" has a direct character and is represented by character 'a'
// but the keycode for the "home key" is represented by KEY_HOME

// NOTE: Any noncharacter key code should be guaranteed to be >= 128,
// this is how we will tell if a keycode should be used for inputting text 

// NOTE: There is special behavior for the shift, alt, and ctrl keys:
// These keys have different key codes for their left and right versions, but
// also have a special code which corresponds to both the left and right versions.
// For example:
// There is a keycode for left shift (KEY_LEFT_SHIFT) and right shift (KEY_RIGHT_SHIFT)
// there is also a key for shift (KEY_SHIFT) which will be set as down if either
// KEY_LEFT_SHIFT or KEY_RIGHT_SHIFT are down

void reset_inputs() {
  csp->mouse.scroll = 0.0f;
  csp->mouse.left.just_pressed = false;
  csp->mouse.right.just_pressed = false;
  for (int i = 0; i < csp->keys_just_pressed.length; i++) {
    uchar c = csp->keys_just_pressed[i];
    csp->key[c].just_pressed = false;
  }
  csp->characters_typed.length = 0;
  csp->keys_just_pressed.length = 0;
}
void on_mouse_down(cocos2d::Event *event) {
  cocos2d::EventMouse *mouse_event = (cocos2d::EventMouse *)event;
  if ((int)mouse_event->getMouseButton() == 1) {
    csp->mouse.right.is_down = csp->mouse.right.just_pressed = true;
  } else if ((int)mouse_event->getMouseButton() == 0) {
    csp->mouse.left.is_down = csp->mouse.left.just_pressed = true;
  }
}
void on_mouse_up(cocos2d::Event *event) {
  cocos2d::EventMouse *mouse_event = (cocos2d::EventMouse *)event;
  if ((int)mouse_event->getMouseButton() == 1) csp->mouse.right.is_down = false;
  else if ((int)mouse_event->getMouseButton() == 0) csp->mouse.left.is_down = false;
}
void on_mouse_move(cocos2d::Event *event) {
  cocos2d::EventMouse *mouse_event = (cocos2d::EventMouse *)event;
  csp->mouse.x = mouse_event->getCursorX();
  csp->mouse.y = mouse_event->getCursorY();
}
void on_mouse_scroll(cocos2d::Event *event) {
  cocos2d::EventMouse *mouse_event = (cocos2d::EventMouse *)event;
  csp->mouse.scroll = mouse_event->getScrollY();
}

// NOTE: This process_keycode procedure may seem ridiculous,
// but the reason why it needs to exist is because the keycodes for
// cocos2d do not correspond to their ascii character code.
// For example: KeyCode::KEY_A is a different value than character 'a' (this directly contradicts the cocos2d documentation, but I tested it and this seems to be the case)

// This is annoying as hell for text input because we want each key
// input to map directly to their corresponding character.

// As a result, this process_keycode procedure as well as several other lines
// of code in this file are dedicated to translating each cocos2d keycode into
// its corresponding character code.

bool process_keycode(KeyCode key_code, uchar *out) { 
  uchar c = (uchar)key_code;
  bool add_key = true;
  if (c >= (uchar)KeyCode::KEY_A && c <= (uchar)KeyCode::KEY_Z) {
    c = (c - (uchar)KeyCode::KEY_A) + 'a';
  } else if (c >= (uchar)KeyCode::KEY_0 && c <= (uchar)KeyCode::KEY_9) {
    c = (c - (uchar)KeyCode::KEY_0) + '0';
  } else {
    switch (key_code) {
    case KeyCode::KEY_F1: c = KEY_F1; break;
    case KeyCode::KEY_F2: c = KEY_F2; break;
    case KeyCode::KEY_F3: c = KEY_F3; break;
    case KeyCode::KEY_F4: c = KEY_F4; break;
    case KeyCode::KEY_F5: c = KEY_F5; break;
    case KeyCode::KEY_F6: c = KEY_F6; break;
    case KeyCode::KEY_F7: c = KEY_F7; break;
    case KeyCode::KEY_F8: c = KEY_F8; break;
    case KeyCode::KEY_F9: c = KEY_F9; break;
    case KeyCode::KEY_F10: c = KEY_F10; break;
    case KeyCode::KEY_F11: c = KEY_F11; break;
    case KeyCode::KEY_F12: c = KEY_F12; break;
    case KeyCode::KEY_ESCAPE: c = KEY_ESCAPE; break;
    case KeyCode::KEY_GRAVE: c = '`'; break;
    case KeyCode::KEY_TAB: c = '\t'; break;
    case KeyCode::KEY_CAPS_LOCK: c = KEY_CAPS_LOCK; break;
    case KeyCode::KEY_SPACE: c = ' '; break;
    case KeyCode::KEY_MINUS: c = '-'; break;
    case KeyCode::KEY_EQUAL: c = '='; break;
    case KeyCode::KEY_BACKSPACE: c = KEY_BACKSPACE; break;
    case KeyCode::KEY_LEFT_BRACKET: c = '['; break;
    case KeyCode::KEY_RIGHT_BRACKET: c = ']'; break;
    case KeyCode::KEY_SEMICOLON: c = ';'; break;
    case KeyCode::KEY_APOSTROPHE: c = '\''; break;
    case KeyCode::KEY_BACK_SLASH: c = '\\'; break;
    case KeyCode::KEY_ENTER: c = '\n'; break; //@REVISE?: Make this a non-character key code?
    case KeyCode::KEY_COMMA: c = ','; break;
    case KeyCode::KEY_PERIOD: c = '.'; break;
    case KeyCode::KEY_SLASH: c = '/'; break;
    case KeyCode::KEY_LEFT_SHIFT: c = KEY_LEFT_SHIFT; break;
    case KeyCode::KEY_RIGHT_SHIFT: c = KEY_RIGHT_SHIFT; break;
    case KeyCode::KEY_LEFT_ALT: c = KEY_ALT; break;
    case KeyCode::KEY_RIGHT_ALT: c = KEY_RIGHT_ALT; break;
    case KeyCode::KEY_LEFT_CTRL: c = KEY_LEFT_CTRL; break;
    case KeyCode::KEY_RIGHT_CTRL: c = KEY_RIGHT_CTRL; break;
    case KeyCode::KEY_INSERT: c = KEY_INSERT; break;
    case KeyCode::KEY_HOME: c = KEY_HOME; break;
    case KeyCode::KEY_PG_UP: c = KEY_PAGE_UP; break;
    case KeyCode::KEY_DELETE: c = KEY_DELETE; break;
    case KeyCode::KEY_END: c = KEY_END; break;
    case KeyCode::KEY_PG_DOWN: c = KEY_PG_DOWN; break;
    case KeyCode::KEY_LEFT_ARROW: c = KEY_LEFT; break;
    case KeyCode::KEY_UP_ARROW: c = KEY_UP; break;
    case KeyCode::KEY_DOWN_ARROW: c = KEY_DOWN; break;
    case KeyCode::KEY_RIGHT_ARROW: c = KEY_RIGHT; break;
    case KeyCode::KEY_PRINT: c = KEY_PRINT_SCREEN; break;
    case KeyCode::KEY_SCROLL_LOCK: c = KEY_SCROLL_LOCK; break;
    case KeyCode::KEY_PAUSE: c = KEY_PAUSE; break;
    default: add_key = false; break;
    }
  }
    
  if (add_key) {
    *out = c;
    return true;
  }
  return false;
}
void on_key_pressed(KeyCode key_code, cocos2d::Event *event) {
  uchar c;
  if (process_keycode(key_code, &c)) {
    if (c < 128) {
      uchar str_char = c;
      if (csp->key[KEY_SHIFT].is_down) {
	if (str_char >= 'a' && str_char <= 'z') {
	  str_char += 'A' - 'a';
	} else {
	  switch (str_char) {
	  case '`': str_char = '~'; break;
	  case '1': str_char = '!'; break;
	  case '2': str_char = '@'; break;
	  case '3': str_char = '#'; break;
	  case '4': str_char = '$'; break;
	  case '5': str_char = '%'; break;
	  case '6': str_char = '^'; break;
	  case '7': str_char = '&'; break;
	  case '8': str_char = '*'; break;
	  case '9': str_char = '('; break;
	  case '0': str_char = ')'; break;
	  case '-': str_char = '_'; break;
	  case '=': str_char = '+'; break;
	  case '[': str_char = '{'; break;
	  case ']': str_char = '}'; break;
	  case ';': str_char = ':'; break;
	  case '\'': str_char = '"'; break;
	  case '\\': str_char = '|'; break;
	  case ',': str_char = '<'; break;
	  case '.': str_char = '>'; break;
	  case '/': str_char = '?'; break;
	  }
	}
      }

      csp->characters_typed.add(str_char);
    } else {
      switch (key_code) {
      case KeyCode::KEY_LEFT_SHIFT:
      case KeyCode::KEY_RIGHT_SHIFT:
	csp->keys_just_pressed.add(KEY_SHIFT);
	csp->key[KEY_SHIFT].just_pressed = true;
	csp->key[KEY_SHIFT].is_down = true;
	break;
      case KeyCode::KEY_LEFT_ALT:
      case KeyCode::KEY_RIGHT_ALT:
	csp->keys_just_pressed.add(KEY_ALT);
	csp->key[KEY_ALT].just_pressed = true;
	csp->key[KEY_ALT].is_down = true;
	break;
      case KeyCode::KEY_LEFT_CTRL:
      case KeyCode::KEY_RIGHT_CTRL:
	csp->keys_just_pressed.add(KEY_CTRL);
	csp->key[KEY_CTRL].just_pressed = true;
	csp->key[KEY_CTRL].is_down = true;
	break;
      }
    }
    
    csp->keys_just_pressed.add(c);
    csp->key[c].just_pressed = true;
    csp->key[c].is_down = true;
  }
}
void on_key_released(KeyCode key_code, cocos2d::Event *event) {
  uchar c;
  if (process_keycode(key_code, &c)) {
    switch (key_code) {
    case KeyCode::KEY_LEFT_SHIFT:
      if (!csp->key[KEY_RIGHT_SHIFT].is_down) csp->key[KEY_SHIFT].is_down = false;
      break;
    case KeyCode::KEY_RIGHT_SHIFT:
      if (!csp->key[KEY_LEFT_SHIFT].is_down) csp->key[KEY_SHIFT].is_down = false;
      break;
    case KeyCode::KEY_LEFT_ALT:
      if (!csp->key[KEY_RIGHT_ALT].is_down) csp->key[KEY_ALT].is_down = false;
      break;
    case KeyCode::KEY_RIGHT_ALT:
      if (!csp->key[KEY_LEFT_ALT].is_down) csp->key[KEY_ALT].is_down = false;
      break;
    case KeyCode::KEY_LEFT_CTRL:
      if (!csp->key[KEY_RIGHT_CTRL].is_down) csp->key[KEY_CTRL].is_down = false;
      break;
    case KeyCode::KEY_RIGHT_CTRL:
      if (!csp->key[KEY_LEFT_CTRL].is_down) csp->key[KEY_CTRL].is_down = false;
      break;
    }
    csp->key[c].is_down = false;
  }
}

void character_callback(GLFWwindow* window, unsigned int codepoint) {
  if (codepoint <= UCHAR_MAX) csp->characters_typed.add(codepoint);
}
