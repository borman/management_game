#include <cstring>
#include <cctype>

#include "Command.h"
#include "Exceptions.h"

class SimpleTextStream
{
  public:
    SimpleTextStream(char *data)
      : data(data), p(0) {}

    SimpleTextStream &operator<<(char c)
    {
      data[p++] = c;
      return *this;
    }
  private:
    char *data;
    size_t p;
};

Command::Command(const char *str)
{
  doParse(str);
  fixType();
}

Command::~Command()
{
  delete[] data;
  delete[] words;
}

const char *Command::operator[](size_t index) const
{
  if (index>n_words)
    throw OutOfBoundsException("Command");
  return words[index];
}

void Command::doParse(const char *src)
{
  enum LexerState
  {
    Whitespace,
    Word,
    Quote
  };

  const size_t length = strlen(src);

  try
  {
    data = new char[length+1];
    n_words = 0;
    SimpleTextStream out(data);

    LexerState state = Whitespace;
    // terminating zero is handled like a normal character 
    for (size_t i=0; i<length+1; i++)
    {
      const char c = src[i];

      switch (state)
      {
        case Whitespace:
          if (c=='"') /* start a quoted string, new token */
            state = Quote;
          else if (c!='\0' && !isspace(c)) /* start a new token */
          {
            state = Word;
            out << c;
          }
          /* else: whitespace -> ignore */
          break;

        case Quote:
          if (c=='"') /* end a quoted string */
            state = Word;
          else if (c!='\0') /* append a char from inside quotes to token */
            out << c;
          else /* '\0' -> set error flag */
            throw ParserException(src);
          break;

        case Word:
          if (c=='\0' || isspace(c)) /* end a token */
          {
            state = Whitespace;
            out << '\0';
            n_words++;
          }
          else if (c=='"') /* start a quoted string */
            state = Quote;
          else /* append a char to token */
            out << c;
          break;
      }    
    }
  }
  catch (ParserException pe)
  {
    delete[] data;
    throw;
  }

  words = new char *[n_words];
  size_t p = 0;
  for (size_t i=0; i<n_words; i++)
  {
    words[i] = data+p;
    p += strlen(data+p) + 1;
  }
}

void Command::fixType()
{
  cmd_type = Regular;
  if (n_words>0 && strlen(words[0])==1)
  {
    switch (words[0][0])
    {
      case '>':
        cmd_type = TextMessage;
        break;
      case '$':
        cmd_type = StateChange;
        break;
      case '+':
        cmd_type = GameData;
        break;
      default:
        break;
    }
  }

  if (cmd_type != Regular)
  {
    for (size_t i=0; i<n_words-1; i++)
      words[i] = words[i+1];
    n_words--;
  }
}
