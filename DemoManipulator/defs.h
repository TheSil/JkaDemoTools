#ifndef DEFS_H
#define DEFS_H

//libraries
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <iostream> 
#include <algorithm>
#include <exception>
#include <cassert>

#define DEMO_NAMESPACE_START namespace DemoJKA{
#define DEMO_NAMESPACE_END };

#define     MAX_MSGLEN 49152
#define     NYT HMAX /* NYT = Not Yet Transmitted */
#define     INTERNAL_NODE (HMAX+1)
#define     HMAX 256 /* Maximum symbol */
#define     BIG_INFO_STRING     8192
#define     MAX_STRING_CHARS    1024
#define     FLOAT_INT_BITS      13
#define     FLOAT_INT_BIAS      (1<<(FLOAT_INT_BITS-1))
#define     MAX_CONFIGSTRINGS   1700
#define     GENTITYNUM_BITS     10
#define     MAX_GENTITIES       (1<<GENTITYNUM_BITS)

enum {
    SIZE_FLOAT = 0,
    SIZE_FLOATINT = 13,
    SIZE_1BIT = 1,
    SIZE_8BITS = 8,
    SIZE_16BITS = 16,
    SIZE_19BITS = 19,   //special stuff in ps_stats
    SIZE_32BITS = 32,
    SIZE_ENTITY_BITS = 10,
};

enum
{
    svc_bad,
    svc_nop,
    svc_gamestate,
    svc_configstring,   // [short] [string] only in gamestate messages
    svc_baseline,       // only in gamestate messages
    svc_serverCommand,  // [string] to be executed by client game module
    svc_download,       // [short] size [size bytes]
    svc_snapshot,
    svc_setgame,
    svc_mapchange,
    svc_EOF
};

class DemoException : public std::exception {
public:
    DemoException(const char* s) : std::exception(s) {};
};

typedef unsigned char byte;

#endif