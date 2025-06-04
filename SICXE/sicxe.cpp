#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <iomanip>
#include <sstream>
#include "assemble.cpp"

std::map<int, std::string> OPCODE_MAP = {
    {0x00, "LDA"}, {0x18, "ADD"}, {0x0C, "STA"},
    {0x50, "LDCH"}, {0x54, "STCH"}, {0x4C, "RSUB"},
    {0x68, "LDB"}, {0x3C, "J"}, {0x30, "JEQ"},
    {0x48, "JSUB"}, {0x04, "LDX"}, {0x10, "STX"},
    {0x1C, "SUB"}, {0x20, "MUL"}, {0x24, "DIV"},
    {0x28, "COMP"}
};

class SICXEMachine {
public:
    static const int MEM_SIZE = 32768; // 32K bytes
    std::vector<unsigned char> memory;
    std::map<std::string, int> registers;

    SICXEMachine() : memory(MEM_SIZE, 0) {
        // 初始化暫存器
        registers["A"] = 0;  // Accumulator
        registers["X"] = 0;  // Index
        registers["L"] = 0;  // Linkage
        registers["PC"] = 0; // Program Counter
        registers["SW"] = 0; // Status Word
        registers["B"] = 0;  // Base
        registers["S"] = 0;  // General
        registers["T"] = 0;  // General
        registers["F"] = 0;  // Floating
    }

    void reset() {
        std::fill(memory.begin(), memory.end(), 0);
        for (auto &reg : registers) reg.second = 0;
    }

    void load_memory(int address, int value) {
        // 將 value (24 bits) 載入到指定記憶體位置
        if (address < 0 || address + 2 >= MEM_SIZE) return;
        memory[address] = (value >> 16) & 0xFF;
        memory[address + 1] = (value >> 8) & 0xFF;
        memory[address + 2] = value & 0xFF;
    }

    int fetch_memory(int address) {
        // 從記憶體讀取 3 bytes (24 bits)
        if (address < 0 || address + 2 >= MEM_SIZE) return 0;
        return (memory[address] << 16) | (memory[address + 1] << 8) | memory[address + 2];
    }

