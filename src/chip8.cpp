#include "chip8.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

CHIP8::CHIP8(float cyclesPerSecond) : cyclesPerSecond(cyclesPerSecond)
{
    Reset();
    LoadFont();
    srand(time(0));
}

void CHIP8::LoadROM(uint8_t *romData, int romSize)
{
    if (romSize > (sizeof(memory) - PROGRAM_START))
    {
        fprintf(stderr, "ROM too large to fit in memory!");
        return;
    }

    Reset();
    memcpy(memory + PROGRAM_START, romData, romSize);
}

void CHIP8::KeyPressed(int key)
{
    keys[key] = true;

    switch (keyWaitState)
    {
    case Waiting:
        keyWaitState = KeyRecieved;
        recievedKey = key;
        break;

    default:
        break;
    }
}

void CHIP8::KeyReleased(int key)
{
    keys[key] = false;
}

bool CHIP8::GetPixel(int x, int y)
{
    return display[x][y];
}

void CHIP8::Advance(float dt)
{
    const float tickInterval = 1 / 60.0;
    const float cyclesPerTick = cyclesPerSecond * tickInterval;

    tickTimeAccumulator += dt;
    for (; tickTimeAccumulator > tickInterval; tickTimeAccumulator -= tickInterval)
    {
        TickTimers();

        cyclesAccumulator += cyclesPerTick;

        int cycleCount = cyclesAccumulator;
        cyclesAccumulator -= cycleCount;
        for (int i = 0; i < cycleCount; ++i)
        {
            ExecuteNextInstruction();
        }
    }
}

void CHIP8::Step(bool advanceTime)
{
    //NOTE: Useful for debuggers
    //      Execute one cycle and move forward that much time towards the internal timers.

    if (advanceTime)
    {
        const float tickInterval = 1 / 60.0;
        const float secondsPerCycle = 1 / cyclesPerSecond;

        tickTimeAccumulator += secondsPerCycle;
        for (; tickTimeAccumulator > tickInterval; tickTimeAccumulator -= tickInterval)
            TickTimers();
    }

    ExecuteNextInstruction();
}

//
// Private Methods
//

void CHIP8::Reset()
{
    memset(memory + PROGRAM_START, 0, sizeof(memory) - PROGRAM_START);
    memset(V, 0, sizeof(V));

    I = 0;
    PC = PROGRAM_START;
    SP = 0;
    delayTimer = 0;
    soundTimer = 0;

    memset(keys, 0, sizeof(keys));
    keyWaitState = NotWaiting;

    ClearDisplay();

    tickTimeAccumulator = 0.0;
    cyclesAccumulator = 0.0;
}

void CHIP8::LoadFont()
{
    uint8_t fontData[] =
        {
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
            0xF0, 0x80, 0xF0, 0x80, 0x80, // F
        };

    memcpy(memory, fontData, sizeof(uint8_t) * 5 * 16);
}

void CHIP8::ClearDisplay()
{
    memset(display, 0, sizeof(display));
}

void CHIP8::TickTimers()
{
    if (delayTimer > 0)
        --delayTimer;

    if (soundTimer > 0)
        --soundTimer;
}

void CHIP8::ExecuteNextInstruction()
{
    // Fetch instruction
    uint16_t inst = memory[PC] << 8 | memory[PC + 1];
    PC += 2;

    DecodeAndExecute(inst);
}

//
// Decode
//

void CHIP8::DecodeAndExecute(uint16_t inst)
{
    switch ((inst & 0xF000) >> 12)
    {
    case 0x0:
        DecodeAndExecute0(inst);
        break;
    case 0x1:
        _1NNN(inst);
        break;
    case 0x2:
        _2NNN(inst);
        break;
    case 0x3:
        _3XNN(inst);
        break;
    case 0x4:
        _4XNN(inst);
        break;
    case 0x5:
        _5XY0(inst);
        break;
    case 0x6:
        _6XNN(inst);
        break;
    case 0x7:
        _7XNN(inst);
        break;
    case 0x8:
        DecodeAndExecute8(inst);
        break;
    case 0x9:
        _9XY0(inst);
        break;
    case 0xA:
        _ANNN(inst);
        break;
    case 0xB:
        _BNNN(inst);
        break;
    case 0xC:
        _CXNN(inst);
        break;
    case 0xD:
        _DXYN(inst);
        break;
    case 0xE:
        DecodeAndExecuteE(inst);
        break;
    case 0xF:
        DecodeAndExecuteF(inst);
        break;
    }
}

void CHIP8::DecodeAndExecute0(uint16_t inst)
{
    switch (inst & 0xFF)
    {
    case 0xE0:
        _00E0(inst);
        break;
    case 0xEE:
        _00EE(inst);
        break;
    default:
        _0NNN(inst);
        break;
    }
}

