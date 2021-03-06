#ifndef STDLIB_H
#define STDLIB_H

#ifdef USE_STL

#include <algorithm>
#include <map>
#include <queue>
#include <string>
#include <vector>

typedef std::string String;
typedef std::map<String, size_t> StringMap;

template<class T>
class Vector: public std::vector<T>
{
};

template<class T>
class Queue: public std::queue<T>
{
};

using namespace std;

#else

#include "String.h"
#include "Queue.h"
#include "Vector.h"
#include "Map.h"
#include "Util.h"

#endif

#endif // STDLIB_H
