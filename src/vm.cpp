#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>

enum DataType {
    INT,
    DOUBLE,
    STRING,
    CHAR,
    FUNCTION,
    NONE,
};

void fatal(std::string msg) {
    std::cout << "Fatal: " << msg;
}

enum Opcodes {
    NOP,
    ADD,
    SUB,
    OUT,
    PUSH,
    HALT,
};

struct stack_t {
    DataType type;
    union {
        int32_t * iptr;
        double * dptr;
        std::string * strptr;
        int8_t * cptr;
    };

    stack_t() {
        type = DataType::NONE;
    }

    stack_t(int val) {
        iptr = new int[1]{val};
    }
    stack_t(double val) {
        dptr = new double[1]{val};
    }
    stack_t(std::string val) {
        strptr = new std::string(val);
    }
    stack_t(int8_t val) {
        cptr = new int8_t[1]{val};
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
            fatal("Try to add different types\n");
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
    }
}

stack_t * constants;
uint32_t * code;

int sp = 0;
int pc = 0;
bool running = true;

stack_t stack[1000];

void push(stack_t value) {
    stack[sp++] = value;
}

stack_t pop() {
    return stack[--sp];
}

void step() {
    char instr = code[pc++];
    switch (instr) {
        case NOP: break;
        case ADD: push(pop() + pop()); break;
        case SUB: { stack_t tmp = pop(); push(pop() - tmp); break; }
        case OUT: std::cout << pop(); break;
        case PUSH: push(constants[code[pc++]]); break;
        case HALT: running = false; break;
        default: std::cout << "unknown opcode " << instr << "\n"; break;
    }
}

void run() {
    while (running) {
        step();
    }
}

uint32_t readUInt32(std::vector<uint8_t> vals, int pos) {
    uint32_t out = 0;
    for (int i = 0; i < 4; i++) {
        out = (out << 8) | vals[pos + i];
    }
    return out;
}

uint32_t readBytes(std::vector<uint8_t> buffer, uint16_t offset, uint32_t *&data) {
    uint32_t size = readUInt32(buffer, offset);
    data = new uint32_t[size];
    for (int i = 0; i < size; i++) {
        data[i] = buffer[i + 4 + offset];
    }
    uint32_t bytesRead = size + 4;
    return bytesRead;
}

int main(int argc, char ** argv) {
    if (argc < 2) {
       std::cout << "Usage: " << argv[0] << " filename\n";
       return -1;
    }
    std::ifstream inp(argv[1], std::ios::binary);
    if (!inp) {
        std::cout << "Could not read file!\n";
        return -1;
    }
    std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(inp), {});
    inp.close();
    uint32_t codeSize = readBytes(buffer, 0, code);
    uint32_t dataSize = readBytes(buffer, codeSize, (uint32_t *)constants);
    run();
    delete[] code;
    delete[] constants;
    return 0;
}
