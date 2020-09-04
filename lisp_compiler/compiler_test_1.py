import re, random

class Value:
    AUTO = -1
    SYMBOL = 0
    
    def __init__(self, value=None, ty=AUTO):
        self.value = value
        
class Data:
    def __init__(self):
        self.data = []

    def addData(self, val):
        self.data.append(val)
        return len(self.data) - 1

    def compile(self):
        print(self.data)

class Symbol:
    def __init__(self, value):
        self.value = value

    def __add__(self, other):
        return self.value + other

    def __str__(self):
        return self.value

    def __repr__(self):
        return f"Symbol('{self.value}')"

    def __hash__(self):
        return hash(self.value)

    def __eq__(self, other):
        if isinstance(other, Symbol):
            return self.value == other.value
        else:
            return self.value == other
    
    def __ne__(self, other):
        return not(self == other)

data = Data()
functions = []

class Env:
    def __init__(self, outer, binds=None, exprs=None):
        self.outer = outer
        self.data = {}
        if binds != None and exprs != None:
            for b, e in zip(binds, exprs):
                self.put(b, e)
        
    def get(self, key):
        env = self.find(key)
        if env == None: raise Exception(f"Not found: '{key}' {self.data}")
        return env.data[key]
    
    def put(self, key, value):
        self.data[key] = value
        
    def find(self, key):
        if self.data.get(key) != None:
            return self
        if self.outer != None:
            return self.outer.find(key)
        return None

class Reader:
    RE_PATTERN = r"""[\s,]*(~@|[\[\]{}()'`~^@]|"(?:[\\].|[^\\"])*"?|;.*|[^\s\[\]{}()'"`@,;]+)"""
    def __init__(self, txt=None, tokens=None):
        self.txt = txt
        self.pos = 0
        if tokens != None:
            self.tokens = tokens
        else:
            self.tokens = self.tokenise(self.txt)

    def tokenise(self, s):
        tre = re.compile(Reader.RE_PATTERN);
        return [t for t in re.findall(tre, s) if t[0] != ';']

    def print_ast(self, obj):
        if type(obj) == type(str()):
            return obj
        if type(obj) == type(int()):
            return str(obj)
        if type(obj) == type(list()):
            return "(" + " ".join([print_ast(x) for x in obj]) + ")"

    def atom(self, token):
        int_re = re.compile(r"-?[0-9]+$")
        float_re = re.compile(r"-?[0-9][0-9.]*$")
        string_re = re.compile(r'"(?:[\\].|[^\\"])*"')
        if re.match(int_re, token): return int(token)
        elif re.match(float_re, token): return float(token)
        elif re.match(string_re, token): return token[1:-1]
        elif token == "nil": return None
        elif token == "true": return True
        elif token == "false": return False
        else: return Symbol(token) #todo: symbol

    def next(self):
        self.pos += 1
        return self.tokens[self.pos - 1]
    
    def peek(self, pos=None):
        if pos == None:
            return self.tokens[self.pos]
        return self.tokens[pos]
        
def parse(reader):
    out = []
    if reader.peek() == "(":
        #if tokens[0] != "(": raise SyntaxError()
        if reader.peek(-1) != ")": raise SyntaxError()
        reader.tokens = reader.tokens[1:-1]
        
    while reader.pos < len(reader.tokens):
        if reader.peek() == "(":
            reader.next()
            
            num_left = 0
            num_right = 0
            inside_param = []
            while not num_right > num_left:
                if reader.peek() == "(":
                    num_left += 1
                elif reader.peek() == ")":
                    num_right += 1
                inside_param.append(reader.next())
                
            out.append(parse(Reader(tokens=inside_param[:-1])))

        elif reader.peek() != ")":
            out.append(reader.atom(reader.next()))
        else:
            #skip over rparan
            reader.next()

    if len(out) == 1:
        return out[0]
    else:
        return out

a = {Symbol('+'): lambda *args: " ".join([str(i) for i in args]) + " add",
     Symbol('-'): lambda *args: " ".join([str(i) for i in args]) + " sub",
     Symbol("*"): lambda *args: " ".join([str(i) for i in args]) + " mul",
     Symbol('/'): lambda *args: " ".join([str(i) for i in args]) + " div"}

test_env = Env(None)
test_env.data = a

def genNewUniqueLabel():
    return "".join([chr(random.randint(ord("a"), ord("z"))) for i in range(10)])

def eval_ast(ast, env):
    if isinstance(ast, Symbol):
        return env.get(ast)
    if type(ast) == list:
        return [evaluate(i, env) for i in ast]
    return ast

def evaluate(ast, env):
    #print("AST: " + str(ast))
    if type(ast) == list:
        if len(ast) == 0:
            return ast
        if ast[0] == "def":
            return env.put(ast[1], evaluate(ast[2], env))

        elif ast[0] == "let":
            nenv = Env(env)
            for i in range(0, len(ast[1]), 2):
                nenv.put(ast[1][i], evaluate(ast[1][i + 1], nenv))
            return evaluate(ast[2], nenv)
        
        elif ast[0] == "do":
            el = eval_ast(ast[1:], env)
            return el[-1]

        elif ast[0] == "if":
            ex = evaluate(ast[1], env)
            trueside = evaluate(ast[2], env)
            falseside = evaluate(ast[3], env)
            truelabel  = genNewUniqueLabel()
            falselabel = genNewUniqueLabel()
            endlabel   = genNewUniqueLabel()
            return f"{ex} cmp 0 jz {truelabel} jnz {falselabel} {truelabel}: {trueside} jp {endlabel} {falselabel}: {falseside} {endlabel}:"
            #if ex != None and ex != False:
            #    return evaluate(ast[2], env)
            #if len(ast) > 3:
            #    return evaluate(ast[3], env)
            #else:
            #    return None

        elif ast[0] == "fn":
            nenv = Env(env)
            functions.append(evaluate(ast[2], Env(env)))
            def fn(*args):
                return "call whatisit"
                #return evaluate(ast[2], Env(env, ast[1], list(args)))
            
            return fn

        x = eval_ast(ast, env)
        return x[0](*x[1:])
    return eval_ast(ast, env)



#print(evaluate(parse(Reader("(+ 1 (+ 2 3))")), test_env))
#print(data.compile())
while True:
    #try:
        print(evaluate(parse(Reader(input("usr> "))), test_env))
    #except Exception as e:
    #    print(e)
        

"""

(def x 5)
(def y (+ x 1)
(def z (fn (a, b) (+ a b)))
(z x y)

5 dup 1 add call z
z: add ret
"""
#print(parse(tokenise("(def a (+ 2 2))")))
#print(parse(tokenise("a")))
#print(print_ast(parse(tokenise("a"))))

#print(evaluate(parse(Reader("nil")), test_env))
#print(evaluate(parse(tokenise("a")), test_env))
