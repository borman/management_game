#ifndef STDLIB_H
#define STDLIB_H

#ifdef USE_STL

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <map>
#include <queue>
#include <string>
#include <vector>

typedef std::string String;
typedef std::ostream OutputStream;
typedef std::istream InputStream;

template<class T>
class Vector: public std::vector<T>
{
};

template<class T>
class Queue: public std::queue<T>
{
};

template<class T>
class StringMap: public std::map<String, T>
{
};

using namespace std;

#else

#include "ministl/String.h"
#include "ministl/Queue.h"
#include "ministl/Vector.h"
#include "ministl/StringMap.h"

#endif

#endif // STDLIB_H
