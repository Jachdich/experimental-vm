//#include <iostream>
//NO MORE STD LIB OK??
#include <cstdlib>
typedef unsigned int uint32_t;
enum class ErrorCode {
    OK,
    NOT_FOUND,
};

template<class T>
class Vector {
public:
    T * cont;
    uint32_t capacity;
    uint32_t size;
    Vector(uint32_t n) {
        cont = (T*)malloc(n * sizeof(T));
        capacity = n;
        size = 0;
    }

    Vector() {
        cont = (T*)malloc(2 * sizeof(T));
        capacity = 2;
        size = 0;
    }

    ~Vector() {
        free(cont);
    }
    
    void push_back(T value) {
        if (capacity == 0) {
            cont = (T*)realloc(cont, 2 * sizeof(T));
            capacity = 2;
        }
        else if (size == capacity) {
            cont = (T*)realloc(cont, capacity * 2 * sizeof(T));
            capacity *= 2;
        }
        cont[size++] = value;
    }

    T& operator[](uint32_t pos) {
        return cont[pos];
    }
};

template<class A, class B> 
class Pair {
public:
    A a;
    B b;
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
        for (int i = 0; i < other.cont.size; i++) {
            this->cont.push_back(other.cont[i]);
        }
    }
};

uint32_t strlen(const char * data) {
    uint32_t sz = 0;
    while (data[sz] != 0) sz++;
    return sz;
}

void strcpy(const char * a, char * b) {
    char ch;
    uint32_t pos = 0;
    while ((ch = a[pos]) != 0) {
        b[pos] = ch;
        pos++;
    }
}

void memcpy(void * from, void * to, uint32_t size) {
    
}

class String {
public:
    char * cont;
    uint32_t size;
    String() {
        cont = new char[0];
        size = 0;
    }
    
    String(const char * data) {
        size = strlen(data);
        cont = new char[size];
        strcpy(data, cont);
    }

    String(const String &str) {
        size = str.size;
//        cont = strcpy()
    }

    bool operator==(const String& other) {
        if (this->size != other.size) return false;
        for (uint32_t i = 0; i < this->size; i++) {
            if (this->cont[i] != other.cont[i]) return false;
        }
        return true;
    }
    
    ~String() {
        delete[] cont;
    }

    String operator+(const String& other) {
        char *temp = new char[other.size + this->size];
        strcpy(cont, temp);
        strcpy(other.cont, temp + size);
        String res = String(temp);
        delete[] temp;
        return res;
    }

    String operator+(const char * other) {
        char *temp = new char[strlen(other) + this->size];
        strcpy(cont, temp);
        strcpy(other, temp + size);
        String res = String(temp);
        delete[] temp;
        return res;
    }
};

String operator+(const char * a, String& b) {
    char *temp = new char[strlen(a) + b.size];
    strcpy(b.cont, temp);
    strcpy(a, temp + b.size);
    String res = String(temp);
    delete[] temp;
    return res;
}

//DEBUG
//std::ostream& operator<<(std::ostream& a, String b) { return a << b.cont; }

void CALError(String message) {
//    std::cout << "Error " << message << "\n";
}

class Env {
private:
    Map<String, uint32_t> map;
    Env *outer;
public:
    Env(Env *outer, Map<String, uint32_t> binds) {
        this->outer = outer;
        map.extend(binds);
    }

    uint32_t get(String key) {
        ErrorCode ec;
        uint32_t v = map.get(key, ec);
        if (ec != ErrorCode::OK) {
            CALError("Variable " + key + " was not found");
        }
    }
};

int main() {
    String a = "aree";
    CALError("Asd " + a + " reeeeeee");
}