#include <iostream>
//NO MORE STD LIB OK??
enum class ErrorCode {
    OK,
    NOT_FOUND,
};

uint32_t strlen(const char * data) {
    uint32_t sz = 0;
    while (data[sz] != 0) sz++;
    return sz;
}

char *strcpy(const char * from, char * to) {
    char ch;
    uint32_t pos = 0;
    while ((ch = from[pos]) != 0) to[pos++] = ch;
    return to;
}

void *memcpy(const void * from, void * to, uint32_t size) {
    for (uint32_t i = 0; i < size; i++) ((char*)to)[i] = ((char*)from)[i];
    return to;
}

//WHAT IS THIS???
// returns true if X and Y are same
int compare(const char *X, const char *Y) {
    while (*X && *Y) {
        if (*X != *Y)
            return 0;

        X++;
        Y++;
    }

    return (*Y == '\0');
}
 
// Function to implement strstr() function
char* strstr(char* X, char* Y) {
    while (*X != '\0') {
        if ((*X == *Y) && compare(X, Y))
            return X;
        X++;
    }
 
    return NULL;
}

template<class T>
class Vector {
public:
    T * cont;
    uint32_t capacity;
    uint32_t size;
    Vector(uint32_t n) {
        cont = new T[n];
        capacity = n;
        size = 0;
    }

    Vector() {
        cont = new T[0];
        capacity = 0;
        size = 0;
    }

    Vector(const Vector<T> &other) {
    	cont = new T[other.capacity];
    	capacity = other.capacity;
    	size = other.size;
    	for (uint32_t i = 0; i < size; i++) {
    	    cont[i] = T(other.cont[i]);
    	}
    }

    Vector<T> operator=(const Vector<T> &other) {
        capacity = other.capacity;
        size = other.size;
        cont = new T[other.capacity];
        for (uint32_t i = 0; i < size; i++) {
            cont[i] = T(other.cont[i]);
        }

        return *this;
    }

    ~Vector() {
        delete[] cont;
    }
    
    void push_back(T value) {
        if (capacity == 0) {
            cont = new T[2];
            capacity = 2;
        }
        else if (size == capacity) {
            T *new_cont = new T[capacity * 2];
            for (uint32_t i = 0; i < size; i++) {
                new_cont[i] = T(cont[i]);
            }
            delete[] cont;
            cont = new_cont;
            capacity *= 2;
        }
        cont[size++] = value;
    }

    T pop_back() {
        if (size == 0) {
            return T();
        }
        T a = cont[--size];
        cont[size].~T();
        return a;
    }

    T& operator[](uint32_t pos) {
        return cont[pos];
    }

    T *ptr(uint32_t pos) {
        return (cont + pos);
    }
};

template<class A, class B> 
class Pair {
public:
    A a;
    B b;
    Pair() {}
    Pair(A a, B b) {
        this->a = a;
        this->b = b;
    }
};

template<class K, class V>
class Map {
public:
    Vector<Pair<K, V>> cont;
    Map() {
        
    }
    
    void put(K key, V value) {
        for (uint32_t i = 0; i < cont.size; i++) {
            if (cont[i].a == key) {
                cont[i].b = value;
                return;
            }
        }
        cont.push_back(Pair<K, V>(key, value));
    }
    
    V get(K key, ErrorCode& code) {
        for (uint32_t i = 0; i < cont.size; i++) {
            if (cont[i].a == key) {
                code = ErrorCode::OK;
                return cont[i].b;
            }
        }
        code = ErrorCode::NOT_FOUND;
        return cont[0].b;
    }

    void extend(Map<K, V>& other) {
        for (uint32_t i = 0; i < other.cont.size; i++) {
            this->cont.push_back(other.cont[i]);
        }
    }
};

class String;
class CALType;

String operator+(const String& lhs, const String& rhs);

class String {
public:
    char * cont;
    uint32_t size;
    String() {
        size = 0;
        cont = nullptr;
    }

    String(const char data) {
        size = 1;
        cont = new char[1];
        cont[0] = data;
    }
    
    String(const char * data) {
        size = strlen(data);
        cont = new char[size];
        memcpy(data, cont, size);
    }

    String(char * data, uint32_t len) {
        cont = new char[len];
        size = len;
        memcpy(data, cont, len);
    }

