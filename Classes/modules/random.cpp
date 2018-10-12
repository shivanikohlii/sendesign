#define LCG_MODULUS 2147483647 // 2^31 - 1

unsigned int lcg_seed = 0xFEEDABEE;
  
inline unsigned int lcg(unsigned int m, unsigned int a, unsigned int c) {
  lcg_seed = (a*lcg_seed + c) % m;
  return lcg_seed;
}

inline unsigned int rand_uint() {
  return lcg(LCG_MODULUS, 48271, 0);
}
inline float rand_float() {
  return (float)rand_uint() / (float)LCG_MODULUS;
}
inline float rand_range(float min, float max) {
  return min + (rand_float() * (max - min));
}
inline int rand_range(int min, int max) {
  return (int)rand_range((float)min, (float)max + 0.999999f); 
}
inline void rand_set_seed(unsigned int seed) {
  lcg_seed = seed;
}
