#include <time.h>
#include <stdbool.h>
#include <assert.h>
#include "common.h"
#include "input.h"
#include "assets.h"
#include "renderer.h"
#include "scene.h"
#include "mysdl.h"
#include "player.h"
#include "hud.h"
#include "enemy.h"

// TODO: Coffins open and reveal Dracula (twice as fast)

// TODO: Monsters to move 2 pixels per frame, consistent with animation (Quake-like).
// TODO: Player to be smooth, and have Commander Keen-like momentum.

// TODO: Light and heavy 'whack' (heavy pushes back, 4 lights and they will reel).
// TODO: Shield bash.

// TODO: Fire blast (circle - Catacomb).
// TODO: Fire column (straight ahead - Catacomb).
// TODO: Health potion.

static const char *GAME_TITLE = "Graveyard Alpha 0.1";
const int ANIMATION_HZ = 1000 / 4;		//12fps
const int RENDER_HZ = 1000 / 60;		//60fps
const int GAME_HZ = 1000 / 60;			//60fps

bool running = true;
SDL_Window *window = NULL;

#ifdef _WIN32
#elif __APPLE__
    Coord windowSize = { 640, 480};
#elif __linux__
    Coord windowSize = { 1280, 960 };   // 320x240
#endif

static void initSDL(void) {
    SDL_Init(/*SDL_INIT_AUDIO | */SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC);
    if(!IMG_Init(IMG_INIT_PNG)) {
        fatalError("Fatal error", "SDL_Image did not initialise.");
    }
    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        fatalError("Fatal error", "SDL_Mixer did not initialise.");
    }
    SDL_InitSubSystem(SDL_INIT_VIDEO);
}

static void initWindow(void) {
    window = SDL_CreateWindow(
        GAME_TITLE,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        (int)windowSize.x,					//dimensions
        (int)windowSize.y,
        SDL_WINDOW_OPENGL
    );
    assert(window != NULL);
}

static void shutdownWindow(void) {
    if(window == NULL) return;			//OK to call if not yet setup (an established subsystem pattern elsewhere)

    SDL_DestroyWindow(window);
    window = NULL;
}

static void shutdownMain(void) {
    shutdownAssets();
    shutdownRenderer();
    shutdownWindow();

    SDL_Quit();
}

int main()  {
    //Seed randomMq number generator
    srand(time(NULL));

    atexit(shutdownMain);

    initSDL();
    initWindow();
    initRenderer();
    initAssets();
	initHud();
    initPlayer();
    initScene();
    initEnemy();
//    play("bell.wav");

	changeMode(MODE_GAME);

    long lastRenderFrameTime = clock();
    long lastGameFrameTime = lastRenderFrameTime;
    long lastAnimFrameTime = lastRenderFrameTime;

    //Main game loop (realtime)
    while(running){
        //Game frame
        if(timer(&lastGameFrameTime, GAME_HZ)) {
            pollInput();
			sceneGameFrame();
            playerGameFrame();
            enemyGameFrame();
			hudGameFrame();
            processSystemCommands();
        }

        //Self-managing animations
		enemyAnimateFrame();

		//Animation frame
		if(timer(&lastAnimFrameTime, ANIMATION_HZ)) {
			sceneAnimateFrame();
            playerAnimateFrame();
		}

        //Renderer frame
        double renderFPS;
        if(timer(&lastRenderFrameTime, RENDER_HZ)) {
			sceneRenderFrame();
			playerRenderFrame();
            enemyRenderFrame();
			hudRenderFrame();

            updateCanvas();
        }
    }

    return 0;
}
