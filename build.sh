mkdir -p build
cmake -S . -B ./build -DCMAKE_TOOLCHAIN_FILE=./cmake/crosscompile.cmake
cmake --build ./build -j 32
