#ifndef RISCV_CONSTANTS_H
#define RISCV_CONSTANTS_H

#include <unordered_map>
#include <functional>
#include <string>
using namespace std;

extern string c_filename;

extern uint32_t PC;
extern vector<int64_t> reg;
extern map<uint64_t, uint8_t> memory_map;
enum InstrType {R, I, S, B, U, J};
extern unordered_map<string, int> inst_type;
extern unordered_map<string, int> alias_to_ind;

//--------------------------------Memory Allocation ------------------------------------------

void store(uint64_t address, uint64_t offset, uint64_t value, int size);
uint64_t load(uint64_t address, uint64_t offset, int size);
void print_datasection(uint64_t size);
void print_stack(uint64_t size);
void print_memory(uint64_t base_address, uint64_t size);


//--------------------------------cache ---------------------------------------------------
extern int cache_flag;
struct CacheBlock {

    bool valid;
    bool dirty;
    bool hit;
    uint64_t tag;
    map<uint64_t, uint8_t> data;
    CacheBlock() : valid(false), dirty(false), hit(0), tag(0) {}
};

class Cache {
private:
    
    int extract_tag(uint32_t address);
    int extract_index(uint32_t address);
    int extract_blkoffset(uint32_t address);
    int choose_victim(int set_id);
    void move_top(int set_id, int block_index);

public:

    int total_sz; // size of cache
    int block_sz; // size of block
    int associativity; // associativity of cache
    string replacement_policy; // replacement policy
    string write_policy; // write policy

    int num_sets; // number of sets
    int blkoffset_bits; // block offset bits
    int id_bits;   // index bits
    int tag_bits; // tag bits
    int num_lines; // number of lines in a set
    

    vector<vector<CacheBlock>> cache; // cache
    vector<list<int>> usage_tracker; // LRU or FIFO tracker
    int mem_access;
    int mem_hit;
    void mem_count();

    Cache(int total_sz, int block_sz, int associativity, const string& replacement_policy, const string& write_policy);
    
    uint64_t read_access(int rs1, int imm, int size);
    void write_access(int rs1, int imm, int rs2, int size);

    uint64_t mem_load(int rs1, int imm, int size);
    void mem_store(int rs1, int imm, int rs2, int size);

    uint64_t read_from_cache(CacheBlock block, int size, uint32_t addr, uint32_t base_addr);
    
    void print_cache();
    void cache_stats();
    void cache_invalidate();
    void cache_dump(string f_name);
};

extern Cache cache;

//-----------------------------------Instructions---------------------------------------------------------

// R-type instructions

void add(int rd, int rs1, int rs2);
void sub(int rd, int rs1, int rs2);
void _xor(int rd, int rs1, int rs2);
void _or(int rd, int rs1, int rs2);
void _and(int rd, int rs1, int rs2);
void sll(int rd, int rs1, int rs2);
void srl(int rd, int rs1, int rs2);
void sra(int rd, int rs1, int rs2);
void slt(int rd, int rs1, int rs2);
void sltu(int rd, int rs1, int rs2);

extern unordered_map<string, function<void(int, int, int)>> R_instruction;

//-----------------------------------------------------------------------------------------------

// I-type instructions

void addi(int rd, int rs1, int imm);
void xori(int rd, int rs1, int imm);
void ori(int rd, int rs1, int imm);
void andi(int rd, int rs1, int imm);
void slli(int rd, int rs1, int imm);
void srli(int rd, int rs1, int imm);
void srai(int rd, int rs1, int imm);
void slti(int rd, int rs1, int imm);
void sltiu(int rd, int rs1, int imm);

void lb(int rd, int rs1, int imm);
void lh(int rd, int rs1, int imm);
void lw(int rd, int rs1, int imm);
void ld(int rd, int rs1, int imm);
void lbu(int rd, int rs1, int imm);
void lhu(int rd, int rs1, int imm);
void lwu(int rd, int rs1, int imm);

void jalr(int rd, int rs1, int imm);

extern unordered_map<string, function<void(int, int, int)>> I_instruction;

//-----------------------------------------------------------------------------------------------

// S-type instructions

void sb(int rs1, int rs2, int imm);
void sh(int rs1, int rs2, int imm);
void sw(int rs1, int rs2, int imm);
void sd(int rs1, int rs2, int imm);

extern unordered_map<string, function<void(int, int, int)>> S_instruction;

//-----------------------------------------------------------------------------------------------

// B-type instructions

int beq(int rs1, int rs2);
int bne(int rs1, int rs2);
int blt(int rs1, int rs2);
int bge(int rs1, int rs2);
int bltu(int rs1, int rs2);
int bgeu(int rs1, int rs2);

extern unordered_map<string, function<int(int, int)>> B_instruction;

//-----------------------------------------------------------------------------------------------

// U-type instructions

void lui(int rd, int imm);
void auipc(int rd, int imm);

extern unordered_map<string, function<void(int, int)>> U_instruction;

//-----------------------------------------------------------------------------------------------

// J-type instructions

void jal(int rd, int imm);

extern unordered_map<string, function<void(int, int)>> J_instruction;

//-----------------------------------------------------------------------------------------------

#endif 
