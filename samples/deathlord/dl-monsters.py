import struct
import re

# change these constants if/when necessary

print_number = False
print_tell = False

num_monsters = 130

special = [
    '', 'KILL', 'DRAIN', 'PARAL', 'POISON', 'ILL', 'STONE', 'SLAYSPELL', 'SUMMON', 'SCREAM', 'BREATH',
    'TODO', 'MOTU', 'L1ZAP', 'L2ZAP', 'L3ZAP', 'L4ZAP', 'L5ZAP', 'L6ZAP', 'L7ZAP']

my_type = ['DRAGON', 'FLEE +', 'DEMON', 'UNDEAD', "FLEE 0", "FLEE -"]


def mon_type(x):
    if x == 0xff:
        return "NORMAL"
    if x >= len(my_type):
        return 'out of range ' + str(x)
    return my_type[x]


# noinspection PyShadowingNames,PyShadowingNames
def read_one_monster(fp, i):
    ch = [0]
    this_special = {}
    string = ""
    if print_tell:
        print('{:x}'.format(fp.tell()))
    ch2 = fp.read(14)
    bytear = [int(a) for a in ch2]
    bytear[3] = 10 - bytear[3]  # int.to_bytes(10, 1, 'big') # bytes(10 - int(ch2[2])) # armor class
    while not ch[0] & 0x80:
        ch = fp.read(1)
        y = (ch[0] ^ 0x65) & 0x7f
        string = string + chr(y)
    table_line = ""
    if print_number:
        table_line = table_line + '|' + '{:02x}'.format(i)
    print(table_line + '|' + string, end="|")
    # print(struct.unpack("2H", ch2[0:4]))
    stat_string = ""
    stat_string = stat_string + mon_type(bytear[0]) + '|'
    stat_string = stat_string + ("%d-%d|%g|" % (bytear[1] * 15, bytear[1] * 50, (bytear[2] * 9) / 2))
    # '{0:g}'.format(float(21)) another way to say above?
    stat_string = stat_string + '|'.join(str(a) for a in bytear[3:7] + bytear[12:14]) + "|"
    for j in range(7, 12):
        this_special[ch2[j]] = 1
    this_special.pop(0, None)
    if len(this_special.keys()) > 0:
        for k in sorted(this_special.keys()):
            stat_string = stat_string + ' ' + special[k]
        stat_string = re.sub(r'\| +', '|', stat_string)
    else:
        stat_string = stat_string + "(None)"
    stat_string = re.sub(r"\|([-\+])", r"| \1", stat_string)  # to make sure gamefaqs markup doesn't see |+ or |-
    print(stat_string + '|')
    return

fp = open("dlsa.aws", "rb")

fp.seek(0xd070, 0)

print(";format:gf-markup")
print()
print("|*Name|*Enemy type|*Treasure|*Avg HP|*AC|*Max#|*Attacks|*Damage(1-x)|*Experience|*?|*Monster Specialt(y/ies)||")
print()

for i in range(0, num_monsters+1):
    read_one_monster(fp, i)