void CHIP8::DecodeAndExecute8(uint16_t inst)
{
    switch (inst & 0xF)
    {
    case 0x0:
        _8XY0(inst);
        break;
    case 0x1:
        _8XY1(inst);
        break;
    case 0x2:
        _8XY2(inst);
        break;
    case 0x3:
        _8XY3(inst);
        break;
    case 0x4:
        _8XY4(inst);
        break;
    case 0x5:
        _8XY5(inst);
        break;
    case 0x6:
        _8XY6(inst);
        break;
    case 0x7:
        _8XY7(inst);
        break;
    case 0xE:
        _8XYE(inst);
        break;
    default:
        InvalidOpcode(inst);
        break;
    }
}

void CHIP8::DecodeAndExecuteE(uint16_t inst)
{
    switch (inst & 0xFF)
    {
    case 0x9E:
        _EX9E(inst);
        break;
    case 0xA1:
        _EXA1(inst);
        break;
    default:
        InvalidOpcode(inst);
        break;
    }
}

void CHIP8::DecodeAndExecuteF(uint16_t inst)
{
    switch (inst & 0xFF)
    {
    case 0x07:
        _FX07(inst);
        break;
    case 0x0A:
        _FX0A(inst);
        break;
    case 0x15:
        _FX15(inst);
        break;
    case 0x18:
        _FX18(inst);
        break;
    case 0x1E:
        _FX1E(inst);
        break;
    case 0x29:
        _FX29(inst);
        break;
    case 0x33:
        _FX33(inst);
        break;
    case 0x55:
        _FX55(inst);
        break;
    case 0x65:
        _FX65(inst);
        break;
    default:
        InvalidOpcode(inst);
        break;
    }
}

//
// Opcodes
//

void CHIP8::_0NNN(uint16_t inst)
{
    // Not needed by most ROMs
}

void CHIP8::_00E0(uint16_t inst)
{
    ClearDisplay();
}

void CHIP8::_00EE(uint16_t inst)
{
    if (SP <= 0)
        return;

    PC = stack[--SP];
}

void CHIP8::_1NNN(uint16_t inst)
{
    int nnn = inst & 0xFFF;
    PC = nnn;
}

void CHIP8::_2NNN(uint16_t inst)
{
    if (SP >= STACK_MAX)
        return;

    int nnn = inst & 0xFFF;

    stack[SP++] = PC;
    PC = nnn;
}

void CHIP8::_3XNN(uint16_t inst)
{
    int x = (inst & 0x0F00) >> 8;
    int nn = inst & 0xFF;

    if (V[x] == nn)
        PC += 2;
}

void CHIP8::_4XNN(uint16_t inst)
{
    int x = (inst & 0x0F00) >> 8;
    int nn = inst & 0xFF;

    if (V[x] != nn)
        PC += 2;
}

void CHIP8::_5XY0(uint16_t inst)
{
    int x = (inst & 0x0F00) >> 8;
    int y = (inst & 0x00F0) >> 4;

    if (V[x] == V[y])
        PC += 2;
}

void CHIP8::_6XNN(uint16_t inst)
{
    int x = (inst & 0x0F00) >> 8;
    int nn = inst & 0xFF;

    V[x] = nn;
}

void CHIP8::_7XNN(uint16_t inst)
{
    int x = (inst & 0x0F00) >> 8;
    int nn = inst & 0xFF;

    V[x] += nn;
}

void CHIP8::_8XY0(uint16_t inst)
{
    int x = (inst & 0x0F00) >> 8;
    int y = (inst & 0x00F0) >> 4;

    V[x] = V[y];
}

void CHIP8::_8XY1(uint16_t inst)
{
    int x = (inst & 0x0F00) >> 8;
    int y = (inst & 0x00F0) >> 4;

    V[x] |= V[y];
}

void CHIP8::_8XY2(uint16_t inst)
{
    int x = (inst & 0x0F00) >> 8;
    int y = (inst & 0x00F0) >> 4;

    V[x] &= V[y];
}

void CHIP8::_8XY3(uint16_t inst)
{
    int x = (inst & 0x0F00) >> 8;
    int y = (inst & 0x00F0) >> 4;

    V[x] ^= V[y];
}

void CHIP8::_8XY4(uint16_t inst)
{
    int x = (inst & 0x0F00) >> 8;
    int y = (inst & 0x00F0) >> 4;

    VF = (V[x] + V[y]) > 255;
    V[x] += V[y];
}

void CHIP8::_8XY5(uint16_t inst)
{
    int x = (inst & 0x0F00) >> 8;
    int y = (inst & 0x00F0) >> 4;

    VF = V[y] > V[x];
    V[x] -= V[y];
}

