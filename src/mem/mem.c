#include <stdlib.h>

#include "mem.h"
#include "obj.h"

void *reallocate(void *ptr, size_t oldSize, size_t newSize) {
  IGNORE(oldSize);

  if (newSize == 0) {
    free(ptr);
    return NULL;
  }

  void *allocated = realloc(ptr, newSize);
  if (allocated == NULL) {
    exit(1);
  }

  return allocated;
}

void freeObjs(Obj *objs) {
  Obj *obj = objs;

  while (obj != NULL) {
    Obj *next = obj->next;

    freeObj(obj);    
    obj = next;
  }
}
