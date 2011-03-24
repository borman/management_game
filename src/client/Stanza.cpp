#include <cstring>
#include <cctype>
#include <cassert>

#include "Stanza.h"
#include "Exceptions.h"

Stanza::Stanza(const String &str1,
           const String &str2,
           const String &str3,
           const String &str4,
           const String &str5)
{
  const String *strs[] = {&str1, &str2, &str3, &str4, &str5};
  size_t n_args = 5;
  while (n_args>0 && strs[n_args-1]->empty())
    n_args--;
  for (size_t i=0; i<n_args; i++)
    words.push_back(*strs[i]);
}


Stanza Stanza::parse(const String &str)
{
  enum LexerState
  {
    Whitespace,
    Word,
    Quote
  };

  const char *c_str = str.c_str();

  LexerState state = Whitespace;
  Stanza stanza;
  String word;

  // terminating zero is handled like a normal character 
  for (size_t i=0; i<str.length()+1; i++)
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
          throw ParserException(str);
        break;

      case Word:
        if (c=='\0' || isspace(c)) /* end a token */
        {
          state = Whitespace;
          stanza.words.push_back(word);
          word.clear();
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

String Stanza::toString() const
{
  String result;
  for (size_t i=0; i<words.size(); i++)
  {
    result += '"';
    result += words[i];
    result += "\" ";
  }
  result += '\n';
  return result;
}
