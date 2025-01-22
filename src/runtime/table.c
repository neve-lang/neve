#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mem.h"
#include "obj.h"
#include "table.h"
#include "val.h"

#define TABLE_MAX_LOAD 0.75

static Entry *findEntry(Entry *entries, uint32_t cap, Val key) {
  uint32_t index = hashVal(key) & (cap - 1);
  Entry *tombstone = NULL;

  while (true) {
    Entry *entry = &entries[index];

    if (IS_VAL_EMPTY(entry->key)) {
      if (IS_VAL_NIL(entry->val)) {
        return tombstone != NULL ? tombstone : entry;
      } 

      if (tombstone == NULL) {
        tombstone = entry;
      }
    } else if (valsEq(entry->key, key)) {
      return entry;
    }

    index = (index + 1) & (cap - 1);
  }
}

static Entry *allocEntries(uint32_t cap) {
  Entry *entries = ALLOC(Entry, cap);

  for (uint32_t i = 0; i < cap; i++) {
    entries[i].key = EMPTY_VAL;
    entries[i].val = NIL_VAL;
  }

  return entries;
}

static void adjustCap(Table *table, uint32_t cap) {
  Entry *entries = allocEntries(cap);

  table->next = 0;
  for (uint32_t i = 0; i < table->cap; i++) {
    Entry *entry = &table->entries[i];

    if (IS_VAL_EMPTY(entry->key)) {
      continue;
    }

    Entry *dest = findEntry(entries, cap, entry->key);
    dest->key = entry->key;
    dest->val = entry->val;

    table->next++;
  }

  FREE_ARR(Entry, table->entries, table->cap);

  table->entries = entries;
  table->cap = cap;
}

static void printEntryVal(Val val) {
  const bool isStr = (
    IS_VAL_OBJ(val) && 
    VAL_AS_OBJ(val)->type == OBJ_STR
  );

  if (isStr) {
    putchar('"');
  }

  printVal(val);

  if (isStr) {
    putchar('"');
  }
}

void initTable(Table *table, const uint32_t cap) {
  if (cap > 0) {
    const uint32_t newCap = cap < 8 ? 8 : cap;

    table->cap = newCap;
    table->next = 0;
    table->entries = allocEntries(newCap);

    return;
  }

  table->cap = 0;
  table->next = 0;
  table->entries = NULL;
}

bool tableSet(Table *table, Val key, Val val) {
  if ((double)table->next >= (double)table->cap * TABLE_MAX_LOAD) {
    uint32_t cap = GROW_CAP(table->cap);
    adjustCap(table, cap);
  }

  Entry *entry = findEntry(table->entries, table->cap, key);

  bool isNewKey = IS_VAL_EMPTY(entry->key);

  table->next += isNewKey && IS_VAL_NIL(entry->val);

  entry->key = key;
  entry->val = val;
  
  return isNewKey;
}

Val tableGet(Table *table, Val key) {
  if (table->next == 0) {
    return NIL_VAL;
  }

  Entry *entry = findEntry(table->entries, table->cap, key);
  if (IS_VAL_EMPTY(entry->key)) {
    return NIL_VAL;
  }

  return entry->val;
}

bool tableDel(Table *table, Val key) {
  if (table->next == 0) {
    return false;
  }

  Entry *entry = findEntry(table->entries, table->cap, key);
  if (IS_VAL_EMPTY(entry->key)) {
    return false;
  }

  entry->key = EMPTY_VAL;
  entry->val = BOOL_VAL(true);

  return true;
}

ObjStr *tableFindStr(
  Table *table,
  const char *chars,
  uint32_t length,
  uint32_t hash
) {
  if (table->next == 0) {
    return NULL;
  }

  uint32_t index = hash & (table->cap - 1);

  while (true) {
    Entry *entry = &table->entries[index];

    if (IS_VAL_EMPTY(entry->key)) {
      return NULL;
    }

    ObjStr *str = VAL_AS_STR(entry->key);

    if (str->length == length && memcmp(str->chars, chars, length) == 0) {
      return str;
    }

    index = (index + 1) & (table->cap - 1);
  }
}

void printTable(Table *table) {
  if (table->next == 0) {
    printf("[:]");
    return;
  }

  printf("[");

  bool isFirst = true;

  for (uint32_t i = 0; i < table->cap; i++) {
    Entry *entry = &table->entries[i];

    const bool isAtEnd = i == table->cap - 1;

    if (IS_VAL_EMPTY(entry->key)) {
      if (isAtEnd) {
        printf("]");
      }

      continue;
    }

    if (!isFirst) {
      printf(", ");
    }

    printEntryVal(entry->key);
    printf(": ");
    printEntryVal(entry->val);

    if (isAtEnd) {
      printf("]");
    }

    isFirst = false;
  }
}

uint32_t tableStrLength(Table *table) {
  if (table->next == 0) {
    // [:]
    return 3;
  }

  const uint32_t sepLength = 2;

  // 1 => initial '['
  uint32_t length = 1;

  bool isFirst = true;

  for (uint32_t i = 0; i < table->cap; i++) {
    Entry *entry = &table->entries[i];

    const bool isAtEnd = i == table->cap - 1;

    if (IS_VAL_EMPTY(entry->key)) {
      length += isAtEnd;
      continue;
    }

    if (!isFirst) {
      length += sepLength;
    }

    length += valStrLength(entry->key);
    length += sepLength;
    length += valStrLength(entry->val);

    length += isAtEnd;

    isFirst = false;
  }

  return length;
}

uint32_t tableAsStr(const char *buffer, const uint32_t size, Table *table) {
  if (table->next == 0) {
    const uint32_t length = 3;

    strncpy((char *)buffer, "[:]", length);
    return length;
  }

  const uint32_t newSize = size + 1;

  uint32_t pos = (uint32_t)snprintf((char *)buffer, newSize, "[");

  bool isFirst = true;

  for (uint32_t i = 0; i < table->cap; i++) {
    Entry *entry = &table->entries[i];

    const bool isAtEnd = i == table->cap - 1;

    if (IS_VAL_EMPTY(entry->key)) {
      if (isAtEnd) {
        pos += (uint32_t)snprintf((char *)&buffer[pos], newSize - pos, "]");
      }
      
      continue;
    }

    if (!isFirst) {
      pos += (uint32_t)snprintf((char *)&buffer[pos], newSize - pos, ", ");
    }

    const uint32_t keySize = valStrLength(entry->key);
    const uint32_t valSize = valStrLength(entry->val);

    pos += valAsStr((char *)&buffer[pos], keySize, entry->key);
    pos += (uint32_t)snprintf((char *)&buffer[pos], newSize - pos, ": ");
    pos += valAsStr((char *)&buffer[pos], valSize, entry->val);

    if (isAtEnd) {
      strncpy((char *)&buffer[pos], "]", newSize - pos);
      pos++;
    }

    isFirst = false;
  }

  return size;
}

void freeTable(Table *table) {
  FREE_ARR(Entry, table->entries, table->cap);  

  table->cap = 0;
  table->next = 0;
  table->entries = NULL;
}
