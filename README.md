# Gambulator - a Nintendo Gameboy emulator

Gambulator is written in C++ and can be run on Linux. 

The emulator is still in a rough state. Some games run 100% correctly (such as Tetris, Dr. Mario), but others are unplayable. In addition, the code still has some bottlenecks and structural issues that occurred after finding workarounds for unexpected bugs. Once most games are playable, restructuring will begin.

To build the source code, you need SDL2 installed. After taking care of that, just run make.

To run, execute ./gameboy.exe <romfile>. 

Tetris   
![alt tag](http://i.imgur.com/4ujSnAN.gif)


Dr. Mario   
![alt tag](http://i.imgur.com/XFbtfDE.gif)
