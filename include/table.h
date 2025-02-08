#ifndef TABLE_H
#define TABLE_H

#include "str.h"
#include "val.h"

typedef struct {
  Val key;

  Val val;
} Entry;

typedef struct {
  uint32_t cap;
  uint32_t next; 

  Entry *entries;
} Table;

void initTable(Table *table, uint32_t cap);

bool tableSet(Table *table, Val key, Val val);
Val tableGet(Table *table, Val key);
bool tableDel(Table *table, Val key);

ObjStr *tableFindStr(
  Table *table,
  const char *chars,
  uint32_t length,
  uint32_t hash
);

ObjUStr *tableFindUStr(
  Table *table,
  const void *chars,
  Encoding encoding,
  uint32_t length,
  uint32_t byteLength,
  uint32_t hash
);

void printTable(Table *table);

uint32_t tableStrLength(Table *table);
uint32_t tableAsStr(const char *buffer, uint32_t size, Table *table);

void freeTable(Table *table);

#endif
