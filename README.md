## Minesweeper DS

![Screenshot](http://i.imgur.com/OamSvMa.png)

Minesweeper DS is a Nintendo DS clone of the Windows game Minesweeper, with the natural addition of touchpad activation of grid sites. I wrote the game in C++ in 2007.

The game was built with [devkitPro](http://devkitpro.org/), which is a toolchain for developing on various game consoles. devkitPro includes a C/C++ compiler for the relevant instruction sets (ARM in the case of GBA and DS) and a library ([libnds](http://libnds.devkitpro.org/)) with convenience methods for accessing DS system registers, etc. Additionally, Martin Korth's [GBATEK](http://problemkaputt.de/gbatek.htm) was an indispensable reference.

## Release Notes

To mark or unmark mines, hold the left shoulder button and touch the grid site. During play, the touch screen will always display a 12 (rows) by 16 (columns) grid of full-sized tiles. The second screen displays a minimap indicating where you are in the grand scheme of things (useful when the playfield is larger than 12 x 16). Use the arrow keys to scroll around the playfield. Upon winning or losing, you can scroll around the finished playfield by holding the right shoulder button and using the arrow keys.

There is no sound and no support for high scores, although the game does time your progress while you play. The maximal dimensions of the playfield (24 rows by 52 columns) are limited by the graphical space available for drawing the minimap.
