#pragma once

#include <stdint.h>

class CHIP8
{
public:
    CHIP8(float cyclesPerSecond = 1000);

    void LoadROM(uint8_t *romData, int romSize);

    void KeyPressed(int key);
    void KeyReleased(int key);
    bool GetPixel(int x, int y);

    void Advance(float dt);
    void Step(bool advanceTime = true);

    // Constants
    static const int DISPLAY_WIDTH = 64;
    static const int DISPLAY_HEIGHT = 32;

private:
    void Reset();
    void LoadFont();

    void ClearDisplay();
    void TickTimers();

    void ExecuteNextInstruction();

    // Decode
    void DecodeAndExecute(uint16_t inst);

    void DecodeAndExecute0(uint16_t inst);
    void DecodeAndExecute8(uint16_t inst);
    void DecodeAndExecuteE(uint16_t inst);
    void DecodeAndExecuteF(uint16_t inst);

    // Opcodes
    void _0NNN(uint16_t inst);
    void _00E0(uint16_t inst);
    void _00EE(uint16_t inst);
    void _1NNN(uint16_t inst);
    void _2NNN(uint16_t inst);
    void _3XNN(uint16_t inst);
    void _4XNN(uint16_t inst);
    void _5XY0(uint16_t inst);
    void _6XNN(uint16_t inst);
    void _7XNN(uint16_t inst);
    void _8XY0(uint16_t inst);
    void _8XY1(uint16_t inst);
    void _8XY2(uint16_t inst);
    void _8XY3(uint16_t inst);
    void _8XY4(uint16_t inst);
    void _8XY5(uint16_t inst);
    void _8XY6(uint16_t inst);
    void _8XY7(uint16_t inst);
    void _8XYE(uint16_t inst);
    void _9XY0(uint16_t inst);
    void _ANNN(uint16_t inst);
    void _BNNN(uint16_t inst);
    void _CXNN(uint16_t inst);
    void _DXYN(uint16_t inst);
    void _EX9E(uint16_t inst);
    void _EXA1(uint16_t inst);
    void _FX07(uint16_t inst);
    void _FX0A(uint16_t inst);
    void _FX15(uint16_t inst);
    void _FX18(uint16_t inst);
    void _FX1E(uint16_t inst);
    void _FX29(uint16_t inst);
    void _FX33(uint16_t inst);
    void _FX55(uint16_t inst);
    void _FX65(uint16_t inst);

    void InvalidOpcode(uint16_t inst);

private:
    // Timing
    const float cyclesPerSecond;
    float tickTimeAccumulator;
    float cyclesAccumulator;

    /* CHIP-8 Internals */
    // Memory
    uint8_t memory[4096];
    static const int PROGRAM_START = 512;

    // Registers
    union
    {
        uint8_t V[16];
        struct
        {
            uint8_t V0;
            uint8_t _[14];
            uint8_t VF;
        };
    };

    uint16_t I;
    uint16_t PC;

    // Stack
    static const int STACK_MAX = 128;
    uint16_t stack[STACK_MAX];
    int SP;

    // Timers
    uint8_t delayTimer;
    uint8_t soundTimer;

    // Input
    bool keys[16];
    uint8_t recievedKey;
    enum
    {
        NotWaiting,
        Waiting,
        KeyRecieved
    } keyWaitState;

    // Graphics
    uint8_t display[DISPLAY_WIDTH][DISPLAY_HEIGHT];
};