Profiling "mini manual"
=======================

Valgrind is a free software very useful for debugging massive software.
We are going to use it to perform code profiling and highlight memory leaks.
The code must always be builded with debug directive (-g) and unstripped for optimal results.

http://valgrind.org/
http://valgrind.org/docs/manual/manual.html

It is available on many platforms:
X86/Linux, AMD64/Linux, ARM/Linux, PPC32/Linux, PPC64/Linux, S390X/Linux, MIPS32/Linux, MIPS64/Linux,
ARM/Android (2.3.x and later), X86/Android (4.0 and later), X86/Darwin and AMD64/Darwin (Mac OS X 10.7, with limited support for 10.8).


Memcheck - Leak detection and misuse of memory related functions:
-----------------------------------------------------------------

> $ valgrind --leak-check=full --leak-resolution=high ./mysoftware

Tremendously useful for finding memory leaks, pinpoint precisely the code responsible
for memory allocation that are never freed, and much more.

http://valgrind.org/docs/manual/mc-manual.html

> "--leak-check=<no|summary|yes|full>" is essential to precisly pinpoint memory leaks  
> "--leak-resolution=<low|med|high>" set to low to merge similar leaks when presenting you the results  
> "--show-reachable=yes" can detect memory block indirectly lost  
> "--verbose" show extra debugging info  


Callgrind - Function call analyser:
-----------------------------------

> $ valgrind --tool=callgrind ./mysoftware

Account for each function called during execution of the program and generate a massive call graph that shows:
- which function is called by which function
- the CPU time spent in each function

http://valgrind.org/docs/manual/cl-manual.html

### GUIs
- kcachegrind (http://kcachegrind.sourceforge.net/) Very "visual" frontend to cachegrind and callgrind. Tied to the KDE framework.
- qcachegrind (Qt 'only' version of kcachegrind)


Cachegrind - CPU cache and branch-prediction profiler:
------------------------------------------------------

> $ valgrind --tool=cachegrind ./mysoftware

Account for each function called during execution of the program and show:
- an estimation of the number of CPU cycle spent in each function
- CPU cache (L1/2/3) read/write opreations and cache miss
- errors in predictions of branch and loop

http://valgrind.org/docs/manual/cg-manual.html

### GUIs
- kcachegrind (http://kcachegrind.sourceforge.net/) Very "visual" frontend to cachegrind and callgrind. Tied to the KDE framework.
- qcachegrind (Qt 'only' version of kcachegrind)
- qtcreator (http://qt-project.org/wiki/Category:Tools::QtCreator) Good integration of valgrind / cachegrind


Massif - Heap profiler:
-----------------------

Generate report:
> $ valgrind --tool=massif [--time-unit=B] [--stacks=yes] ./mysoftware

Print restults:
> $ ms_print massif.out.xxxxx --x=128 --y=128

Massif produces snapshots of the memory heap state at regular intervals (so it may not 
necessarily catch the peaks uses). For each snapshot we have:
- Its number.
- The time it was taken. In this case, the time unit is bytes, due to the use of --time-unit=B.
- The total memory consumption at that point.
- The number of useful heap bytes allocated at that point. This reflects the number of bytes asked for by the program.
- The number of extra heap bytes allocated at that point. This reflects the number of bytes allocated in excess of what the program asked for. There are two sources of extra heap bytes.

http://valgrind.org/docs/manual/ms-manual.html
