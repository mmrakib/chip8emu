#ifndef CHIP8_H
#define CHIP8_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define SCREEN_SIZE 64 * 32
#define MEMORY_SIZE 4096
#define FONTSET_SIZE 80

#define FLAG_REGISTER 0xF

#define FONTSET_START_ADDR 0x50
#define ROM_START_ADDR 0x200

const uint8_t fontset[FONTSET_SIZE];

struct Chip8 {
    uint8_t registers[16];
    uint16_t stack[16];
    uint8_t memory[MEMORY_SIZE];

    uint16_t index;
    uint16_t pc;
    uint8_t sp;
    uint16_t opcode;

    uint8_t delay_timer;
    uint8_t sound_timer;

    uint8_t keypad[16];
    uint32_t display[SCREEN_SIZE];
} state;

void error(const char *message, bool fatal);
int random();

void initialise();
void loadROM(const char *filename);
void cycle();

// Instructions (named after opcodes)
void OP_NULL(); // Not a real CHIP-8 instruction, placeholder that does nothing
void OP_00E0(); // Clear display
void OP_00EE(); // Return from subroutine
void OP_1NNN(); // Jump to address
void OP_2NNN(); // Call subroutine at address
void OP_3XKK(); // Skip next instruction if register == byte
void OP_4XKK(); // Skip next instruction if register != byte
void OP_5XY0(); // Skip next instruction if register X == register Y
void OP_6XKK(); // Set register = byte
void OP_7XKK(); // ADD byte to register
void OP_8XY0(); // Set register X = register Y
void OP_8XY1(); // OR register X and register Y, set register X
void OP_8XY2(); // AND register X and register Y, set register X
void OP_8XY3(); // XOR register X and register Y, set register X
void OP_8XY4(); // ADD register X to register Y (VF set as carry)
void OP_8XY5(); // SUB register Y from register X (VF set as borrow)
void OP_8XY6(); // SHR (shift right) register X by 1 (VF set as carry if LSB is 1)
void OP_8XY7(); // SUBN register Y from register X (VF set as NOT borrow)
void OP_8XYE(); // SHL (shift left) register X by 1 (VF set as carry if MSB is 1)
void OP_9XY0(); // Skip next instruction if register X != register Y
void OP_ANNN(); // Set I to address
void OP_BNNN(); // Jump to address + offset at V0
void OP_CXKK(); // Set register X as random byte AND byte
void OP_DXYN(); // Draw n-byte sprite starting from addr I at (VX, VY), set VF as collision
void OP_EX9E(); // Skip next instruction if key with value VX is pressed
void OP_EXA1(); // Skip next instruction if key with value VX is NOT pressed
void OP_FX07(); // Set VX = delay timer
void OP_FX0A(); // Wait for key press, set value of pressed key in VX
void OP_FX15(); // Set delay timer = VX
void OP_FX18(); // Set sound timer = VX
void OP_FX1E(); // Set I as I + offset at VX 
void OP_FX29(); // Set I = location of sprite for font character VX
void OP_FX33(); // Store BCD representation of VX in I (hundreds), I + 1 (tens), I + 2 (ones)
void OP_FX55(); // Store registers V0-VF in memory location starting at I
void OP_FX65(); // Read registers V0-VX from memory location starting at I

#endif
