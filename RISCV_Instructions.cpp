#include <bits/stdc++.h>
#include <fstream>
#include <sstream>
#include <functional>
#include "RISCV_Constants.h"
using namespace std;

vector<int64_t> reg(32, 0);


//--------------------------------------------------------------------------------------------
unordered_map<string, int> inst_type = { {"add", R}, {"sub", R}, {"sll", R}, {"slt", R}, {"sltu", R}, {"xor", R}, {"srl", R}, {"sra", R}, {"or", R}, {"and", R},
 {"addi", I}, {"slti", I}, {"sltiu", I}, {"xori", I}, {"ori", I}, {"andi", I}, {"slli", I}, {"srli", I}, {"srai", I},
 {"lb", I}, {"lh", I}, {"lw", I}, {"ld", I}, {"lbu", I}, {"lhu", I}, {"lwu", I},
 {"sb", S}, {"sh", S}, {"sw", S}, {"sd", S},
 {"beq", B}, {"bne", B}, {"blt", B}, {"bge", B}, {"bltu", B}, {"bgeu", B},
 {"lui", U}, {"auipc", U},
 {"jal", J}, {"jalr", I},
 {"ecall", I}, {"ebreak", I}};

unordered_map<string, int> alias_to_ind = { {"zero", 0}, {"ra", 1}, {"sp", 2}, {"gp", 3}, {"tp", 4}, {"t0", 5}, {"t1", 6}, {"t2", 7}, {"s0", 8}, {"fp", 8}, {"s1", 9}, {"a0", 10}, {"a1", 11}, {"a2", 12}, {"a3", 13}, {"a4", 14}, {"a5", 15}, {"a6", 16}, {"a7", 17}, {"s2", 18}, {"s3", 19}, {"s4", 20}, {"s5", 21}, {"s6", 22}, {"s7", 23}, {"s8", 24}, {"s9", 25}, {"s10", 26}, {"s11", 27}, {"t3", 28}, {"t4", 29}, {"t5", 30}, {"t6", 31}};

//--------------------------------------------------------------------------------------------

// R-type instructions
void add(int rd, int rs1, int rs2) {
    reg[rd] = reg[rs1] + reg[rs2];
}

void sub(int rd, int rs1, int rs2) {
    reg[rd] = reg[rs1] - reg[rs2];
}

void _xor(int rd, int rs1, int rs2) {
    reg[rd] = reg[rs1] ^ reg[rs2];
}

void _or(int rd, int rs1, int rs2) {
    reg[rd] = reg[rs1] | reg[rs2];
}

void _and(int rd, int rs1, int rs2) {
    reg[rd] = reg[rs1] & reg[rs2];
}

void sll(int rd, int rs1, int rs2) {
    reg[rd] = reg[rs1] << reg[rs2]; // Shift left logical
}

void srl(int rd, int rs1, int rs2) {
    reg[rd] = static_cast<uint64_t>(reg[rs1]) >> reg[rs2]; // Shift right logical (unsigned)
}

void sra(int rd, int rs1, int rs2) {
    reg[rd] = reg[rs1] >> reg[rs2]; // Shift right arithmetic (sign-extend)
}

void slt(int rd, int rs1, int rs2) {
    reg[rd] = (reg[rs1] < reg[rs2]) ? 1 : 0; // Set Less Than
}

void sltu(int rd, int rs1, int rs2) {
    reg[rd] = (static_cast<uint64_t>(reg[rs1]) < static_cast<uint64_t>(reg[rs2])) ? 1 : 0; // Set Less Than Unsigned
}

unordered_map<string, function<void(int, int, int)>> R_instruction = { {"add", add}, {"sub", sub}, {"xor", _xor}, {"or", _or}, {"and", _and}, {"sll", sll}, {"srl", srl}, {"sra", sra}, {"slt", slt}, {"sltu", sltu}};

//--------------------------------------------------------------------------------------------
// I-type instructions

void addi(int rd, int rs1, int imm) {
    reg[rd] = reg[rs1] + imm;
}

void xori(int rd, int rs1, int imm) {
    reg[rd] = reg[rs1] ^ imm;
}

void ori(int rd, int rs1, int imm) {
    reg[rd] = reg[rs1] | imm;
}

void andi(int rd, int rs1, int imm) {
    reg[rd] = reg[rs1] & imm;
}

void slli(int rd, int rs1, int imm) {
    reg[rd] = reg[rs1] << (imm & 0x3F); // shift amount must be within 6 bits
}

void srli(int rd, int rs1, int imm) {
    reg[rd] = static_cast<uint64_t>(reg[rs1]) >> (imm & 0x3F); // shift amount within 6 bits
}

void srai(int rd, int rs1, int imm) {
    reg[rd] = reg[rs1] >> (imm & 0x3F); // arithmetic shift right with sign extension
}

void slti(int rd, int rs1, int imm) {
    reg[rd] = (reg[rs1] < imm) ? 1 : 0;
}

void sltiu(int rd, int rs1, int imm) {
    reg[rd] = (static_cast<uint64_t>(reg[rs1]) < static_cast<uint64_t>(imm)) ? 1 : 0;
}

// load returns unsigned 64 bit value
void lb(int rd, int rs1, int imm) {
    int64_t value;
    if(cache_flag ) {
        cache.mem_count();
        value = static_cast<int8_t>(cache.read_access(rs1, imm, 1));
    }
    else {
        value = static_cast<int8_t>(load(reg[rs1], imm, 1)); 
    }
    reg[rd] = value;
}

