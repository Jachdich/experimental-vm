import re

class Value:
    AUTO = -1
    SYMBOL = 0
    
    def __init__(self, value=None, ty=AUTO):
        self.value = value
        

class Env:
    def __init__(self, outer, binds=None, exprs=None):
        self.outer = outer
        self.data = {}
        if binds != None and exprs != None:
            for b, e in zip(binds, exprs):
                self.put(b, e)
        
    def get(self, key):
        env = self.find(key)
        if env == None: raise Exception("Not found: " + key)
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
        else: return str(token) #todo: symbol

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


a = {'+': lambda a,b: a+b,
            '-': lambda a,b: a-b,
            '*': lambda a,b: a*b,
            '/': lambda a,b: int(a/b),
            '=': lambda a,b: a == b,
            '<': lambda a,b: a < b,
            '>': lambda a,b: a > b,
     '>=': lambda a,b: a >= b,
     '<=': lambda a,b: a <= b,}
test_env = Env(None)
test_env.data = a

def eval_ast(ast, env):
    if type(ast) == type(str()):
        return env.get(ast)
    if type(ast) == type(list()):
        return [evaluate(i, env) for i in ast]
    return ast

def evaluate(ast, env):
    #print("AST: " + str(ast))
    if type(ast) == type(list()):
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
            if ex != None and ex != False:
                return evaluate(ast[2], env)
            if len(ast) > 3:
                return evaluate(ast[3], env)
            else:
                return None

        elif ast[0] == "fn":
            nenv = Env(env)
            def fn(*args):
                return evaluate(ast[2], Env(env, ast[1], list(args)))
            
            return fn
            
        x = eval_ast(ast, env)
        return x[0](*x[1:])
    return eval_ast(ast, env)


"""
while True:
    try:
        print(evaluate(parse(Reader(input("usr> "))), test_env))
    except Exception as e:
        print(e)
        
        """
print(evaluate(parse(Reader("(def fib (fn (n)" + 
        "(if (= n 0)"+
            " (1) "+
            "(if (= n 1)"+
                " (1) "+
                "(+ (fib (- n 1)) (fib (- n 2)))"+
            ")"+
        ")"+
    "))")), test_env))
print(evaluate(parse(Reader("(fib 25)")), test_env))
#print(print_ast(parse(tokenise("a"))))

#print(evaluate(parse(Reader("nil")), test_env))
#print(evaluate(parse(tokenise("a")), test_env))
