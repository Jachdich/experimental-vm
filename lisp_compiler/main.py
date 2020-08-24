import re

class Env:
    def __init__(self, outer):
        self.outer = outer
        self.data = {}
        
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

def tokenise(s):
    return [x for x in s.replace("(", "( ").replace(")", " )").split(" ") if x != ""]

def parse(tokens):
    print(tokens)
    pos = 0
    out = []
    if tokens[0] == "(":
        #if tokens[0] != "(": raise SyntaxError()
        if tokens[-1] != ")": raise SyntaxError()
        tokens = tokens[1:-1]
    while pos < len(tokens):
        if tokens[pos] == "(":
            pos += 1
            num_left = 0
            num_right = 0
            inside_param = []
            while not num_right > num_left:
                if tokens[pos] == "(":
                    num_left += 1
                elif tokens[pos] == ")":
                    num_right += 1
                inside_param.append(tokens[pos])
                pos += 1
            out.append(parse(inside_param[:-1]))

        elif tokens[pos] != ")":
            out.append(atom(tokens[pos]))
            pos += 1
        else:
            #skip over rparan
            pos += 1
    return out


a = {'+': lambda a,b: a+b,
            '-': lambda a,b: a-b,
            '*': lambda a,b: a*b,
            '/': lambda a,b: int(a/b)}
test_env = Env(None)
test_env.data = a

def eval_ast(ast, env):
    if type(ast) == type(str()):
        return env.get(ast)
    if type(ast) == type(list()):
        return [evaluate(i, env) for i in ast]
    return ast

def evaluate(ast, env):
    print("AST: " + str(ast))
    if type(ast) == type(list()):
        if len(ast) == 0:
            return ast
        if ast[0] == "def":
            env.put(ast[1], evaluate(ast[2], env))
            return None
        elif ast[0] == "let":
            nenv = Env(env)
            
        x = eval_ast(ast, env)
        return x[0](*x[1:])
    return eval_ast(ast, env)


def print_ast(obj):
    if type(obj) == type(str()):
        return obj
    if type(obj) == type(int()):
        return str(obj)
    if type(obj) == type(list()):
        return "(" + " ".join([print_ast(x) for x in obj]) + ")"

def atom(token):
    try: return int(token)
    except ValueError:
        try: return float(token)
        except ValueError:
            return str(token)
#print(parse(tokenise("(def a (+ 2 2))")))
#print(parse(tokenise("a")))
print(evaluate(parse(tokenise("(def a 4)")), test_env))
print(evaluate(parse(tokenise("a")), test_env))
