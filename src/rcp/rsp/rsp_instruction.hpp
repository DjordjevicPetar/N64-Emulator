#pragma once

#include "../../utils/types.hpp"

namespace n64::rcp {

union RSPInstruction {

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

    struct {
        u32 funct : 6;            // [5:0]
        u32 vd : 5;               // [10:6]
        u32 vs : 5;               // [15:11]
        u32 vt : 5;               // [20:16]
        u32 e : 4;                // [24:21]
        u32 one : 1;              // [25]
        u32 opcode : 6;           // [31:26]
    } v_type;
    
    struct {
        u32 offset : 7;
        u32 element : 4;
        u32 opcode : 5;
        u32 vt : 5;
        u32 base : 5;
        u32 prefix : 6;
    } l_type;

    RSPInstruction(u32 value) : raw(value) {}
    explicit RSPInstruction() : raw(0) {}
};

}