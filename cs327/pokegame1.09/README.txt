Program to create a Pokemon-esque game.

The game implements a user interface that takes (unbuffered) input to move the pc ('@') around the current map.
The game iterates in "turns" dictated by each pc move.
The game continues until a quit command is received.

User commands are as follows:
    Input           Action
    --------------------------------------------------------------
    '7', 'y'        Attempt to move PC one cell to the upper-left
    '8', 'k'        Attempt to move PC one cell up
    '9', 'u'        Attempt to move PC one cell to the upper-right
    '6', 'l'        Attempt to move PC one cell to the right
    '3', 'n'        Attempt to move PC one cell to the lower-right
    '2', 'j'        Attempt to move PC one cell down
    '1', 'b'        Attempt to move PC one cell to the lower-left
    '4', 'h'        Attempt to move PC one cell to the left
    '5', ' ', '.'   Rest for a turn (NPC's still move)
    '>'             Attempt to enter a building. 
                    Works only if standing on a building. 
                    Leads to a user interface for the appropriate building.
                    Exit with '<'.
    '>'             When in a building interface, exit to map. 
    'f'             Fly to a specified map in the world, range: [-200,200]x[-200,200].
    'b'             Access pc's bag. Items include pokeballs, potions, and revives.
    't'             Display a list of trainers on the map, with symbol and position relative to PC 
                    (e.g. “r, 2 north, 14 west”).
                    Scroll list (if applicable) with Up Arrow and Down Arrow.
                    Return to map with 'esc'.
    Up Arrow        When displaying trainer list, if entire list does not fit in screen 
                    and not currently at top of list, scroll list up.
    Down Arrow      When displaying trainer list, if entire list does not fit in screen 
                    and not currently at bottom of list, scroll list down.
    'esc'           When displaying trainer list, return to map.
    'q'             Quit the game.  
    --------------------------------------------------------------

Makefile run...
    make 
    ./poke327