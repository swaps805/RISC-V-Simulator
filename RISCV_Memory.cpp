#include <bits/stdc++.h>
#include <fstream>
#include <sstream>
#include <functional>
#include <cmath>
#include "RISCV_Constants.h"
using namespace std;

map<uint64_t, uint8_t> memory_map;

//--------------------------------Memory Allocation ------------------------------------------

//  stores the value in the memory_map at the given address
void store(uint64_t address, uint64_t offset, uint64_t value, int size) {
    for (int i = 0; i < size; i++) {
        memory_map[address + offset + i] = static_cast<uint8_t>((value >> (i * 8)) & 0xff);
    }
}

//  loads the value from the memory_map at the given address
uint64_t load(uint64_t address, uint64_t offset, int size) {
    uint64_t start_addr = address + offset;
    uint64_t value = 0;

    for (int i = 0; i < size; ++i) {
        uint64_t cur_addr = start_addr + i;
        if (memory_map.find(cur_addr) != memory_map.end()) {
            value |= static_cast<uint64_t>(memory_map[cur_addr]) << (i * 8);
        }
    }

    // if (size == 1) {
    //     value &= 0xff;
    // }
    // else if (size == 2) {
    //     value &= 0xffff;
    // }
    // else if (size == 4) {
    //     value &= 0xffffffff;
    // }
    // else if (size == 8) {
    //     value &= 0xffffffffffffffff;
    // }

    return value;
    // return type is unsigned 64 bit
}

//  prints the data section of the memory from 0x10000  in 64 bit format
void print_datasection(uint64_t size) {
    uint64_t base_address = 0x10000;
    uint64_t limit_address = base_address + size;

    for (uint64_t addr = base_address; addr < limit_address; addr += 8) {
        cout << "Memory Address: 0x" << hex << setw(16) << setfill('0') << addr << " :  0x";

        for (int i = 7; i >= 0; i--) {
            uint64_t current_address = addr + i;
            if (memory_map.find(current_address) != memory_map.end()) {
                cout << hex << setw(2) << setfill('0') << static_cast<int>(memory_map[current_address]);
            }
            else {
                cout << "00";
            }
        }
        cout << endl;
    }
    cout << endl;
}

//  prints the stack section of the memory from 0x50000 in 64 bit format
void print_stack(uint64_t size) {
    uint64_t base_address = 0x50000 - 8;
    uint64_t limit_address = base_address - size;

    for (uint64_t addr = base_address; addr > limit_address; addr -= 8) {
        cout << "Memory Address: 0x" << hex << setw(16) << setfill('0') << addr << " :  0x";

        for (int i = 7; i >= 0; i--) {
            uint64_t current_address = addr + i;
            if (memory_map.find(current_address) != memory_map.end()) {
                cout << hex << setw(2) << setfill('0') << static_cast<int>(memory_map[current_address]);
            }
            else {
                cout << "00";
            }
        }
        cout << endl;
    }
    cout << endl;
}

//  prints the memory from base memory in 8 bit format
void print_memory(uint64_t base_address, uint64_t size) {
    // uint64_t base_address = 0x10000;
    uint64_t limit_address = base_address + size;


    for (uint64_t addr = base_address; addr < limit_address; addr++) {
        cout << "Memory[0x" << hex << setw(5) << setfill('0') << addr << "] = 0x";

        if (memory_map.find(addr) != memory_map.end()) {
            cout << hex  << static_cast<int>(memory_map[addr]);
        }
        else {
            cout << "0"; // If the address is not found in memory_map, print 0x00.
        }

        cout << endl;
    }
    cout << endl;
}

//--------------------------------cache ---------------------------------------------------



int Cache::extract_tag(uint32_t address) {
    return address >> (blkoffset_bits + id_bits);
}

int Cache::extract_index(uint32_t address) {
    if (associativity == 0) return 0;
    return (address >> blkoffset_bits) & ((1 << id_bits) - 1);
}

int Cache::extract_blkoffset(uint32_t address) {
    return address & ((1 << blkoffset_bits) - 1);
}

