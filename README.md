# rpg-mapping-tools

A collection of my old mapping tools for various games

This contains some maps I wrote for games I wrote guides for at GameFAQs, but more importantly, it contains tools I used to create the maps, and their refinements.

Unfortunately many of the individual original maps are lost and probably not going to be replaced. But I think the applications have some merit, and I enjoyed engineering a better version.

# MAPCPY and MAPCONV

The main apps are mapcpy and mapconv. I built them with Visual C++ 6. They should have no Windows dependencies. They respectively copy data from binary files and use icons to convert that data to a human-readable bitmap.

I should eventually be able to merge them together if needed, but that may be a way off.

# U4 and U5

My Ultima IV and V map crawlers have Windows dependencies since they are GUI. I made them to be able to poke through the Ultima dungeons without playing a game. Oh and to get comfortable with Windows graphics/GUI programming.

# Samples

Most of these are map samples. However, some subdirectories have something extra.

# Deathlord

Deathlord has item and monster extractors. It was a tricky game, so I never really delved into what the items did. But the project seemed ideal for learning python, and the result is the two scripts you can see in the subdirectory.

# Editor

This is a GUI based editor for 3-d (perspective) mapped games. I haven't documented it well, but you can use (shift)-IJKM or (shift)-arrow keys to create or change walls, as well as put in special square notifications.