    String(const String &str) {
        size = str.size;
        cont = new char[str.size];
        memcpy(str.cont, cont, str.size);
    }

    String operator=(const String &str) {
        size = str.size;
        char * newcont = new char[str.size];
        memcpy(str.cont, newcont, str.size);
        if (cont != nullptr) delete[] cont;
        cont = newcont;
        return *this;
    }

    bool operator==(const String& other) {
        if (cont == nullptr || other.cont == nullptr) return false;
        if (this->size != other.size) return false;
        for (uint32_t i = 0; i < this->size; i++) {
            if (this->cont[i] != other.cont[i]) return false;
        }
        return true;
    }

    bool operator!=(const String& other) {
        return !(*this == other);
    }
    
    ~String() {
        if (cont != nullptr) {
            delete[] cont;
        }
    }

    String replace(String rep, String with) {
        char *orig = new char[size + 1];
        char *orig_start = orig;
        memcpy(cont, orig, size);
        orig[size] = 0;
        
        char *result;
        char *ins;
        char *tmp;
        int len_front;

        int count;
        
        if (!size || !rep.size) return String("");
        if (rep.size == 0) return String("");
        if (!with.size)
            with = "";
    
        ins = orig;
        for (count = 0; (tmp = strstr(ins, rep.cont)); ++count) {
            ins = tmp + rep.size;
        }
    
        tmp = result = new char[size + (with.size - rep.size) * count + 1];

        while (count--) {
            ins = strstr(orig, rep.cont);
            len_front = ins - orig;
            tmp = /*strncpy*/(char*)memcpy(orig, tmp, len_front) + len_front;
            tmp = strcpy(with.cont, tmp) + with.size;
            orig += len_front + rep.size;
        }
        strcpy(orig, tmp);
        String res(result);
        delete[] orig_start;
        delete[] result;
        return res;
    }

    Vector<String> split(String delim) {
        char *part = cont;
        while (String(part, delim.size) == delim) part++;
        Vector<String> res;
        uint32_t pos = 0;
        while ((pos + part) <= (cont + size)) {
            if (String(part + pos, delim.size) == delim) {
                res.push_back(String(part, pos));
                part = part + pos + delim.size;
                pos = 0;
                while (String(part, delim.size) == delim) part++;
            } else {
                pos += 1;
            }
        }
        return res;
    }

    bool isDigit() {
        for (uint32_t i = 0; i < size; i++) {
            if (!(cont[i] >= '0' && cont[i] <= '9')) {
                return false;
            }
        }
        return true;
    }

    uint32_t toInt() {
        if (!isDigit()) return 0;
        uint32_t val = 0;
        for (uint32_t i = 0; i < size; i++) {
            val = val * 10 + (cont[i] - '0');
        }
        return val;
    }

    String operator+=(const String& b) {
        char *nstr = new char[size + b.size];
        memcpy(cont, nstr, size);
        memcpy(b.cont, nstr + size, b.size);
        delete[] cont;
        cont = nstr;
        size = size + b.size;
        return *this;
    }

    String operator+=(const CALType& type);
};

String operator+(const String& lhs, const String& rhs) {
    char *temp = new char[rhs.size + lhs.size];
    memcpy(lhs.cont, temp, lhs.size);
    memcpy(rhs.cont, temp + lhs.size, rhs.size);
    String res = String(temp, lhs.size + rhs.size);
    delete[] temp;
    return res;
}

String operator+(const String& a, const char * other) {
    char *temp = new char[strlen(other) + a.size];
    memcpy(a.cont, temp, a.size);
    strcpy(other, temp + a.size);
    String res = String(temp, strlen(other) + a.size);
    delete[] temp;
    return res;
}

String operator+(const char * a, const String& b) {
    char *temp = new char[strlen(a) + b.size];
    memcpy(b.cont, temp + strlen(a), b.size);
    strcpy(a, temp);
    String res = String(temp, strlen(a) + b.size);
    delete[] temp;
    return res;
}

std::ostream& operator<<(std::ostream& a, String b) {
    for (uint32_t i = 0; i < b.size; i++) {
        a << b.cont[i];
    }
    return a;
}

template<class A, class B>
std::ostream& operator<<(std::ostream& a, Pair<A, B> b) { return a; }