void CHIP8::_8XY6(uint16_t inst)
{
    int x = (inst & 0x0F00) >> 8;
    int y = (inst & 0x00F0) >> 4;

    VF = V[x] & 1;
    V[x] >>= 1;
}

void CHIP8::_8XY7(uint16_t inst)
{
    int x = (inst & 0x0F00) >> 8;
    int y = (inst & 0x00F0) >> 4;

    VF = V[x] > V[y];
    V[x] = V[y] - V[x];
}

void CHIP8::_8XYE(uint16_t inst)
{
    int x = (inst & 0x0F00) >> 8;
    int y = (inst & 0x00F0) >> 4;

    VF = V[x] & 0x80;
    V[x] <<= 1;
}

void CHIP8::_9XY0(uint16_t inst)
{
    int x = (inst & 0x0F00) >> 8;
    int y = (inst & 0x00F0) >> 4;

    if (V[x] != V[y])
        PC += 2;
}

void CHIP8::_ANNN(uint16_t inst)
{
    int nnn = inst & 0xFFF;
    I = nnn;
}

void CHIP8::_BNNN(uint16_t inst)
{
    int nnn = inst & 0xFFF;
    PC = V0 + nnn;
}

void CHIP8::_CXNN(uint16_t inst)
{
    int x = (inst & 0x0F00) >> 8;
    int nn = inst & 0xFF;

    V[x] = (rand() % 255) & nn;
}

void CHIP8::_DXYN(uint16_t inst)
{
    int x = (inst & 0x0F00) >> 8;
    int y = (inst & 0x00F0) >> 4;
    int n = inst & 0xF;

    VF = 0;

    for (int i = 0; i < n; ++i)
    {
        uint8_t screenY = V[y] + i;
        if (screenY >= DISPLAY_HEIGHT)
            continue;

        uint8_t spriteRow = memory[I + i];
        for (int j = 0; j < 8; ++j)
        {
            uint8_t screenX = V[x] + j;
            if (screenX >= DISPLAY_WIDTH)
                continue;

            uint8_t spritePixel = spriteRow & (0x80 >> j);
            if (spritePixel)
            {
                // XOR screen with sprite
                // VF set to true if any pixel is already on and turned off
                VF |= display[screenX][screenY];
                display[screenX][screenY] = !display[screenX][screenY];
            }
        }
    }
}

void CHIP8::_EX9E(uint16_t inst)
{
    int x = (inst & 0x0F00) >> 8;

    if (V[x] < 16 && keys[V[x]])
        PC += 2;
}

void CHIP8::_EXA1(uint16_t inst)
{
    int x = (inst & 0x0F00) >> 8;

    if (V[x] < 16 && !keys[V[x]])
        PC += 2;
}

void CHIP8::_FX07(uint16_t inst)
{
    int x = (inst & 0x0F00) >> 8;
    V[x] = delayTimer;
}

void CHIP8::_FX0A(uint16_t inst)
{
    int x = (inst & 0x0F00) >> 8;

    switch (keyWaitState)
    {
    case NotWaiting:
        keyWaitState = Waiting;
    case Waiting:
        PC -= 2;
        break;
    case KeyRecieved:
        V[x] = recievedKey;
        keyWaitState = NotWaiting;
        break;
    }
}

void CHIP8::_FX15(uint16_t inst)
{
    int x = (inst & 0x0F00) >> 8;
    delayTimer = V[x];
}

void CHIP8::_FX18(uint16_t inst)
{
    int x = (inst & 0x0F00) >> 8;
    soundTimer = V[x];
}

void CHIP8::_FX1E(uint16_t inst)
{
    int x = (inst & 0x0F00) >> 8;
    I += V[x];
}

void CHIP8::_FX29(uint16_t inst)
{
    int x = (inst & 0x0F00) >> 8;
    I = (V[x] & 0xF) * 5;
}

void CHIP8::_FX33(uint16_t inst)
{
    int x = (inst & 0x0F00) >> 8;

    int hundreds = (V[x] / 100) % 10;
    int tens = (V[x] / 10) % 10;
    int ones = V[x] % 10;

    memory[I] = hundreds;
    memory[I + 1] = tens;
    memory[I + 2] = ones;
}

void CHIP8::_FX55(uint16_t inst)
{
    int x = (inst & 0x0F00) >> 8;

    for (int i = 0; i <= x; ++i)
        memory[I + i] = V[i];
}

void CHIP8::_FX65(uint16_t inst)
{
    int x = (inst & 0x0F00) >> 8;

    for (int i = 0; i <= x; ++i)
        V[i] = memory[I + i];
}

void CHIP8::InvalidOpcode(uint16_t inst)
{
    fprintf(stderr, "Invalid opcode: %04X\n", inst);
}
