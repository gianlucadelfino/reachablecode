mkdir build
cd build
cmake ../
make
ASAN_OPTIONS=detect_leaks=1 LSAN_OPTIONS=suppressions=sanitizer_blacklist.txt ./cUDP
