#ifndef STANZA_H
#define STANZA_H

#include <cstddef>
#include <string>
#include <vector>

/** Stanza class
 *
 * Stores a game protocol line broken into words
 */

class Stanza
{
  public:
    Stanza() {}
    Stanza(const std::string &str1,
           const std::string &str2 = std::string(),
           const std::string &str3 = std::string(),
           const std::string &str4 = std::string(),
           const std::string &str5 = std::string());

    size_t size() const { return words.size(); }
    std::string operator[](size_t index) const { return words[index]; } 

    std::string toString() const;

    static Stanza parse(const std::string &str);

  private:
    std::vector<std::string> words;
};

#endif // STANZA_H
