# We don't need kludgy automatizations here,
# let's use a simple Makefile.
# Just tweak the values below to fix your paths
# included is also a Makefile.am example for automake


CPP = g++
CXX = g++
LINKER = ld

# debugging flags:
CXXFLAGS = -pipe -Wall -ggdb -I.

# optimized flags:
# CXXFLAGS = -pipe -Wall -O2 -fomit-frame-pointer -ffast-math

# Darwin/OSX flags: uncomment all below and comment the rest
# CXXFLAGS = -pipe -Wall -ggdb -I. -I/sw/include -L/sw/lib

# flags to compile slang linking to dynamic system lib
LIBS = -lslang

# flags to compile slang linking to dynamic libs on BSD
# CXXFLAGS = -Wall -ggdb -I. -I/usr/pkg/include -DHAVE_BSD
# LIBS = -L/usr/pkg/lib -lslang

# flags to include static slang library from the source
# (need to provide the full path to your libslang.a)
#SLANGPATH  = ../slang-2.0.6
#CPPFLAGS = -Wall -ggdb
#CXXFLAGS = -Wall -ggdb -I. -I$(SLANGPATH)/src
#LIBS = $(SLANGPATH)/src/objs/libslang.a -ltermcap

DEPS = slw_console.o slw.o slw_popup.o abs_text_console.o slw_text.o linklist.o jutils.o

# generic make rules
#%.o: %.cpp
#	$(CXX) $(CXXFLAGS) -c -o $@ $<
#%: %.cpp
#	$(CXX) $(CXXFLAGS) -o $@ $< $(DEPS) $(LIBS)

all: libslw.a testkey

depend:
	mkdep $(CXXFLAGS) linklist.cpp jutils.cpp slw.cpp slw_console.cpp slw_text.cpp slw_popup.cpp abs_text_console.cpp

libslw.a: $(DEPS)
	ar cru libslw.a $(DEPS)

testkey: $(DEPS) testkey.o
	$(CXX) -o testkey $(DEPS) testkey.o $(LIBS)

clean:
	rm -rf *.o *! libslw.a testkey

