#include <core/chip8.h>
#include <utils/safe_string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define CHIP8_ERROR_MESSAGE_SIZE 50

#define CHIP8_INTERPRETER_START_ADDRESS 0
#define CHIP8_INTERPRETER_END_ADDRESS 0x1ff

#define CHIP8_FONTSET_START_ADDRESS 0x50
#define CHIP8_ROM_START_ADDRESS 0x200

static char CHIP8ErrorMessage[CHIP8_ERROR_MESSAGE_SIZE] = "";

static void CHIP8SetError(CHIP8Result result);

// instructions
static void CHIP8_00e0(CHIP8 *chip8);
static CHIP8Result CHIP8_00ee(CHIP8 *chip8);
static void CHIP8_1nnn(CHIP8 *chip8, uint16_t nnn); 
static CHIP8Result CHIP8_2nnn(CHIP8 *chip8, uint16_t nnn);
static CHIP8Result CHIP8_3xkk(CHIP8 *chip8, uint8_t x, uint8_t kk);
static CHIP8Result CHIP8_4xkk(CHIP8 *chip8, uint8_t x, uint8_t kk);
static CHIP8Result CHIP8_5xy0(CHIP8 *chip8, uint8_t x, uint8_t y);
static void CHIP8_6xkk(CHIP8 *chip8, uint8_t x, uint8_t kk);
static void CHIP8_7xkk(CHIP8 *chip8, uint8_t x, uint8_t kk);
static void CHIP8_8xy0(CHIP8 *chip8, uint8_t x, uint8_t y);
static void CHIP8_8xy1(CHIP8 *chip8, uint8_t x, uint8_t y);
static void CHIP8_8xy2(CHIP8 *chip8, uint8_t x, uint8_t y);
static void CHIP8_8xy3(CHIP8 *chip8, uint8_t x, uint8_t y);
static void CHIP8_8xy4(CHIP8 *chip8, uint8_t x, uint8_t y);
static void CHIP8_8xy5(CHIP8 *chip8, uint8_t x, uint8_t y);
static void CHIP8_8xy6(CHIP8 *chip8, uint8_t x, uint8_t y);
static void CHIP8_8xy7(CHIP8 *chip8, uint8_t x, uint8_t y);
static void CHIP8_8xye(CHIP8 *chip8, uint8_t x, uint8_t y);
static CHIP8Result CHIP8_9xy0(CHIP8 *chip8, uint8_t x, uint8_t y);
static void CHIP8_annn(CHIP8 *chip8, uint16_t nnn);
static void CHIP8_bnnn(CHIP8 *chip8, uint16_t nnn);
static void CHIP8_cxkk(CHIP8 *chip8, uint8_t x, uint8_t kk);
static void CHIP8_dxyn(CHIP8 *chip8, uint8_t x, uint8_t y, uint8_t n);
static CHIP8Result CHIP8_ex9e(CHIP8 *chip8, uint8_t x);
static CHIP8Result CHIP8_exa1(CHIP8 *chip8, uint8_t x);
static void CHIP8_fx07(CHIP8 *chip8, uint8_t x);
static CHIP8Result CHIP8_fx0a(CHIP8 *chip8, uint8_t x);
static void CHIP8_fx15(CHIP8 *chip8, uint8_t x);
static void CHIP8_fx18(CHIP8 *chip8, uint8_t x);
static CHIP8Result CHIP8_fx1e(CHIP8 *chip8, uint8_t x);
static CHIP8Result CHIP8_fx29(CHIP8 *chip8, uint8_t x);
static void CHIP8_fx33(CHIP8 *chip8, uint8_t x);
static CHIP8Result CHIP8_fx55(CHIP8 *chip8, uint8_t x);
static CHIP8Result CHIP8_fx65(CHIP8 *chip8, uint8_t x);

