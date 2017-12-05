PREFIX = 
export CC = $(PREFIX)gcc
export CXX = $(PREFIX)g++
export LD = $(PREFIX)g++
export AR = $(PREFIX)gcc-ar
export FIND = find
export RM = rm -f
export CP = cp -p
export TAR = tar

ifneq (, $(findstring __amd64__,$(shell $(CXX) -dM -E - < /dev/null)))
 LIBDIRNAME = lib64
else
 LIBDIRNAME = lib
endif

CEXT   = .c
CXXEXT = .cpp
OBJEXT = .o
LIBEXT = .a
BINEXT = .exe
DOCDIR = doc
SRCDIR = src
SCRIPTDIR = script
LIBDIR = $(LIBDIRNAME)
INCDIR = include
DSTDIR = bin

#DEBUG = 1
#ENABLE_LTO = 1
#UNICODE = -D_UNICODE

CWFLAGS = -Wall -Wextra -Wformat -pedantic -Wshadow -Wconversion -Wparentheses -Wunused -Wno-missing-field-initializers
CDFLAGS = -DNOMINMAX -D_USE_MATH_DEFINES -DWIN32 -D_WIN32_WINNT=0x0600 -DWIN32_LEAN_AND_MEAN -D_LARGEFILE64_SOURCE -DBOOST_THREAD_USE_LIB -DBOOST_SPIRIT_USE_PHOENIX_V3=1 -DHAS_STRICMP
ifeq (,$(DEBUG))
 CPPFLAGS = -I$(INCDIR) -I$(SRCDIR)
 ifeq (,$(ENABLE_LTO))
  BASE_CFLAGS = -O2 -static -mstackrealign -fno-ident -mtune=core2 -march=core2 -ffunction-sections -fdata-sections -ffast-math -fgraphite -fno-devirtualize -fomit-frame-pointer -DNDEBUG $(CDFLAGS) $(UNICODE)
  LDFLAGS = -O2 -s -static -Wl,--allow-multiple-definition -Wl,--gc-sections -L$(LIBDIR)
 else
  BASE_CFLAGS = -O2 -static -mstackrealign -fno-ident -mtune=core2 -march=core2 -flto -flto-partition=none -fno-devirtualize -ftree-vectorize -fgraphite-identity -ffast-math -fgraphite -fomit-frame-pointer -DNDEBUG $(CDFLAGS) $(UNICODE)
  LDFLAGS = -O2 -s -static -Wl,--allow-multiple-definition -flto -flto-partition=none -fno-devirtualize -ftree-vectorize -fgraphite-identity -fipa-icf -fmerge-all-constants -Wl,--gc-sections -L$(LIBDIR)
 endif
else
 CPPFLAGS = -I$(INCDIR) -I$(SRCDIR)
 BASE_CFLAGS = -Wa,-mbig-obj -Og -g3 -ggdb -gdwarf-3 -static -mstackrealign -fno-devirtualize -fno-omit-frame-pointer -mtune=core2 -march=core2 $(CDFLAGS) $(UNICODE)
 LDFLAGS = -static -fno-omit-frame-pointer -Wl,--allow-multiple-definition -L$(LIBDIR)
endif
ifneq (,$(UNICODE))
 LDFLAGS += -municode
endif
export CFLAGS = -std=gnu99 $(BASE_CFLAGS)
export CXXFLAGS = -Wcast-qual -Wnon-virtual-dtor -Wold-style-cast -Wno-unused-parameter -Wno-long-long -Wno-maybe-uninitialized -std=c++03 $(BASE_CFLAGS)

include common.mk
