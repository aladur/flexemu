EXTRA_DIST = daa.txt conv.c tounix.txt cputest.txt rcvfile.txt undoc.txt

bin_PROGRAMS = bin2s19 fromflex rcvflex toflex send2flx testdaa

AM_CFLAGS = @OS_TYPE@

bin2s19_LINK  = $(CCLD) -o $@
fromflex_LINK = $(CCLD) -o $@
rcvflex_LINK  = $(CCLD) -o $@
toflex_LINK   = $(CCLD) -o $@
send2flx_LINK = $(CCLD) -o $@
testdaa_LINK  = $(CCLD) -o $@

bin2s19_SOURCES  = bin2s19.c
fromflex_SOURCES = fromflex.c
rcvflex_SOURCES  = rcvflex.c
toflex_SOURCES   = toflex.c
send2flx_SOURCES = send2flx.c
testdaa_SOURCES  = testdaa.c

