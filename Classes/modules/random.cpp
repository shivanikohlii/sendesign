#ifndef CSP_RANDOM_CPP

#define CSP_RANDOM_LCG_MODULUS 2147483647 // 2^31 - 1

unsigned int __csp_random_lcg_seed = 0xFEEDABEE;
  
inline unsigned int lcg(unsigned int m, unsigned int a, unsigned int c) {
  __csp_random_lcg_seed = (a*__csp_random_lcg_seed + c) % m;
  return __csp_random_lcg_seed;
}

inline unsigned int rand_uint() {
  return lcg(CSP_RANDOM_LCG_MODULUS, 48271, 0);
}
inline float rand_float() {
  return (float)rand_uint() / (float)CSP_RANDOM_LCG_MODULUS;
}
inline float rand_range(float min, float max) {
  return min + (rand_float() * (max - min));
}
inline int rand_range(int min, int max) {
  return (int)rand_range((float)min, (float)max + 0.999999f); 
}
inline void rand_set_seed(unsigned int seed) {
  __csp_random_lcg_seed = seed;
}

#define CSP_RANDOM_CPP
#endif
