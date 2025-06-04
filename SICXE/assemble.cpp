#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <map>
#include <vector>
#include <algorithm>

using namespace std;

map<string, int> OPTAB = {
    {"LDA", 0x00}, {"ADD", 0x18}, {"STA", 0x0C},
    {"LDCH", 0x50}, {"STCH", 0x54}, {"RSUB", 0x4C},
    {"LDB", 0x68}, {"J", 0x3C}, {"JEQ", 0x30},
    {"JSUB", 0x48}, {"LDX", 0x04}, {"STX", 0x10},
    {"SUB", 0x1C}, {"MUL", 0x20}, {"DIV", 0x24},
    {"COMP", 0x28}
};


struct Line {
    string label, opcode, operand;
    int address;
    string objectCode;
    bool isData = false;
};

map<string, int> SYMTAB;

int hexstr_to_int(const string& s) {
    return std::stoi(s, nullptr, 16); // 必須用 base=16
}

string int_to_hexstr(int x, int width=6) {
    stringstream ss;
    ss << uppercase << setfill('0') << setw(width) << hex << x;
    return ss.str();
}

vector<Line> parse_source(const string& filename, int& dataStart) {
    ifstream infile(filename);
    vector<Line> lines;
    string line;
    bool inData = false;
    while (getline(infile, line)) {
        istringstream iss(line);
        string label, opcode, operand;
        if (line.empty()) continue;
        if (line[0] != ' ' && line[0] != '\t') iss >> label;
        else label = "";
        iss >> opcode >> operand;
        if (opcode == "ORG") {
            inData = true;
            dataStart = hexstr_to_int(operand);
            lines.push_back({label, opcode, operand, 0, "", inData});
            continue;
        }
        lines.push_back({label, opcode, operand, 0, "", inData});
    }
    return lines;
}

void pass1(vector<Line>& lines, int& startAddr, int& dataStart, int& codeEndAddr) {
    int locctr = 0;
    bool inData = false;
    SYMTAB.clear();
    // 處理 START 指令
    if (lines.size() > 0 && lines[0].opcode == "START") {
        startAddr = hexstr_to_int(lines[0].operand);
        locctr = startAddr;
        lines[0].address = locctr;
    }

    // 處理每一行
    for (size_t i = 1; i < lines.size(); ++i) {
        // 處理 ORG 指令
        cout << "處理行: " << lines[i].label << " " << lines[i].opcode << " " << lines[i].operand << endl;
        if (lines[i].opcode == "ORG") {
            inData = true;
            dataStart = hexstr_to_int(lines[i].operand);
            locctr = dataStart;
            cout << "Data section starts at: " << dataStart << endl;
            if (!lines[i].label.empty()) {
                SYMTAB[lines[i].label] = locctr;
            }
            lines[i].address = locctr;
            continue;
        }

        // 設定目前行位址
        lines[i].address = locctr;

        // 處理標籤
        if (!lines[i].label.empty()) {
            // 檢查標籤是否重複
            if (SYMTAB.count(lines[i].label)) {
                cerr << "錯誤: 重複標籤 " << lines[i].label << endl;
            } else {
                SYMTAB[lines[i].label] = locctr;
            }
        }

        // 更新位址計數器
        if (OPTAB.count(lines[i].opcode)) {
            locctr += 3;
            if (!inData) codeEndAddr = locctr; // 只在程式區時更新
        } else if (lines[i].opcode == "WORD") locctr += 3;
        else if (lines[i].opcode == "RESW") locctr += 3 * stoi(lines[i].operand);
        else if (lines[i].opcode == "RESB") locctr += stoi(lines[i].operand);
        else if (lines[i].opcode == "BYTE") {
            if (lines[i].operand[0] == 'C')
                locctr += lines[i].operand.length() - 3;
            else if (lines[i].operand[0] == 'X')
                locctr += (lines[i].operand.length() - 3) / 2;
        }
    }
}

