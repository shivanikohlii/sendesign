// A fairly flexible math library for 2d, 3d, and 4d vectors
// as well as 3d and 4d vectors with unsigned
// 8 bit integer components (for colors)
//
// The 2d vectors are for 2d game math and the
// 3d and 4d vectors are primarily for working
// with colors more easily

// The structs below may look complicated, but
// it's just for flexibility. You can really 
// think of each of these as:
// struct V2 {float x, y;};
// struct V3 {float x, y, z;};
// struct V4 {float x, y, z, w;};
// struct Color3B {u8 r, g, b;};
// struct Color4B {u8 r, g, b, a;};

#define PI 3.14159265358979323846264338327950288f

union V2 {
  struct {float x, y;};
  float f[2];
};
union V3 {
  struct {
    union {
      V2 xy;
      struct {float x, y;};
    };
    float z;
  };
  struct {
    union {
      V2 rg;
      struct {float r, g;};
    };
    float b;
  };
  float f[3];
  static const V3 WHITE;
  static const V3 BLACK;
};
typedef V3 Color3;
const Color3 Color3::WHITE = { 1.0f, 1.0f, 1.0f };
const Color3 Color3::BLACK = { 0.0f, 0.0f, 0.0f };

union V4 {
  struct {
    union {
      V3 xyz;
      struct {
	union {
	  V2 xy;
	  struct {float x, y;};
	};
	float z;
      };
    };
    float w;
  };
  struct {
    union {
      V3 rgb;
      struct {
	union {
	  V2 rg;
	  struct {float r, g;};
	};
	float b;
      };
    };
    float a;
  };
  float f[4];
  static const V4 WHITE;
  static const V4 BLACK;
};
typedef V4 Color4;
const Color4 Color4::WHITE = {1.0f, 1.0f, 1.0f, 1.0f};
const Color4 Color4::BLACK = {0.0f, 0.0f, 0.0f, 1.0f};

union Color3B {
  struct {u8 r, g, b;};
  u8 c[3];
  static const Color3B WHITE;
  static const Color3B BLACK;
};
const Color3B Color3B::WHITE = {255, 255, 255};
const Color3B Color3B::BLACK = {0, 0, 0};
union Color4B {
  struct {
    union {
      Color3B rgb;
      struct {u8 r, g, b;};
    };
    u8 a;
  };
  u8 c[4];
  static const Color4B WHITE;
  static const Color4B BLACK;
};
const Color4B Color4B::WHITE = { 255, 255, 255, 255 };
const Color4B Color4B::BLACK = {0, 0, 0, 255};

struct Rect {
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
inline V2 v2(float i) {return {i, i};}
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

inline V3 v3(float x, float y, float z) {return {x, y, z};}
inline V3 v3(V2 v, float z) {return {v.x, v.y, z};}
inline V3 v3(float i) { return { i, i, i }; };
inline Color3 color3(float r, float g, float b) {return {r, g, b};}
inline Color3 color3(float i) { return { i, i, i }; };
inline Color3 color3(Color3B b) {return {(float)b.r/255.0f, (float)b.g/255.0f, (float)b.b/255.0f};}
inline V3 operator+(V3 a, V3 b) {return v3(a.x + b.x, a.y + b.y, a.z + b.z);}
inline V3 operator-(V3 a, V3 b) {return v3(a.x - b.x, a.y - b.y, a.z - b.z);}
inline V3 operator*(V3 a, float b) {return v3(a.x*b, a.y*b, a.z*b);}
inline V3 operator*(float a, V3 b) {return v3(b.x*a, b.y*a, b.z*a);}
inline V3 operator*(V3 a, V3 b) {return v3(a.x*b.x, a.y*b.y, a.z*b.y);}
inline V3 operator/(V3 a, float b) {return v3(a.x/b, a.y/b, a.z/b);}
inline V3 operator-(V3 a) {return v3(-a.x, -a.y, -a.z);}
inline V3 &operator*=(V3 &a, float b) {return (a = a*b);}
inline V3 &operator/=(V3 &a, float b) {return (a = a/b);}
inline V3 &operator+=(V3 &a, V3 b) {return (a = a + b);}
inline V3 &operator-=(V3 &a, V3 b) {return (a = a - b);}
inline bool operator==(V3 a, V3 b) {return a.x == b.x && a.y == b.y && a.z == b.y;}
inline bool operator!=(V3 a, V3 b) {return !(a == b);}
inline float mag(V3 v) {return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);}
inline V3 normalize(V3 v) {
  float m = mag(v);
  if (m == 0.0f) return v3(0.0f, 0.0f, 0.0f);
  return v / m;
}

