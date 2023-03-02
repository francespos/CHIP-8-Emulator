#include <core/app.h>
#include <stdio.h>

#define SDL_APP_WINDOW_NAME "CHIP-8 Emulator"
#define APP_AUDIO_FILE_NAME "media/audio.wav"

#define DISPLAY_SPRITE_COLOUR 0xFFCCCCCC
#define DISPLAY_BACKGROUND_COLOUR 0xCCAAAAAA

#define APP_SHOW_TIME 14
#define APP_EXECUTE_TIME 1
#define APP_UPDATE_TIME_REGISTER_TIME 14

typedef struct {
	App *app;
	CHIP8 *chip8;
} AppCallbackParameter;

static int AppShowFrame(App *app, CHIP8 *chip8);
static void AppOnKeyDown(App *app, CHIP8 *chip8, SDL_KeyboardEvent *event);
static void AppOnKeyUp(App *app, CHIP8 *chip8, SDL_KeyboardEvent *event);

static uint32_t AppShowFrameCallback(uint32_t interval, void *parameter);
static uint32_t AppExecuteCallback(uint32_t interval, void *parameter);
static uint32_t AppUpdateTimeRegisterCallback(uint32_t interval, void *parameter);

App *AppInit(int windowWidth, int windowHeight) {
    App *app = (App *) malloc(sizeof(App));

    if ((SDL_Init(SDL_INIT_VIDEO)) < 0) {
        return NULL;
    }

	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		return NULL;
	}

    app->window = SDL_CreateWindow(
        SDL_APP_WINDOW_NAME, 
        SDL_WINDOWPOS_UNDEFINED, 
		SDL_WINDOWPOS_UNDEFINED,
        windowWidth,
		windowHeight,
        0
    );

    if (app->window == NULL) {
        return NULL;
    }

    app->renderer = SDL_CreateRenderer(app->window, -1, SDL_RENDERER_ACCELERATED);
    if (app->renderer == NULL) {
        return NULL;
    }

	// SDL_RenderSetLogicalSize(app->renderer, CHIP8_DISPLAY_WIDTH, CHIP8_DISPLAY_HEIGHT);

    if (SDL_RenderSetScale(app->renderer, windowWidth/64, windowHeight/32) < 0) {
        return NULL;
    }

	app->texture = SDL_CreateTexture(
        app->renderer,
        SDL_PIXELFORMAT_ARGB8888, 
		SDL_TEXTUREACCESS_STREAMING,
        CHIP8_DISPLAY_WIDTH,
		CHIP8_DISPLAY_HEIGHT
    );

	if (app->texture == NULL) {
		return NULL;
	}

	SDL_AudioSpec wavSpec;

	if (SDL_LoadWAV(APP_AUDIO_FILE_NAME, &wavSpec, &app->wavBuffer, &app->wavLenght) == NULL) {
		return NULL;
	}

	app->audioDeviceID = SDL_OpenAudioDevice(NULL, 0, &wavSpec, NULL, 0);
	if (app->audioDeviceID == 0) {
		return NULL;
	}

    return app;
}

void AppDestroy(App *app) {
	SDL_CloseAudioDevice(app->audioDeviceID);
	SDL_FreeWAV(app->wavBuffer);

	SDL_DestroyTexture(app->texture);
	SDL_DestroyRenderer(app->renderer);
	SDL_DestroyWindow(app->window);

	SDL_Quit();
    free(app);
}

void AppLoop(App *app, CHIP8 *chip8) {
	bool quit = false;

	AppCallbackParameter callbackParameter = { app, chip8 };

	SDL_TimerID showFrameTimer = SDL_AddTimer(APP_SHOW_TIME, AppShowFrameCallback, (void *) &callbackParameter);
	SDL_TimerID executeTimer = SDL_AddTimer(APP_EXECUTE_TIME, AppExecuteCallback, (void *) chip8);
	SDL_TimerID updateTimeRegisterTimer = SDL_AddTimer(APP_UPDATE_TIME_REGISTER_TIME, AppUpdateTimeRegisterCallback, (void *) &callbackParameter);

	while (!quit) {
		SDL_Event event;

		while (SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_QUIT:
					quit = true;
					break;
				case SDL_KEYDOWN:
					AppOnKeyDown(app, chip8, &event.key);
					break;
				case SDL_KEYUP:
					AppOnKeyUp(app, chip8, &event.key);
					break;
				default:
					break;
			}
		}
	}

	SDL_RemoveTimer(showFrameTimer);
	SDL_RemoveTimer(executeTimer);
	SDL_RemoveTimer(updateTimeRegisterTimer);
}

