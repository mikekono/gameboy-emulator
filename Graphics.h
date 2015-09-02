#include <SDL2/SDL.h>
#include <chrono>
#include <thread>

#define NUM_TILES 384
#define NUM_SPRITES 256
#define VERTICAL_RES 144
#define HORIZONTAL_RES 160

#define OAM_BEGIN 0xFE00
#define OAM_END 0xFE9F

class Graphics {
public:
    SDL_Window * window;
    SDL_Renderer * renderer;
    SDL_Texture * texture;
    SDL_Surface * surface;
    Memory& mem;
    Uint32 * pixels;

    int scrollX, scrollY;
    Uint32 tile[NUM_TILES][8][8]; // store the 384 8x8 tiles found in memory
    Uint32 background1[256][256];
    Uint32 background2[256][256];
    Uint32 gba_window[VERTICAL_RES][HORIZONTAL_RES];

    Uint32 sprite[NUM_SPRITES][8][8];

    const unsigned char horizontalRes = 160;
    const unsigned char verticalRes = 144;

    // for raster graphics
    unsigned int modeClock = 0;
    byte mode = 0;
    byte line = 1;

    // BANG - make private?
    std::chrono::_V2::system_clock::time_point lastTime;

   Graphics(Memory& _mem) : mem(_mem){
	lastTime = std::chrono::system_clock::now();
    }

    void init();

    void createBackground1();
    void createBackground2();

    void printTileToBG(int mapLocation, int tileNum, Uint32 background[256][256]);
    void printBackgroundToPixels(unsigned char scrollX, unsigned char scrollY, Uint32 background[256][256]);

    void processPixels(int tileNum, Uint32 memAddr, Uint32 pixelArray[384][8][8]);

    void createTileArray(int tileNum);
    void createSpriteArray(int tileNum);

    void printSprites(unsigned char scrollX, unsigned char scrollY);

    void updateScreen();
    void draw();

    // raster graphics
    void step(unsigned int numCPUCycles);	

};
