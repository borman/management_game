#ifndef COMMAND_H
#define COMMAND_H

/** Command class
 *
 * Stores a game protocol line broken into words
 */

class Command
{
  public:
    Command(const char *str);
    ~Command();

    size_t size() const { return n_words; }
    const char *operator[](size_t index) const;

  private:
    Command(const Command &cmd) {}; // Disabled copy
    void doParse(const char *str);

    char *data;
    size_t n_words;
    char **words;
};

#endif // COMMAND_H
