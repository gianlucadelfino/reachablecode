mkdir -p build &&
cd build &&
cmake ../ &&
make cUDPreceiver &&
ASAN_OPTIONS=detect_leaks=1 LSAN_OPTIONS=suppressions=sanitizer_blacklist.txt ./cUDPreceiver
