#ifndef GAMEINFO_H
#define GAMEINFO_H

#include "Stanza.h"

struct GameInfo
{
  typedef unsigned int price;
  typedef unsigned int count;

  count marketRawCount;
  price marketRawPrice;
  count marketProductCount;
  price marketProductPrice;

  // Update with new data from stanza
  void consume(const Stanza &stanza);
};

#endif // GAMEINFO_H

