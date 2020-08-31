#include <vector>
#include <cstdint>
#include <string>
#include <iostream>

enum DataType {
    INT,
    DOUBLE,
    STRING,
    CHAR,
    FUNCTION,
    NONE,
};

enum Opcodes {
    NOP,
    ADD,
    SUB,
    OUT,
    PUSH,
    HALT,
};

uint8_t code[] = {
    PUSH,
    1,
    PUSH,
    0,
    SUB,
    OUT,
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
};

stack_t data[] = {
    stack_t(2),
    stack_t(1),
};

void writeUInt32(uint32_t x, std::vector<uint8_t> &v) {
    v.push_back((x >> 24) & 0xFF);
    v.push_back((x >> 16) & 0xFF);
    v.push_back((x >> 8) & 0xFF);
    v.push_back((x >> 0) & 0xFF);
}

int main() {
    std::vector<uint8_t> output;
    writeUInt32(sizeof(code), output);
    for (int i = 0; i < sizeof(code); i++) {
        output.push_back(code[i]);
    }
    writeUInt32(sizeof(data) / sizeof(stack_t) * 9, output);
    for (int i = 0; i < sizeof(data) / sizeof(stack_t); i++) {
        output.push_back((uint8_t)data[i].type);
        switch (data[i].type) {
            case DataType::INT: writeUInt32(4, output); writeUInt32(*data[i].iptr, output); break;
        }
    }
    for (int i = 0; i < output.size(); i++) {
        std::cout << output[i];
    }
    return 0;
}