CHIP8 *CHIP8Init() {
	CHIP8 *chip8 = (CHIP8 *) malloc(sizeof(CHIP8));
	if (chip8 == NULL) {
		CHIP8SetError(CHIP8_ERROR_INIT_FAILED);
		return NULL;
	}

	chip8->pc = CHIP8_ROM_START_ADDRESS;
	chip8->sp = 0;
	chip8->dt = 0;
	chip8->st = 0;

	for (size_t i = 0; i < CHIP8_NUM_KEYS; ++i) {
		chip8->keyboard[i] = CHIP8_KEY_NOT_PRESSED;
	}

	for (size_t i = 0; i < 64; ++i) {
		for (size_t j = 0; j < 32; ++j) {
			chip8->display[i][j] = CHIP8_PIXEL_OFF;
		}
	}

	return chip8;
}

void CHIP8Destroy(CHIP8 *chip8) {
	free(chip8);
}

CHIP8Result CHIP8LoadFontset(CHIP8 *chip8, const uint8_t *fontset, size_t fontsetSize) {
	if (fontsetSize != 80) {
		CHIP8SetError(CHIP8_ERROR_INVALID_FONTSET);
		return CHIP8_ERROR_INVALID_FONTSET;
	}
	
	for (size_t i = 0; i < 80; ++i) {
		chip8->memory[CHIP8_FONTSET_START_ADDRESS + i] = fontset[i];
	}

	return CHIP8_SUCCESS;
}

CHIP8Result CHIP8LoadROM(CHIP8 *chip8, const char *fileName) {
	FILE *file;
	if ((file = fopen(fileName, "rb")) == NULL) {
		CHIP8SetError(CHIP8_ERROR_OPEN_FILE_FAILED);
		return CHIP8_ERROR_OPEN_FILE_FAILED;
	}
	
	fseek(file, 0, SEEK_SET);
	
	size_t i = CHIP8_ROM_START_ADDRESS;
	for (; !feof(file) && i < CHIP8_MEMORY_SIZE; ++i) {
		chip8->memory[i] = (uint8_t) fgetc(file);

		#ifdef DEBUG
		printf("Half instruction %hhx loaded in location n. %hhx.\n", chip8->memory[i], i);
		#endif
	}

	fclose(file);

	if (i == CHIP8_MEMORY_SIZE) {
		CHIP8SetError(CHIP8_ERROR_SEGFAULT);
		return CHIP8_ERROR_SEGFAULT;
	}
	
	return CHIP8_SUCCESS;
}

