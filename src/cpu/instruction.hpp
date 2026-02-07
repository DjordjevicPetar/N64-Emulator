#pragma once

#include "../utils/types.hpp"

namespace n64::cpu {

union Instruction {

    u32 raw;
    struct {
        u32 immediate : 16;   // [15:0]
        u32 rt : 5;           // [20:16]
        u32 rs : 5;           // [25:21]
        u32 opcode : 6;       // [31:26]
    } i_type;

    struct {
        u32 target_address : 26;  // [25:0]
        u32 opcode : 6;           // [31:26]
    } j_type;

    struct {
        u32 funct : 6;            // [5:0]
        u32 shift_amount : 5;     // [10:6]
        u32 rd : 5;               // [15:11]
        u32 rt : 5;               // [20:16]
        u32 rs : 5;               // [25:21]
        u32 opcode : 6;           // [31:26]
    } r_type;

    // COP1 (FPU) arithmetic format: ADD.S, MUL.D, etc.
    struct {
        u32 funct : 6;            // [5:0]   - function (0=ADD, 1=SUB, 2=MUL, etc.)
        u32 fd : 5;               // [10:6]  - destination FPR
        u32 fs : 5;               // [15:11] - source FPR
        u32 ft : 5;               // [20:16] - source 2 FPR
        u32 fmt : 5;              // [25:21] - format (16=S, 17=D, 20=W, 21=L)
        u32 opcode : 6;           // [31:26] - 010001 (COP1)
    } cop1_type;

    // COP1 branch format: BC1T, BC1F
    struct {
        u32 offset : 16;          // [15:0]  - branch offset
        u32 tf : 1;               // [16]    - true/false (0=BC1F, 1=BC1T)
        u32 nd : 1;               // [17]    - nullify delay (0=normal, 1=likely)
        u32 : 3;                  // [20:18] - unused
        u32 sub : 5;              // [25:21] - 01000 (BC)
        u32 opcode : 6;           // [31:26] - 010001 (COP1)
    } cop1_bc_type;

    Instruction(u32 value) : raw(value) {}
    explicit Instruction() : raw(0) {}
};

}