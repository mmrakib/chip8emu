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

void load_rom(const char *filename) {
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
    // Fetch
    state.opcode = (state.memory[state.pc] << 8) | state.memory[state.pc + 1];

    // Increment PC
    state.pc += 2;

    // Decode
    void (*instruction)(void);

    switch ((state.opcode & 0xF000) >> 12) {
        case 0x0:
            switch (state.opcode & 0xF) {
                case 0x0:
                    instruction = &OP_00E0;
                    break;

                case 0xE:
                    instruction = &OP_00EE;
                    break;

                default:
                    instruction = &OP_NULL;
            }

        case 0x1:
            instruction = &OP_1NNN;
            break;

        case 0x2:
            instruction = &OP_2NNN;
            break;

        case 0x3:
            instruction = &OP_3XKK;
            break;

        case 0x4:
            instruction = &OP_4XKK;
            break;

        case 0x5:
            instruction = &OP_5XY0;
            break;

        case 0x6: 
            instruction = &OP_6XKK;
            break;

        case 0x7:
            instruction = &OP_7XKK;
            break;

        case 0x8:
            switch (state.opcode & 0xF) {
                case 0x0:
                    instruction = &OP_8XY0;
                    break;

                case 0x1:
                    instruction = &OP_8XY1;
                    break;

                case 0x2:
                    instruction = &OP_8XY2;
                    break;

                case 0x3:
                    instruction = &OP_8XY3;
                    break;

                case 0x4:
                    instruction = &OP_8XY4;
                    break;

                case 0x5:
                    instruction = &OP_8XY5;
                    break;

                case 0x6:
                    instruction = &OP_8XY6;
                    break;

                case 0x7:
                    instruction = &OP_8XY7;
                    break;

                case 0xE:
                    instruction = &OP_8XYE;
                    break;

                default:
                    instruction = &OP_NULL;
            }

        case 0x9:
            instruction = &OP_9XY0;
            break;

        case 0xA:
            instruction = &OP_ANNN;
            break;

        case 0xB:
            instruction = &OP_BNNN;
            break;

        case 0xC:
            instruction = &OP_CXKK;
            break;

        case 0xD:
            instruction = &OP_DXYN;
            break;

        case 0xE:
            switch (state.opcode & 0xFF) {
                case 0x9E:
                    instruction = &OP_EX9E;
                    break;

                case 0xA1:
                    instruction = &OP_EXA1;
                    break;

                default:
                    instruction = &OP_NULL;
            }

        case 0xF:
            switch (state.opcode & 0xFF) {
                case 0x07:
                    instruction = &OP_FX07;
                    break;

                case 0x0A:
                    instruction = &OP_FX0A;
                    break;

                case 0x15:
                    instruction = &OP_FX15;
                    break;

                case 0x18:
                    instruction = &OP_FX18;
                    break;

                case 0x1E:
                    instruction = &OP_FX1E;
                    break;

                case 0x29:
                    instruction = &OP_FX29;
                    break;

                case 0x33:
                    instruction = &OP_FX33;
                    break;

                case 0x55:
                    instruction = &OP_FX55;
                    break;

                case 0x65:
                    instruction = &OP_FX65;
                    break;

                default:
                    instruction = &OP_NULL;
            }

        default:
            instruction = &OP_NULL;
    }

    // Execute
    instruction();

    // Decrement timers
    if (state.delay_timer > 0) state.delay_timer--;
    if (state.sound_timer > 0) state.sound_timer--;
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
    uint16_t addr = state.opcode & 0x0FFF; // Calc dest addr
    state.pc = addr + state.registers[0]; // Set PC to dest addr + offset
}

void OP_CXKK() {
    uint8_t vx = (state.opcode & 0x0F00) >> 8; // Get register number
    uint8_t byte = state.opcode & 0x00FF; // Get literal byte

    state.registers[vx] = random() & byte; // Set register random byte AND literal byte
}

