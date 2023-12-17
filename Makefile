linux:
	g++ -I ./src/ ./src/build-gh-pages.cpp -o ./bin/build-gh-pages -Wall -Wextra -g

linux-release:
	g++ -I ./src/ ./src/build-gh-pages.cpp -o ./bin/build-gh-pages -Wall -Wextra -O3

linux-mingw:
	i686-w64-mingw32-c++ -I ./src/ ./src/build-gh-pages.cpp -o ./bin/build-gh-pages -Wall -Wextra -g -static -DWIN32

linux-mingw-release:
	i686-w64-mingw32-c++ -I ./src/ ./src/build-gh-pages.cpp -o ./bin/build-gh-pages -Wall -Wextra -DWIN32 -O3 -static


windows-cygwin:
	gcc -I ./src/ ./src/build-gh-pages.c -o ./bin/build-gh-pages -Wall -Wextra -g

windows-cygwin-release:
	gcc -I ./src/ ./src/build-gh-pages.c -o ./bin/build-gh-pages -Wall -Wextra -O2


windows-mingw-release:
	mingw32-g++ -I ./src/ ./src/build-gh-pages.cpp -o ./bin/build-gh-pages -Wall -Wextra -O3 -static -DWIN32


all:
	g++ -I ./src/ ./src/build-gh-pages.cpp -o ./bin/build-gh-pages-amd64 -Wall -Wextra -O3
	i686-w64-mingw32-c++ -I ./src/ ./src/build-gh-pages.cpp -o ./bin/build-gh-pages-win64 -Wall -Wextra -DWIN32 -O3 -static