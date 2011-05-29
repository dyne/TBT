SUBDIRS = slw


AM_CXXFLAGS = -I$(top_srcdir)/src/slw

SOURCES = tbt.cpp rtclock.cpp

noinst_HEADERS = tbt.h rtclock.h

LIBS = -lpthread -lslang $(top_builddir)/src/slw/libslw.a


bin_PROGRAMS = tbt recmail tbtcheck rtctest

tbt_SOURCES = $(SOURCES) cmdline.cpp
tbt_LDADD = $(LIBS)

recmail_SOURCES = $(SOURCES) recmail.cpp
recmail_LDADD = $(LIBS)

tbtcheck_SOURCES = $(SOURCES) tbtcheck.cpp
tbtcheck_LDADD = $(LIBS)

rtctest_SOURCES = $(SOURCES) rtctest.cpp
