#include "Graphics.h"
  
    void Graphics::init(){
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow("gb emu",
				  SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, HORIZONTAL_RES, VERTICAL_RES, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	renderer = SDL_CreateRenderer(window, -1, 0);
	texture = SDL_CreateTexture(renderer,
				    SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, HORIZONTAL_RES, VERTICAL_RES);

	pixels = new Uint32[HORIZONTAL_RES * VERTICAL_RES];
	memset(pixels, 255, HORIZONTAL_RES * VERTICAL_RES * sizeof(Uint32));
    }

    // creates a pixel image consisting of background 1
    void Graphics::createBackground1(){
	Uint32 count = 0;
	// mesh the tiles together into one background image
	for (Uint32 i = 0x9800; i <= 0x9BFF; i++){
	    // check to see which tile map data we are using
	    byte LCDC = mem.read(0xFF40);
	    // need to handle the two different places we get our background data from
	    if ((LCDC & 0x10) != 0){
		printTileToBG(count, mem.memory[i], background1);
	    }
	    else{
	        byte tileIndex = mem.memory[i] + 128; // this mode treats the index as signed
		printTileToBG(count, tileIndex + 128, background1);
	    }
	    count++;
	}
    }

    void Graphics::createBackground2(){
	Uint32 count = 0;
	for (Uint32 i = 0x9C00; i <= 0x9FFF; i++){
	    // check to see which tile map data we are using
	    byte LCDC = mem.read(0xFF40);
	    // need to handle the two different places we get our background data from
	    if ((LCDC & 0x10) != 0){
		printTileToBG(count, mem.memory[i], background2);
	    }
	    else{
	        signed char tileIndex = mem.memory[i]; // this mode treats the index as signed
		
		printTileToBG(count, tileIndex + 256, background2);
	    }
	    count++;
	}
    }

    // prints the pixel representation of each tile to our background array
    void Graphics::printTileToBG(int mapLocation, int tileNum, Uint32 background[256][256]){
	// PRINT TILE
	unsigned int horizontalOffset = (mapLocation % 32) * 8;
	unsigned int verticalOffset = (mapLocation / 32) * 8;
	unsigned int totalOffset = horizontalOffset + verticalOffset;
	for (int i = 0; i < 8; i++){
	    for (int j = 0; j < 8; j++){
		//pixels[i * 256 + j + totalOffset] = tile[i][j];
		background[i + verticalOffset][j + horizontalOffset] = tile[tileNum][i][j];
	    }
	}
    }

    // outputs the appropriate section of the background image onto the screen
    void Graphics::printBackgroundToPixels(unsigned char scrollX, unsigned char scrollY, Uint32 background[256][256]){
	unsigned char yCord = 0;
	unsigned char xCord = 0;
	for (int i = 0; i < VERTICAL_RES; i++){
	    for (int j = 0; j < HORIZONTAL_RES; j++){
		yCord = i + scrollY;
		xCord = j + scrollX;
		pixels[i * HORIZONTAL_RES + j] = background[yCord][xCord];
	    }
	}
    }


    // converts the gameboy representation of pixels to the RGB values our computer recognizes
    void Graphics::processPixels(int tileNum, Uint32 memAddr, Uint32 pixelArray[384][8][8]) {
	int tileOffset = tileNum * 16;
	Uint32 baseAddr = memAddr + tileOffset;

	for (unsigned int i = 0; i < 16; i+=2){
	    unsigned char bitMask;      
	    for (unsigned int j = 0; j < 8; j++){
		bitMask = 1 << (7-j); // use this mask to read each bit
		// 11 - 0x0
		if ((bitMask & mem.memory[baseAddr + i] & mem.memory[baseAddr + i + 1]) != 0){
		    pixelArray[tileNum][i >> 1][j] = COLOR_BLACK;
		}
		// 10 - 0x555555
		else if ((bitMask & mem.memory[baseAddr + i]) != 0){
		    pixelArray[tileNum][i >> 1][j] = COLOR_DARK_GRAY;
		}
		// 01 - 0xaaaaaa
		else if ((bitMask & mem.memory[baseAddr + i + 1]) != 0){
		    pixelArray[tileNum][i >> 1][j] = COLOR_LIGHT_GRAY;
		}
		// 00 - 0xffffff
		else {
		    pixelArray[tileNum][i >> 1][j] = COLOR_WHITE;
		}
	    }
	}
    }

 
    void Graphics::createTileArray(int tileNum){
	processPixels(tileNum, 0x8000, tile);
    }

    void Graphics::createSpriteArray(int tileNum){
	processPixels(tileNum, 0xFE9F, sprite);
    }

    void Graphics::printSprites(unsigned char scrollX, unsigned char scrollY){
	// get info from the sprite attribute table (OAM)
	for (int i = OAM_BEGIN; i <= OAM_END; i+=4){
	    byte y_pos = mem.memory[i];
	    byte x_pos = mem.memory[i+1];
	    byte pattern_num = mem.memory[i+2];
	    byte flags = mem.memory[i+3];

	    unsigned char yCord = 0;
	    unsigned char xCord = 0;
	    // BANG - only print those that are specified in front of background for now
	    if (1){
		for (int j = 0; j < 8; j++){
		    for (int k = 0; k < 8; k++){
			// -16 and -8 because setting y and x to zero is supposed to hide the sprite
			yCord = j + y_pos - 16;
			xCord = k + x_pos - 8;
			// if priority 1, hide sprite behind any color not white
			// if priority 0, just print
			if (((flags & 0x80) == 0) || (pixels[yCord * HORIZONTAL_RES + xCord] == COLOR_WHITE)){
			    // also handle transparency when sprite background is white 
			    // BANG / TODO: actually read what the sprite colors are from OBP0/1
			    if (tile[pattern_num][j][k] != COLOR_WHITE){
				pixels[yCord * HORIZONTAL_RES + xCord] = tile[pattern_num][j][k];
			    }
			}
		    }
		}
	    }
	}
    }

    void Graphics::draw(){
	// limit the clock rate to 60 Hz
	lastTime = std::chrono::system_clock::now() + std::chrono::milliseconds(17);
	//std::this_thread::sleep_until(lastTime);

	// print the texture to the screen
	SDL_UpdateTexture(texture, NULL, pixels, HORIZONTAL_RES * sizeof(Uint32));	
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);

    }

    void Graphics::updateScreen(){
	for (int i = 0; i < NUM_TILES; i++){
	    createTileArray(i);
	}
	// TODO: support for 8x16 sprites
	for (int i = 0; i < NUM_SPRITES; i++){
	    createSpriteArray(i);
	}

	byte LCDC = mem.read(0xFF40);

	// display background 1
	if ( (LCDC & 0x08) == 0){
	    createBackground1();
	    printBackgroundToPixels(scrollX,scrollY, background1);
	}
	else {
	    createBackground2();
	    printBackgroundToPixels(scrollX,scrollY, background2);
	}
	printSprites(scrollX,scrollY);
    }
  
    // **** RASTER GRAPHICS ****
    void Graphics::step(unsigned int numCPUCycles){
	modeClock+=numCPUCycles;
	//printf("modeClock: %i \n", modeClock);
	switch(mode){
	    // horizontal blank - 204 cycles
	case 0:
	    if (modeClock >= 204){
		modeClock -= 204;
		line++;
		mem.write(0xff44, line);
		if (line == 143){
		    mode = 1; // Vblank
		    // BANG - PRINT 
		    //printf("PRINTING \n");
		    updateScreen();
		    draw();
		    // enable vblank int
		    mem.write(INTERRUPT_FLAG_REG, mem.read(INTERRUPT_FLAG_REG) | 0x01);
		    //printf("VBLANK EN \n");
		}
		else { // draw sprite and background
		    mode = 2;
		}
	    }
	    break;
	    // Vblank -  4560 cycles
	case 1:
	    //vblank interrupt
	    // if ((line >= 143)){
	    // 	mem.write(INTERRUPT_FLAG_REG, mem.read(INTERRUPT_FLAG_REG) | 0x01);
	    // }
	    if (modeClock >= 456){
		modeClock -= 456;
		line++;
		mem.write(0xff44, line);
		if (line >= 153){
		    //printf("VBLANK DIS\n");
		    //mem.write(INTERRUPT_FLAG_REG, mem.read(INTERRUPT_FLAG_REG) & 0xFE);
		    line = 0;
		    mem.write(0xff44, line);
		    mode = 2;
		}
	    }
	    break;
	    // OAM ram - 80 cycles
	case 2:
	    if (modeClock >= 80){
		modeClock -= 80;
		mode = 3;
	    }
	    break;
	    // VRAM - 172 cycles
	case 3:
	    if (modeClock >= 172){
		modeClock -= 172;
		mode = 0;

		// WRITE LINE TO BUFFER

	    }
	}
    }



