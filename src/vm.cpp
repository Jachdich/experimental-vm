#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>
#include <sstream>

enum DataType {
    INT,
    DOUBLE,
    STRING,
    CHAR,
    FUNCTION,
    NONE,
    BOOLEAN,
};

void fatal(std::string msg) {
    std::cout << "Fatal: " << msg;
}

uint32_t readUInt32(std::vector<uint8_t> vals, int pos) {
    uint32_t out = 0;
    for (int i = 0; i < 4; i++) {
        out = out | vals[pos + i] << 8 * (3 - i);
    }
    return out;
}

enum Opcodes {
    NOP,
    ADD,
    SUB,
    OUT,
    PUSH,
    LSTACK,
    SSTACK,
    CALL,
    RET,
    HALT,
    CMP,
    JZ,
    JP,
    POP,
    POP_UNDER,
};

struct stack_t {
    DataType type;
    union {
        int32_t * iptr;
        double * dptr;
        std::string * strptr;
        int8_t * cptr;
        bool boolean;
    };

    stack_t copy() {
        switch (type) {
        case INT: return stack_t(*iptr);
        case DOUBLE: return stack_t(*dptr);
        case CHAR: return stack_t(*cptr);
        case STRING: return stack_t(*strptr);
        case BOOLEAN: return stack_t(boolean);
        default: return stack_t();
        }
    }

    stack_t() {
        type = DataType::NONE;
    }

    stack_t(int32_t val) {
        iptr = new int[1]{val};
        type = DataType::INT;
    }
    stack_t(double val) {
        dptr = new double[1]{val};
        type = DataType::DOUBLE;
    }
    stack_t(std::string val) {
        strptr = new std::string(val);
        type = DataType::STRING;
    }
    stack_t(int8_t val) {
        cptr = new int8_t[1]{val};
        type = DataType::CHAR;
    }

    stack_t(bool val) {
        this->boolean = val;
        type = DataType::BOOLEAN;
    }

    bool asBoolean() {
        if (type == DataType::BOOLEAN) {
            return boolean;
        } else {
            if (type != DataType::NONE) {
                return true;
            }
        }
        return false;
    }

    stack_t operator==(const stack_t& other) {
        if (this->type != other.type) return stack_t(false);
        switch (type) {
            case INT: return stack_t(*iptr == *other.iptr);
            default: return stack_t(false);
        }
    }

    stack_t operator+(stack_t other) {
        if (type != other.type) {
            fatal("Try to add different types\n");
        }
        switch (type) {
            case INT: return stack_t(*iptr + *other.iptr);
            case DOUBLE: return stack_t(*dptr + *other.dptr);
            case STRING: return stack_t(*strptr + *other.strptr);
        }
    }

    stack_t operator-(stack_t other) {
        if (type != other.type) {
            fatal("Try to subtract different types\n");
        }
        switch (type) {
            case INT: return stack_t(*iptr - *other.iptr);
            case DOUBLE: return stack_t(*dptr - *other.dptr);
            case STRING: fatal("Try to subtract strings\n"); break;
        }
    }
};

std::ostream &operator<<(std::ostream& a, const stack_t& b) {
    switch (b.type) {
        case INT: return a << std::to_string(*b.iptr);
        case DOUBLE: return a << std::to_string(*b.dptr);
        case STRING: return a << *b.strptr;
        case BOOLEAN: if (b.boolean) { return a << "true"; } else { return a << "false"; }
    }
}

std::vector<stack_t> constants;
std::vector<uint8_t> code;

int sp = 0;
int bp = 0;
int pc = 0;
int ret_sp = 0;
bool running = true;
bool debug = false;

stack_t stack[1000];
int ret_stack[1000];


void push(stack_t value) {
    stack[sp++] = value;
}

stack_t pop() {
    return stack[--sp];
}

std::string getInstrName(char opcode) {
    switch (opcode) {
        case NOP: return "NOP";
        case ADD: return "ADD";
        case SUB: return "SUB";
        case OUT: return "OUT";
        case PUSH: return "PUSH";
        case LSTACK: return "LSTACK";
        case SSTACK: return "SSTACK";
        case CALL: return "CALL";
        case RET: return "RET";
        case HALT: return "HALT";
        case CMP: return "CMP";
        case JZ: return "JZ";
        case JP: return "JP";
        case POP: return "POP";
        case POP_UNDER: return "POP_UNDER";
        default: return std::to_string((int)opcode);
    }
}

