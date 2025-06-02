#ifndef _DEBUG_H
#define _DEBUG_H

#include <stdio.h>

#define TRACE(x) printf("%s : %d", #x, x)
#define HEXTRACE(x) printf("%s : %x", #x, x)
#define STRTRACE(x) printf("%s : %s", #x, x)
#define STRINFO(x) printf("%s[%d] : %s", #x, x, strlen(x)+1)
#define DEBUGPRINT(msg) printf("[DEBUG] : %s [END]\n", msg);

#else

#define TRACE(x)
#define HEXTRACE(x)
#define STRTRACE(x)
#define STRINFO(x)
#define DEBUGPRINT(msg)

#endif