int AppShowFrame(App *app, CHIP8 *chip8) {
    uint32_t buffer[CHIP8_DISPLAY_WIDTH * CHIP8_DISPLAY_HEIGHT];

    for (uint32_t i = 0; i < CHIP8_DISPLAY_WIDTH; ++i) {
        for (uint32_t j = 0; j < CHIP8_DISPLAY_HEIGHT; ++j) {
            buffer[j * CHIP8_DISPLAY_HEIGHT + i] = ((DISPLAY_SPRITE_COLOUR * chip8->display[i][j]) | DISPLAY_BACKGROUND_COLOUR);
        }
    }

    int result = SDL_UpdateTexture(app->texture, NULL, buffer, CHIP8_DISPLAY_WIDTH * 4);
    if (result < 0) {
        return result;
    }

    result = SDL_RenderClear(app->renderer);
    if (result < 0) {
        return result;
    }

    SDL_Rect destinationRectangle = {0, 0, CHIP8_DISPLAY_WIDTH, CHIP8_DISPLAY_HEIGHT};

    result = SDL_RenderCopy(app->renderer, app->texture, NULL, &destinationRectangle);
    if (result < 0) {
        return result;
    }

    SDL_RenderPresent(app->renderer);

    return 0;
}

void AppOnKeyDown(App *app, CHIP8 *chip8, SDL_KeyboardEvent *event) {
    if (event->repeat == 0) {
        if (event->keysym.scancode == SDL_SCANCODE_0) {
            chip8->keyboard[0x0] = CHIP8_KEY_PRESSED;
        } 
        if (event->keysym.scancode == SDL_SCANCODE_1) {
            chip8->keyboard[0x1] = CHIP8_KEY_PRESSED;
        } 
        if (event->keysym.scancode == SDL_SCANCODE_2) {
            chip8->keyboard[0x2] = CHIP8_KEY_PRESSED;
        } 
        if (event->keysym.scancode == SDL_SCANCODE_3) {
            chip8->keyboard[0x3] = CHIP8_KEY_PRESSED;
        } 
        if (event->keysym.scancode == SDL_SCANCODE_4) {
            chip8->keyboard[0x4] = CHIP8_KEY_PRESSED;
        } 
        if (event->keysym.scancode == SDL_SCANCODE_5) {
            chip8->keyboard[0x5] = CHIP8_KEY_PRESSED;
        } 
        if (event->keysym.scancode == SDL_SCANCODE_6) {
            chip8->keyboard[0x6] = CHIP8_KEY_PRESSED;
        } 
        if (event->keysym.scancode == SDL_SCANCODE_7) {
            chip8->keyboard[0x7] = CHIP8_KEY_PRESSED;
        } 
        if (event->keysym.scancode == SDL_SCANCODE_8) {
            chip8->keyboard[0x8] = CHIP8_KEY_PRESSED;
        } 
        if (event->keysym.scancode == SDL_SCANCODE_9) {
            chip8->keyboard[0x9] = CHIP8_KEY_PRESSED;
        } 
        if (event->keysym.scancode == SDL_SCANCODE_A) {
            chip8->keyboard[0xA] = CHIP8_KEY_PRESSED;
        } 
        if (event->keysym.scancode == SDL_SCANCODE_B) {
            chip8->keyboard[0xB] = CHIP8_KEY_PRESSED;
        } 
        if (event->keysym.scancode == SDL_SCANCODE_C) {
            chip8->keyboard[0xC] = CHIP8_KEY_PRESSED;
        }
        if (event->keysym.scancode == SDL_SCANCODE_D) {
            chip8->keyboard[0xD] = CHIP8_KEY_PRESSED;
        } 
        if (event->keysym.scancode == SDL_SCANCODE_E) {
            chip8->keyboard[0xE] = CHIP8_KEY_PRESSED;
        } 
        if (event->keysym.scancode == SDL_SCANCODE_F) {
            chip8->keyboard[0xF] = CHIP8_KEY_PRESSED;
        }
    }
}

