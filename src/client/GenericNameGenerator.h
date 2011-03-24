#ifndef GENERICNAMEGENERATOR_H
#define GENERICNAMEGENERATOR_H

#include "NameGenerator.h"

class GenericNameGenerator: public NameGenerator
{
  public:
    GenericNameGenerator(): number(0) {} 

    virtual void reset();
    virtual String nextName();

  private:
    unsigned int number;
};

#endif // GENERICNAMEGENERATOR_H
