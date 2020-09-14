def readBytes(x):
    return (x[0] << 24) | (x[1] << 16) | (x[2] << 8) | x[3]

def doEscapes(x):
    return x.replace("\n", "\\n")

def pad(s, num=2, char="0", after=False):
    if after:
        return s + char * (num - len(s))
    return char * (num - len(s)) + s

class DataObj:
    def __init__(self, ty, l, d):
        self.data = d
        self.type = ty
        self.len = l

    def __str__(self):
        if self.type == 0:
            return str(readBytes(self.data))
        if self.type == 2:
            return '"' + doEscapes("".join([chr(x) for x in self.data])) + '"'

def disassemble(cbytes):
    opcode_dis = [
        ("nop", 0),
        ("add", 0),
        ("sub", 0),
        ("out", 0),
        ("push", 1),
        ("lstack", 1),
        ("sstack", 1),
        ("call", 2),
        ("ret", 0),
        ("halt", 0),
        ("cmp", 0),
        ("jz", 1),
        ("jp", 1),
        ("pop", 0),
        ("pop_under", 0),
        ("dup", 0),
        ("jnz", 1),
        ("ptype", 0),
        ("inttostr", 0),
        ("doubletostr", 0),
        ]
    codelen = readBytes(cbytes[:4])
    code = cbytes[4:codelen + 4]
    datalen = readBytes(cbytes[codelen + 4:codelen + 8])
    data = cbytes[codelen + 8:]
    pos = 0
    dataobjs = []
    while pos < datalen:
        ty = data[pos]
        pos += 1
        l = readBytes(data[pos:pos + 4])
        pos += 4
        d = data[pos:pos + l]
        pos += l
        dataobjs.append(DataObj(ty, l, d))
        
    pos = 0
    out = ""
    while pos < codelen:
        by = code[pos]
        oc = opcode_dis[by]
        pos += 1
        out += pad(str(pos), 5, " ") + ":   " + pad(oc[0], 12, " ", True)
        for i in range(oc[1]):
            param = readBytes(code[pos:pos + 4])
            pos += 4
            if oc[0] == "push":
                param = dataobjs[param]
            out += pad(str(param) + ",", 3, " ", True)
        out += "\n"
        
    return out

with open("../test.vm", "rb") as f:
    cbytes = bytearray(f.read())

print(disassemble(cbytes))