CHIP8Result CHIP8Execute(CHIP8 *chip8) {
	// Fetch
	uint8_t msbyte = chip8->memory[chip8->pc]; 
	++chip8->pc;
	uint8_t lsbyte = chip8->memory[chip8->pc]; 
	++chip8->pc;

	#ifdef DEBUG
	printf("Execute instruction %hhx.\n", msbyte << 8 | lsbyte);
	#endif

	// Decode
	uint8_t opcode = msbyte >> 4;

	uint8_t x = msbyte & 0x0F;		
	uint8_t y = lsbyte >> 4; 
	uint8_t n = lsbyte & 0x0F; 	 
	
	uint8_t kk = lsbyte; 	
	uint16_t nnn = x << 8 | lsbyte;

	switch(opcode) {
		case 0x0:
			switch(nnn) {
				case 0x0e0:
					CHIP8_00e0(chip8);
					break;
				case 0x0ee:
					return CHIP8_00ee(chip8);
					break;
				default:
					CHIP8SetError(CHIP8_ERROR_INSTRUCTION_NOT_FOUND);
					return CHIP8_ERROR_INSTRUCTION_NOT_FOUND;
					break;
			}
			break;
		case 0x1:
			CHIP8_1nnn(chip8, nnn);
			break;
		case 0x2:
			return CHIP8_2nnn(chip8, nnn);
			break;
		case 0x3:
			return CHIP8_3xkk(chip8, x, kk);
			break;
		case 0x4:
			return CHIP8_4xkk(chip8, x, kk);
			break;
		case 0x5:
			switch(n) {
				case 0x0:
					return CHIP8_5xy0(chip8, x, y);
					break;
				default:
					CHIP8SetError(CHIP8_ERROR_INSTRUCTION_NOT_FOUND);
					return CHIP8_ERROR_INSTRUCTION_NOT_FOUND;
					break;
			}
			break;
		case 0x6:
			CHIP8_6xkk(chip8, x, kk);
			break;
		case 0x7:
			CHIP8_7xkk(chip8, x, kk);
			break;
		case 0x8:
			switch(n) {
				case 0x0:
					CHIP8_8xy0(chip8, x, y);
					break;
				case 0x1:
					CHIP8_8xy1(chip8, x, y);
					break;
				case 0x2:
					CHIP8_8xy2(chip8, x, y);
					break;
				case 0x3:
					CHIP8_8xy3(chip8, x, y);
					break;
				case 0x4:
					CHIP8_8xy4(chip8, x, y);
					break;
				case 0x5:
					CHIP8_8xy5(chip8, x, y);
					break;
				case 0x6:
					CHIP8_8xy6(chip8, x, y);
					break;
				case 0x7:
					CHIP8_8xy7(chip8, x, y);
					break;
				case 0xe:
					CHIP8_8xye(chip8, x, y);
					break;
				default:
					CHIP8SetError(CHIP8_ERROR_INSTRUCTION_NOT_FOUND);
					return CHIP8_ERROR_INSTRUCTION_NOT_FOUND;
					break;
			}
			break;
		case 0x9:
			switch(n) {
				case 0x0:
					return CHIP8_9xy0(chip8, x, y);
					break;
				default:
					CHIP8SetError(CHIP8_ERROR_INSTRUCTION_NOT_FOUND);
					return CHIP8_ERROR_INSTRUCTION_NOT_FOUND;
					break;
			}
			break;
		case 0xa:
			CHIP8_annn(chip8, nnn);
			break;
		case 0xb:
			CHIP8_bnnn(chip8, nnn);
			break;
		case 0xc:
			CHIP8_cxkk(chip8, x, kk);
			break;
		case 0xd:
			CHIP8_dxyn(chip8, x, y, n);
			break;
		case 0xe:
			switch(kk) {
				case 0x9e:
					return CHIP8_ex9e(chip8, x);
					break;
				case 0xa1:
					return CHIP8_exa1(chip8, x);
					break;
				default:
					CHIP8SetError(CHIP8_ERROR_INSTRUCTION_NOT_FOUND);
					return CHIP8_ERROR_INSTRUCTION_NOT_FOUND;
					break;
			}
			break;
		case 0xf:
			switch(kk) {
				case 0x07:
					CHIP8_fx07(chip8, x);
					break;
				case 0x0a:
					return CHIP8_fx0a(chip8, x);
					break;
				case 0x15:
					CHIP8_fx15(chip8, x);
					break;
				case 0x18:
					CHIP8_fx18(chip8, x);
					break;
				case 0x1e:
					return CHIP8_fx1e(chip8, x);
					break;
				case 0x29:
					return CHIP8_fx29(chip8, x);
					break;
				case 0x33:
					CHIP8_fx33(chip8, x);
					break;
				case 0x55:
					return CHIP8_fx55(chip8, x);
					break;
				case 0x65:
					return CHIP8_fx65(chip8, x);
					break;
				default:
					CHIP8SetError(CHIP8_ERROR_INSTRUCTION_NOT_FOUND);
					return CHIP8_ERROR_INSTRUCTION_NOT_FOUND;
					break;
			}
			break;
		default:
			CHIP8SetError(CHIP8_ERROR_INSTRUCTION_NOT_FOUND);
			return CHIP8_ERROR_INSTRUCTION_NOT_FOUND;
			break;
	}

	return CHIP8_SUCCESS;
}

const char *CHIP8GetError() {
	return CHIP8ErrorMessage;
}