void Cache::move_top(int set_id, int block_index) {
    usage_tracker[set_id].remove(block_index);  // Remove if exists
    usage_tracker[set_id].push_back(block_index);  // Add to back (most recent)
}

int Cache::choose_victim(int set_id) {
    if (replacement_policy == "LRU" || replacement_policy == "FIFO") {
        return usage_tracker[set_id].front();
    }
    else{
        return rand() % num_lines;
    }
    return 0;
}

uint64_t Cache::mem_load(int rs1, int imm, int size){
    uint64_t val = load(reg[rs1], imm, size);
    return val;
}

void Cache::mem_store(int rs1, int imm, int rs2, int size){
    store(reg[rs1], imm, reg[rs2], size);
    return ;
}

void Cache::mem_count(){
    mem_access++;
}

void print_log(CacheBlock& cache_block, int set_id, int block_id, uint32_t address,int type) {
        ofstream output_f(c_filename, ios::app);
        string c_status;
        if (cache_block.valid) {  
            c_status = cache_block.dirty ? "Dirty" : "Clean";
            string h_status = cache_block.hit ? "Hit" : "Miss";
            string op_type = type  ? "W" : "R";  

            output_f << op_type << ": Address: 0x" << hex << address
                << ", Set: 0x" << hex << set_id
                << ", " << h_status
                << ", Tag: 0x" << cache_block.tag
                << ", " << c_status << endl;
            }
        output_f.close();

}


Cache::Cache(int total_sz, int block_sz, int associativity, const string& replacement_policy, const string& write_policy)
    : total_sz(total_sz), block_sz(block_sz), associativity(associativity), replacement_policy(replacement_policy), write_policy(write_policy) {

    // fully associative cache
    if (associativity == 0) {
        // cache[0][block...]
        num_sets = 1;
        num_lines = total_sz / block_sz;
        cache.resize(num_sets, vector<CacheBlock>(num_lines));
        
        blkoffset_bits = log2(block_sz);
        id_bits = 0;
        tag_bits = 32 - blkoffset_bits;
        usage_tracker.resize(num_sets);    
    } 
    // direct mapped
    else if (associativity == 1){
        // cache[set][0] only 1 block in a set
        num_lines = associativity;
        num_sets = total_sz / (block_sz * associativity);
        cache.resize(num_sets, vector<CacheBlock>(num_lines));

        blkoffset_bits = log2(block_sz);
        id_bits = log2(num_sets);
        tag_bits = 32 - (id_bits + blkoffset_bits);
        usage_tracker.resize(num_sets);
    }
    // set associative
    else if (associativity > 1){
        num_lines = associativity;
        num_sets = total_sz / (block_sz * associativity);
        cache.resize(num_sets, vector<CacheBlock>(num_lines));

        blkoffset_bits = log2(block_sz);
        id_bits = log2(num_sets);
        tag_bits = 32 - (id_bits + blkoffset_bits);
        usage_tracker.resize(num_sets);
    }
    mem_hit = 0;
    mem_access = 0;

    
    
}

uint64_t Cache::read_from_cache(CacheBlock block, int size, uint32_t addr, uint32_t base_addr) {
    int start_offset = addr - base_addr;
    uint64_t value = 0;
    for (int i = 0; i < size; i++) {
        uint64_t cur_addr = start_offset + i;
        if (block.data.find(cur_addr) != block.data.end()) {
            value |= static_cast<uint64_t>(block.data[cur_addr]) << (i * 8);
        }
    }
    return value;
    
}




