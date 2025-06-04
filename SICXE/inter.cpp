#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <iomanip>

struct Registers {
    int A = 0; // 累加器
    int X = 0; // 索引暫存器
    int L = 0; // 連結暫存器
    int PC = 0; // 程式計數器
    int SW = 0; // 狀態暫存器
};

struct Instruction {
    std::string op;
    std::string arg;
};

std::vector<Instruction> parseASM(const std::string& filename) {
    std::vector<Instruction> prog;
    std::ifstream fin(filename);
    std::string line;
    while (std::getline(fin, line)) {
        std::istringstream iss(line);
        std::string op, arg;
        iss >> op;
        iss >> arg;
        if (!op.empty())
            prog.push_back({op, arg});
    }
    return prog;
}

void runSICXE(const std::vector<Instruction>& prog) {
    Registers reg;
    std::unordered_map<std::string, int> mem;
    int pc = 0;
    while (pc < prog.size()) {
        const auto& inst = prog[pc];
        if (inst.op == "LDA") {
            if (mem.count(inst.arg))
                reg.A = mem[inst.arg];
            else
                reg.A = std::stoi(inst.arg);
        } else if (inst.op == "STA") {
            mem[inst.arg] = reg.A;
        } else if (inst.op == "ADD") {
            if (mem.count(inst.arg))
                reg.A += mem[inst.arg];
            else
                reg.A += std::stoi(inst.arg);
        } else if (inst.op == "SUB") {
            if (mem.count(inst.arg))
                reg.A -= mem[inst.arg];
            else
                reg.A -= std::stoi(inst.arg);
        } else if (inst.op == "J") {
            pc = std::stoi(inst.arg) - 1;
            continue;
        } else if (inst.op == "COMP") {
            int val = mem.count(inst.arg) ? mem[inst.arg] : std::stoi(inst.arg);
            if (reg.A == val) reg.SW = 0;
            else if (reg.A < val) reg.SW = -1;
            else reg.SW = 1;
        } else if (inst.op == "JEQ") {
            if (reg.SW == 0) {
                pc = std::stoi(inst.arg) - 1;
                continue;
            }
        } else if (inst.op == "JLT") {
            if (reg.SW < 0) {
                pc = std::stoi(inst.arg) - 1;
                continue;
            }
        } else if (inst.op == "JGT") {
            if (reg.SW > 0) {
                pc = std::stoi(inst.arg) - 1;
                continue;
            }
        } else if (inst.op == "RSUB" || inst.op == "END") {
            break;
        }
        pc++;
    }

    std::cout << "A暫存器最終值: " << reg.A << std::endl;
    std::cout << "記憶體內容:" << std::endl;
    for (const auto& kv : mem) {
        std::cout << kv.first << " : " << kv.second << std::endl;
    }
}

int main() {
    std::string filename = "input.asm";
    auto prog = parseASM(filename);
    runSICXE(prog);
    return 0;
}
