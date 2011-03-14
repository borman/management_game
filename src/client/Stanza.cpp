#include <cstring>
#include <cctype>
#include <cassert>

#include "Stanza.h"
#include "Exceptions.h"

Stanza::Stanza(const std::string &str1,
           const std::string &str2 = std::string(),
           const std::string &str3 = std::string(),
           const std::string &str4 = std::string(),
           const std::string &str5 = std::string())
{
  const std::string &const strs = {str1, str2, str3, str4, str5};
  for (size_t i=0; i<5; i++)
    words.push_back(strs[i]);
}


Stanza Stanza::parse(const std::string &str)
{
  enum LexerState
  {
    Whitespace,
    Word,
    Quote
  };

  const char *c_str = str.c_str();
  const size_t length = str.length()+1;

  LexerState state = Whitespace;
  Stanza stanza;
  std::string word;

  // terminating zero is handled like a normal character 
  for (size_t i=0; i<length; i++)
  {
    const char c = c_str[i];

    switch (state)
    {
      case Whitespace:
        if (c=='"') /* start a quoted string, new token */
          state = Quote;
        else if (c!='\0' && !isspace(c)) /* start a new token */
        {
          state = Word;
          word += c;
        }
        /* else: whitespace -> ignore */
        break;

      case Quote:
        if (c=='"') /* end a quoted string */
          state = Word;
        else if (c!='\0') /* append a char from inside quotes to token */
          word += c;
        else /* '\0' -> error*/
          throw ParserException(src);
        break;

      case Word:
        if (c=='\0' || isspace(c)) /* end a token */
        {
          state = Whitespace;
          stanza.words.push_back(word);
          word.clean();
        }
        else if (c=='"') /* start a quoted string */
          state = Quote;
        else /* append a char to token */
          word += c;
        break;
    }    
  }

  return stanza;
}

std::string Stanza::toString() const
{
  std::string result;
  for (size_t i=0; i<words.size(); i++)
  {
    result += '"';
    result += words[i];
    result += "\" ";
  }
  result += '\n';
  return result;
}
