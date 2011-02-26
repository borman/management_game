#ifndef STANZA_H
#define STANZA_H

#include <cstddef>

/** Stanza class
 *
 * Stores a game protocol line broken into words
 */

class Stanza
{
  public:
    enum Type
    {
      Regular,
      TextMessage,
      GameData,
      StateChange
    };

    Stanza(const char *str);
    ~Stanza();

    size_t size() const { return n_words; }
    Type type() const { return stanza_type; }

    const char *operator[](size_t index) const;
    bool match(const char *str) const;

  private:
    Stanza(const Stanza &) {} // Disabled copy
    void doParse(const char *str);
    void fixType();

    char *data;
    size_t n_words;
    char **words;

    Type stanza_type;
};

class MakeStanza
{
  public:
    MakeStanza(const char *str1, 
               const char *str2 = NULL, 
               const char *str3 = NULL, 
               const char *str4 = NULL, 
               const char *str5 = NULL);
    ~MakeStanza();

    const char *c_str() const { return str; }
  private:
    char *str;
};

#endif // STANZA_H