void CHIP8SetError(CHIP8Result errorCode) {
	switch (errorCode) {
		case CHIP8_ERROR_INIT_FAILED:
			safeStringCopy(CHIP8ErrorMessage, "Initialization failed", CHIP8_ERROR_MESSAGE_SIZE);
			break;		
		case CHIP8_ERROR_INVALID_FONTSET:
			safeStringCopy(CHIP8ErrorMessage, "Invalid fontset", CHIP8_ERROR_MESSAGE_SIZE);
			break;
		case CHIP8_ERROR_OPEN_FILE_FAILED:
			safeStringCopy(CHIP8ErrorMessage, "Cannot open file", CHIP8_ERROR_MESSAGE_SIZE);
			break;
		case CHIP8_ERROR_SEGFAULT:
			safeStringCopy(CHIP8ErrorMessage, "Segmentation fault", CHIP8_ERROR_MESSAGE_SIZE);
			break;
		case CHIP8_ERROR_STACK_UNDERFLOW:
			safeStringCopy(CHIP8ErrorMessage, "Stack undeflow", CHIP8_ERROR_MESSAGE_SIZE);
			break;
		case CHIP8_ERROR_STACK_OVERFLOW:
			safeStringCopy(CHIP8ErrorMessage, "Stack overflow", CHIP8_ERROR_MESSAGE_SIZE);
			break;
		case CHIP8_ERROR_KEY_NOT_FOUND:
			safeStringCopy(CHIP8ErrorMessage, "Keyboard key not found", CHIP8_ERROR_MESSAGE_SIZE);
			break;
		case CHIP8_ERROR_INSTRUCTION_NOT_FOUND:
			safeStringCopy(CHIP8ErrorMessage, "Instruction does not exist", CHIP8_ERROR_MESSAGE_SIZE);
			break;
		default:
			safeStringCopy(CHIP8ErrorMessage, "Error code does not exist", CHIP8_ERROR_MESSAGE_SIZE);
			break;
	}
}

void CHIP8_00e0(CHIP8 *chip8) {
	for (uint8_t i = 0; i < 64; ++i) {
		for (uint8_t j = 0; j < 32; ++j) {
			chip8->display[i][j] = CHIP8_PIXEL_OFF;
		}
	}
}

CHIP8Result CHIP8_00ee(CHIP8 *chip8) {
	if (chip8->sp == 0) {
		CHIP8SetError(CHIP8_ERROR_STACK_UNDERFLOW);
		return CHIP8_ERROR_STACK_UNDERFLOW;
	}

	--chip8->sp;
	chip8->pc = chip8->stack[chip8->sp];

	return CHIP8_SUCCESS;
}

void CHIP8_1nnn(CHIP8 *chip8, uint16_t nnn) {
	chip8->pc = nnn;
}

CHIP8Result CHIP8_2nnn(CHIP8 *chip8, uint16_t nnn) {
	if (chip8->sp == CHIP8_STACK_SIZE)  {
		CHIP8SetError(CHIP8_ERROR_STACK_OVERFLOW);
		return CHIP8_ERROR_STACK_OVERFLOW;
	}

	chip8->stack[chip8->sp] = chip8->pc;
	++chip8->sp;

	chip8->pc = nnn;

	return CHIP8_SUCCESS;
}

CHIP8Result CHIP8_3xkk(CHIP8 *chip8, uint8_t x, uint8_t kk) {
	if (chip8->v[x] == kk) {
		if (chip8->pc > CHIP8_MEMORY_SIZE - 3) {
			CHIP8SetError(CHIP8_ERROR_SEGFAULT);
			return CHIP8_ERROR_SEGFAULT;
		}

		chip8->pc += 2;
	}

	return CHIP8_SUCCESS;
}

CHIP8Result CHIP8_4xkk(CHIP8 *chip8, uint8_t x, uint8_t kk) {
	if (chip8->v[x] != kk) {
		if (chip8->pc > CHIP8_MEMORY_SIZE - 3) {
			CHIP8SetError(CHIP8_ERROR_SEGFAULT);
			return CHIP8_ERROR_SEGFAULT;
		}

		chip8->pc += 2;
	}

	return CHIP8_SUCCESS;
}

CHIP8Result CHIP8_5xy0(CHIP8 *chip8, uint8_t x, uint8_t y) {
	if (chip8->v[x] == chip8->v[y]) {
		if (chip8->pc > CHIP8_MEMORY_SIZE - 3) {
			CHIP8SetError(CHIP8_ERROR_SEGFAULT);
			return CHIP8_ERROR_SEGFAULT;
		}

		chip8->pc += 2;
	}

	return CHIP8_SUCCESS;
}

void CHIP8_6xkk(CHIP8 *chip8, uint8_t x, uint8_t kk) {
	chip8->v[x] = kk;
}

void CHIP8_7xkk(CHIP8 *chip8, uint8_t x, uint8_t kk) {
	chip8->v[x] += kk;
}

void CHIP8_8xy0(CHIP8 *chip8, uint8_t x, uint8_t y) {
	chip8->v[x] = chip8->v[y];
}

