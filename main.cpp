#include <bits/stdc++.h>
#include <fstream>
#include <sstream>
#include <functional>
#include "RISCV_Constants.h"
using namespace std;

string c_filename = "";
vector<string> input;

map<uint32_t, vector<string>> PC_instructions; // map from PC to  parsed instruction
map<int, uint32_t> line_pc;                    // map from line number to program counter
unordered_map<string, uint32_t> labels;        // map from label to address
set<uint32_t> breakpoints;                     // set of breakpoints
uint32_t PC = 0;                               // program counter
vector<pair<string, int>> fn_stack;            // function call stack

int cache_flag = 0;
int cache_size;
int cache_blocksz;
int cache_assc;
string cache_repl;
string cache_wb;
Cache cache(1, 1, 1, "", "");

// ---------------------Utility Functions----------------------------------

// Trim leading and trailing whitespaces and special characters
string trim(string &str) {
    size_t first = str.find_first_not_of(" \n\r\t");
    if (first == string::npos)
        return "";

    size_t last = str.find_last_not_of(" \n\r\t");
    return str.substr(first, (last - first + 1));
}

// Convert string to lower case
string toLower(string &str) {
    transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

// return id where directive like (dword, word, half, byte) ends (.dword  -> 6)
int extractDirective(string &line, int pos) {
    size_t i = pos + 1;
    while (i < line.length() && (isalnum(line[i]) || line[i] == '_')) {
        i++;
    }
    return i;
}

// evaluate immediate values for I, S, U, J type instructions
// 32 bit signed integer
int imm_eval(string &num) {
    int val = 0;
    if (num[1] == 'x') {
        val = stoi(num, nullptr, 16);
    }
    else if (num[1] == 'b') {
        val = stoi(num.substr(2), nullptr, 2);
    }

    else {
        val = stoi(num);
    }
    return val;
}

// evaluate immediate to be stored in data section
// 64 bit unsigned integer
uint64_t data_eval(string &num) {
    uint64_t val = 0;
    if (num[1] == 'x') {
        val = stoull(num, nullptr, 16);
    }
    else if (num[1] == 'b') {
        val = stoull(num.substr(2), nullptr, 2);
    }

    else {
        val = stoull(num);
    }
    return val;
}

// find PC from line number
int find_line(uint32_t PC) {
    for (auto &l : line_pc) {
        if (l.second == PC) {
            return l.first;
        }
    }
    return -1;
}

// ---------------------Instruction Parsing----------------------------------

// Process brcakets in instructions
void bracket(vector<string> &tokens) {
    for (auto &token : tokens) {
        size_t pos = token.find('(');
        // for instructions like 100(x1)
        // extracts x1, 100 and removes ( and )
        if (pos != string::npos) {
            int k = token.length();
            string imm = token.substr(0, pos);
            string reg = token.substr(pos + 1, k - pos - 2);
            tokens.pop_back();

            // for jalr rd, rs1(imm) interchange imm and reg to support RIPES
            if (imm[0] == 'x' || alias_to_ind.find(imm) != alias_to_ind.end()) {
                string temp = imm;
                imm = reg;
                reg = temp;
                tokens.push_back(reg);
                tokens.push_back(imm);
                tokens.push_back("1");
            }
            else {
                tokens.push_back(reg);
                tokens.push_back(imm);
                tokens.push_back("0");
            }
        }
    }
}

// Process labels and find the address of labels
void parseInstructions(vector<string> &input, vector<int> &line_num) {
    int program_cntr = 0;

    for (int i = 0; i < input.size(); i++) {
        string line = input[i];
        size_t colon_pos = line.find(':');
        if (colon_pos != string::npos) { // if line contains a label
            string label = line.substr(0, colon_pos);
            label = trim(label); // extract label and remove whitespaces
            labels[label] = program_cntr;

            line = line.substr(colon_pos + 1); // line after label
            line = trim(line);
        }

        // if line is not empty, add to instructions
        if (!line.empty()) {
            replace(line.begin(), line.end(), ',', ' ');
            istringstream iss(line); // remove white spaces
            string token;
            vector<string> tokens;
            while (iss >> token) {
                tokens.push_back(token);
            }
            bracket(tokens);
            PC_instructions[program_cntr] = tokens;
            line_pc[line_num[i]] = program_cntr;
            program_cntr += 4;
        }
    }
}

// --------------------------Instruction Processing-----------------------------

// Execute Instructions and updates PC
void compute(vector<string> tokens) {
    // obtain the intruction, its types and register arguments, labels, immediate values
    string instr = toLower(tokens[0]);
    vector<string> args(tokens.begin() + 1, tokens.end());
    int type = inst_type[instr];
    int rd, rs1, rs2, imm;

    // check instruction based on type
    switch (type) {
    case R:
        if (args[0][0] == 'x') {
            rd = stoi(args[0].substr(1));
        }
        else {
            rd = alias_to_ind[args[0]];
        }

        if (args[1][0] == 'x') {
            rs1 = stoi(args[1].substr(1));
        }
        else {
            rs1 = alias_to_ind[args[1]];
        }

        if (args[2][0] == 'x') {
            rs2 = stoi(args[2].substr(1));
        }
        else {
            rs2 = alias_to_ind[args[2]];
        }

        R_instruction[instr](rd, rs1, rs2);

        PC += 4;
        break;

    case I:
        if (args[0][0] == 'x') {
            rd = stoi(args[0].substr(1));
        }
        else {
            rd = alias_to_ind[args[0]];
        }

        if (args[1][0] == 'x') {
            rs1 = stoi(args[1].substr(1));
        }
        else {
            rs1 = alias_to_ind[args[1]];
        }
        imm = imm_eval(args[2]);
        I_instruction[instr](rd, rs1, imm);

        if (instr == "jalr") {
            if (!fn_stack.empty()) {
                fn_stack.pop_back();
            }
        }
        else {
            PC += 4;
        }
        break;

    case S:
        if (args[0][0] == 'x') {
            rs2 = stoi(args[0].substr(1));
        }
        else {
            rs2 = alias_to_ind[args[0]];
        }

        if (args[1][0] == 'x') {
            rs1 = stoi(args[1].substr(1));
        }
        else {
            rs1 = alias_to_ind[args[1]];
        }

        imm = imm_eval(args[2]);
        // rs2 -> Mem[rs1 + imm] (taken care of in store function)
        S_instruction[instr](rs1, rs2, imm);

        PC += 4;
        break;

    case B:
        if (args[0][0] == 'x') {
            rs1 = stoi(args[0].substr(1));
        }
        else {
            rs1 = alias_to_ind[args[0]];
        }

        if (args[1][0] == 'x') {
            rs2 = stoi(args[1].substr(1));
        }
        else {
            rs2 = alias_to_ind[args[1]];
        }

        // if condition is true, jump to label
        if (B_instruction[instr](rs1, rs2)) {
            PC = labels[args[2]];
        }
        else {
            PC += 4;
        }
        break;

    case U:
        if (args[0][0] == 'x') {
            rd = stoi(args[0].substr(1));
        }
        else {
            rd = alias_to_ind[args[0]];
        }
        U_instruction[instr](rd, imm_eval(args[1]));
        PC += 4;
        break;

    case J:
        if (args[0][0] == 'x') {
            rd = stoi(args[0].substr(1));
        }
        else {
            rd = alias_to_ind[args[0]];
        }

       
        uint32_t target_addr = labels[args[1]]; // get target address of function
        J_instruction[instr](rd, target_addr);
        string fn_name = args[1];
        int prev = PC - 4 > 0  ? PC - 4 : 0;
        fn_stack.push_back({fn_name, find_line(prev)}); // push function name and line number prceding the function label declration
        break;
    }
}
// --------------------------Load Source code-------------------------------

void load(string file_name) {
    vector<string> input; // store raw instructions from each line
    vector<int> line_num; // store line num of each instruction
    int l = 1;
    int inst_start = 0;
    vector<pair<uint64_t, int>> data; // values to be stored in data section

    ifstream input_file(file_name);
    string line;

    if (!input_file.is_open()) {
        cerr << "Error: File " << file_name << " does not exist or cannot be opened." << endl;
        return;
    }

    if(cache_flag){ 
        int k = file_name.length();
        c_filename = file_name.substr(0, file_name.find(".")) + ".output";
        //cout << c_filename << endl;
        ofstream output_f(c_filename);
        output_f.close();
    }

    // find the starting of the .text section
    while(getline(input_file, line)){
        if(line.find(".text") != string::npos){
            inst_start = l;
            
            while(getline(input_file, line)){
                line = trim(line);
                if(!line.empty()){
                    inst_start = l;
                    break;
                }
                l++;
            }
        }
        l ++;
    }
    input_file.clear();
    input_file.seekg(0, ios::beg);


    l = 1;
    line.clear();
    // clear existing data and initilise registers and memory
    PC = 0;
    for (int i = 0; i < 32; i++) {
        reg[i] = 0;
    }
    for (int i = 0x10000; i < 0x50000; i++) {
        memory_map[i] = 0;
    }
    line_pc.clear();
    labels.clear();
    PC_instructions.clear();
    breakpoints.clear();
    fn_stack.clear();
    cache.cache_invalidate();

    while (getline(input_file, line)) {
        // remove leading and trailing whitespaces
        line = trim(line);

        // for lines like : .word 0x100, 0x200, 0x300
        size_t dot_pos = line.find('.');
        if (dot_pos != string::npos) {
            l += 1;

            // extract "word" from .word 0x100, 0x200, 0x300
            int k = line.length();
            int id = extractDirective(line, dot_pos);
            string directive = line.substr(dot_pos, id - dot_pos);
            directive = trim(directive);

            string values = "";
            if (id < k) {
                values = line.substr(id + 1);
                values = trim(values);

                // ignore comments starting with # and ;
                // lines like :  "0x100, 0x200, 0x300 # comment"
                size_t comment_pos = values.find('#');
                if (comment_pos != string::npos) {
                    values = values.substr(0, comment_pos);
                    values = trim(values);
                }
                comment_pos = values.find(';');
                if (comment_pos != string::npos) {
                    values = values.substr(0, comment_pos);
                    values = trim(values);
                }
                // line is now like :  "0x100, 0x200, 0x300"
                replace(values.begin(), values.end(), ',', ' ');
                // values can be "" but iss handles it correctly and ignores it
            }

            // ignore .data and .text directives
            if (directive == ".data" || directive == ".text") {
                continue;
            }

            // store values  and sizes in data section
            else if (directive == ".dword") {
                istringstream iss(values);
                string val;
                while (iss >> val) {
                    data.push_back({data_eval(val), 8});
                }
                continue;
            }
            else if (directive == ".word") {
                istringstream iss(values);
                string val;
                while (iss >> val) {
                    data.push_back({data_eval(val), 4});
                }
                continue;
            }
            else if (directive == ".half") {
                istringstream iss(values);
                string val;
                while (iss >> val) {
                    data.push_back({data_eval(val), 2});
                }
                continue;
            }
            else if (directive == ".byte") {
                istringstream iss(values);
                string val;
                while (iss >> val) {
                    data.push_back({data_eval(val), 1});
                }
                continue;
            }
        }

        // for lines like : add x1, x2, x3 # comment
        // ignore comments starting with # and ;
        size_t comment_pos = line.find('#');
        if (comment_pos != string::npos) {
            line = line.substr(0, comment_pos);
            line = trim(line);
        }
        comment_pos = line.find(';');
        if (comment_pos != string::npos) {
            line = line.substr(0, comment_pos);
            line = trim(line);
        }
        // ignore empty lines
        if (!line.empty()) {
            input.push_back(line);
            line_num.push_back(l);
        }

        l += 1;
    }

    input_file.close();

    // store data section in memory
    uint64_t base_addr = 0x10000;
    for (auto &d : data) {
        store(base_addr, 0, d.first, d.second);
        base_addr += d.second;
    }

    fn_stack.push_back({"main", inst_start}); // push main function to function stack

    parseInstructions(input, line_num); 
    cout << endl;
    // find locations of labels and instructions
    // cout << "Loaded " << file_name << endl
    //      << endl;

    // print_datasection();

    // cout <<"data section"<<endl;
    // for(auto &l: data){
    //     cout << l.first << "-> " << l.second << endl;
    // }
    // cout << endl;

    // cout <<"line number"<<endl;
    // for(auto &l: line_num){
    //     cout << l << "- ";
    // }
    // cout << endl<< endl;

    // cout << "Instructions" << endl;
    // for (auto &line: PC_instructions) {
    //     cout << dec  <<line.first << ": ";
    //     for (auto &token: line.second) {
    //         cout << token << "-";
    //     }
    //     cout << endl;
    // }
    // cout << endl;

    // cout << "PC to line number mapping" << endl;
    // for (auto &l: line_pc) {
    //     cout << dec << l.first << " : " << l.second << endl;
    // }
}

// --------------------------Program Execution -------------------------------
void printExecution(vector<string> tokens) {
    cout << "Executed ";
    string instr = toLower(tokens[0]);
    set<string> s = {"lb", "lh", "lw", "ld", "lbu", "lhu", "lwu", "jalr", "sb", "sh", "sw", "sd"};
    if (s.find(instr) != s.end()) {
        if(tokens.back() == "0"){
            cout << instr << " " << tokens[1] << ", " << tokens[3] << "(" << tokens[2] << "); ";
            cout << "PC=" << "0x" << hex << setw(8) << setfill('0') << (uint32_t)PC << endl;
            return;
        }
        else{
            cout << instr << " " << tokens[1] << ", " << tokens[2] << "(" << tokens[3] << "); ";
            cout << "PC=" << "0x" << hex << setw(8) << setfill('0') << (uint32_t)PC << endl;
            return;
        }
    }

    
    int k = tokens.size();
    cout << instr << " ";
    for (int i = 1; i < k - 1; i++) {
        cout << tokens[i] << ", ";
    }
    cout << tokens[k - 1] << "; ";
    cout << "PC=" << "0x" << hex << setw(8) << setfill('0') << (uint32_t)PC << endl;
    return;
}

void run() {
    // max PC value
    uint32_t prog_end = (uint32_t)PC_instructions.rbegin()->first;
    if (PC > prog_end) {
        cout << "Execution complete " << endl
             << endl;
        return;
    }

    // run till PC reaches end of program or breakpoint is encountered
    while (PC <= prog_end) {
        if (breakpoints.find(PC) != breakpoints.end()) {
            cout << "Execution stopped at breakpoint" << endl
                 << endl;
            return; // Stop execution
        }
        fn_stack.back().second = find_line(PC);
        // if PC is at the end of the program, clear the function stack
        if (PC == prog_end) {
            fn_stack.clear();
            
        }

        vector<string> tokens = PC_instructions[PC]; // get instruction at PC
        printExecution(tokens);                      // print the instruction
        compute(tokens);                            // execute the instruction
        if (PC > prog_end && cache_flag == 1) {
            cache.cache_stats();
            //cache.cache_invalidate();
        }                          
    }
    cout << endl;
}

void step() {
    // max PC value
    uint32_t prog_end = (uint32_t)PC_instructions.rbegin()->first;
    if (PC > prog_end) {
        cout << "Nothing to step" << endl
             << endl;
        return;
    }
    // if PC is at the end of the program, clear the function stack
    fn_stack.back().second = find_line(PC);
    if (PC == prog_end) {
        fn_stack.clear();
        
    }

    vector<string> tokens = PC_instructions[PC];
    printExecution(tokens); // print the instruction
    compute(tokens);        // execute the instruction
    if (PC > prog_end and cache_flag == 1) {
        cache.cache_stats();
        //cache.cache_invalidate();
    }
    cout << endl;           
}

// Show function stack
void show_stack() {
    uint32_t prog_end = (uint32_t)PC_instructions.rbegin()->first;
    if (PC > prog_end && fn_stack.empty()) {
        cout << "Empty Call Stack: Execution complete" << endl
             << endl;
        return;
    }

    cout << "Call Stack:" << endl;
    for(auto &e: fn_stack){
        cout << e.first << ":" << dec <<e.second << endl;
    }
    cout << endl;
}

// --------------------------Main Function -------------------------------
int main(int argc, char *argv[]) {
    string command;

    while (true) {
        
        getline(cin, command);

        stringstream ss(command);
        string cmd;
        ss >> cmd;

        // load source code
        if (cmd == "load") {
            string file_name;
            ss >> file_name;
            load(file_name);
        }

        // display contents of registers
        else if (cmd == "regs") {
           for (int i = 0; i < 32; i++) {
                if (i >= 0 && i <= 9) {
                    cout << "x" << dec << i << "  = 0x" 
                         << hex << setw(16) << setfill('0') << reg[i] << endl;
                } else {
                    cout << "x" << dec << i << " = 0x" 
                         << hex << setw(16) << setfill('0') << reg[i] << endl;
                }
            }
            cout << endl;
        }

        // execute next instruction
        else if (cmd == "step") {
            step();
        }
        // run the program till breakpoint or end
        else if (cmd == "run") {
            run();
        }

        // set breakpoint at line number
        else if (cmd == "break") {
            int line;
            ss >> line;
            if (line_pc.find(line) == line_pc.end()) {
                cout << "No Instruction found at line " << dec << line << endl
                     << endl;
                continue;
            }
            breakpoints.insert(line_pc[line]);
            cout << "Breakpoint set at line " << dec << line << endl
                 << endl;
        }

        // remove breakpoint at line number
        else if (cmd == "del") {
            int line;
            string str;
            ss >> str;
            if (str == "break") {
                ss >> line;
                if (breakpoints.find(line_pc[line]) == breakpoints.end()) {
                    cout << "No breakpoint at line " << dec << line << endl;
                }
                else {
                    breakpoints.erase(line_pc[line]);
                    cout << "Breakpoint removed at line " << dec << line << endl;
                }
                cout << endl;
            }
        }

        // show contents of function call stack
        else if (cmd == "show-stack") {
            show_stack();
        }

        // show contents of data section
        else if (cmd == "mem") {
            uint64_t size;
            uint64_t addr;
            if (!(ss >> hex >> addr)) {
                cout << "Error: Ensure it is a valid address and starts with 0x in hex format." << endl << endl;
                continue;
            }
    

            if (!(ss >> dec >> size)) {
                cout << "Error: Size not provided or invalid." << endl << endl;
                continue;
            }
            print_memory(addr, size);
        }

        // show contents of stack memory from 0x50000 in 64 bit format
        else if (cmd == "stack") {
            uint64_t size;
            ss >> size;
            print_stack(size);
        }

        // show contents of data section from 0x10000 in 64 bit format
        else if (cmd == "data") {
            uint64_t size;
            ss >> size;
            print_datasection(size);
        }

        // exit the simulator
        else if (cmd == "exit") {
            cout << "Exited the simulator" << endl;
            break;
        }



        else if (cmd == "cache_sim"){
            string cmd2;
            ss >> cmd2;
            if(cmd2 == "enable"){
                cache_flag = 1;
                string config_file;
                ss >> config_file;
                
                ifstream cnfg(config_file);
                if(!cnfg.is_open()){
                    cout << "Error: File " << config_file << " does not exist or cannot be opened." << endl;
                    continue;
                }
                cnfg >> cache_size >> cache_blocksz >> cache_assc >> cache_repl >> cache_wb;
                cache = Cache(cache_size, cache_blocksz, cache_assc, cache_repl, cache_wb);
                cout  << endl;

            }
            else if(cmd2 == "disable"){
                cache_flag = 0;
                cout << "disabled cache" << endl << endl;
            }
            else if(cmd2 == "status"){
                if(cache_flag){
                    cout << "Cache is enabled" << endl << endl;
                    cout << "Cache Size " << cache_size  << endl;
                    cout << "Block Size " << cache_blocksz << endl;
                    cout << "Associativity " << cache_assc << endl;
                    cout << "Replacement Policy " << cache_repl << endl;
                    cout << "Write Policy " << cache_wb << endl;
                    cout << endl;
                }
                else{
                    cout << "Cache is disabled" << endl;
                    cout << endl;
                }
            }
            
            else if(cmd2 == "dump"){
                if(cache_flag){
                    string f_name;
                    ss >> f_name;
                    cache.cache_dump(f_name);
                }
                else{
                    cout << "Cache is disabled" << endl;
                    cout << endl;
                }
            }
            else if(cmd2 == "stats"){
                if(cache_flag){
                    cache.cache_stats();
                }
                else{
                    cout << "Cache is disabled" << endl;
                    cout << endl;
                }
            }
            else if(cmd2 == "invalidate"){
                if(cache_flag){
                    for (int s = 0; s < cache.num_sets; s++) {
                        for (int b = 0; b < cache.num_lines; b++)   {
                            if(cache.cache[s][b].valid && cache.cache[s][b].dirty){
                                const CacheBlock& block = cache.cache[s][b];
                                uint32_t addr = (block.tag << (cache.id_bits + cache.blkoffset_bits)) | (s << cache.blkoffset_bits);
                                for (int i = 0; i < cache.block_sz; i++) {
                                    store(addr + i, 0, block.data.at(i), 1);
                                }

                            }
                        }
                    }
                    cache.cache_invalidate();
                    cout << "Cache invalidated" << endl;
                    cout << endl;
                }
                else{
                    cout << "Cache is disabled" << endl;
                    cout << endl;
                }
            }
            


        }
        else if(cmd == "log"){
            if(cache_flag){
                cache.print_cache();
            }
            else{
                cout << "Cache is disabled" << endl;
                cout << endl;
            }
        }

        else{
            cout << "Invalid Command Please refer README for usage" << endl;
            cout << endl;
        }
    }

    return 0;
}
