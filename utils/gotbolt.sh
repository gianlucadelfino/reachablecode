g++ -Wall -Werror -std=c++2a -O2 -S -c ${1} -o /dev/stdout | c++filt | sed -e '/\s\.[a-z]/d'