void AppOnKeyUp(App *app, CHIP8 *chip8, SDL_KeyboardEvent *event) {
    if (event->repeat == 0) {
        if (event->keysym.scancode == SDL_SCANCODE_0) {
            chip8->keyboard[0x0] = CHIP8_KEY_NOT_PRESSED;
        } 
        if (event->keysym.scancode == SDL_SCANCODE_1) {
            chip8->keyboard[0x1] = CHIP8_KEY_NOT_PRESSED;
        } 
        if (event->keysym.scancode == SDL_SCANCODE_2) {
            chip8->keyboard[0x2] = CHIP8_KEY_NOT_PRESSED;
        } 
        if (event->keysym.scancode == SDL_SCANCODE_3) {
            chip8->keyboard[0x3] = CHIP8_KEY_NOT_PRESSED;
        } 
        if (event->keysym.scancode == SDL_SCANCODE_4) {
            chip8->keyboard[0x4] = CHIP8_KEY_NOT_PRESSED;
        } 
        if (event->keysym.scancode == SDL_SCANCODE_5) {
            chip8->keyboard[0x5] = CHIP8_KEY_NOT_PRESSED;
        } 
        if (event->keysym.scancode == SDL_SCANCODE_6) {
            chip8->keyboard[0x6] = CHIP8_KEY_NOT_PRESSED;
        } 
        if (event->keysym.scancode == SDL_SCANCODE_7) {
            chip8->keyboard[0x7] = CHIP8_KEY_NOT_PRESSED;
        } 
        if (event->keysym.scancode == SDL_SCANCODE_8) {
            chip8->keyboard[0x8] = CHIP8_KEY_NOT_PRESSED;
        } 
        if (event->keysym.scancode == SDL_SCANCODE_9) {
            chip8->keyboard[0x9] = CHIP8_KEY_NOT_PRESSED;
        } 
        if (event->keysym.scancode == SDL_SCANCODE_A) {
            chip8->keyboard[0xA] = CHIP8_KEY_NOT_PRESSED;
        } 
        if (event->keysym.scancode == SDL_SCANCODE_B) {
            chip8->keyboard[0xB] = CHIP8_KEY_NOT_PRESSED;
        } 
        if (event->keysym.scancode == SDL_SCANCODE_C) {
            chip8->keyboard[0xC] = CHIP8_KEY_NOT_PRESSED;
        }
        if (event->keysym.scancode == SDL_SCANCODE_D) {
            chip8->keyboard[0xD] = CHIP8_KEY_NOT_PRESSED;
        } 
        if (event->keysym.scancode == SDL_SCANCODE_E) {
            chip8->keyboard[0xE] = CHIP8_KEY_NOT_PRESSED;
        } 
        if (event->keysym.scancode == SDL_SCANCODE_F) {
            chip8->keyboard[0xF] = CHIP8_KEY_NOT_PRESSED;
        }
    }
}

uint32_t AppShowFrameCallback(uint32_t interval, void *parameter) {
	AppCallbackParameter *callbackParameter = (AppCallbackParameter *) parameter;

	App *app = callbackParameter->app;
	CHIP8 *chip8 = callbackParameter->chip8;

	if (AppShowFrame(app, chip8) < 0)  {
		exit(EXIT_FAILURE);
	}

	return interval;
}

uint32_t AppExecuteCallback(uint32_t interval, void *parameter) {
	CHIP8 *chip8 = (CHIP8 *) parameter;

	if (CHIP8Execute(chip8) != CHIP8_SUCCESS) {
		fprintf(stderr, "Error: %s.\n", CHIP8GetError());
		exit(EXIT_FAILURE);
	}

	return interval;
}

uint32_t AppUpdateTimeRegisterCallback(uint32_t interval, void *parameter) {
	AppCallbackParameter *callbackParameter = (AppCallbackParameter *) parameter;

	App *app = callbackParameter->app;
	CHIP8 *chip8 = callbackParameter->chip8;

	if (chip8->dt > 0) {
		--chip8->dt;
	}

	if (chip8->st > 0) {
		if (SDL_QueueAudio(app->audioDeviceID, app->wavBuffer,app->wavLenght) < 0) {
			return 0;
		}
		SDL_PauseAudioDevice(app->audioDeviceID, 0);

		--chip8->st;
	} else {
		SDL_PauseAudioDevice(app->audioDeviceID, 1);
	}

	return interval;
}