    void dump_memory(int start, int end) {
        if (start < 0 || end >= MEM_SIZE || start > end) return;
        for (int i = start; i <= end; ++i) {
            if ((i - start) % 16 == 0) std::cout << std::endl << std::hex << std::setw(4) << std::setfill('0') << i << ": ";
            std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)memory[i] << " ";
        }
        std::cout << std::dec << std::endl;
    }

    void show_registers() {
        for (const auto& reg : registers) {
            std::cout << reg.first << ": " << std::hex << reg.second << "  ";
        }
        std::cout << std::dec << std::endl;
        dump_memory(0x1000, 0x1040);
    }
    void load_object(const std::string& filename) {
    std::ifstream infile(filename);
    if (!infile) {
        std::cout << "無法開啟物件檔: " << filename << std::endl;
        return;
    }

    std::string line;
    while (std::getline(infile, line)) {
        if (line.empty()) continue;
        if (line[0] == 'H') {
            // Header record，可解析程式名稱、起始位址、長度（如有需要）
            continue;
        } else if (line[0] == 'T') {
            // Text record: T^startAddr^code^
            size_t pos1 = line.find('^');
            size_t pos2 = line.find('^', pos1 + 1);
            if (pos1 == std::string::npos || pos2 == std::string::npos) continue;
            std::string startAddrStr = line.substr(pos1 + 1, pos2 - pos1 - 1);
            std::string codeStr = line.substr(pos2 + 1);

            int startAddr = std::stoi(startAddrStr, nullptr, 16);

            // codeStr 是以 ^ 分隔的多個物件碼
            size_t start = 0;
            size_t end = codeStr.find('^');
            int memIndex = startAddr;
            while (end != std::string::npos) {
                std::string objCode = codeStr.substr(start, end - start);
                if (objCode.length() == 6) { // 3 bytes
                    int val = std::stoi(objCode, nullptr, 16);
                    memory[memIndex] = (val >> 16) & 0xFF;
                    memory[memIndex + 1] = (val >> 8) & 0xFF;
                    memory[memIndex + 2] = val & 0xFF;
                    memIndex += 3;
                } else if (objCode.length() == 4) { // 2 bytes
                    int val = std::stoi(objCode, nullptr, 16);
                    memory[memIndex] = (val >> 8) & 0xFF;
                    memory[memIndex + 1] = val & 0xFF;
                    memIndex += 2;
                } else if (objCode.length() == 2) { // 1 byte
                    int val = std::stoi(objCode, nullptr, 16);
                    memory[memIndex] = val & 0xFF;
                    memIndex += 1;
                }
                start = end + 1;
                end = codeStr.find('^', start);
            }
        } else if (line[0] == 'E') {
             // End record: E^startAddr
            size_t pos = line.find('^');
            if (pos != std::string::npos && line.length() > pos + 1) {
                std::string execAddrStr = line.substr(pos + 1);
                int execAddr = std::stoi(execAddrStr, nullptr, 16);
                registers["PC"] = execAddr;
            }
        }
    }
    std::cout << "物件程式已載入記憶體。" << std::endl;
}


    void run() {
        std::cout << "模擬執行開始..." << std::endl;
        if (registers["PC"] == 0) registers["PC"] = 0x1000;

        int step = 0;
        while (true) {
            int pc = registers["PC"];
            if (pc < 0 || pc + 2 >= MEM_SIZE) break;

            int instr = fetch_memory(pc);
            int opcode = (instr >> 16) & 0xFF;
            int address = instr & 0xFFFF;

            switch(opcode) {
                case 0x00: // LDA
                    registers["A"] = fetch_memory(address);
                    registers["PC"] += 3;
                    break;
                case 0x18: // ADD
                    registers["A"] += fetch_memory(address);
                    registers["PC"] += 3;
                    break;
                case 0x0C: // STA
                    load_memory(address, registers["A"]);
                    registers["PC"] += 3;
                    break;
                case 0x50: // LDCH
                    registers["A"] = (registers["A"] & 0xFFFF00) | (memory[address] & 0xFF);
                    registers["PC"] += 3;
                    break;
                case 0x54: // STCH
                    memory[address] = registers["A"] & 0xFF;
                    registers["PC"] += 3;
                    break;
                case 0x4C: // RSUB
                    std::cout << "遇到 RSUB，執行結束" << std::endl;
                    show_registers();
                    return;
                case 0x68: // LDB
                    registers["B"] = fetch_memory(address);
                    registers["PC"] += 3;
                    break;
                case 0x3C: // J
                    registers["PC"] = address;
                    break;
                case 0x30: // JEQ
                    if ((registers["SW"] & 0x4) != 0) // = 標誌
                        registers["PC"] = address;
                    else
                        registers["PC"] += 3;
                    break;
                case 0x48: // JSUB
                    registers["L"] = registers["PC"] + 3;
                    registers["PC"] = address;
                    break;
                case 0x04: // LDX
                    registers["X"] = fetch_memory(address);
                    registers["PC"] += 3;
                    break;
                case 0x10: // STX
                    load_memory(address, registers["X"]);
                    registers["PC"] += 3;
                    break;
                case 0x1C: // SUB
                    registers["A"] -= fetch_memory(address);
                    registers["PC"] += 3;
                    break;
                case 0x20: // MUL
                    registers["A"] *= fetch_memory(address);
                    registers["PC"] += 3;
                    break;
                case 0x24: // DIV
                    {
                        int divisor = fetch_memory(address);
                        if (divisor != 0)
                            registers["A"] /= divisor;
                        else
                            std::cerr << "除零錯誤!" << std::endl;
                        registers["PC"] += 3;
                    }
                    break;
                case 0x28: // COMP
                    {
                        int a = registers["A"];
                        int b = fetch_memory(address);
                        if (a < b)
                            registers["SW"] = 0x8;  // 小於
                        else if (a == b)
                            registers["SW"] = 0x4;  // 等於
                        else
                            registers["SW"] = 0x2;  // 大於
                        registers["PC"] += 3;
                    }
                    break;
            }

            std::cout << "Step " << step++ << " | PC: " << std::hex << pc
                    << " | 指令: " << std::setw(6) << std::setfill('0') << instr
                    << " | A: " << std::setw(6) <<(registers["A"] & 0xFFFFFF) 
                    << " | B: " << std::setw(6) <<(registers["B"] & 0xFFFFFF) 
                    << " | X: " << std::setw(6) <<(registers["X"] & 0xFFFFFF) 
                    << " | L: " << std::setw(6) <<(registers["L"] & 0xFFFFFF) 
                    << " | SW: " << std::setw(6) <<(registers["SW"] & 0xFFFFFF) 
                    << std::endl;

            if (step > 100) {
                std::cout << "執行步數超過限制，終止模擬" << std::endl;
                break;
            }
        }
        std::cout << "模擬執行結束" << std::endl;
        show_registers();
    }

};
SICXEMachine machine;