String operator*(const String& a, int b) {
    if (b == 0) { return String(""); }
    char *buf = new char[a.size * b];
    for (int i = 0; i < b; i++) {
        memcpy(a.cont, buf + i * a.size, a.size);
    }
    String e(buf, a.size * b);
    delete[] buf;
    return e;
}

void CALError(String message) {
    std::cout << "Error " << message << "\n";
}

enum class CALTypeEnum {
    INT,
    SYM,
    CODE,
    NONE,
};

class AST;

struct Function {
    AST * ast;
    Vector<String> binds;
};

class CALType {
public:
    int32_t i;
    String sym;
    Function code;
    CALTypeEnum type;

    CALType(String n) {
        sym = n;
        type = CALTypeEnum::SYM;
    }

    CALType(int32_t n) {
        i = n;
        type = CALTypeEnum::INT;
    }

    CALType() {
        type = CALTypeEnum::NONE;
    }

    CALType(Function code) {
        this->code = code;
        type = CALTypeEnum::CODE;
    }
    
};

String String::operator+=(const CALType& type) {
    String b = "";
    switch (type.type) {
        case CALTypeEnum::SYM: b = type.sym; break;
        case CALTypeEnum::INT: b = '0' + type.i; break;
        case CALTypeEnum::CODE: b = "code"; break;
        default: b = "ab"; break;
    }

    return (*this) += b;
}


//DEBUG
std::ostream& operator<<(std::ostream& a, CALType b);

class AST {
public:
    Vector<AST> nodes;
    CALType val;
    
    AST(int32_t value) : val(value) {}
    AST(String value)  : val(value) {}
    AST(CALType value) : val(value) {}

    AST(Vector<AST> nodes) {
        this->nodes = nodes;
    }

    void append(AST node) {
        this->nodes.push_back(node);
    }

    void append(String value) {
        nodes.push_back(AST(value));
    }

    void append(int value) {
        nodes.push_back(AST(value));
    }

    void append(CALType value) {
        nodes.push_back(AST(value));
    }

    AST() {
    }

    void print(int indent) {
        std::cout << String(" ") * indent << val << "\n";
        for (uint32_t i = 0; i < nodes.size; i++) {
            nodes[i].print(indent + 4);
        }
    }

    String toString() {
        String out = "";
        if (val.type != CALTypeEnum::NONE) {
            out += val;//String((char)((int)val.type + '0'));
        }

        if (nodes.size > 0) {
            out += " (";
            for (uint32_t i = 0; i < nodes.size; i++) {
                out += nodes[i].toString();
                out += " ";
            }
            out += ")";
        }
        return out;
    }

    AST operator[](uint32_t n) {
        return nodes[n];
    }

    AST* ptr(uint32_t n) {
        return nodes.ptr(n);
    }
};
std::ostream& operator<<(std::ostream& a, AST b) { return a; }

//DEBUG
std::ostream& operator<<(std::ostream& a, CALType b) {
    switch (b.type) {
        case CALTypeEnum::INT: return a << b.i;
        case CALTypeEnum::SYM: return a << b.sym;
        case CALTypeEnum::NONE: return a << "NONE";
        case CALTypeEnum::CODE: {
            a << "function(";
            for (uint32_t i = 0; i < b.code.binds.size; i++) {
                a << b.code.binds[i];
                if (i != b.code.binds.size - 1) {
                    a << ", ";
                }
            }
            a << ") -> " << b.code.ast->toString();
            return a;
        }
        default: return a;
    }
}

class Env {
public:
    Map<String, AST> map;
    Env *outer = nullptr;
    
    Env(Env *outer, Map<String, AST> binds) {
        this->outer = outer;
        map.extend(binds);
    }
    Env(Env *outer) {
        this->outer = outer;
    }
    Env(Map<String, AST> binds) {
        map.extend(binds);
    }
    Env() {}

    AST get(String key) {
        Env *env = find(key);
        if (env == nullptr) {
            CALError("Variable " + key + " was not found");
            return AST();
        }
        ErrorCode ec;
        AST v = env->map.get(key, ec);
        if (ec != ErrorCode::OK) {
            CALError("Variable " + key + " was not found");
            return AST();
        }
        return v;
    }

    void put(String key, AST value) {
        map.put(key, value);
    }

    Env *find(String key) {
        ErrorCode ec;
        map.get(key, ec);
        if (ec == ErrorCode::OK) {
            return this;
        } else {
            if (outer != nullptr) {
                return outer->find(key);
            }
        }
        return nullptr;
    }
};

