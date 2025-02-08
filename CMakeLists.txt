cmake_minimum_required(VERSION 3.0.0)
project(neve) # VERSION 0.0.0-20211225

add_executable(neve
  src/main/main.c
  src/compiler/bytecode.c
  src/compiler/compiler.c
  src/compiler/const.c
  src/err/err.c
  src/err/render.c
  src/mem/mem.c
  src/runtime/val.c
  src/runtime/table.c
  src/runtime/obj.c
  src/vm/debug.c
  src/vm/chunk.c
  src/vm/vm.c
)

target_include_directories(neve PRIVATE 
  include/
)

target_compile_options(neve PRIVATE
  -Wall
  -Wextra
  -Wconversion
  -Werror
  -pedantic
  -fopt-info
  -g
)

add_custom_target(
  clang-tidy-check clang-tidy -p ${CMAKE_BINARY_DIR}/compile_commands.json -checks=cert* ${sources}
  DEPENDS ${sources}
)

target_link_libraries(neve
  -lm
)
