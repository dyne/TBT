# We don't need kludgy automatizations here,
# let's use a simple Makefile.
# Just tweak the values below to fix your paths


# adjust for your installation path
PREFIX = /usr

CPP = g++
CXX = g++
LINKER = ld




# debugging flags:
# CXXFLAGS = -Wall -ggdb -pg -Islw -I. -I/usr/pkg/include -DHAVE_BSD

# optimized flags:
CXXFLAGS = -Wall -O2 -fomit-frame-pointer -ffast-math -Islw -I. -I/usr/pkg/include -DHAVE_BSD

# Darwin/OSX flags: uncomment all below and comment the rest
# CPPFLAGS = -pipe -Wall -ggdb -I../slw -I. -I/sw/include -L/sw/lib -DHAVE_BSD

# fix different paths for slang headers in various distros
CXXFLAGS += -I/usr/include/slang 

# flags to compile slang linking to dynamic system lib

LIBS = -lslang -lpthread slw/libslw.a

# flags to compile slang linking to dynamic libs on BSD

#LIBS = -L/usr/pkg/lib -lpthread -lslang ../slw/libslw.a 

# flags to include static slang library from the source
# (need to provide the full path to your libslang.a)
#SLANGPATH  = ../slang-2.0.6
#CPPFLAGS = -Wall -ggdb
#CXXFLAGS = -Wall -ggdb -I. -I$(SLANGPATH)/src
#LIBS = $(SLANGPATH)/src/objs/libslang.a -ltermcap

# flags to compile with memodebugging

DEPS = tbt.o linklist.o jutils.o rtclock.o


# generic make rules
#%.o: %.cpp
#	$(CXX) $(CXXFLAGS) -c -o $@ $<
#%: %.cpp
#	$(CXX) $(CXXFLAGS) -o $@ $< $(DEPS) $(LIBS)

all: slw tbt tbtcheck tbtcheck_ascii rtctest

depend:
	mkdep $(CXXFLAGS) tbt.cpp cmdline.cpp

slw: slw/slw.o
	make -C slw

tbt: cmdline.o $(DEPS)
	$(CXX) $(CXXFLAGS) -o tbt cmdline.o $(DEPS) $(LIBS)
	ln -sf tbt rectext
	ln -sf tbt playtext
	ln -sf tbt recmail

recmail: recmail.o $(DEPS)
	$(CPP) $(CXXFLAGS) -o recmail recmail.o $(DEPS) $(LIBS)

tbtcheck: $(DEPS) tbtcheck.o
	$(CPP) $(CXXFLAGS) -o tbtcheck tbtcheck.o $(DEPS) $(LIBS)

tbtcheck_ascii: $(DEPS) tbtcheck_ascii.o
	$(CPP) $(CXXFLAGS) -o tbtcheck_ascii tbtcheck_ascii.o $(DEPS) $(LIBS)

rtctest: $(DEPS) rtctest.o
	$(CPP) $(CXXFLAGS) -o rtctest rtctest.o $(DEPS) $(LIBS)

install: all
	install tbt $(PREFIX)/bin
	ln -sf $(PREFIX)/bin/tbt $(PREFIX)/bin/rectext
	ln -sf $(PREFIX)/bin/tbt $(PREFIX)/bin/playtext
	ln -sf $(PREFIX)/bin/tbt $(PREFIX)/bin/recmail
	install tbt.1 $(PREFIX)/share/man/man1/
	
clean:
	rm -rf *.o *~ tbt
	rm -f record.tbt
	find . -type l -exec rm -f {} \;
	rm -f tbtcheck tbtcheck_ascii rtctest
	make -C web clean
	make -C slw clean

#%: %.c

#	$(CC) $(CFLAGS) -o $@ $< $(LIBS)
#%.o: %.c
#	$(CC) $(CFLAGS) -c -o $@ $<

