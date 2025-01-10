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

static void adjustCap(Table *table, uint32_t cap) {
  Entry *entries = ALLOC(Entry, cap);

  for (uint32_t i = 0; i < cap; i++) {
    entries[i].key = EMPTY_VAL;
    entries[i].val = NIL_VAL;
  }

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

void initTable(Table *table) {
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

  for (uint32_t i = 0; i < table->next; i++) {
    Entry *entry = &table->entries[i];

    if (IS_VAL_EMPTY(entry->key)) {
      if (i == table->next - 1) {
        printf("]");
      }

      continue;
    }

    printVal(entry->key);
    printf(": ");
    printVal(entry->val);

    printf(i == table->next - 1 ? "]" : ", ");
  }
}

uint32_t tableStrLength(Table *table) {
  const uint32_t sepLength = 2;

  uint32_t length = 0;

  for (uint32_t i = 0; i < table->next; i++) {
    Entry *entry = &table->entries[i];

    if (IS_VAL_EMPTY(entry->key)) {
      length += i == table->next - 1;
      continue;
    }

    length += valStrLength(entry->key);
    length += sepLength;
    length += valStrLength(entry->val);

    length += i == table->next - 1 ? 1 : sepLength;
  }

  return length;
}

uint32_t tableAsStr(const char *buffer, Table *table) {
  const uint32_t initialSize = 32;
  const uint32_t newSize = tableStrLength(table);

  buffer = GROW_ARR(char, (char *)buffer, initialSize, newSize);

  uint32_t pos = 0;

  for (uint32_t i = 0; i < table->next; i++) {
    Entry *entry = &table->entries[i];

    if (IS_VAL_EMPTY(entry->key)) {
      if (i == table->next - 1) {
        pos += (uint32_t)sprintf((char *)&buffer[pos], "]");
      }
      
      continue;
    }

    pos += valAsStr((char *)&buffer[pos], entry->key);
    pos += (uint32_t)sprintf((char *)&buffer[pos], ": ");
    pos += valAsStr((char *)&buffer[pos], entry->val);

    pos += (uint32_t)sprintf(
      (char *)&buffer[pos], i == table->next - 1 ? "]" : ", "
    );
  }

  return newSize;
}

void freeTable(Table *table) {
  FREE_ARR(Entry, table->entries, table->cap);  

  table->cap = 0;
  table->next = 0;
  table->entries = NULL;
}
