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

void strcpy(const char * a, char * b) {
    char ch;
    uint32_t pos = 0;
    while ((ch = a[pos]) != 0) {
        b[pos] = ch;
        pos++;
    }
}

void memcpy(void * from, void * to, uint32_t size) {
    for (uint32_t i = 0; i < size; i++) {
    	((char*)to)[i] = ((char*)from)[i];
    }
}

template<class T>
class Vector {
public:
    T * cont;
    uint32_t capacity;
    uint32_t size;
    Vector(uint32_t n) {
        if (n < 2) { n = 2; }
        cont = new T[n];
        capacity = n;
        size = 0;
    }

    Vector() {
        cont = new T[2];
        capacity = 2;
        size = 0;
    }

    Vector(const Vector<T> &other) {
    	cont = new T[other.capacity];
    	capacity = other.capacity;
    	size = other.size;
    	memcpy(other.cont, cont, other.size);
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
            memcpy(cont, new_cont, capacity);
            delete[] cont;
            cont = new_cont;
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
    Pair() {}
    Pair(A a, B b) {
        this->a = a;
        this->b = b;
    }

    Pair(const Pair& other) {
        this->a = A(other.a);
        this->b = B(other.b);
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

class String {
public:
    char * cont;
    uint32_t size;
    String() {
        size = 0;
        cont = nullptr;
    }
    
    String(const char * data) {
        size = strlen(data);
        cont = new char[size];
        strcpy(data, cont);
    }

    String(const String &str) {
        size = str.size;
        cont = new char[str.size];
        memcpy(str.cont, cont, str.size);
    }

    String operator=(const String &str) {
        size = str.size;
        newcont = new char[str.size];
        memcpy(str.cont, newcont, str.size);
        if (cont != nullptr) delete[] cont;
        cont = newcont;
    }

    bool operator==(const String& other) {
        if (cont == nullptr || other.cont == nullptr) return false;
        if (this->size != other.size) return false;
        for (uint32_t i = 0; i < this->size; i++) {
            if (this->cont[i] != other.cont[i]) return false;
        }
        return true;
    }
    
    ~String() {
        if (cont != nullptr) {
            delete[] cont;
        }
    }
};

String operator+(const String& lhs, const String& rhs) {
    char *temp = new char[rhs.size + lhs.size];
    strcpy(lhs.cont, temp);
    strcpy(rhs.cont, temp + lhs.size);
    String res = String(temp);
    delete[] temp;
    return res;
}

String operator+(const String& a, const char * other) {
    char *temp = new char[strlen(other) + a.size];
    strcpy(a.cont, temp);
    strcpy(other, temp + a.size);
    String res = String(temp);
    delete[] temp;
    return res;
}

String operator+(const char * a, const String& b) {
    char *temp = new char[strlen(a) + b.size];
    strcpy(b.cont, temp + b.size);
    strcpy(a, temp);
    String res = String(temp);
    delete[] temp;
    return res;
}

//DEBUG
std::ostream& operator<<(std::ostream& a, String b) { return a << b.cont; }

void CALError(String message) {
    std::cout << "Error " << message << "\n";
}

class Env {
public:
    Map<String, uint32_t> map;
    Env *outer = nullptr;
    
    Env(Env *outer, Map<String, uint32_t> binds) {
        this->outer = outer;
        map.extend(binds);
    }
    Env(Env *outer) {
        this->outer = outer;
    }
    Env(Map<String, uint32_t> binds) {
        map.extend(binds);
    }
    Env() {}

    uint32_t get(String key) {
        Env *env = find(key);
        if (env == nullptr) {
            CALError("Variable " + key + " was not found");
            return -1;
        }
        ErrorCode ec;
        uint32_t v = env->map.get(key, ec);
        if (ec != ErrorCode::OK) {
            CALError("Variable " + key + " was not found");
            return -1;
        }
        return v;
    }

    void put(String key, uint32_t value) {
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

int main() {
/*
    Env a;
    a.put("a", 10);
    a.put("b", 20);
    Env b(&a);
    b.put("b", 25);
    b.put("c", 30);
    std::cout << b.get("a") << " " << a.get("a") << " " << b.get("b") << " " << b.get("c") << "\n";*/
    Map<String, uint32_t> map;
    map.put("ree", 10);
    map.put("a", 3);
    map.put("asd", 4);
    ErrorCode ec1;
    ErrorCode ec2;
    std::cout << map.get("a", ec1) << ", " << map.get("b", ec2) << "\n";
    std::cout << (int)ec1 << ", " << (int)ec2 << "\n";
}
