cmake -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON -DBUILD_EXAMPLES=ON
cmake --build build/
ctest --test-dir build/tests --output-on-failure 
