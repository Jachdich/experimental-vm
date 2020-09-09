from pyparsing import *

def Operator(name):
    if len(name) == 2:
        return Group(ident + Literal(name[0]).setParseAction(lambda a,b,c: name) + Group(Optional(expr) + ZeroOrMore(Suppress(",") + expr)) + Suppress(name[1]))
    if len(name) == 1:
        return None #todo

expr = Forward()
equation = Forward()
code = Forward()

number = Word(nums+".")
ident  = Word(alphas, alphanums + "_")

term = (Group("(" + expr + ")") | (ident | number))
mulexpr = Forward()
mulexpr << (Group(term + (Literal("/") | Literal("*") + mulexpr)) | term)
expr << (Operator("()") | Operator("[]") | Operator("{}") | Group(mulexpr + (Literal("+") | Literal("-")) + expr) | mulexpr)

equation << Group(Optional(ident) + ident + Literal("=") + expr)

ifsmt = Group(Literal("if") + expr + Suppress("{") + Group(OneOrMore(equation)) + Suppress("}"))
funcdef = Group(ident.copy().setParseAction(lambda a,b,c: ["def",] + list(c)) + ident + Suppress("(") + Group(Optional(ident) + ZeroOrMore(Suppress(",") + ident)) + Suppress(")") + Suppress("{") + Group(OneOrMore(equation)) + Suppress("}"))

code << ZeroOrMore(equation | ifsmt | funcdef)

#print(equation.parseString("a b = c(1, 3*3)"))


print(code.parseString("""
int ree(a, b) {
    big ree = 8* 9+ 3
}

"""))

'''
from pyparsing import *

# sample string with enums and other stuff
sample = """
    enum hello {
        Zero,
        One,
        Two,
        Three,
        Five=5,
        Six,
        Ten=10
        };
    enum blah
        {
        alpha,
        beta,
        gamma = 10 ,
        zeta = 50
        };
    """

# syntax we don't want to see in the final parse tree
LBRACE, RBRACE, EQ, COMMA = map(Suppress, "{}=,")
_enum = Suppress("enum")
identifier = Word(alphas, alphanums + "_")
integer = Word(nums)
enumValue = Group(identifier("name") + Optional(EQ + integer("value")))
enumList = Group(enumValue + ZeroOrMore(COMMA + enumValue))
enum = _enum + identifier("enum") + LBRACE + enumList("names") + RBRACE
print(list(enum.scanString(sample)))
# find instances of enums ignoring other syntax
for item, start, stop in enum.scanString(sample):
    id = 0
    for entry in item.names:
        if entry.value != "":
            id = int(entry.value)
        print("%s_%s = %d" % (item.enum.upper(), entry.name.upper(), id))
        id += 1
'''
