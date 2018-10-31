#
# dl-uniq.py
# maps where bytes pop up first and their frequencies in a save state
# this will tell where to find maps etc.
#
# the threshold can be defined as well, but it's not on the command line, because this is an old script where I didn't use configs much.
#

import re
import glob
import os

finished = []

max_threshold = 1024
min_threshold = 2

save_states = glob.glob("c:\\emu\\apple\\dlsave\\*")

total_files = read_files = 0

for my_file in save_states:
    file_size = os.path.getsize(my_file)
    if file_size != 145400:
        print("Non-save state file", my_file, "should have size 145400 but has", file_size)
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
    my_short = re.sub(r".*\\", "", my_file)
    if my_short in finished:
        continue
    try:
        fp = open(my_file, "rb")
    except KnownExceptions:
        print("Trouble opening", my_file)
        exit()
    read_files = read_files + 1
    byte_array = fp.read(4096)
    frequency = {}
    found_at_x = {}
    found_at_y = {}
    for x in range(0, len(byte_array)):
        if byte_array[x] in frequency.keys():
            frequency[byte_array[x]] = frequency[byte_array[x]] + 1
        else:
            frequency[byte_array[x]] = 1
            found_at_x[byte_array[x]] = x % 64
            found_at_y[byte_array[x]] = x // 64
    for y in frequency.keys():
        if frequency[y] <= max_threshold and frequency[y] >= max_threshold:
            print(my_short,
                  'icon {:02x}'.format(y), 'x={:02x}'.format(found_at_x[y]),
                  'y={:02x}'.format(found_at_y[y]), frequency[y])
