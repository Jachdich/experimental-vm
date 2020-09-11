from pyparsing import *
import assembler, sys

##expr = Forward()
##expr << infixNotation(Word(nums), [
##    (oneOf("* /"), 2, opAssoc.LEFT),
##    (oneOf("+ -"), 2, opAssoc.LEFT)])
##print(expr.parseString("1 * 3 + 4 + 3 * 3 + 4 * 4 + 1 + 1 + 1 + 1"))
##sys.exit(0)

def Operator(name, expr):
    if len(name) == 2:
        return Group(ident + Literal(name[0]).setParseAction(lambda a,b,c: name) \
                     + Group(Optional(expr) + ZeroOrMore(Suppress(",") + expr)) \
                     + Suppress(name[1]))
    if len(name) == 1:
        return None #todo

OPERATORS = ["+", "-", "*", "/", "==", "()", "[]", "{}"]

expr = Forward()
assignment = Forward()
vardecl = Forward()
code = Forward()

number = Word(nums)
ident  = Word(alphas + "_", alphanums + "_")

term = (Group("(" + expr + ")") | (ident | number))
mulexpr = Forward()
mulexpr << (Operator("()", expr) | Operator("[]", expr) | Operator("{}", expr) |
            Group(term + (Literal("/") | Literal("*") | Literal("==") + mulexpr)) | term)

expr << (Group(mulexpr + OneOrMore(Literal("+") | Literal("-")) + expr) | mulexpr)

tupledef = Group(Optional(Suppress("(")) + expr + OneOrMore(Suppress(",") + expr) + Optional(Suppress(")")))

vardecl << Group(ident.copy().setParseAction(lambda a,b,c: ["declare",] + list(c)) \
                 + (tupledef | ident) + Suppress("=") + (tupledef | expr))
assignment << Group(ident + Literal("=") + expr)

elsesmt = (Suppress("else") + Suppress("{") + code + Suppress("}"))
elifsmt = (Literal("elif").setParseAction(lambda a,b,c: "if") + expr + Suppress("{") + code + Suppress("}"))

ifsmt = Group(Literal("if") + expr + Suppress("{") + code + Suppress("}") +\
              Optional(Group(OneOrMore(elifsmt) + Optional(elsesmt)))) + Optional(elsesmt)


funcdef = Group(ident.copy().setParseAction(lambda a,b,c: ["def",] + list(c)) + ident + \
                Suppress("(") + Group(ZeroOrMore(Group(ident + ident + (Suppress(",") | Suppress(")"))))) \
                + Suppress("{") + code + Suppress("}"))

returnsmt = Group(Literal("return") + expr)
smt = (vardecl | assignment | ifsmt | funcdef | returnsmt | expr)
code << ZeroOrMore(smt)

#print(equation.parseString("a b = c(1, 3*3)"))

#int ree(int, a, b, str, c) {
#    int c = a + b + c
#}

#print(ree(2, 2, " is 4"))
#    return str(a * b) + c
ast = code.parseString("""
int ree(int a, int b, str c) {
    return str(a + b) + c
}

print(ree(2, 2))

""")

class Function:
    def __init__(self, rettype, params, code):
        self.rettype = rettype
        self.params = params
        self.code = code

class Env:
    def __init__(self, locals_=None):
        if locals_ == None:
            self.locals = {}
        else:
            self.locals = locals_

        self.labelval = -1
        self.funcs = {}

    def defFunc(self, rettype, name, params, code):
        self.funcs[self.getFullFuncName(name, params)] = Function(rettype, params, code)

    def getFunc(self, name, params):
        return self.funcs[self.getFullFuncName(name, params)]

    def getFullFuncName(self, name, params):
        return name + "".join([x[0] for x in params])

    def newLabel(self):
        self.labelval += 1
        return "lab_" + str(self.labelval)

    def get(self, name):
        return self.locals[name]

    def malloc(self, name):
        if self.locals == {}: val = 0
        else:
            val = self.locals[max(self.locals)] + 1
        self.locals[name] = val
        return val

    def getNumVars(self):
        if self.locals == {}: return 0
        return self.locals[max(self.locals)] + 1

class Number:
    def __init__(self, value):
        self.value = value

    def _eval(self, env):
        return f"push {self.value}\n"

    def __repr__(self):
        return f"Number({self.value})"

    def __str__(self): return self.__repr__()

class Var:
    def __init__(self, value):
        self.value = value

    def _eval(self, env):
        return f"lstack {env.get(self.value)}\n"

    def __repr__(self):
        return f"Var({self.value})"

    def __str__(self): return self.__repr__()

