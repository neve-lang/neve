clang-tidy src/compiler/*.c -- -Iinclude/
clang-tidy src/err/*.c -- -Iinclude/
clang-tidy src/main/*.c -- -Iinclude/
clang-tidy src/mem/*.c -- -Iinclude/
clang-tidy src/runtime/*.c -- -Iinclude/
clang-tidy src/vm/*.c -- -Iinclude/
clang-tidy include/*.h -- -Iinclude/

cd build
make

cd ..