inline V4 v4(float x, float y, float z, float w) {return {x, y, z, w};}
inline V4 v4(V3 v, float w) {return {v.x, v.y, v.z, w};}
inline V4 v4(V2 v, float z, float w) {return {v.x, v.y, z, w};}
inline V4 v4(V2 v1, V2 v2) {return {v1.x, v1.y, v2.x, v2.y};}
inline V4 v4(float i) {return {i, i, i, i};}
inline Color4 color4(float r, float g, float b, float a) {return {r, g, b, a};}
inline Color4 color4(Color3 v, float a) {return {v.x, v.y, v.z, a};}
inline Color4 color4(float i) {return {i, i, i, i};}
inline Color4 color4(Color3B b, u8 a) {return {(float)b.r/255.0f, (float)b.g/255.0f, (float)b.b/255.0f, (float)a/255.0f};}
inline Color4 color4(Color4B b) {return {(float)b.r/255.0f, (float)b.g/255.0f, (float)b.b/255.0f, (float)b.a/255.0f};}
inline V4 operator+(V4 a, V4 b) {return v4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);}
inline V4 operator-(V4 a, V4 b) {return v4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);}
inline V4 operator*(V4 a, float b) {return v4(a.x*b, a.y*b, a.z*b, a.w*b);}
inline V4 operator*(float a, V4 b) {return v4(b.x*a, b.y*a, b.z*a, b.w*a);}
inline V4 operator*(V4 a, V4 b) {return v4(a.x*b.x, a.y*b.y, a.z*b.z, a.w*b.w);}
inline V4 operator/(V4 a, float b) {return v4(a.x/b, a.y/b, a.z/b, a.w/b);}
inline V4 operator-(V4 a) {return v4(-a.x, -a.y, -a.z, -a.w);}
inline V4 &operator*=(V4 &a, float b) {return (a = a*b);}
inline V4 &operator/=(V4 &a, float b) {return (a = a/b);}
inline V4 &operator+=(V4 &a, V4 b) {return (a = a + b);}
inline V4 &operator-=(V4 &a, V4 b) {return (a = a - b);}
inline float mag(V4 v) {return sqrt(v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w);}
inline V4 normalize(V4 v) {
  float m = mag(v);
  if (m == 0.0f) return v4(0.0f, 0.0f, 0.0f, 0.0f);
  return v / m;
}

inline Color3B color3b(u8 x, u8 y, u8 z) {return {x, y, z};}
inline Color3B color3b(u8 i) { return { i, i, i }; };
inline Color3B color3b(Color3 c) {return {(u8)(c.r*255.0f), (u8)(c.g*255.0f), (u8)(c.b*255.0f)};}
inline Color3B operator+(Color3B a, Color3B b) {return color3b(a.r + b.r, a.g + b.g, a.b + b.b);}
inline Color3B operator-(Color3B a, Color3B b) {return color3b(a.r - b.r, a.g - b.g, a.b - b.b);}
inline Color3B operator*(Color3B a, u8 b) {return color3b(a.r*b, a.g*b, a.b*b);}
inline Color3B operator*(u8 a, Color3B b) {return color3b(b.r*a, b.g*a, b.b*a);}
inline Color3B operator*(Color3B a, Color3B b) {return color3b(a.r*b.r, a.g*b.g, a.b*b.g);}
inline Color3B operator/(Color3B a, u8 b) {return color3b(a.r/b, a.g/b, a.b/b);}
inline Color3B operator-(Color3B a) {return color3b(-a.r, -a.g, -a.b);}
inline Color3B &operator*=(Color3B &a, u8 b) {return (a = a*b);}
inline Color3B &operator/=(Color3B &a, u8 b) {return (a = a/b);}
inline Color3B &operator+=(Color3B &a, Color3B b) {return (a = a + b);}
inline Color3B &operator-=(Color3B &a, Color3B b) {return (a = a - b);}
inline bool operator==(Color3B a, Color3B b) {return a.r == b.r && a.g == b.g && a.b == b.g;}
inline bool operator!=(Color3B a, Color3B b) {return !(a == b);}

