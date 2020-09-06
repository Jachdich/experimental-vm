#from assembler import assembler, definition_stackvm
import re, sys

class Function:
    def __init__(self, value, args, code, varmaps):
        self.value = value
        self.args = args
        self.code = code
        self.varmaps = varmaps

    def __add__(self, other):
        return self.value + other

    def __str__(self):
        return self.value

    def __repr__(self):
        return f"Function('{self.code}')"

    def __hash__(self):
        return hash(self.value)

    def __eq__(self, other):
        if isinstance(other, Function):
            return self.value == other.value
        else:
            return self.value == other
    
    def __ne__(self, other):
        return not(self == other)
    
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
        if reader.peek(-1) != ")": raise SyntaxError("Expected ')' to close '('")
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

def intToBytes(x):
    return [
        (x >> 24) & 0xFF,
        (x >> 16) & 0xFF,
        (x >> 8 ) & 0xFF,
        x & 0xFF]

class Compiler:
    def __init__(self, code):
        self.func = {
        "+": "add",
        "-": "sub",
        "==": "cmp",
        "print": "out",   
        }

        self.global_vars = []
        self.funcasm = []
        self.code = code
        self.last_label = 0

    def genNewLabel(self):
        self.last_label += 1
        return "label" + str(self.last_label)

    def expr(self, ast, varmaps=None):
        if varmaps == None: varmaps = {}
        if type(ast) == list:
            out = []
            for x in ast:
                asm, varmaps = self.genCode(x, varmaps)
                out.append(asm)
            return out
        elif type(ast) == int:
            return "push " + str(ast)
        elif type(ast) == str:
            if ast in varmaps:
                if isinstance(varmaps[ast], Function):
                    return "call " + varmaps[ast].value + " " + str(varmaps[ast].args)
                return "lstack " + str(varmaps[ast])
            else:
                return self.func[ast]

    def genCode(self, ast, varmaps=None):
        if varmaps == None: varmaps = {}
        if type(ast) == list:
            if ast[0] == "fn":
                paramlist = list(ast[1])
                code = list(ast[2])
                paramlist.reverse()
                #asm, _varmaps = self.genCode(code, varmaps={**varmaps, **{i:j for j,i in enumerate(paramlist)}})
                label = self.genNewLabel()
                func = Function(label, len(paramlist), code, {**varmaps, **{i:j for j,i in enumerate(paramlist)}})
                self.funcasm.append(func)
                return func, varmaps
            
            if ast[0] == "def":
                name = ast[1]
                val, _varmaps = self.genCode(ast[2], varmaps)
                if isinstance(val, Function):
                    varmaps[name] = val
                else:
                    varmaps[name] = len(varmaps) #todo make sure this is right
                return "", varmaps

            if ast[0] == "if":
                expr, varmaps = self.genCode(ast[1], varmaps)
                trueside, varmaps = self.genCode(ast[2], varmaps)
                falseside, varmaps = self.genCode(ast[3], varmaps)
                
                truelabel = self.genNewLabel()
                endlabel  = self.genNewLabel()

                return f"{expr}\njz {truelabel}\n{falseside}\njp {endlabel}\n{truelabel}:\n{trueside}\n{endlabel}:", varmaps
                
            x = self.expr(ast, varmaps)
            x.reverse()
            return "\n".join(x), varmaps
        return self.expr(ast, varmaps), varmaps

    def compileCode(self):
        out = ""
        varmaps = {}
        for line in self.code.split("\n"):
            if line.strip() == "": continue
            asm, varmaps = self.genCode(parse(Reader(line)), varmaps)
            out += str(asm) + "\n"

        compiled_funcs = []
        for func in self.funcasm:
            asm, varmaps = self.genCode(func.code, {**varmaps, **func.varmaps})
            compiled_funcs.append(func.value + ":\n" + asm + "\n" + "pop_under\n" * func.args + "ret")
            
        return out, compiled_funcs

def assemble(asm, funcasm):
    asm += "\nhalt\n"
    asm += "\n".join(funcasm)
    constants = []
    code = []
    for line in asm.split("\n"):
        if line.strip() == "": continue
        parts = line.split(" ")
        opcode = parts[0]
        if len(parts) > 1:
            args = parts[1:]
        else:
            args = []

        if opcode == "nop":
            code.append(0)
        elif opcode == "add":
            code.append(1)
        elif opcode == "sub":
            code.append(2)
        elif opcode == "out":
            code.append(3)
        elif opcode == "push":
            code.append(4)
            if args[0].isdigit():
                constants.append(int(args[0]))
            else:
                constants.append(args[0])
            code.extend(intToBytes(len(constants) - 1))
        elif opcode == "lstack":
            code.append(5)
            code.extend(intToBytes(int(args[0])))
        elif opcode == "sstack":
            code.append(6)
            code.extend(intToBytes(int(args[0])))
        elif opcode == "call":
            code.append(7)
            code.extend(intToBytes(int(args[1])))
            code.append(args[0])
        elif opcode == "ret":
            code.append(8)
    
        elif opcode == "halt":
            code.append(9)
        elif opcode == "cmp":
            code.append(10)
        elif opcode == "jz":
            code.append(11)
            code.append(args[0])
        elif opcode == "jp":
            code.append(12)
            code.append(args[0])
        elif opcode == "pop":
            code.append(13)
        elif opcode == "pop_under":
            code.append(14)
        else:
            code.append(opcode)

    jumps = {}
    i = 0

    a = code[:]
    code = []
    for c in a:
        if type(c) == str and c.endswith(":"):
            jumps[c[:-1]] = i
        elif type(c) == str:
            code.append(c)
            i += 4
        else:
            code.append(c)
            i += 1

    a = code[:]
    code = []
    for x in a:
        if type(x) == str:
            code.extend(intToBytes(jumps[x]))
        else:
            code.append(x)

    f = bytearray()
    
    codelen = intToBytes(len(code))
    f += bytearray(codelen)
    for x in code:
        f += bytearray([x,])

    datalen = intToBytes(len(constants) * 9)
    f += bytearray(datalen)
    for x in constants:
        f += bytearray([0, 0, 0, 0, 4])
        f += bytearray(intToBytes(x))

    return f
                
        #asmbler = assembler.Assembler(asm, definition_stackvm)
        #return asmbler.assemble()

#asm = comp("""(def x (fn (a b c) (print (+ (+ a 1) (+ (+ b 1) (+ c 1))))))
#(x 1 2 3)""")

#asm = comp("""(def asp (fn (a b) (print (+ a b))))
#(asp 999999999 1)""")
#with open("test.vm", "wb") as f:
#    f.write(assemble(asm, funcasm))

infile = sys.argv[1]
out = sys.argv[2]

with open(infile, "r") as f:
    source = f.read()

c = Compiler(source)
asm, funcasm = c.compileCode()
print(asm)
print("\n\n\n".join(funcasm))
assembled_bytes = assemble(asm, funcasm)
with open(out, "wb") as f:
    f.write(assembled_bytes)

