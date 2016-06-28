#pragma once

#ifdef min
#undef min
#endif
#define min(a, b) \
  ({ __typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b; })

#ifdef max
#undef max
#endif
#define max(a, b) \
  ({ __typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b; })

// Turn macro into a string
#define STR_DETAIL(x) #x
#define STR(x) STR_DETAIL(x)

// sprint
// Get Serial.printf like ability on all Arduino platforms
// Takes 1-8 paramters
#define sprint1(s1) \
  { Serial.print(s1); }

#define sprint2(s1, s2) \
  {                     \
    sprint1(s1);        \
    Serial.print(s2);   \
  }

#define sprint3(s1, s2, s3) \
  {                         \
    sprint2(s1, s2);        \
    Serial.print(s3);       \
  }

#define sprint4(s1, s2, s3, s4) \
  {                             \
    sprint3(s1, s2, s3);        \
    Serial.print(s4);           \
  }

#define sprint5(s1, s2, s3, s4, s5) \
  {                                 \
    sprint4(s1, s2, s3, s4);        \
    Serial.print(s5);               \
  }

#define sprint6(s1, s2, s3, s4, s5, s6) \
  {                                     \
    sprint5(s1, s2, s3, s4, s5);        \
    Serial.print(s6);                   \
  }

#define sprint7(s1, s2, s3, s4, s5, s6, s7) \
  {                                         \
    sprint6(s1, s2, s3, s4, s5, s6);        \
    Serial.print(s7);                       \
  }

#define sprint8(s1, s2, s3, s4, s5, s6, s7, s8) \
  {                                             \
    sprint7(s1, s2, s3, s4, s5, s6, s7);        \
    Serial.print(s8);                           \
  }

#define GET_MACRO(_1, _2, _3, _4, _5, _6, _7, _8, NAME, ...) NAME
#define sprint(...)                                                                              \
  GET_MACRO(__VA_ARGS__, sprint8, sprint7, sprint6, sprint5, sprint4, sprint3, sprint2, sprint1) \
  (__VA_ARGS__)

#define warn_sprint(...)                    \
  do {                                      \
    if (OPC_SERV_WARN) sprint(__VA_ARGS__); \
  } while (0)

#define info_sprint(...)                    \
  do {                                      \
    if (OPC_SERV_INFO) sprint(__VA_ARGS__); \
  } while (0)

#define perf_sprint(...)                    \
  do {                                      \
    if (OPC_SERV_PERF) sprint(__VA_ARGS__); \
  } while (0)

#define debug_sprint(...)                                                          \
  do {                                                                             \
    if (OPC_SERV_DEBUG) {                                                          \
      sprint((strrchr(__FILE__, '/') + 1), ":" STR(__LINE__) ":", __func__, "()"); \
      sprint(__VA_ARGS__);                                                         \
    }                                                                              \
  } while (0)