void lh(int rd, int rs1, int imm) {
    
    int64_t value;
    if(cache_flag ) {
        cache.mem_count();
        value = static_cast<int16_t>(cache.read_access(rs1, imm, 2));
    }
    else{
        value = static_cast<int16_t>(load(reg[rs1], imm, 2)); 
    }
    reg[rd] = value;
}

void lw(int rd, int rs1, int imm) {
    
    int64_t value;
    if(cache_flag) {
        cache.mem_count();
        value = static_cast<int32_t>(cache.read_access(rs1, imm, 4));
    }
    else {
        value = static_cast<int32_t>(load(reg[rs1], imm, 4)); 
    }
    reg[rd] = value;
}

void ld(int rd, int rs1, int imm) {
    
    int64_t value;
    if(cache_flag) {
        cache.mem_count();
        value = static_cast<int64_t>(cache.read_access(rs1, imm, 8));
    }
    else {
        value = static_cast<int64_t>(load(reg[rs1], imm, 8)); 
    }
    reg[rd] = value;
}

void lbu(int rd, int rs1, int imm) {
    
    uint64_t value;
    if(cache_flag) {
        cache.mem_count();
        value = cache.read_access(rs1, imm, 1);
    }
    else {
        value = load(reg[rs1], imm, 1);
    }
    reg[rd] = value;
}

void lhu(int rd, int rs1, int imm) {

    uint64_t value;
    if(cache_flag) {
        cache.mem_count();
        value = cache.read_access(rs1, imm, 2);
    }
    else {
        value = load(reg[rs1], imm, 2);
    }
    reg[rd] = value;
}

void lwu(int rd, int rs1, int imm) {
    
    uint64_t value;
    if(cache_flag) {
        cache.mem_count();
        value = cache.read_access(rs1, imm, 4);
    }
    else {
        value = load(reg[rs1], imm, 4);
    }
    reg[rd] = value;
}

void jalr(int rd, int rs1, int imm) {
    if (rd != 0) {
        reg[rd] = PC + 4;
    }
    PC = (reg[rs1] + imm);
}

unordered_map<string, function<void(int, int, int)>> I_instruction = { {"addi", addi}, {"xori", xori}, {"ori", ori}, {"andi", andi}, {"slli", slli}, {"srli", srli}, {"srai", srai}, {"slti", slti}, {"sltiu", sltiu}, {"lb", lb}, {"lh", lh}, {"lw", lw}, {"ld", ld}, {"lbu", lbu}, {"lhu", lhu}, {"lwu", lwu}, {"jalr", jalr}};
//--------------------------------------------------------------------------------------------
// S-type instructions

void sb(int rs1, int rs2, int imm) {
    
    if(cache_flag){
        cache.mem_count();
        cache.write_access(rs1, imm, rs2, 1);
    }
    else{
        store(reg[rs1], imm, reg[rs2], 1);
    }
}

void sh(int rs1, int rs2, int imm) {
    
    if(cache_flag){
        cache.mem_count();
        cache.write_access(rs1, imm, rs2, 2);
    }
    else{
        store(reg[rs1], imm, reg[rs2], 2);
    }
}

void sw(int rs1, int rs2, int imm) {
    if(cache_flag){
        cache.mem_count();
        cache.write_access(rs1, imm, rs2, 4);
    }
    else{
        store(reg[rs1], imm, reg[rs2], 4);
    }
}

void sd(int rs1, int rs2, int imm) {
    
    if(cache_flag){
        cache.mem_count();
        cache.write_access(rs1, imm, rs2, 8);
    }
    else{
        store(reg[rs1], imm, reg[rs2], 8);
    }
}

unordered_map<string, function<void(int, int, int)>> S_instruction = { {"sb", sb}, {"sh", sh}, {"sw", sw}, {"sd", sd}};

//--------------------------------------------------------------------------------------------

// B-type instructions

int beq(int rs1, int rs2) {
    return (reg[rs1] == reg[rs2]) ? 1 : 0;
}

int bne(int rs1, int rs2) {
    return (reg[rs1] != reg[rs2]) ? 1 : 0;
}

int blt(int rs1, int rs2) {
    return (reg[rs1] < reg[rs2]) ? 1 : 0;
}

int bge(int rs1, int rs2) {
    return (reg[rs1] >= reg[rs2]) ? 1 : 0;
}

int bltu(int rs1, int rs2) {
    return (static_cast<uint64_t>(reg[rs1]) < static_cast<uint64_t>(reg[rs2])) ? 1 : 0;
}

int bgeu(int rs1, int rs2) {
    return (static_cast<uint64_t>(reg[rs1]) >= static_cast<uint64_t>(reg[rs2])) ? 1 : 0;
}

unordered_map<string, function<int(int, int)>> B_instruction = { {"beq", beq}, {"bne", bne}, {"blt", blt}, {"bge", bge}, {"bltu", bltu}, {"bgeu", bgeu}};

//--------------------------------------------------------------------------------------------

// U type instructions

void lui(int rd, int imm) {
    reg[rd] = imm << 12;
}

void auipc(int rd, int imm) {
    reg[rd] = PC + (imm << 12);
}

unordered_map<string, function<void(int, int)>> U_instruction = { {"lui", lui}, {"auipc", auipc}};

//--------------------------------------------------------------------------------------------

// J-type instructions
void jal(int rd, int imm) {
    if (rd != 0) {
        reg[rd] = PC + 4;
    }
    PC = imm; // imm is the label address
}

unordered_map<string, function<void(int, int)>> J_instruction = { {"jal", jal}};

//--------------------------------------------------------------------------------------------