void step() {
    char instr = code[pc++];
    if (debug) {
        std::stringstream debugLine;
        for (int i = 0; i < sp; i++) {
            debugLine << stack[i] << " ";
        }
        debugLine << std::string(30 - debugLine.str().size(), ' ');
        debugLine << getInstrName(instr) << std::string(12 - getInstrName(instr).size(), ' ');
        debugLine << bp << std::string(10 - std::to_string(bp).size(), ' ');
        debugLine << sp << std::string(10 - std::to_string(sp).size(), ' ');
        debugLine << pc << "\n";
        std::cout << debugLine.str();
    }
    switch (instr) {
        case NOP: break;
        case ADD: push(pop() + pop()); break;
        case SUB: { stack_t tmp = pop(); push(tmp - pop()); break; }
        case OUT: std::cout << pop(); break;
        case PUSH: push(constants[readUInt32(code, pc)]); pc += 4; break;
        case LSTACK:
            push(stack[bp + readUInt32(code, pc)].copy());
            pc += 4;
            break;
        case SSTACK:
            stack[bp + readUInt32(code, pc)] = pop();
            pc += 4;
            break;
        case CALL:
            ret_stack[ret_sp++] = pc + 8; //store pc
            ret_stack[ret_sp++] = bp;     //store base pointer
            ret_stack[ret_sp++] = readUInt32(code, pc); //store num of parameters
            bp = sp - readUInt32(code, pc);
            pc += 4;
            pc = readUInt32(code, pc);
            break;
        case RET:
            sp = bp + 1;// + ret_stack[--ret_sp] + 1;
            --ret_sp;
            bp = ret_stack[--ret_sp];
            pc = ret_stack[--ret_sp];
            break;
        case HALT: running = false; break;
        case CMP:  push(pop() == pop()); break;
        case JZ: {
            uint32_t address = readUInt32(code, pc);
            pc += 4;
            if (pop().asBoolean()) {
                pc = address;
            }
            break;
        }
        case POP: pop(); break;
        case POP_UNDER: {
            stack_t tmp = pop();
            sp--;
            push(tmp); //TODO more efficient version
            break;
        }
        case JP: pc = readUInt32(code, pc); break;
        default:   std::cout << "unknown opcode " << (int)instr << "\n"; break;
    }
}

void run() {
    while (running) {
        step();
    }
}

uint32_t readBytes(std::vector<uint8_t>& buffer, uint16_t offset, std::vector<uint8_t> * data) {
    uint32_t size = readUInt32(buffer, offset);
    data->resize(size);
    for (int i = 0; i < size; i++) {
        (*data)[i] = buffer[i + 4 + offset];
    }
    uint32_t bytesRead = size + 4;
    return bytesRead;
}

void readConstantsFromBytes(std::vector<uint8_t> bytes, std::vector<stack_t> &consts) {
    uint32_t pos = 0;
    //TODO alocate size instead of push_back
    while (pos < bytes.size()) {
        uint8_t type = bytes[pos]; pos++;
        uint32_t size = readUInt32(bytes, pos); pos += 4;
        switch ((DataType) type) {
            case DataType::INT: {
                if (size != 4) {
                    fatal("Error reading file: INT datatype != 4 bytes\n");
                } else {
                    int32_t val = (int32_t)readUInt32(bytes, pos); pos += 4;
                    consts.push_back(stack_t(val));
                }
                break;
            }
            case DataType::STRING: {
                std::vector<uint8_t> buf(size);
                for (int i = 0; i < size; i++) {
                    buf.push_back(bytes[pos++]);
                }
                consts.push_back(stack_t(std::string(buf.begin(), buf.end())));
            }

        }
    }
}

/*
type: 1 byte;
size: 4 bytes;
data: size bytes read into stack_t;
*/

int main(int argc, char ** argv) {
    if (argc < 2) {
       std::cout << "Usage: " << argv[0] << " filename\n";
       return -1;
    }
    if (argc == 3) {
        if (std::string(argv[2]) == "-d") {
            debug = true;
            std::cout << "Stack                         ";
            std::cout << "Instruction ";
            std::cout << "BasePtr   ";
            std::cout << "StackPtr  ";
            std::cout << "ProgCounter\n";
            std::cout << "-------------------------------------------------------------------------\n";
        }
    }
    std::ifstream inp(argv[1], std::ios::binary);
    if (!inp) {
        std::cout << "Could not read file!\n";
        return -1;
    }
    std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(inp), {});
    inp.close();
    std::vector<uint8_t> constantsBytes;
    uint32_t codeSize = readBytes(buffer, 0, &code);
    uint32_t dataSize = readBytes(buffer, codeSize, &constantsBytes);
    readConstantsFromBytes(constantsBytes, constants);
    run();
    return 0;
}
