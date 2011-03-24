#ifndef TERM_H
#define TERM_H

#include "StdLib.h"

namespace Term
{
  // Ansi SGR (set graphic redition) code
  class SGR
  {
    public:
      SGR(int code)
        : m_code(code) {}
      int code() const { return m_code; }
    private:
      int m_code;
  };

  template<int start, int end>
  class SGRBlock
  {
    public:
      SGRBlock(const std::string &text) : m_text(text) {}
      const std::string &text() const { return m_text; }

    private:
      const std::string &m_text;
  };

  // Whether to really use colored output
  extern bool allowColor;

  inline std::ostream &operator <<(std::ostream &os, const SGR &sgr)
  {
    if (allowColor)
      return os << "\x1b[" << sgr.code() << 'm';
    else
      return os;
  }

  template<int start, int end>
  inline std::ostream &operator <<(std::ostream &os, const SGRBlock<start, end> &colored)
  {
    return os << SGR(start) << colored.text() << SGR(end);
  }

  typedef SGRBlock<30, 39> Black;
  typedef SGRBlock<31, 39> Red;
  typedef SGRBlock<32, 39> Green;
  typedef SGRBlock<33, 39> Brown;
  typedef SGRBlock<34, 39> Blue;
  typedef SGRBlock<35, 39> Magenta;
  typedef SGRBlock<36, 39> Cyan;
  typedef SGRBlock<37, 39> White;

  typedef SGRBlock<1, 22> Bold; 

  const SGR SetBlack  (30);
  const SGR SetRed    (31);
  const SGR SetGreen  (32);
  const SGR SetBrown  (33);
  const SGR SetBlue   (34);
  const SGR SetMagenta(35);
  const SGR SetCyan   (36);
  const SGR SetWhite  (37);
  const SGR ResetColor(39);

  const SGR SetBold   ( 1);
  const SGR SetFaint  ( 2);
  const SGR SetRegular(22);
}

#endif // TERM_H
