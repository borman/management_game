#include <cstdio>
#include "GenericNameGenerator.h"

void GenericNameGenerator::reset() 
{ 
  number = 0; 
} 

std::string GenericNameGenerator::nextName()
{
  char name[32];
  number++;
  sprintf(name, "Player%u", number);
  return name;
}
