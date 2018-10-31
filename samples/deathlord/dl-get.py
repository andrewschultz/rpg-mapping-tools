#
# dl-get = get all the icons and oconvert them to names, so we can find special characters etc.
#

import re
import glob
import sys
import os

# errata: Stairs dungeon is a word dungeon. Fix that.
# fort wintergreen should be labeled.
# kawahara map is ordered wrong
# 1 2 5
# 3 4 6
# what is the grey square in the Troll Hole?

# also, mark off chutes and special treasures like the Ruby Ring

finished = [
  'dl-osozaki', 'dl-red-shogun', 'dl-kodan-s', 'dl-kodan', 'dl-kodan-e', 'dl-isle-dead', 'dl-asagata', 'dl-elements', 'dl-akmihr',
  'dl-giluin-n', 'dl-giluin-s', 'dl-narawn', 'dl-sirion-w', 'dl-sirion-e', 'dl-nyuku', 'dl-tsumani', 'dl-chigaku', 'dl-skull'
  ]
# finished = []

important_dungeon = {
  0x08: 'message', 0x34: 'message',
  0xc5: 'word gate'
  }
important_inside = {
  0x03: 'stairs down', 0x04: 'stairs up',
  0x27: 'curtain', 0x38: 'Arkham\'s wall',
  0x8d: 'ruby ring chest', 0xdb: 'emerald rod chest'
  }
important_out = {
  0x0b: 'tundra dungeon', 0x0e: 'dungeon', 0x10: 'snow mtn dungeon', 0x13: 'north palace', 0x14: 'town', 0x15: 'ruins', 0x16: 'temple', 0x17: 'village',
  0x19: 'city', 0x1c: 'snow town', 0x1d: 'snow ruins', 0x1f: 'tundra village', 0x20: 'winter fort', 0x22: 'tower', 0x23: 'sultan\'s palace', 0x24: 'desert town',
  0x26: 'skull keep', 0x27: 'desert village', 0x28: 'fort', 0x2a: 'pyramid'
  }

text_name = important_out.copy()
text_name.update(important_dungeon)
text_name.update(important_inside)

inspected = {}

for x in finished:
    inspected[x] = False

def print_ico(a, b):
    global fp
    if b < 0 or b > 4095:
        return(b + ' is out of range.')
        return
    fp.seek(b + 0xc70)
    z = fp.read(1)
    return(a + ": " + "0x{0:02x}, ".format(z[0]) + '{0:02d}'.format(z[0]))

def worth_listing(file_name, my_str):
    global important_out
    global important_dungeon
    global important_inside
    if re.search('-d-', file_name):
        return my_str in important_dungeon.keys() or my_str in important_inside.keys()
    if re.search("-(t|town)-", file_name):
        return my_str in important_inside.keys()
    if not re.search('-t-', file_name) and not re.search('-d-', file_name):
        return my_str in important_out.keys()
    return False

def look_for_important(file_name):
    global text_name
    found = False
    fp.seek(0xc70)
    z = fp.read(4096)
    isdun = re.search("-d-", file_name)
    default_level = ( 4*((int(re.sub(".*-", "", file_name)) - 1)//4) + 1 ) if(re.search("[0-9]$", file_name)) else 0
    for x in range(0,len(z)):
        the_level = ((x % 2048) // 1024) + 2 * (x // 2048)
        x_actual = (x % 32) + 64 * ((x % 1024) // 32) + 32 * ((x % 2048) // 1024) + 2048 * (x // 2048) if isdun else x
        x2 = z[x_actual]
        if worth_listing(file_name, x2):
            dungeon_text = '| ' + ('L' + str(the_level + default_level) if re.search(r'-(d|dungeon)-', file_name) else 'N/A')
            found = True
            print('| ' + file_name + '|{0:02x}'.format(x_actual % 64) + '|{0:02x}'.format(x_actual // 64) + '| '+ text_name[x2] + dungeon_text + '| (comment here)|')
            # print('{0:02x}'.format(x2 % 64) + ', {0:02x}'.format(x2 // 64) + ' icon {0:02x} '.format(x) + int(x) + '(' + important[x] + ').')
    if not found:
        print('Found nothing special.')
        return 1
    return 0

# my_file = ""

unspecial = 0

count = 1

save_states = glob.glob("c:\\emu\\apple\\dlsave\\*")

total_files = read_files = 0

for my_file in save_states:
    total_files = total_files + 1
    filesize = os.path.getsize(my_file)
    # filesize = os.stat('C:\\Python27\\Lib\\genericpath.py').st_size
    if filesize != 145400:
        print ("Non-save state file", my_file, "should have size 145400 but has", filesize )
        continue
    if re.search("\.bak", my_file):
        print("Ignoring backup file", my_file)
        continue
    if re.search("\.aws", my_file):
        print("Ignoring default save file", my_file)
        continue
    if re.search("\.bmp", my_file):
        print("Ignoring bitmap file", my_file)
        continue
    my_short = my_file
    my_short = re.sub(r".*\\", "", my_file)
    inspected[my_short] = True
    if my_short in finished:
        continue
    try:
        fp = open(my_file, "rb")
    except:
        print("Bad file name", my_file)
        exit()
    read_files = read_files + 1
    print(my_file + ('*' * 20))
    fp.seek(0xfc76, 0)
    x = fp.read(2)
    fp.seek(0xfcbf, 0)
    x = x + fp.read(1)
    offset = 64 * int(x[1]) + int(x[0])
    print('|' + my_file + '|{0:02x}'.format(x[0]) + ', ' + '{0:02x}'.format(x[1]) + '| level {0:02x}'.format(x[2]))
    print(print_ico('north', offset - 64), print_ico('west', offset - 1), print_ico('east', offset + 1), print_ico('south', offset + 64))
    unspecial = unspecial + look_for_important(my_short)
    count = count + 1
# note that 1N of Red Shogun is the secret dungeon, 2N is where you enter it

print(total_files, 'total files', read_files, 'read files', unspecial, 'unspecial')

for a in inspected.keys():
    if inspected[a] is False:
        print(a, 'was skipped.')