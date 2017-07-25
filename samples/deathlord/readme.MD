# DEATHLORD extraction files



These are written in Python..

* dl-monsters.py reads a save state to find all 130 monsters. The final byte's function is unknown.
* dl-items.py reads a save state to find all 75 monsters. All bytes are accounted for.

The save file used should make no difference. The monster and item information are loaded into memory when a game starts, and there is no need to shuffle them around.

Still to come: mapping with teleports, etc. I would need to use the DSK images, and I think one was slightly corrupt, and I would need mapcpy to understand Deathlord's compression scheme (not done as of July 2017).

These scripts were used to create the item and monster tables for the in-depth guide (monsters and such) you can find [here](https://www.gamefaqs.com/appleii/574838-deathlord/faqs). It contains more than that, but those were a huge catalyst.