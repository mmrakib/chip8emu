#include "chip8.h"

const uint8_t fontset[FONTSET_SIZE] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void error(const char *message, bool fatal) {
    fprintf(stderr, "ERROR: %s\n", message);
    if (fatal) exit(1);
}

int random() {
    int byte = rand() % 255;
    return byte;
}

void initialise() {
    for (int i = 0; i < 16; i++) state.registers[i] = 0;
    for (int i = 0; i < 16; i++) state.stack[i] = 0;
    for (int i = 0; i < MEMORY_SIZE; i++) state.memory[i] = 0;

    state.index = 0;
    state.pc = ROM_START_ADDR;
    state.sp = 0;
    state.opcode = 0;

    state.delay_timer = 0;
    state.sound_timer = 0;

    for (int i = 0; i < 16; i++) state.keypad[i] = 0;
    for (int i = 0; i < SCREEN_SIZE; i++) state.display[i] = 0;

    // Init RNG
    srand(time(NULL));

    // Load fontset
    for (int i = 0; i < FONTSET_SIZE; i++) {
        state.memory[FONTSET_START_ADDR + i] = fontset[i];
    }
}

void loadROM(const char *filename) {
    FILE *fp;
    size_t fs;

    if ((fp = fopen(filename, "rb")) == NULL) error("Failed to open ROM file", true);

    fseek(fp, 0L, SEEK_END);
    fs = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    uint8_t *buffer = (uint8_t *)malloc(fs);

    if (fread(buffer, 1, fs, fp) < fs && !feof(fp)) error("Failed to read ROM file", true);

    for (long i = 0; i < fs; i++) {
        state.memory[ROM_START_ADDR + i] = buffer[i];
    }

    free(buffer);
    fclose(fp);
}

void cycle() {
    
}

// 
// Instructions
// 