inline Color4B color4b(u8 x, u8 y, u8 z, u8 w) {return {x, y, z, w};}
inline Color4B color4b(Color3B v, u8 w) {return {v.r, v.g, v.b, w};}
inline Color4B color4b(u8 i) {return {i, i, i, i};}
inline Color4B color4b(Color3 c, float a) {return {(u8)(c.r*255.0f), (u8)(c.g*255.0f), (u8)(c.b*255.0f), (u8)(a*255.0f)};}
inline Color4B color4b(Color4 c) {return {(u8)(c.r*255.0f), (u8)(c.g*255.0f), (u8)(c.b*255.0f), (u8)(c.a*255.0f)};}
inline Color4B operator+(Color4B a, Color4B b) {return color4b(a.r + b.r, a.g + b.g, a.b + b.b, a.a + b.a);}
inline Color4B operator-(Color4B a, Color4B b) {return color4b(a.r - b.r, a.g - b.g, a.b - b.b, a.a - b.a);}
inline Color4B operator*(Color4B a, u8 b) {return color4b(a.r*b, a.g*b, a.b*b, a.a*b);}
inline Color4B operator*(u8 a, Color4B b) {return color4b(b.r*a, b.g*a, b.b*a, b.a*a);}
inline Color4B operator*(Color4B a, Color4B b) {return color4b(a.r*b.r, a.g*b.g, a.b*b.b, a.a*b.a);}
inline Color4B operator/(Color4B a, u8 b) {return color4b(a.r/b, a.g/b, a.b/b, a.a/b);}
inline Color4B operator-(Color4B a) {return color4b(-a.r, -a.g, -a.b, -a.a);}
inline Color4B &operator*=(Color4B &a, u8 b) {return (a = a*b);}
inline Color4B &operator/=(Color4B &a, u8 b) {return (a = a/b);}
inline Color4B &operator+=(Color4B &a, Color4B b) {return (a = a + b);}
inline Color4B &operator-=(Color4B &a, Color4B b) {return (a = a - b);}

inline Rect make_rect(float x, float y, float w, float h) {return {x, y, w, h};}

inline V2 rotate_point(V2 point, V2 center, float sin_theta, float cos_theta) {
  return v2((point.x - center.x)*cos_theta - (point.y - center.y)*sin_theta + center.x,
	    (point.x - center.x)*sin_theta + (point.y - center.y)*cos_theta + center.y);
}
inline V2 rotate_point(V2 point, V2 center, float theta) {
  return rotate_point(point, center, sinf(theta), cosf(theta));
}
inline bool point_in_rect(V2 point, Rect rect) {
  return (point.x > rect.x && point.x < (rect.x + rect.w) &&
	  point.y > rect.y && point.y < (rect.y + rect.h));
}
inline bool point_in_rect(V2 point, Rect rect, float theta) {
  if (theta == 0.0f) return point_in_rect(point, rect);
  return point_in_rect(rotate_point(point, v2(rect.x + rect.w/2.0f, rect.y + rect.h/2.0f),
				    -theta), rect);
}

// This is a little random but I can't think of a better place for it:
bool is_power_of_2(int n) {
  while ((n % 2) == 0) n /= 2;
  if (n > 1) return false;
  return true;
}