uint64_t Cache::read_access(int rs1, int imm, int size) {
    // num_sets = set_id
    uint32_t address = reg[rs1] + imm;  
    int set_id = extract_index(address);
    int tag = extract_tag(address);
    int blk_offset = extract_blkoffset(address);

    // block to be fetched from memory if miss
    uint32_t mask = ~((1 << blkoffset_bits) - 1);
    uint32_t base_addr = address & mask;
    int offset = address - base_addr;

    if (address + size > base_addr + block_sz) {
        cerr << "Error: Memory access exceeds block size" << endl;
        exit(1);
    }

    // Cache hit, no replacement needed
    for (int i = 0; i < num_lines; i++) {
        CacheBlock& block = cache[set_id][i];
        if (block.valid && block.tag == tag) {
            mem_hit += 1;
            block.hit = 1;
            if (replacement_policy == "LRU") {
                move_top(set_id, i);  // Move block to back of LRU list
            }
            // return value to register
            print_log(block, set_id, i, address, 0);
            return read_from_cache(block, size, address, base_addr);
        }
    }

    // Cache miss

    // Direct mapped
    // If its a miss we always fetch a clean block from memory
    
    if(associativity == 1){
        // if dirty block evicted, write back to memory
        if (cache[set_id][0].dirty) {
            uint32_t start_addr = (cache[set_id][0].tag << (blkoffset_bits + id_bits)) | (set_id << blkoffset_bits);
            for (int i = 0; i < block_sz; i++) {
                store(start_addr, i, cache[set_id][0].data[i], 1);
            }
        }
        // get a new block thats clean
        cache[set_id][0].hit = 0;
        cache[set_id][0].tag = tag;
        cache[set_id][0].valid = true;
        cache[set_id][0].dirty = 0;

        // load the block from memory (clean)
        for(int i = 0; i < block_sz; i++){
            cache[set_id][0].data[i] = load(base_addr, i, 1);
        }
        
        
        print_log(cache[set_id][0], set_id, 0, address, 0);
        // return value to register
        return read_from_cache(cache[set_id][0], size, address, base_addr);
    }
    // Set associative & fully associative
    else if(associativity > 1 || associativity == 0){
        int free_blk_id = -1;
        for(int i = 0; i < num_lines; i++){
            if(!cache[set_id][i].valid){
                free_blk_id = i;
                break;
            }
        }
        // found empty block
        if(free_blk_id != -1){
            cache[set_id][free_blk_id].hit = 0;
            cache[set_id][free_blk_id].tag = tag;
            cache[set_id][free_blk_id].valid = true;
            cache[set_id][free_blk_id].dirty = 0;

            // load the block from memory byte by byte
            for(int i = 0; i < block_sz; i++){
                cache[set_id][free_blk_id].data[i] = load(base_addr, i, 1);
            }
            
            if (replacement_policy == "LRU" || replacement_policy == "FIFO") {
                usage_tracker[set_id].push_back(free_blk_id);
            }
            
            print_log(cache[set_id][free_blk_id], set_id, free_blk_id, address, 0);
            return read_from_cache(cache[set_id][free_blk_id], size, address, base_addr);
        }
        //evict a block
        else{
            int v = choose_victim(set_id);
            if(cache[set_id][v].dirty){
                uint32_t start_addr = (cache[set_id][v].tag << (blkoffset_bits + id_bits)) | (set_id << blkoffset_bits);
                for (int i = 0; i < block_sz; i++) {
                    store(start_addr, i, cache[set_id][v].data[i], 1);
                }
            }
            cache[set_id][v].hit = 0;
            cache[set_id][v].tag = tag;
            cache[set_id][v].valid = true;
            cache[set_id][v].dirty = 0;
            for(int i = 0; i < block_sz; i++){
                cache[set_id][v].data[i] = load(base_addr, i, 1);
            }
            
            // in both cases victim blocks contents are cleared and new content is placed int it
            // its brought to the top of stcak marking it as recently used
            if (replacement_policy == "LRU" || replacement_policy == "FIFO") {
                move_top(set_id, v);  
            }
            print_log(cache[set_id][v], set_id, v, address, 0);
            return read_from_cache(cache[set_id][v], size, address, base_addr);
        }
        
    }
    return 0;

}



