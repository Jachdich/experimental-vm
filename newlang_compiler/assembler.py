def intToBytes(x):
    x = int(x) #Workaround for negetive numvers, FIX
    return [
        (x >> 24) & 0xFF,
        (x >> 16) & 0xFF,
        (x >> 8 ) & 0xFF,
        x & 0xFF]

def doEscapes(x):
    out = ""
    escapeNext = False
    for c in x:
        if escapeNext:
            if c == "\\":
                out += "\\"
            elif c == "n":
                out += "\n"
            else:
                error(f"Syntax error: Invalid escape sequence '{x}'")
            escapeNext = False
            continue
        if c == "\\":
            escapeNext = True
            continue
        out += c
    return out

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
        elif opcode == "dup":
            code.append(15)
        elif opcode == "jnz":
            code.append(16)
            code.append(args[0])
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

    #datalen = intToBytes(len(constants) * 9)
    #f += bytearray(datalen)

    consts = bytearray()
    for x in constants:
        if type(x) == int:
            consts += bytearray([0, 0, 0, 0, 4])
            consts += bytearray(intToBytes(x))
        elif type(x) == str:
            x = doEscapes(x)
            consts += bytearray([2,])
            consts += bytearray(intToBytes(len(x)))
            for c in x:
                consts += bytearray([ord(c),])

    f += bytearray(intToBytes(len(consts)))
    f += consts
    return f
