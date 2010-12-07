#ifndef C_EXT_H
#define C_EXT_H

#ifdef USE_C_EXT
/* Mark printf wrappers for compile-time format string checks */
# define ATTR_PRINTF(str,fmt) __attribute__ ((format (printf, str, fmt)))
#else
# define ATTR_PRINTF(str,fmt)
#endif

#endif

