Debugging mini manual
=====================

What is GDB? (from https://www.gnu.org/software/gdb/)

GDB, the GNU Project debugger, allows you to see what is going on `inside' another program while it executes -- or what another program was doing at the moment it crashed.

GDB can do four main kinds of things (plus other things in support of these) to help you catch bugs in the act:

- Start your program, specifying anything that might affect its behavior.
- Make your program stop on specified conditions.
- Examine what has happened, when your program has stopped.
- Change things in your program, so you can experiment with correcting the effects of one bug and go on to learn about another.
The program being debugged can be written in Ada, C, C++, Objective-C, Pascal (and many other languages). Those programs might be executing on the same machine as GDB (native) or on another machine (remote). GDB can run on most popular UNIX and Microsoft Windows variants.

Debbugging using a GUI
----------------------

Most of today's IDE have great integration with GDB, so you should probably start by using it through your favorite IDE. 

Other GUI includes:
- Nemiver (https://wiki.gnome.org/Apps/Nemiver)
- DDD (http://www.gnu.org/software/ddd/)

Debbugging with GDB
-------------------

> $ gdb --args ./myprogram -with -arguments

Then, type "r" to run the program, and in case of a crash "bt" to print the backtrace.
