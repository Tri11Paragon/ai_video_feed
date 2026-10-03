mkdir -p build
cmake -S . -B ./build -DCMAKE_TOOLCHAIN_FILE=./cmake/crosscompile.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build ./build -j 32
