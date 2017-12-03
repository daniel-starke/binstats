binstats
========

Symbol statistics viewer for GNU binaries.  

Usage
=====

1. Start the application
2. Select path to [nm](https://sourceware.org/binutils/docs/binutils/nm.html)
3. Select path to binary with compatible symbol table (e.g. object files)
4. Adjust search pattern to select only a range of symbols
5. View output lists

Set the default path to nm by setting the environment variable NM to the specific path.  

Building
========

The following dependencies are given:  
- C99
- C++03
- Boost cstdint.hpp (this header file only)
- FLTK 1.3.4-1

Edit Makefile to match your target system configuration.  
Building the program:  

    make

License
=======

See [copying file](doc/COPYING).  
