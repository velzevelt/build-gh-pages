SRC=./src
BIN=./bin

$(shell mkdir -p $(BIN))

g++-debug: 
	g++ -I $(SRC) $(SRC)/build-gh-pages.cpp -o $(BIN)/build-gh-pages-linux-amd64 -Wall -Wextra -g

g++-release: 
	g++ -I $(SRC) $(SRC)/build-gh-pages.cpp -o $(BIN)/build-gh-pages-linux-amd64 -Wall -Wextra -O3 -s


mingw-debug: 
	i686-w64-mingw32-c++ -I $(SRC) $(SRC)/build-gh-pages.cpp -o $(BIN)/build-gh-pages-win64 -Wall -Wextra -g -static -DWIN32

mingw-release: 
	i686-w64-mingw32-c++ -I $(SRC) $(SRC)/build-gh-pages.cpp -o $(BIN)/build-gh-pages-win64 -Wall -Wextra -DWIN32 -O3 -static -s


clean: 
	rm -rf $(BIN)/*


all:
	$(MAKE) g++-release
	$(MAKE) mingw-release