class SICXEInterpreter {
private:
    std::vector<std::string> program;
    bool running = true;

public:
    void load(const std::string& filename) {
        std::ifstream infile(filename);
        if (!infile) {
            std::cout << "無法開啟檔案: " << filename << std::endl;
            return;
        }
        program.clear();
        std::string line;
        while (std::getline(infile, line)) {
            program.push_back(line);
        }
        std::cout << "已載入程式: " << filename << std::endl;
    }

    void save(const std::string& filename) {
        std::ofstream outfile(filename);
        for (const auto& line : program) {
            outfile << line << std::endl;
        }
        std::cout << "已儲存程式到: " << filename << std::endl;
    }

    void clear() {
        program.clear();
        std::cout << "程式已清除" << std::endl;
    }

    void list() const {
        if (program.empty()) {
            std::cout << "目前沒有程式內容" << std::endl;
        } else {
            for (size_t i = 0; i < program.size(); ++i) {
                std::cout << i + 1 << ": " << program[i] << std::endl;
            }
        }
    }

    void edit(size_t line, const std::string& content) {
        if (line == 0 || line > program.size()) {
            std::cout << "行號錯誤" << std::endl;
            return;
        }
        program[line - 1] = content;
        std::cout << "第 " << line << " 行已修改" << std::endl;
    }

    void exit() {
        running = false;
        std::cout << "離開解譯器" << std::endl;
    }

    void run() {
        std::cout << "模擬執行程式..." << std::endl;
        machine.run();
        std::cout << "執行結束" << std::endl;
    }
   void assemble(const std::string& src, const std::string& obj) {
    int startAddr = 0, dataStart = -1, codeEndAddr = 0;
    auto lines = parse_source(src, dataStart);
    pass1(lines, startAddr, dataStart, codeEndAddr); // 傳入 codeEndAddr
    pass2(lines);
    write_object(lines, obj, startAddr, dataStart, codeEndAddr); // 傳入 codeEndAddr
    std::cout << "組譯完成，已產生物件程式: " << obj << std::endl;
}



    void repl() {
        std::string input;
        std::cout << "SICXE 解譯器啟動，輸入 'exit' 離開。" << std::endl;
        while (running) {
            std::cout << ">> ";
            std::getline(std::cin, input);
            std::istringstream iss(input);
            std::string cmd;
            iss >> cmd;
            if (cmd == "load") {
                std::string filename;
                iss >> filename;
                load(filename);
            } else if (cmd == "save") {
                std::string filename;
                iss >> filename;
                save(filename);
            } else if (cmd == "clear" || cmd == "new") {
                clear();
            } else if (cmd == "list") {
                list();
            } else if (cmd == "edit") {
                size_t line;
                iss >> line;
                std::string content;
                std::getline(iss, content);
                // 去除前導空白
                size_t pos = content.find_first_not_of(" ");
                if (pos != std::string::npos) content = content.substr(pos);
                edit(line, content);
            } else if (cmd == "exit") {
                exit();
            } else if (cmd == "run") {
                run();
            } else if (cmd.empty()) {
                continue;
            } else if (cmd == "loadobj") {
                std::string filename;
                iss >> filename;
                machine.load_object(filename);
            } else if (cmd == "assemble") {
                std::string src, obj;
                iss >> src >> obj;
                if (src.empty() || obj.empty()) {
                    std::cout << "用法: assemble [src.txt] [obj.obj]" << std::endl;
                } else {
                    assemble(src, obj); // 呼叫下面的組譯器函式
                }
            } else {
                std::cout << "未知指令。" << std::endl;
            }
        }
    }
};

int main() {
    SICXEInterpreter interpreter;
    interpreter.repl();
    return 0;
}
