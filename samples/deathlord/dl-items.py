# dlw.py
# scopes out Deathlord weapons and other items
#

import re

zap_whitespace = True

num_items = 75

slots = ['HAND', 'MISSILE', 'ARMOR', 'SHIELD', 'ACCESSORY', 'RING', 'USABLE', 'DOCUMENT']

classes = [
    'SENSHI', 'KISHI', 'RYOSHI', 'YABANJIN', 'KICHIGAI', 'SAMURAI', 'RONIN', 'YAKUZA',
    'ANSATSUSHA', 'NINJA', 'SHUKENJA', 'SHISAI', 'SHIZEN', 'MAHOTSUKAI', 'GENKAI', 'KOSAKU']

item_effects = [
    'INOCHI', 'ALNASU', 'ZUMA', 'KOROSU', 'MOINOCHI', 'TSUIHO', 'KAKUSU',
    'HOHYO', 'MOAKARI', 'ICHIHAN', 'HITATE', 'SANTATE', 'dispel curtain', 'see in Hell', 'message']

# this was used to focus on remaining columns/bytes
# ignore = [0, 2, 3, 4, 6, 7, 8, 9, 0xa, 0xb, 0xc]
# however, bytes 1 and 2 are tied together, so I'll just be ignoring #2
ignore = [2]

headers = [
    'SELL', 'WHO CAN EQUIP IT           ', '?2', 'ATT', 'DMG+', 'TO-HIT', 'AC',
    'MTYPE', 'RACE ', 'CLASS-USE', 'USES', 'EFFECT', 'SLOT        ']

valid_headers = []


def what_it_means(myar, y):
    global slots
    global classes
    global ignore
    x = myar[y]
    if y in ignore:
        return ""
    temp = ""
    if y is 0:
        temp = 'CAN\'T' if x is 0 else str(x)
    elif y is 1:
        if myar[9] != 255:
            temp = classes[myar[9]] + ' ONLY'
        elif myar[2] == 0:
            temp = 'ALL'
        elif myar[1] == 255 and myar[2] == 255:
            temp = 'NONE/AUTO'
        elif myar[1] == 0:
            temp = 'NO HEALERS'
        elif myar[1] == 1:
            temp = 'FIGHTERS/HEALERS'
        elif myar[1] == 2:
            temp = 'NO CASTERS'
        elif myar[1] == 3:
            temp = 'FIGHTERS ONLY'
        elif myar[2] == 1:
            temp = 'NO NINJA/SHUKENJA/MAGES'
        elif myar[2] == 2:
            temp = 'FIGHTERS/HEALERS NO YABANJIN'
        elif myar[2] == 3:
            temp = 'SENSHI/KISHI/SAMURAI/RONIN'
        else:
            temp = str(x) + ' ' + str(myar[2])
    elif y is 2:
        temp = ''
    elif y is 5 or y is 6:
        temp = str(x - 256 if x > 128 else x)
    elif y is 7:
        temp = {
            0: 'DRAGON',
            1: 'GIANT',
            2: 'UNDEAD',
            3: 'DEMON',
            4: 'DLORD'
        }.get(x, 'NONE')
    elif y is 8:
        temp = {
            1: 'TOSHI',
            3: 'KOBITO'
        }.get(x, 'NONE')
    elif y is 9:
        temp = 'N/A' if x is 255 else (str(x) if x >= len(classes) else classes[x])
    elif y is 0xa:
        temp = '++' if x is 0 else ('N/A' if x is 255 else str(x))
    elif y is 0xb:
        temp = 'NONE' if x is 255 else ('??' + str(x) if x > len(item_effects) else item_effects[x])
    elif y is 0xc:
        temp = slots[x] + '/' + str(x) if x < len(slots) else 'UNKNOWN'
    else:
        temp = str(x)
        # temp = 'NONE' if x is 255 else str(x)
    if len(temp) < len(headers[y]) + 1:
        temp = temp + " " * (1 + len(headers[y]) - len(temp))
    return temp


def zapwhite(string, do_i_zap):
    return re.sub(" +\|", "|", string) if do_i_zap else string


def read_one_item(fp, i):
    global zap_whitespace
    string = ""
    ch = [0]
    ch2 = fp.read(13)
    bytear = [int(a) for a in ch2]
    while not ch[0] & 0x80:
        ch = fp.read(1)
        y = (ch[0] ^ 0x65) & 0x7f
        string = string + chr(y)
    #    print('|' + string + '|' + '|'.join(what_it_means(bytear[a], a) for a in range(0, len(bytear))) + '|')
    to_print = '|{:02x}  '.format(i) + '|{:14s}'.format(string) + '|' + '|'.join(
        '{:3s}'.format(what_it_means(bytear, a)) for a in valid_headers) + '|'
    to_print = re.sub(r"\|([-\+])", r"| \1", to_print)  # to make sure gamefaqs markup doesn't see |+ or |-
    print(zapwhite(to_print, zap_whitespace))


#    print('|' + '{:14s}'.format(string) + '|' + '|'.join('{:3d}'.format(a) for a in bytear) + '|')

# I could use numPy for this, but I didn't have it installed at the time
for a in range(0, len(headers)):
    if a not in ignore:
        valid_headers.append(a)

fp = open("dli-0-00", "rb")

fp.seek(0x9b70, 0)

print(";format:gf-markup")
print()

print(zapwhite('|*NUM|*ITEM NAME    |*' + '|*'.join(headers[a] for a in valid_headers) + '|', zap_whitespace))

print_tell = False

for i in range(0, num_items):
    if print_tell:
        print('{:x}'.format(fp.tell()))
    read_one_item(fp, i)
