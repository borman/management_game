#ifndef COMMAND_H
#define COMMAND_H

#include <cstddef>

/** Command class
 *
 * Stores a game protocol line broken into words
 */

class Command
{
  public:
    enum Type
    {
      Regular,
      TextMessage,
      GameData,
      StateChange
    };

    Command(const char *str);
    ~Command();

    size_t size() const { return n_words; }
    Type type() const { return cmd_type; }

    const char *operator[](size_t index) const;

  private:
    Command(const Command &) {} // Disabled copy
    void doParse(const char *str);
    void fixType();

    char *data;
    size_t n_words;
    char **words;

    Type cmd_type;
};

class OutCommand
{
  public:
    OutCommand(const char *str1, const char *str2 = NULL, const char *str3 = NULL, 
               const char *str4 = NULL, const char *str5 = NULL);
    ~OutCommand();

    const char *c_str() const { return str; }
  private:
    char *str;
};

#endif // COMMAND_H
