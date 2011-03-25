#ifndef UTIL_H
#define UTIL_H

template<class T>
T min(T a, T b)
{
  return a<b? a : b;
}

template<class T>
T max(T a, T b)
{
  return a>b? a : b;
}

#endif // UTIL_H
