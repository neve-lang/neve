#ifndef DEBUG_H
#define DEBUG_H

#include "chunk.h"

void disasmChunk(Chunk *ch, Val *regs, const char *name);
size_t disasmInstr(Chunk *ch, Val *regs, size_t offset);

#endif