void OP_DXYN() {
    uint8_t vx = (state.opcode & 0x0F00) >> 8;
    uint8_t vy = (state.opcode & 0x00F0) >> 4;

    uint8_t height = state.opcode & 0x000F;

    uint8_t x_pos = state.registers[vx] % SCREEN_WIDTH;
    uint8_t y_pos = state.registers[vy] % SCREEN_HEIGHT;

    state.registers[FLAG_REGISTER] = 0;

    for (unsigned int row = 0; row < height; row++) {
        uint8_t spriteByte = state.memory[state.index + row];

        for (unsigned int col = 0; col < 8; col++) {
            uint8_t spritePixel = spriteByte & (0x80 >> col);
            uint32_t *screenPixel = &state.display[(y_pos + row) * SCREEN_WIDTH + (x_pos + col)];

            if (spritePixel) {
                if (*screenPixel == 0xFFFFFFFF) state.registers[FLAG_REGISTER] = 1;
                *screenPixel ^= 0xFFFFFFFF;
            }
        }
    }
}

void OP_EX9E() {
    uint8_t vx = (state.opcode & 0x0F00) >> 8;
    uint8_t key = state.registers[vx];

    if (state.keypad[key]) state.pc += 2;
}

void OP_EXA1() {
    uint8_t vx = (state.opcode & 0x0F00) >> 8;
    uint8_t key = state.registers[vx];

    if (!state.keypad[key]) state.pc += 2;
}

void OP_FX07() {
    uint8_t vx = (state.opcode & 0x0F00) >> 8;

    state.registers[vx] = state.delay_timer;
}

void OP_FX0A() {
    uint8_t vx = (state.opcode & 0x0F00) >> 8;

    if (state.keypad[0x0]) {
        state.registers[vx] = 0x0;
    } else if (state.keypad[0x1]) {
        state.registers[vx] = 0x1;
    } else if (state.keypad[0x2]) {
        state.registers[vx] = 0x2;
    } else if (state.keypad[0x3]) {
        state.registers[vx] = 0x3;
    } else if (state.keypad[0x4]) {
        state.registers[vx] = 0x4;
    } else if (state.keypad[0x5]) {
        state.registers[vx] = 0x5;
    } else if (state.keypad[0x6]) {
        state.registers[vx] = 0x6;
    } else if (state.keypad[0x7]) {
        state.registers[vx] = 0x7;
    } else if (state.keypad[0x8]) {
        state.registers[vx] = 0x8;
    } else if (state.keypad[0x9]) {
        state.registers[vx] = 0x9;
    } else if (state.keypad[0xA]) {
        state.registers[vx] = 0xA;
    } else if (state.keypad[0xB]) {
        state.registers[vx] = 0xB;
    } else if (state.keypad[0xC]) {
        state.registers[vx] = 0xC;
    } else if (state.keypad[0xD]) {
        state.registers[vx] = 0xD;
    } else if (state.keypad[0xE]) {
        state.registers[vx] = 0xE;
    } else if (state.keypad[0xF]) {
        state.registers[vx] = 0xF;
    } else {
        state.pc -= 2;
    }
}

void OP_FX15() {
    uint8_t vx = (state.opcode & 0x0F00) >> 8;

    state.delay_timer = state.registers[vx];
}

void OP_FX18() {
    uint8_t vx = (state.opcode & 0x0F00) >> 8;

    state.sound_timer = state.registers[vx];
}

void OP_FX1E() {
    uint8_t vx = (state.opcode & 0x0F00) >> 8;

    state.index = state.registers[vx];
}

void OP_FX29() {
    uint8_t vx = (state.opcode & 0x0F00) >> 8;
    uint8_t digit = state.registers[vx];

    state.index = FONTSET_START_ADDR + (digit * 5);
}

void OP_FX33() {
    uint8_t vx = (state.opcode & 0x0F00) >> 8;
    uint8_t value = state.registers[vx];

    state.memory[state.index + 2] = value % 10;
    value /= 10;

    state.memory[state.index + 1] = value % 10;
    value /= 10;

    state.memory[state.index] = value % 10;
}

void OP_FX55() {
    uint8_t vx = (state.opcode & 0x0F00) >> 8;

    for (uint8_t i = 0; i <= vx; i++) state.memory[state.index + i] = state.registers[i];
}

void OP_FX65() {
    uint8_t vx = (state.opcode & 0x0F00) >> 8;

    for (uint8_t i = 0; i <= vx; i++) state.registers[i] = state.memory[state.index + i];
}
