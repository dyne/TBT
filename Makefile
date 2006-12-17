# We don't need kludgy automatizations here,
# let's use a simple Makefile.
# Just tweak the values below to fix your paths


CPP = g++
CXX = g++
LINKER = ld




# debugging flags:
# CPPFLAGS = -pipe -Wall -ggdb -I.

# optimized flags:
# CPPFLAGS = -pipe -Wall -O2 -fomit-frame-pointer -ffast-math

# Darwin/OSX flags: uncomment all below and comment the rest
# CPPFLAGS = -pipe -Wall -ggdb -I. -I/sw/include -L/sw/lib

# flags to compile slang linking to dynamic system lib
LIBS = -lslang -lpthread

# flags to compile slang linking to dynamic libs on BSD
CXXFLAGS = -Wall -ggdb -I../slw -I. -I/usr/pkg/include -DHAVE_BSD
LIBS = -L/usr/pkg/lib -lpthread -lslang ../slw/libslw.a 

# flags to include static slang library from the source
# (need to provide the full path to your libslang.a)
#SLANGPATH  = ../slang-2.0.6
#CPPFLAGS = -Wall -ggdb
#CXXFLAGS = -Wall -ggdb -I. -I$(SLANGPATH)/src
#LIBS = $(SLANGPATH)/src/objs/libslang.a -ltermcap

# flags to compile with memodebugging

DEPS = tbt.o linklist.o jutils.o


# generic make rules
#%.o: %.cpp
#	$(CXX) $(CXXFLAGS) -c -o $@ $<
#%: %.cpp
#	$(CXX) $(CXXFLAGS) -o $@ $< $(DEPS) $(LIBS)

all: rectext playtext recmail

depend:
	mkdep $(CXXFLAGS) tbt.cpp \
	                  playtext.cpp rectext.cpp

rectext: rectext.o $(DEPS)
	$(CPP) $(CXXFLAGS) -o rectext rectext.o $(DEPS) $(LIBS)

playtext: $(DEPS) playtext.o
	$(CPP) $(CXXFLAGS) -o playtext playtext.o $(DEPS) $(LIBS)

recmail: recmail.o $(DEPS)
	$(CPP) $(CXXFLAGS) -o recmail recmail.o $(DEPS) $(LIBS)

tbtcheck: $(DEPS) tbtcheck.o
	$(CPP) $(CXXFLAGS) -o tbtcheck tbtcheck.o $(DEPS) $(LIBS)

rtctest: $(DEPS) rtctest.o
	$(CPP) $(CXXFLAGS) -o rtctest rtctest.o $(DEPS) $(LIBS)

clean:
	rm -rf *.o *~ rectext playtext recmail
#%: %.c

#	$(CC) $(CFLAGS) -o $@ $< $(LIBS)
#%.o: %.c
#	$(CC) $(CFLAGS) -c -o $@ $<

