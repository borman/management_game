#ifndef TERM_H
#define TERM_H

#include "StdLib.h"

namespace Term
{
#define GEN_SGR(code) "\x1b[" #code "m"
  const char *const SetBlack   = GEN_SGR(30);
  const char *const SetRed     = GEN_SGR(31);
  const char *const SetGreen   = GEN_SGR(32);
  const char *const SetBrown   = GEN_SGR(33);
  const char *const SetBlue    = GEN_SGR(34);
  const char *const SetMagenta = GEN_SGR(35);
  const char *const SetCyan    = GEN_SGR(36);
  const char *const SetWhite   = GEN_SGR(37);
  const char *const ResetColor = GEN_SGR(39);

  const char *const SetBold    = GEN_SGR( 1);
  const char *const SetFaint   = GEN_SGR( 2);
  const char *const SetRegular = GEN_SGR(22);
#undef GEN_SGR
}

#endif // TERM_H
