import struct
import re

# change these constants if/when necessary

printNumber = False
printTell = True

numMonsters = 130

special = [ '', 'KILL', 'DRAIN', 'PARAL', 'POISON', 'ILL', 'STONE', 'SLAYSPELL', 'SUMMON', 'SCREAM', 'BREATH', 'TODO', 'MOTU',
  'L1ZAP', 'L2ZAP', 'L3ZAP', 'L4ZAP', 'L5ZAP', 'L6ZAP', 'L7ZAP']

myType = ['DRAGON', 'FLEE +', 'DEMON', 'UNDEAD', "FLEE 0", "FLEE -"]


def monType(x):
    if x == 0xff:
        return "NORMAL"
    if x >= len(myType):
        return 'out of range ' + str(x)
    return myType[x]


def readOneMonster(fp, i):
    ch = [0]
    thisSpecial = {}
    string = ""
    if printTell:
        print('{:x}'.format(fp.tell()))
    ch2 = fp.read(14)
    bytear = [int(a) for a in ch2]
    bytear[3] = 10 - bytear[3] # int.to_bytes(10, 1, 'big') # bytes(10 - int(ch2[2])) # armor class
    while not ch[0] & 0x80:
        ch = fp.read(1)
        y = (ch[0] ^ 0x65) & 0x7f
        string = string + chr(y)
    tableLine = ""
    if printNumber:
        tableLine = tableLine + '|' + '{:02x}'.format(i)
    print(tableLine + '|' + string, end="|")
    # print(struct.unpack("2H", ch2[0:4]))
    statString = ""
    statString = statString + monType(bytear[0]) + '|'
    statString = statString + ("%d-%d|%g|" % (bytear[1] * 15, bytear[1] * 50, (bytear[2] * 9) / 2)) # '{0:g}'.format(float(21))
    statString = statString + '|'.join(str(a) for a in bytear[3:7] + bytear[12:14]) + "|"
    for j in range(7,12):
        thisSpecial[ch2[j]] = 1
    thisSpecial.pop(0, None)
    if len(thisSpecial.keys()) > 0:
        for k in sorted(thisSpecial.keys()):
            statString  = statString + ' ' + special[k]
        statString = re.sub(r'\| +', '|', statString)
    else:
        statString = statString + "(None)"
    statString = re.sub("\|-", "| -", statString)
    print (statString + '|')
    return

fp = open("dlsa.aws", "rb")

fp.seek(0xd070, 0)

print(";format:gf-markup")
print()
print("|*Name|*Enemy type|*Treasure|*Avg HP|*AC|*Max#|*Attacks|*Damage(1-x)|*Experience|*?|*Monster Specialt(y/ies)||")
print()

for i in range(0,numMonsters+1):
    readOneMonster(fp, i)
