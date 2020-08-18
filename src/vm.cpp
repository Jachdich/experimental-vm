#include <iostream>
#include <string>

enum DataType {
    INT,
    DOUBLE,
    STRING,
    CHAR,
    FUNCTION,
};

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
    void *data;
    stack_t operator+(stack_t other) {
        switch (type) {
            case INT: return {INT, new int[1] {*static_cast<int*>(data) + *static_cast<int*>(other.data)}};
            case DOUBLE: return {DOUBLE, new double[1] {*static_cast<double*>(data) + *static_cast<double*>(other.data)}};
            case STRING: return {STRING, new std::string(*static_cast<std::string*>(data) + *static_cast<std::string*>(other.data))};
        }
    }

    stack_t operator-(stack_t other) {
        return {INT, (void*)nullptr};
    }
};

std::ostream &operator<<(std::ostream& a, const stack_t& b) {
    switch (b.type) {
        case INT: return a << std::to_string(*static_cast<int*>(b.data));
        case DOUBLE: return a << std::to_string(*static_cast<double*>(b.data));
        case STRING: return a << *static_cast<std::string*>(b.data);
    }
}

stack_t constants[2];

char code[] = {
    PUSH,
    0x0,
    PUSH,
    0x1,
    ADD,
    OUT,
    HALT,
};

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

int main(int argc, char ** argv) {
    //if (argc < 2) {
    //    usage();
    //}
    //std::ifstream inp;
    //inp.open(argv[1]);
    //std::string content((std::istreambuf_iterator<char>(afile)), (std::istreambuf_iterator<char>()));
    //inp.close();
    constants[0].type = STRING;
    constants[1].type = STRING;
    constants[0].data = new std::string("hey guys I bet this");
    constants[1].data = new std::string(" doesn't work\n");
    run();
    return 0;

}
