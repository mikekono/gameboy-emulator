SDL_LIB = -L/usr/local/lib -lSDL2 -Wl,-rpath=/usr/local/lib
SDL_INCLUDE = -I/usr/local/include

make: ProcessorDebug.cpp
	./get_opcode.py
	g++ -o gameboy.exe Gameboy.cpp -std=gnu++11 $(SDL_INCLUDE) $(SDL_LIB)
