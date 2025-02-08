#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define IGNORE(x) (void)(x)

#define EOF_PADDING_SIZE 16
// i’m sorry
#define EOF_PADDING                                           \
  "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"                          \
  "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
#define EOF_PADDING_BYTE 0xFF

#define NEVE_MAGIC_NUMBER 0xbadbed00
#define NEVE_CONST_HEADER_SEPARATOR 0x1c 

#define MAX_INTERNED_STR_SIZE 128

#define DEBUG_EXEC
#define DEBUG_COMPILE

#endif
