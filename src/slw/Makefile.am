SOURCES = slw_console.cpp slw.cpp slw_popup.cpp slw_log.cpp slw_prompt.cpp \
	abs_text_console.cpp slw_text.cpp ../linklist.cpp ../jutils.cpp

noinst_LTLIBRARIES = libslw.la
libslw_la_SOURCES = $(SOURCES)
libslw_la_LIBADD = -lslang
libslw_la_CXXFLAGS = -I$(top_srcdir)/src

# libfreej_la_LDFLAGS = $(FREEJ_LIBS) -version-info $(FREEJ_VERSION_INFO)