void Cache::write_access(int rs1, int imm, int rs2, int size) {
    // num_sets = set_id
    uint32_t address = reg[rs1] + imm;
    int64_t value = reg[rs2];

    int set_id = extract_index(address);
    int tag = extract_tag(address);
    int blk_offset = extract_blkoffset(address);

    // block to be fetched from memory if miss
    uint32_t mask = ~((1 << blkoffset_bits) - 1);
    uint32_t base_addr = address & mask;
    int offset = address - base_addr;

    if (address + size > base_addr + block_sz) {
        cerr << "Error: Memory access exceeds block size" << endl;
        exit(1);
    }

    // cache hit
    for (int i = 0; i < num_lines; i++) {
        CacheBlock& block = cache[set_id][i];
        if (block.valid && block.tag == tag) {
            mem_hit += 1;
            block.hit = 1; 

            //write to cache the new value
            for(int i = 0; i < size; i++){
                block.data[offset + i] = static_cast<uint8_t>((value >> (i * 8)) & 0xff);
            }
            
            if (replacement_policy == "LRU") {
                move_top(set_id, i);  
            }
            // if value writen mark cache as dirty in WB
            if(write_policy == "WB"){
                block.dirty = 1;            
            }
            else{
                // in WT  write to memory also
                store(reg[rs1], imm, reg[rs2], size);
            }
            print_log(block, set_id, i, address, 1);
            return;  
        }
    }

    // cache miss on WT, no change to cache as directly writen to main memory
    // cache miss on WB, change cache
    // clean block brought to memory, but value writen to it so its dirty
    if(write_policy == "WB"){
        // Direct mapped NO replacement policy, simply replace the block
        if(associativity == 1){
            // write the dirty block to memory
            if (cache[set_id][0].dirty) {
                uint32_t start_addr = (cache[set_id][0].tag << (blkoffset_bits + id_bits)) | (set_id << blkoffset_bits);
                for (int i = 0; i < block_sz; i++) {
                    store(start_addr, i, cache[set_id][0].data[i], 1);
                }
            }

            cache[set_id][0].hit = 0;
            cache[set_id][0].tag = tag;
            cache[set_id][0].valid = true;
            cache[set_id][0].dirty = 1;
            // allocate old block from  memory to cache (clean)
            for(int i = 0; i < block_sz; i++){
                cache[set_id][0].data[i] = load(base_addr, i, 1);
            }
            // write the new value to cache (dirty)
            for (int i = 0; i < size; i++) {
                cache[set_id][0].data[offset + i] = static_cast<uint8_t>((value >> (i * 8)) & 0xff);
            }
            print_log(cache[set_id][0], set_id, 0, address, 1);
            return;
        }
        // Set associative & fully associative
        else if(associativity > 1 || associativity == 0){
            int free_blk_id = -1;
            for(int i = 0; i < num_lines; i++){
                if(!cache[set_id][i].valid){
                    free_blk_id = i;
                    break;
                }
            }
            // found empty block
            if(free_blk_id != -1){
                cache[set_id][free_blk_id].hit = 0;
                cache[set_id][free_blk_id].tag = tag;
                cache[set_id][free_blk_id].valid = true;
                cache[set_id][free_blk_id].dirty = 1;

                // get the old value from memory to cache (clean)
                for(int i = 0; i < block_sz; i++){
                    cache[set_id][free_blk_id].data[i] = load(base_addr, i, 1);
                }
                // write the new value to cache (dirty)
                for (int i = 0; i < size; i++) {
                    cache[set_id][free_blk_id].data[offset + i] = static_cast<uint8_t>((value >> (i * 8)) & 0xff);
                }

                if (replacement_policy == "LRU" || replacement_policy == "FIFO") {
                    usage_tracker[set_id].push_back(free_blk_id);
                }
                
                print_log(cache[set_id][free_blk_id], set_id, free_blk_id, address, 1);
                return;
            }
            //evict a block
            else{
                int v = choose_victim(set_id);
                if(cache[set_id][v].dirty){
                    uint32_t start_addr = (cache[set_id][v].tag << (blkoffset_bits + id_bits)) | (set_id << blkoffset_bits);
                    for (int i = 0; i < block_sz; i++) {
                        store(start_addr, i, cache[set_id][v].data[i], 1);
                    }
                }
                cache[set_id][v].hit = 0;
                cache[set_id][v].tag = tag;
                cache[set_id][v].valid = true;
                cache[set_id][v].dirty = 1; 

                // get the old block from memory to cache (clean)
                for(int i = 0; i < block_sz; i++){
                    cache[set_id][v].data[i] = load(base_addr, i, 1);
                }
                // write the new value to cache (dirty)
                for (int i = 0; i < size; i++) {
                    cache[set_id][v].data[offset + i] = static_cast<uint8_t>((value >> (i * 8)) & 0xff);
                }


                // for FIFO also as same block evicted and new block placed on top of stack (new and onld are the same block)
                if (replacement_policy == "LRU" || replacement_policy == "FIFO") {
                    move_top(set_id, v);  
                }
                print_log(cache[set_id][v], set_id, v, address, 0);
                return;
            }

        }

    }
    else{
        // WT On miss directly write to memory
        store(reg[rs1], imm, reg[rs2], size);
        ofstream output_f(c_filename, ios::app);
        output_f << "W: Address: 0x" << hex << address 
            << ", Set: 0x" << set_id 
            << ", Miss"
            << ", Tag: 0x" << tag 
            << ", Clean" << endl;
        output_f.close();
        return ;
    }

}




