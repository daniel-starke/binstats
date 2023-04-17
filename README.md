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

FAQ
====

**Q:** Why is there more than on destructor of a single class?  
**A:** There are actually 3 times of destructors. Standard, `virtual` and `delete` on `virtual`. See [mangling rules](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#definitions).

**Q:** Can I disable symbol demangling?  
**A:** Yes, just set the environment variable `DISABLE_DEMANGLING=1`.

**Q:** Can I pass the path of NM to binstats?  
**A:** Yes, set the environment variable `NM` accordingly.

**Q:** How can I update the view after the object file was re-built?  
**A:** Press `F5`.

**Q:** How can I copy the content of the lists?  
**A:** Press `CTRL-C` for the current line and `SHIFT-CTRL-C` for the whole list.

**Q:** Why are there symbols with negative sizes?  
**A:** This may be a bug in nm. Please consult the binutils community.

License
=======

See [copying file](doc/COPYING).  
