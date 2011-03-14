#ifndef NAMEGENERATOR_H
#define NAMEGENERATOR_H

#include <string>

class NameGenerator
{
  virtual void reset() = 0;
  virtual std::string nextName() = 0;
};

#endif // NAMEGENERATOR_H