void CHIP8_8xy1(CHIP8 *chip8, uint8_t x, uint8_t y) {
	chip8->v[x] |= chip8->v[y];
}

void CHIP8_8xy2(CHIP8 *chip8, uint8_t x, uint8_t y) {
	chip8->v[x] &= chip8->v[y];
}

void CHIP8_8xy3(CHIP8 *chip8, uint8_t x, uint8_t y) {
	chip8->v[x] ^= chip8->v[y];
}

void CHIP8_8xy4(CHIP8 *chip8, uint8_t x, uint8_t y) {
	uint16_t sum = chip8->v[x] + chip8->v[y];

	if (sum > 255) {
		chip8->v[0xf] = 1; 
	} else {
		chip8->v[0xf] = 0;
	}

	chip8->v[x] = (uint8_t) sum;
}

void CHIP8_8xy5(CHIP8 *chip8, uint8_t x, uint8_t y) {
	if (chip8->v[x] > chip8->v[y]) {
		chip8->v[0xf] = 1;
	} else {
		chip8->v[0xf] = 0;
	}

	chip8->v[x] -= chip8->v[y];
}

void CHIP8_8xy6(CHIP8 *chip8, uint8_t x, uint8_t y) {
	chip8->v[0xf] = chip8->v[x] & 0x1;

	chip8->v[x] >>= 1;
}

void CHIP8_8xy7(CHIP8 *chip8, uint8_t x, uint8_t y) {
	if (chip8->v[y] > chip8->v[x]) {
		chip8->v[0xf] = 1;
	} else {
		chip8->v[0xf] = 0;
	}

	chip8->v[x] = chip8->v[y] - chip8->v[x];
}

void CHIP8_8xye(CHIP8 *chip8, uint8_t x, uint8_t y) {
	chip8->v[0xf] = chip8->v[x] >> 7;

	chip8->v[x] <<= 1;
}

CHIP8Result CHIP8_9xy0(CHIP8 *chip8, uint8_t x, uint8_t y) {
	if (chip8->v[x] != chip8->v[y]) {
		if (chip8->pc > CHIP8_MEMORY_SIZE - 3) {
			CHIP8SetError(CHIP8_ERROR_SEGFAULT);
			return CHIP8_ERROR_SEGFAULT;
		}

		chip8->pc += 2;
	}

	return CHIP8_SUCCESS;
}

void CHIP8_annn(CHIP8 *chip8, uint16_t nnn) {
	chip8->i = nnn;
}

void CHIP8_bnnn(CHIP8 *chip8, uint16_t nnn) {
	chip8->pc = nnn + chip8->v[0x0];
}

void CHIP8_cxkk(CHIP8 *chip8, uint8_t x, uint8_t kk) {
	srand(time(NULL));
	chip8->v[x] = ((uint8_t) rand()) & kk; 
}

void CHIP8_dxyn(CHIP8 *chip8, uint8_t x, uint8_t y, uint8_t n) {
 	uint8_t pixelX = chip8->v[x] % 64;
    uint8_t pixelY = chip8->v[y] % 32;
    uint8_t height = n;

    chip8->v[0xf] = 0;

    for (uint8_t j = 0; j < height; ++j) {
        uint8_t spriteAddress = chip8->memory[chip8->i + j];

        for (uint8_t i = 0; i < 8; ++i) {
            if (spriteAddress & (0x80 >> i)) {
                if (chip8->display[pixelX + i][pixelY + j] == CHIP8_PIXEL_ON) {
                    chip8->v[0xf] = 1;
                }

                chip8->display[pixelX + i][pixelY + j] ^= CHIP8_PIXEL_ON;
            }
        }
    }
}

CHIP8Result CHIP8_ex9e(CHIP8 *chip8, uint8_t x) {
	if (chip8->v[x] >= CHIP8_NUM_KEYS) {
		CHIP8SetError(CHIP8_ERROR_KEY_NOT_FOUND);
		return CHIP8_ERROR_KEY_NOT_FOUND;
	}

	if (chip8->keyboard[chip8->v[x]] == CHIP8_KEY_PRESSED) {
		if (chip8->pc > CHIP8_MEMORY_SIZE - 3) {
			CHIP8SetError(CHIP8_ERROR_SEGFAULT);
			return CHIP8_ERROR_SEGFAULT;
		}

		chip8->pc += 2;
	}

	return CHIP8_SUCCESS;
}

