#include "chunk.h"
#include "mem.h"

Chunk newChunk() {
  Chunk ch = {
    .cap = 0,
    .next = 0,
    .code = NULL,
    .consts = newValArr(),
    .lines = newLineArr()
  };

  return ch;
}

void writeChunk(Chunk *ch, uint8_t byte, int line) {
  if (ch->next == ch->cap) {
    uint32_t oldCap = ch->cap;

    ch->cap = GROW_CAP(oldCap);
    ch->code = GROW_ARR(
      uint8_t,
      ch->code,
      oldCap,
      ch->cap
    );
  }

  writeLineArr(&ch->lines, line, ch->next);

  ch->code[ch->next++] = byte;
}

void freeChunk(Chunk *ch) {
  freeValArr(&ch->consts);
  freeLineArr(&ch->lines);

  ch->code = NULL;
  ch->cap = 0;
  ch->next = 0;  
}

void writeConst(Chunk *ch, Val val, int line) {
  const int index = addConst(ch, val);

  const uint8_t byteLength = 8;

  if (index <= UINT8_MAX) {
    writeChunk(ch, OP_PUSH, line); 
    writeChunk(ch, (uint8_t)index, line);
  } else {
    writeChunk(ch, OP_PUSHLONG, line);
    writeChunk(ch, (uint8_t)(index & UINT8_MAX), line);
    writeChunk(ch, (uint8_t)((index >> byteLength) & UINT8_MAX), line);
    writeChunk(ch, (uint8_t)((index >> byteLength * 2) & UINT8_MAX), line);
  }
}

int addConst(Chunk *ch, Val val) {
  writeValArr(&ch->consts, val);
  return (int)(ch->consts.next - 1);
}

LineArr newLineArr() {
  LineArr arr = {
    .cap = 0,
    .next = 0,
    .lines = NULL 
  };

  return arr;
}

void writeLineArr(LineArr *arr, int line, uint32_t offset) {
  if (arr != NULL && arr->next > 0) {
    int lastLine = arr->lines[arr->next - 1].line;

    if (line == lastLine) {
      return;
    }
  }

  if (arr->next == arr->cap) {
    uint32_t oldCap = arr->cap;

    arr->cap = GROW_CAP(oldCap);
    arr->lines = GROW_ARR(
      Line,
      arr->lines,
      oldCap,
      arr->cap
    );
  }

  Line lineStruct = {
    .offset = offset,
    .line = line
  };

  arr->lines[arr->next++] = lineStruct;
}

void freeLineArr(LineArr *arr) {
  FREE_ARR(Line, arr->lines, arr->cap);

  arr->cap = 0;
  arr->next = 0;
}

int getLine(Chunk *ch, uint32_t offset) {
  LineArr lines = ch->lines;

  if (lines.lines == NULL) {
    return -1;
  }

  uint32_t start = 0;
  uint32_t end = lines.next - 1;

  while (true) {
    const uint32_t mid = (start + end) / 2;
    const uint32_t next = mid + 1;
    Line line = lines.lines[mid];

    if (offset < line.offset) {
      end = mid - 1; 
    } else if (
      mid == lines.next - 1 || offset < lines.lines[next].offset
    ) {
      return line.line;
    } else {
      start = mid + 1;
    }
  }
}
