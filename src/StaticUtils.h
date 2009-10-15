#ifndef _STATIC_UTILS_H_
#define _STATIC_UTILS_H_

class StaticUtils {
  
  public:
    static char *copyString(char *str);
    static bool isNumeric(char *str);
    static float millisecondsToSamples(float delayInMs, float sampleRate);
    static char *joinPaths(const char *path0, const char *path1);
    static float sineApprox(float x);
  
  private:
    StaticUtils(); // a private constructor. No instances of this object should be made.
    ~StaticUtils();
};

#endif // _STATIC_UTILS_H_