class BinaryOp:
    def __init__(self, lhs, rhs, op):
        self.lhs = lhs
        self.rhs = rhs
        self.op = op

    def _eval(self, env):
        if self.op in "+-*/==":
            return f"{self.lhs._eval(env)}{self.rhs._eval(env)}{self.getOp(self.op)}\n"
        elif self.op == "()":
            params = "".join([x._eval(env) for x in self.rhs])
            return f"{params}call {self.lhs.value} {len(self.rhs)}\n"

    def getOp(self, op):
        if op == "+": return "add"
        if op == "-": return "sub"
        if op == "*": return "mul"
        if op == "/": return "div"
        if op == "==": return "cmp"

    def __repr__(self):
        return f"BinaryOp({self.lhs} {self.op} {self.rhs})"

    def __str__(self):
        return self.__repr__()

class VarDeclaration:
    def __init__(self, ty, vname, value):
        self.ty = ty
        self.vname = vname
        self.value = value

    def _eval(self, env):
        loc = env.malloc(self.vname)
        return f"{self.value._eval(env)}sstack {loc}\n"

    def __repr__(self):
        return f"VariableDeclaration({self.ty} {self.vname} = {self.value})"

    def __str__(self):
        return self.__repr__()

class IfStatement:
    def __init__(self, expression, body, elsesmt=None):
        self.expr = expression
        self.body = body
        self.elsesmt = elsesmt

    def _eval(self, env):
        if self.elsesmt == None:
            endlabel = env.newLabel()
            cond = self.expr._eval(env)
            body = self.body._eval(env)
            return f"{cond}jnz {endlabel}\n{body}{endlabel}:\n"
        else:
            endlabel = env.newLabel()
            truelabel = env.newLabel()
            cond = self.expr._eval(env)
            body = self.body._eval(env)
            elsesmt = self.elsesmt._eval(env)
            return f"{cond}jz {truelabel}\n{elsesmt}jp {endlabel}\n{truelabel}:\n{body}{endlabel}:\n"
        

    def __repr__(self):
        return f"IfStatement({self.expr} ( {self.body} ) )"

    def __str__(self):
        return self.__repr__()

class FuncDefinition:
    def __init__(self, ty, name, args, code):
        self.ty = ty
        self.name = name
        self.args = args
        self.code = code

    def _eval(self, env):
        env.defFunc(self.ty, self.name, self.args, self.code)
        return ""

    def __repr__(self):
        return f"FuncDefinition({self.ty} {self.name}({self.args}) {{ {self.code} }} )"

    def __str__(self):
        return self.__repr__()

def makeAST(ast):
    if type(ast) == int: return Number(ast)
    if type(ast) == str: return Var(ast)
    l = len(ast)
    if l == 0: pass
    if l == 1: pass
    if l == 2: pass
    if l == 3:
        if ast[1] in OPERATORS:
            return BinaryOp(makeAST(ast[0]), makeAST(ast[2]), ast[1])
        if ast[0] == "if":
            return IfStatement(makeAST(ast[1]), makeAST(ast[2]))
        
    if l == 4:
        if ast[0] == "if":
            return IfStatement(makeAST(ast[1]), makeAST(ast[2]), makeAST(ast[3]))
        if ast[0] == "declare":
            return VarDeclaration(ast[1], ast[2], makeAST(ast[3]))

    if l == 5:
        if ast[0] == "def":
            return FuncDefinition(ast[1], ast[2], ast[3], makeAST(ast[4]))
    #if l >= 0:
    #    if ast[0] == "if":
    #        
    #        return IfStatement(makeAST(ast[1]), makeAST(ast[2]))

    return [makeAST(x) for x in ast]
        
def malloc(ty):
    global varloc
    if ty == "int":
        loc = varloc
        varloc += 1
        return loc

def atom(token):
    int_re = re.compile(r"-?[0-9]+$")
    float_re = re.compile(r"-?[0-9][0-9.]*$")
    string_re = re.compile(r'"(?:[\\].|[^\\"])*"')
    if re.match(int_re, token): return int(token)
    elif re.match(float_re, token): return float(token)
    elif re.match(string_re, token): return token
    elif token == "nil": return None
    elif token == "true": return True
    elif token == "false": return False
    else: return str(token) #todo: symbol

def toAtoms(ast):
    if type(ast) == list:
        return [toAtoms(x) for x in ast]
    return atom(ast)

ast = ast.asList()

print(toAtoms(ast))
ast = makeAST(toAtoms(ast))
print(ast)
env = Env()
asm = "".join([x._eval(env) for x in ast])
numVars = env.getNumVars()
asm = "push 0\n" * numVars + asm
print(asm)
bytecode = assembler.assemble(asm, ["print:\nout\nret\n"])
with open("../test.vm", "wb") as f:
    f.write(bytecode)
