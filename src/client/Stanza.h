#ifndef STANZA_H
#define STANZA_H

#include <cstddef>
#include "StdLib.h"

/** Stanza class
 *
 * Stores a game protocol line broken into words
 */

class Stanza
{
  public:
    Stanza() {}
    Stanza(const String &str1,
           const String &str2 = String(),
           const String &str3 = String(),
           const String &str4 = String(),
           const String &str5 = String());

    size_t size() const { return words.size(); }
    String operator[](size_t index) const { return words[index]; } 

    String toString() const;

    static Stanza parse(const String &str);

  private:
    Vector<String> words;
};

#endif // STANZA_H
