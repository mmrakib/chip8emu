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
    // TODO
}

// 
// Instructions
// 

void OP_NULL() {
    return;
}

void OP_00E0() {
    // Sets all display pixels to 0, thus clearing
    for (int i = 0; i < SCREEN_SIZE; i++) {
        state.display[i] = 0;
    }
}

void OP_00EE() {
    state.sp--; // Pops stack
    state.pc = state.stack[state.sp]; // Sets PC to top of stack
}

void OP_1NNN() {
    uint16_t addr = state.opcode & 0x0FFF; // Get dest addr as NNN bits
    state.pc = addr; // Sets PC to dest addr
}

void OP_2NNN() {
    uint16_t addr = state.opcode & 0x0FFF; // Get dest addr as NNN bits
    state.stack[state.sp] = state.pc; // Save instruction after CALL on top of stack
    state.sp++; // Pushes stack
    state.pc = addr; // Set next instruction to dest addr
}

void OP_3XKK() {
    uint8_t vx = (state.opcode & 0x0F00) >> 8; // Get register number
    uint8_t byte = state.opcode & 0x00FF; // Get literal byte

    // If equal, skip instruction
    if (state.registers[vx] == byte) {
        state.pc += 2; 
    }
}

void OP_4XKK() {
    uint8_t vx = (state.opcode & 0x0F00) >> 8; // Get register number
    uint8_t byte = state.opcode & 0x00FF; // Get literal byte

    // If *not* equal, skip instruction
    if (state.registers[vx] != byte) {
        state.pc += 2;
    }
}

void OP_5XY0() {
    uint8_t vx = (state.opcode & 0x0F00) >> 8; // Get register number X
    uint8_t vy = (state.opcode & 0x00F0) >> 4; // Get register number Y

    // If register X == register Y, skip instruction
    if (state.registers[vx] == state.registers[vy]) {
        state.pc += 2;
    }
}

void OP_6XKK() {
    uint8_t vx = (state.opcode & 0x0F00) >> 8; // Get register number
    uint8_t byte = state.opcode & 0x00FF; // Get literal byte

    // Load byte into register
    state.registers[vx] = byte;
}

void OP_7XKK() {
    uint8_t vx = (state.opcode & 0x0F00) >> 8; // Get register number
    uint8_t byte = state.opcode & 0x00FF; // Get literal byte

    // ADD byte into register
    state.registers[vx] += byte;
}

void OP_8XY0() {
    uint8_t vx = (state.opcode & 0x0F00) >> 8; // Get register X
    uint8_t vy = (state.opcode & 0x00F0) >> 4; // Get register Y

    // Load register Y into register X
    state.registers[vx] = state.registers[vy];
}

void OP_8XY1() {
    uint8_t vx = (state.opcode & 0x0F00) >> 8; // Get register X
    uint8_t vy = (state.opcode & 0x00F0) >> 4; // Get register Y

    // OR byte into register
    state.registers[vx] |= state.registers[vy];
}

void OP_8XY2() {
    uint8_t vx = (state.opcode & 0x0F00) >> 8; // Get register X
    uint8_t vy = (state.opcode & 0x00F0) >> 4; // Get register Y

    // AND byte into register
    state.registers[vx] &= state.registers[vy];
}

void OP_8XY3() {
    uint8_t vx = (state.opcode & 0x0F00) >> 8; // Get register X
    uint8_t vy = (state.opcode & 0x00F0) >> 4; // Get register Y

    // XOR byte into register
    state.registers[vx] ^= state.registers[vy];
}

void OP_8XY4() {
    uint8_t vx = (state.opcode & 0x0F00) >> 8; // Get register X
    uint8_t vy = (state.opcode & 0x00F0) >> 4; // Get register Y

    // Calculate whole sum
    uint16_t sum = state.registers[vx] + state.registers[vy];

    // If whole sum greater than 255 (1 byte), set carry bit in flag register
    if (sum > 255) state.registers[FLAG_REGISTER] = 1;
    else           state.registers[FLAG_REGISTER] = 0;

    // Save least significant byte to VX
    state.registers[vx] = sum & 0xFF;
}

void OP_8XY5() {
    uint8_t vx = (state.opcode & 0x0F00) >> 8; // Get register X
    uint8_t vy = (state.opcode & 0x00F0) >> 4; // Get register Y

    if (state.registers[vx] > state.registers[vy]) state.registers[FLAG_REGISTER] = 1;
    else                                           state.registers[FLAG_REGISTER] = 0;

    state.registers[vx] = state.registers[vx] - state.registers[vy];
}

void OP_8XY6() {
    uint8_t vx = (state.opcode & 0x0F00) >> 8; // Get register X

    state.registers[FLAG_REGISTER] = (state.registers[vx] & 0x1);

    state.registers[vx] >>= 1;
}

void OP_8XY7() {
    uint8_t vx = (state.opcode & 0x0F00) >> 8; // Get register X
    uint8_t vy = (state.opcode & 0x00F0) >> 4; // Get register Y

    if (state.registers[vy] > state.registers[vx]) state.registers[FLAG_REGISTER] = 1;
    else                                           state.registers[FLAG_REGISTER] = 0;

    state.registers[vx] = state.registers[vy] - state.registers[vx];
}

void OP_8XYE() {
    uint8_t vx = (state.opcode & 0x0F00) >> 8; // Get register X

    // Save MSB in VF
    state.registers[FLAG_REGISTER] = (state.registers[vx] & 0x80) >> 7;

    state.registers[vx] <<= 1;
}

void OP_9XY0() {
    uint8_t vx = (state.opcode & 0x0F00) >> 8; // Get register X
    uint8_t vy = (state.opcode & 0x00F0) >> 4; // Get register Y

    // Skip to next instruction if not equal
    if (state.registers[vx] != state.registers[vy]) state.pc += 2;
}

void OP_ANNN() {
    uint16_t addr = state.opcode & 0x0FFF; // Calc dest addr
    state.index = addr; // Set index to dest addr
}

void OP_BNNN() {
    
}


