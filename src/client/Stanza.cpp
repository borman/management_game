#include <cstring>
#include <cctype>
#include <cassert>

#include "Stanza.h"
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
    SimpleTextStream &operator<<(const char *str)
    {
      strcpy(data+p, str);
      p += strlen(str);
      return *this;
    }
  private:
    char *data;
    size_t p;
};

Stanza::Stanza(const char *str)
{
  doParse(str);
  fixType();
}

Stanza::~Stanza()
{
  delete[] data;
  delete[] words;
}

const char *Stanza::operator[](size_t index) const
{
  if (index>n_words)
    throw OutOfBoundsException("Stanza");
  return words[index];
}

bool Stanza::match(const char *str) const
{
  return size() > 0 && strcmp(words[0], str) == 0;
}

void Stanza::doParse(const char *src)
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

void Stanza::fixType()
{
  stanza_type = Regular;
  if (n_words>0 && strlen(words[0])==1)
  {
    switch (words[0][0])
    {
      case '>':
        stanza_type = TextMessage;
        break;
      case '$':
        stanza_type = StateChange;
        break;
      case '+':
        stanza_type = GameData;
        break;
      default:
        break;
    }
  }

  // Remove first (indicator) word
  if (stanza_type != Regular)
  {
    for (size_t i=0; i<n_words-1; i++)
      words[i] = words[i+1];
    n_words--;
  }
}


MakeStanza::MakeStanza(const char *str1, const char *str2, 
    const char *str3, const char *str4, const char *str5)
{
  assert(str1 != NULL);

  const char *strs[] = {str1, str2, str3, str4, str5};
  size_t total_length = 1; // '\n'
  for (size_t i=0; i<5; i++)
    if (strs[i] != NULL)
      total_length += strlen(strs[i]) + 3; // + '"" '

  str = new char[total_length+1];
  SimpleTextStream out(str);

  for (size_t i=0; i<5; i++)
    if (strs[i] != NULL)
      out << '"' << strs[i] << '"' << ' ';
  out << '\n' << '\0';
}

MakeStanza::~MakeStanza()
{
  delete[] str;
}