void pass2(vector<Line>& lines) {
    for (auto& line : lines) {
        if (OPTAB.count(line.opcode)) {
            int code = OPTAB[line.opcode] << 16;
            int addr = 0;
            if (line.opcode != "RSUB" && SYMTAB.count(line.operand)) {
                addr = SYMTAB[line.operand];
            }
            line.objectCode = int_to_hexstr(code | addr);
        } else if (line.opcode == "WORD") {
            line.objectCode = int_to_hexstr(stoi(line.operand));
        } else if (line.opcode == "BYTE" && line.operand[0] == 'C') {
            string s = line.operand.substr(2, line.operand.size()-3);
            stringstream ss;
            for (char c : s) ss << setw(2) << setfill('0') << hex << (int)c;
            line.objectCode = ss.str();
        }
    }
}

void write_object(const std::vector<Line>& lines, const std::string& filename,int startAddr, int dataStart, int codeEndAddr) {
    std::ofstream outfile(filename);
    if (!outfile) {
        std::cerr << "無法開啟輸出檔案: " << filename << std::endl;
        return;
    }

    // H 記錄：程式名稱、起始位址、程式長度（僅指令區）
    int dataEndAddr = dataStart;
    for (const auto& line : lines) {
        if (line.isData) {
            if (line.opcode == "WORD") dataEndAddr += 3;
            else if (line.opcode == "RESW") dataEndAddr += 3 * stoi(line.operand);
            else if (line.opcode == "RESB") dataEndAddr += stoi(line.operand);
            else if (line.opcode == "BYTE") {
                if (line.operand[0] == 'C') dataEndAddr += line.operand.length() - 3;
                else if (line.operand[0] == 'X') dataEndAddr += (line.operand.length() - 3) / 2;
            }
        }
    }
    int progLen = (codeEndAddr - startAddr) + (dataEndAddr - dataStart);
    outfile << "H^" << std::setw(6) << std::left << std::setfill(' ')
            << lines[0].label << "^" << int_to_hexstr(startAddr, 6)
            << "^" << int_to_hexstr(progLen, 6) << std::endl;

    // 指令區 T 記錄
    int codeByteCount = 0;
    string codeBuf;
    for (const auto& line : lines) {
        if (!line.isData && !line.objectCode.empty()) {
            codeBuf += line.objectCode+ "^";
            codeByteCount += line.objectCode.length() / 2; // 每6字元為3 bytes
        }
    }
    outfile << "T^" << int_to_hexstr(startAddr, 6) << "^"
            //<< int_to_hexstr(codeByteCount, 2) << "^"
            << codeBuf << endl;

    // 資料區 T 記錄（僅當有資料區時）
    int dataByteCount = 0;
    string dataBuf;
    for (const auto& line : lines) {
        if (line.isData) {
            if (!line.objectCode.empty()) {
                string upperObjCode = line.objectCode;
                transform(upperObjCode.begin(), upperObjCode.end(), upperObjCode.begin(), ::toupper);
                dataBuf += upperObjCode + "^";
                dataByteCount += line.objectCode.length() / 2;
            } else if (line.opcode == "RESW") {
                int n = stoi(line.operand);
                for (int i = 0; i < n; ++i) {
                    dataBuf += "000000^";
                    dataByteCount += 3;
                }
            } else if (line.opcode == "RESB") {
                int n = stoi(line.operand);
                for (int i = 0; i < n; ++i) {
                    dataBuf += "00^";
                    dataByteCount += 1;
                }
            }
        }
    }
    if (dataByteCount > 0) {
        outfile << "T^" << int_to_hexstr(dataStart, 6) << "^"
                //<< int_to_hexstr(dataByteCount, 2) << "^"
                << dataBuf << endl;
    }

    // E 記錄：程式入口點（假設為 FIRST 標籤）
    if (SYMTAB.find("FIRST") == SYMTAB.end()) {
        std::cerr << "錯誤：找不到程式入口點 FIRST" << std::endl;
        return;
    }
    int execAddr = SYMTAB["FIRST"];
    outfile << "E^" << int_to_hexstr(execAddr, 6) << std::endl;
}

