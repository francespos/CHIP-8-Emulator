#ifndef CORE_CHIP8_H
#define CORE_CHIP8_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define CHIP8_MEMORY_SIZE 4096
#define CHIP8_STACK_SIZE 16
#define CHIP8_NUM_V_REGISTERS 16 

#define CHIP8_NUM_KEYS 16			
#define CHIP8_DISPLAY_WIDTH 600	
#define CHIP8_DISPLAY_HEIGHT 600	

typedef enum {
	CHIP8_KEY_NOT_PRESSED = 0, 
	CHIP8_KEY_PRESSED 
} CHIP8Key;

typedef enum { 
	CHIP8_PIXEL_OFF = 0,
	CHIP8_PIXEL_ON
} CHIP8Pixel;

typedef struct {	
	uint8_t memory[CHIP8_MEMORY_SIZE];
	uint16_t stack[CHIP8_STACK_SIZE];

	uint8_t v[CHIP8_NUM_V_REGISTERS];	
	uint16_t i; 			

	uint16_t pc; 				
	uint8_t sp; 				

	uint8_t dt; 				
	uint8_t st; 			

	CHIP8Key keyboard[CHIP8_NUM_KEYS];
	CHIP8Pixel display[CHIP8_DISPLAY_WIDTH][CHIP8_DISPLAY_HEIGHT];
} CHIP8;

typedef enum { 
	CHIP8_ERROR_INSTRUCTION_NOT_FOUND = -8,
	CHIP8_ERROR_KEY_NOT_FOUND,	
	CHIP8_ERROR_STACK_OVERFLOW,
	CHIP8_ERROR_STACK_UNDERFLOW,
	CHIP8_ERROR_SEGFAULT,
	CHIP8_ERROR_OPEN_FILE_FAILED,
	CHIP8_ERROR_INVALID_FONTSET,
	CHIP8_ERROR_INIT_FAILED, 
	CHIP8_SUCCESS
} CHIP8Result;

CHIP8 *CHIP8Init();
void CHIP8Destroy(CHIP8 *chip8);

CHIP8Result CHIP8LoadFontset(CHIP8 *chip8, const uint8_t *fontset, size_t fontsetSize);
CHIP8Result CHIP8LoadROM(CHIP8 *chip8, const char *fileName);
CHIP8Result CHIP8Execute(CHIP8 *chip8);

const char *CHIP8GetError();

#endif