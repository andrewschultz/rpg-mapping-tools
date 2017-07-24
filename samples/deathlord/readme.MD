# DEATHLORD extraction files

These are written in python.

* dl-monsters.py reads a save state to find all 130 monsters. 1 byte's function is unknown.
* dl-items.py reads a save state to find all 75 monsters. All bytes are accounted for.

The save file used should make no difference. The monster and item information are loaded into memory when a game starts, and there is no need to shuffle them around.

Still to come: mapping (?) with teleports, etc. I would need to use the DSK images, and I think one was slightly corrupt.

