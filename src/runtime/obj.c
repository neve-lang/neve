#include <stdio.h>
#include <string.h>

#include "mem.h"
#include "obj.h"

#define ALLOC_OBJ(vm, type, objType)                        \
  (type *)allocObj(vm, sizeof (type), objType)

static Obj *allocObj(NeveVM *vm, size_t size, ObjType type) {
  Obj *obj = (Obj *)reallocate(NULL, 0, size);
  obj->type = type;

  obj->next = vm->objs;
  vm->objs = obj;

  return obj;
}

ObjStr *allocStr(NeveVM *vm, bool ownsStr, const char *chars, uint32_t length) {
  ObjStr *str = ALLOC_OBJ(vm, ObjStr, OBJ_STR);
  str->ownsStr = ownsStr;
  str->length = length;
  str->chars = chars;

  return str;
}

void printObj(Val val) {
  switch (OBJ_TYPE(val)) {
    case OBJ_STR:
      printf("%.*s", (int)(VAL_AS_STR(val)->length), VAL_AS_CSTR(val));
      break;
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
  }
}

uint32_t objAsStr(const char *buffer, Obj *obj) {
  if (obj->type == OBJ_STR) {
    ObjStr *str = (ObjStr *)obj;

    buffer = str->chars;

    // avoiding “unused-but-set-parameter”
    IGNORE(buffer);
    return str->length;
  }

  /*
  switch (obj->type) {
    ...
  }
  */

  return 0;
}
