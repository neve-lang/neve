#include <stdio.h>
#include <string.h>

#include "mem.h"
#include "obj.h"
#include "table.h"

#define ALLOC_OBJ(vm, type, objType)                        \
  (type *)allocObj(vm, sizeof (type), objType)

#define INITIAL_STR_HASH_VAL 2166136261U
#define HASH_FACTOR 16777619

static Obj *allocObj(NeveVM *vm, size_t size, ObjType type) {
  Obj *obj = (Obj *)reallocate(NULL, 0, size);
  obj->type = type;

  obj->next = vm->objs;
  vm->objs = obj;

  return obj;
}

ObjStr *allocStr(
  NeveVM *vm,
  bool ownsStr,
  const char *chars,
  uint32_t length,
  uint32_t hash
) {
  ObjStr *interned = tableFindStr(&vm->strs, chars, length, hash);
  if (interned != NULL) {
    if (ownsStr) {
      FREE_ARR(char, (char *)chars, length);
    }

    return interned;
  }

  ObjStr *str = ALLOC_OBJ(vm, ObjStr, OBJ_STR);
  str->ownsStr = ownsStr;
  str->length = length;
  str->chars = chars;
  str->hash = hash;

  tableSet(&vm->strs, OBJ_VAL(str), NIL_VAL);

  return str;
}

ObjTable *newTable(NeveVM *vm) {
  ObjTable *obj = ALLOC_OBJ(vm, ObjTable, OBJ_TABLE);  

  obj->table = ALLOC(Table ,1);
  initTable(obj->table);

  return obj;
}

uint32_t hashStr(const char *key, uint32_t length) {
  uint32_t hash = INITIAL_STR_HASH_VAL; 

  for (uint32_t i = 0; i < length; i++) {
    hash ^= (uint8_t)key[i];
    hash *= HASH_FACTOR;
  }

  return hash;
}

void printObj(Val val) {
  switch (OBJ_TYPE(val)) {
    case OBJ_STR:
      printf("%.*s", (int)(VAL_AS_STR(val)->length), VAL_AS_CSTR(val));
      break;

    case OBJ_TABLE:
      printTable(VAL_AS_TABLE(val)->table); 
  }
}

void freeObj(Obj *obj) {
  switch (obj->type) {
    case OBJ_STR: {
      ObjStr *str = (ObjStr *)obj;

      if (str->ownsStr) {
        FREE_ARR(char, (char *)str->chars, str->length);
      }

      FREE(ObjStr, obj);
      break;
    }

    case OBJ_TABLE: {
      ObjTable *table = (ObjTable *)obj;

      freeTable(table->table);
      FREE(Table, table->table);
      
      FREE(ObjTable, table);
      break;
    }
  }
}

uint32_t objStrLength(Obj *obj) {
  switch (obj->type) {
    case OBJ_STR:
      return ((ObjStr *)obj)->length;

    case OBJ_TABLE:
      return tableStrLength(((ObjTable *)obj)->table);
  }

  return 0;
}

uint32_t objAsStr(const char *buffer, const uint32_t size, Obj *obj) {
  switch (obj->type) {
    case OBJ_TABLE:
      return tableAsStr(buffer, size, ((ObjTable *)obj)->table); 
    
    case OBJ_STR: {
      ObjStr *str = (ObjStr *)obj;

      strncpy((char *)buffer, str->chars, str->length);

      return str->length;
    }
  }

  return 0;
}
