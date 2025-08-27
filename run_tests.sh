ncat -lk -p 8080 --sh-exec "cat tests/hello.html" &
NCAT_PID=$!
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON -DBUILD_EXAMPLES=ON
cmake --build build/
ctest --test-dir build/tests --output-on-failure 
#./build/tests/curl_tests
kill -9 $NCAT_PID
