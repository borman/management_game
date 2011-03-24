#ifndef NAMEGENERATOR_H
#define NAMEGENERATOR_H

#include "StdLib.h"

class NameGenerator
{
  public:
    virtual void reset() = 0;
    virtual std::string nextName() = 0;
};

#endif // NAMEGENERATOR_H