class Reader {
public:
    String source;
    uint32_t pos = 0;
    Vector<String> tokens;
    Reader(String source) {
        this->source = source;
        tokenise();
    }
    Reader(Vector<String> nTokens) {
        tokens = nTokens;
    }

    uint32_t size() {
        return tokens.size;
    }

    void tokenise() {
        tokens = source.replace("(", " ( ").replace(")", " ) ").split(" ");
    }

    CALType atom(String token) {
        if (token.isDigit()) {
            return CALType((int32_t)token.toInt());
        } else {
            return CALType(token);
        }
    }

    String next() {
        return tokens[pos++];
    }

    String peek() {
        return tokens[pos];
    }

    String peek(int n) {
        return tokens[n];
    }
};

AST parse(Reader reader) {
    AST out;
    if (reader.peek() == "(") {
        if (reader.peek(reader.size() - 1) != ")") {
            CALError("Syntax Error: Expected ')' to close '('");
        }
        reader.next();
    }

    while (reader.pos < reader.size()) {
        if (reader.peek() == "(") {
            reader.next();

            int num_left = 0;
            int num_right = 0;
            Vector<String> inside_param;
            while (!(num_right > num_left)) {
                if (reader.peek() == "(") {
                    num_left += 1;
                } else if (reader.peek() == ")") {
                    num_right += 1;
                }
                inside_param.push_back(reader.next());
            }
            inside_param.pop_back();
            out.append(parse(Reader(inside_param)));
        } else if (reader.peek() != ")") {
            out.append(reader.atom(reader.next()));
        } else {
            reader.next();
        }
    }

    return out;
}

AST evaluate(AST ast, Env env);
AST eval_ast(AST ast, Env env);

AST eval_operator(AST ast, Env env) {
    if (ast[0].val.type == CALTypeEnum::INT) {
        switch (ast[0].val.i) {
            case '+':
                return ast[1].val.i + ast[2].val.i;
            case '-':
                return ast[1].val.i - ast[2].val.i;
            case '*':
                return ast[1].val.i * ast[2].val.i;
            case '/':
                return ast[1].val.i / ast[2].val.i;
            default: return AST();
        }
    } else if (ast[0].val.type == CALTypeEnum::CODE) {
        Map<String, AST> binds;
        for (uint32_t i = 0; i < ast[0].val.code.binds.size; i++) {
            binds.put(ast[0].val.code.binds[i], ast[i + 1]);
        }
        Env newenv(&env, binds);
        ast[0].val.code.ast->print(0);
        AST ret = evaluate(*ast[0].val.code.ast, newenv);
        std::cout << "AST EVALUATE THING\n";
        ret.print(0);
        std::cout << "AST EVALUATE THING END\n";
        return ret;
    }
}

AST eval_ast(AST ast, Env env) {
    if (ast.val.type == CALTypeEnum::SYM) {
        AST e = env.get(ast.val.sym);
    }
    if (ast.nodes.size > 0) {
        AST out;
        for (uint32_t i = 0; i < ast.nodes.size; i++) {
            out.append(evaluate(ast[i], env));
        }
        return out;
    }
    return ast;
}

AST evaluate(AST ast, Env env) {
    if (ast.nodes.size > 0) {
        if (ast[0].val.type == CALTypeEnum::SYM && ast[0].val.sym == "fn") {
            Vector<String> vec;
            for (uint32_t i = 0; i < ast[1].nodes.size; i++) {
                vec.push_back(ast[1].nodes[i].val.sym);
            }
            return AST(Function{new AST(ast[2]), vec});
        } else {
            AST x = eval_ast(ast, env);
            //x.print(0);
            return eval_operator(x, env);
        }
    } else {
        return eval_ast(ast, env);
    }
}


int main() {
    //std::cout << String("(a (b c (d e f g))(h i))").replace("(", " ( ").replace(")", " ) ") << "\n";
    Reader reader("((fn (a b) (+ a b)) 1 2)");
    Env env;
    env.put("+", AST((int)'+'));
    env.put("-", AST((int)'-'));
    env.put("*", AST((int)'*'));
    env.put("/", AST((int)'/'));
    AST ast = parse(reader);
    ast.print(0);
    std::cout << "\n\n\n";
    AST res = evaluate(ast, env);
    //res.print(0);
    return 0;
}
