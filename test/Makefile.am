
bin_PROGRAMS = rtctest tbtcheck

rtctest_SOURCES =  rtctest.cpp
rtctest_CPPFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)/src/slw
rtctest_LDADD = $(top_builddir)/src/rtclock.o				\
		$(top_builddir)/src/slw/libslw.a -lslang -lpthread

tbtcheck_SOURCES = tbtcheck.cpp
tbtcheck_CPPFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)/src/slw
tbtcheck_LDADD = $(top_builddir)/src/tbt.o				\
		$(top_builddir)/src/rtclock.o				\
		$(top_builddir)/src/slw/libslw.a -lslang -lpthread