void Cache::print_cache() {
    cout << "Cache Contents:\n";
    for (int set = 0; set < num_sets; ++set) {
        for (int block = 0; block < num_lines; ++block) {
            const CacheBlock& cache_block = cache[set][block];

            if (cache_block.valid) {  
                //int data_sz = cache_block.data.size();
                string c_status = cache_block.dirty ? "Dirty" : "Clean";
                string h_status = cache_block.hit ? "Hit" : "Miss"; 

                cout << "Tag: 0x" << cache_block.tag
                     << ", Set: 0x" << hex << set
                     << ", " << h_status
                     << ", " << c_status <<" : [ " ;
                for (int i = 0; i < block_sz; ++i) {
                    if(cache_block.data.find(i) == cache_block.data.end()){
                        cout << hex << setw(2) << setfill('0') << "00" << " ";
                    }
                    else{
                        cout << hex << setw(2) << setfill('0') << static_cast<int>(cache_block.data.at(i)) << " ";
                    }
                    
                }
                cout << " ]" << endl;
            }
            
        }
    }
    cout << endl;
}

void Cache::cache_invalidate() {
    mem_hit = 0;
    mem_access = 0;
    for (int s = 0; s < num_sets; s++) {
        for (int b = 0; b < num_lines; b++) {
            cache[s][b].valid = 0;
            cache[s][b].tag = 0;
            cache[s][b].dirty = 0;
            cache[s][b].hit = 0;
            cache[s][b].data.clear();
        }
    }
}

void Cache::cache_dump(string f_name) {
    ofstream output_f(f_name);
    cout << "Cache Dumped to " << f_name << endl;
    for (int s = 0; s < num_sets; ++s) {
        for (int b = 0; b < num_lines; ++b) {
            const CacheBlock& cache_block = cache[s][b];
            if (cache_block.valid) {  
                string status = cache_block.dirty ? "Dirty" : "Clean";
                output_f << "Set: 0x" << hex << s
                     << ", Tag: 0x" << cache_block.tag
                     << ", " << status << endl;
            }
        }
    }
    output_f.close();
    cout << endl;
}

void Cache::cache_stats() {
    int mem_miss = mem_access - mem_hit; 
    if(mem_access == 0){
        cout << "D-cache statistics: Accesses=0, Hit=0, Miss=0, Hit Rate=0" << endl;
        return;
    }
    double hit_rate = static_cast<double>(mem_hit) / mem_access; 
    cout << "D-cache statistics: Accesses=" << dec <<mem_access 
         << ", Hit=" << dec << mem_hit 
         << ", Miss=" << dec << mem_miss 
         << ", Hit Rate=" << fixed << setprecision(2)  << hit_rate << endl;
}