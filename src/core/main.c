#define SDL_MAIN_HANDLED
#include <core/app.h>

#include <stdio.h>

static const uint8_t fontset[] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0,
	0x20, 0x60, 0x20, 0x20, 0x70,
	0xF0, 0x10, 0xF0, 0x80, 0xF0,
	0xF0, 0x10, 0xF0, 0x10, 0xF0,
	0x90, 0x90, 0xF0, 0x10, 0x10,
	0xF0, 0x80, 0xF0, 0x10, 0xF0,
	0xF0, 0x80, 0xF0, 0x90, 0xF0,
	0xF0, 0x10, 0x20, 0x40, 0x40,
	0xF0, 0x90, 0xF0, 0x90, 0xF0,
	0xF0, 0x90, 0xF0, 0x10, 0xF0,
	0xF0, 0x90, 0xF0, 0x90, 0x90,
	0xE0, 0x90, 0xE0, 0x90, 0xE0, 
	0xF0, 0x80, 0x80, 0x80, 0xF0,
	0xE0, 0x90, 0x90, 0x90, 0xE0,
	0xF0, 0x80, 0xF0, 0x80, 0xF0,
	0xF0, 0x80, 0xF0, 0x80, 0x80
};

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Error: No ROM provided.\n");
        exit(EXIT_FAILURE);
    }
    if (argc < 4) {
        fprintf(stderr, "Error: No window resolution provided.\n");
        exit(EXIT_FAILURE);
    }

    int windowWidth = (int) strtol(argv[2], NULL, 10);
    int windowHeight = (int) strtol(argv[3], NULL, 10);

	CHIP8 *chip8 = CHIP8Init();
	if (chip8 == NULL) {
		fprintf(stderr, "Error: %s.\n", CHIP8GetError());
		exit(EXIT_FAILURE);
	}

	if (CHIP8LoadFontset(chip8, fontset, sizeof(fontset)) != CHIP8_SUCCESS) {
		fprintf(stderr, "Error: %s.\n", CHIP8GetError());
		exit(EXIT_FAILURE);
	}

	if (CHIP8LoadROM(chip8, argv[1]) != CHIP8_SUCCESS) {
		fprintf(stderr, "Error: %s.\n", CHIP8GetError());
		exit(EXIT_FAILURE);
	}

	App *app = AppInit(windowWidth, windowHeight);
	if (app == NULL) {
		fprintf(stderr, "Error: %s.\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

    AppLoop(app, chip8);

	AppDestroy(app);
	CHIP8Destroy(chip8);
}