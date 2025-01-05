```
[Magic Number] 
(
  [ValType (1 byte)]
  (
    [Number value (double) (8 bytes)] |
    (
      [ObjType (1 byte)]
      (
        [String length (4 bytes)] 
        [String contents (variable size)]
      )
    )
  )
)*
[Header Separator (1 byte)]
[Debug Header Length (2 bytes)]
[Absolute Source File Path Length (2 bytes)]
[Absolute Source File Path (variable size)]
(
  [Instruction Offset (4 bytes)]
  [Line Number (4 bytes)]
)+
[Header Separator (1 byte)]
[Opcodes]
[EOF Padding (16 bytes)]
```
