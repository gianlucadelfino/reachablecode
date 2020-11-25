mkdir -p build &&
cd build &&
cmake ../ &&
make cUDPsender  &&
ASAN_OPTIONS=detect_leaks=1 LSAN_OPTIONS=suppressions=sanitizer_blacklist.txt ./cUDPsender