CHIP8Result CHIP8_exa1(CHIP8 *chip8, uint8_t x) {
	if (chip8->v[x] >= CHIP8_NUM_KEYS) {
		CHIP8SetError(CHIP8_ERROR_KEY_NOT_FOUND);
		return CHIP8_ERROR_KEY_NOT_FOUND;
	}

	if (chip8->keyboard[chip8->v[x]] == CHIP8_KEY_NOT_PRESSED) {
		if (chip8->pc > CHIP8_MEMORY_SIZE - 3) {
			CHIP8SetError(CHIP8_ERROR_SEGFAULT);
			return CHIP8_ERROR_SEGFAULT;
		}

		chip8->pc += 2;
	}

	return CHIP8_SUCCESS;
}

void CHIP8_fx07(CHIP8 *chip8, uint8_t x) {
	chip8->v[x] = chip8->dt;
}

CHIP8Result CHIP8_fx0a(CHIP8 *chip8, uint8_t x) {
	for (uint8_t i = 0; i < CHIP8_NUM_KEYS; ++i) {
		if (chip8->keyboard[i] == CHIP8_KEY_PRESSED) {
			chip8->v[x] = chip8->keyboard[i];
			return CHIP8_SUCCESS;
		}
	}

	if (chip8->pc - 2 <= CHIP8_INTERPRETER_END_ADDRESS) {
		CHIP8SetError(CHIP8_ERROR_SEGFAULT);
		return CHIP8_ERROR_SEGFAULT;
	}

	chip8->pc -= 2;

	return CHIP8_SUCCESS;
}

void CHIP8_fx15(CHIP8 *chip8, uint8_t x) {
	chip8->dt = chip8->v[x];
}

void CHIP8_fx18(CHIP8 *chip8, uint8_t x) {
	chip8->st = chip8->v[x];
}

CHIP8Result CHIP8_fx1e(CHIP8 *chip8, uint8_t x) {
	if (chip8->i >= CHIP8_MEMORY_SIZE - chip8->v[x]) {
		CHIP8SetError(CHIP8_ERROR_SEGFAULT);
		return CHIP8_ERROR_SEGFAULT;
	}

	chip8->i += chip8->v[x];

	return CHIP8_SUCCESS;
}

CHIP8Result CHIP8_fx29(CHIP8 *chip8, uint8_t x) {
	if (chip8->v[x] > 15) {
		CHIP8SetError(CHIP8_ERROR_SEGFAULT);
		return CHIP8_ERROR_SEGFAULT;
	}

	chip8->i = CHIP8_FONTSET_START_ADDRESS + chip8->v[x] * 5;

	return CHIP8_SUCCESS;
}

void CHIP8_fx33(CHIP8 *chip8, uint8_t x) {
	uint8_t value = chip8->v[x];

	chip8->memory[chip8->i + 2] = value % 10;
	value /= 10;

	chip8->memory[chip8->i + 1] = value % 10;
	value /= 10;

	chip8->memory[chip8->i] = value % 10;
}

CHIP8Result CHIP8_fx55(CHIP8 *chip8, uint8_t x) {
	if (chip8->i > CHIP8_MEMORY_SIZE - x) {
		CHIP8SetError(CHIP8_ERROR_SEGFAULT);
		return CHIP8_ERROR_SEGFAULT;
	}

	for (uint8_t i = 0; i <= x; ++i) {
		chip8->memory[chip8->i + i] = chip8->v[i];
	}

	return CHIP8_SUCCESS;
}

CHIP8Result CHIP8_fx65(CHIP8 *chip8, uint8_t x) {
	if (chip8->i > CHIP8_MEMORY_SIZE - x) {
		CHIP8SetError(CHIP8_ERROR_SEGFAULT);
		return CHIP8_ERROR_SEGFAULT;
	}

	for (uint8_t i = 0; i <= x; ++i) {
		chip8->v[i] = chip8->memory[chip8->i + i];
	}

	return CHIP8_SUCCESS;
}