#================================================================
# Makefile to produce TimLib test program
#================================================================

include /ps/dsc/src/co/Make.auto

CFLAGS = -g -Wall -fwritable-strings -I. -I/ps/local/$(CPU)/include

LDLIBS =

ALL  = PushTcp.$(CPU) PullTcp.$(CPU)

SRCS = PushTcp.c PullTcp.c

HDRS = PushTcp.h

all: $(ALL)

clean:
	$(RM) *.$(CPU) *.$(CPU).o ,*

# Run on Workstation only

PushTcp.$(CPU).o: PushTcp.c

PullTcp.$(CPU).o: PullTcp.c

install: PushTcp.$(CPU) PullTcp.$(CPU)
	@for a in $(ACCS); do \
		d=/ps/dsc/$$a/$(CPU)/tim;\
		echo Installing PushTcp in  $$d;\
		dsc_install PushTcp.$(CPU)  $$d;\
		echo Installing PullTcp in  $$d;\
		dsc_install PullTcp.$(CPU)  $$d;\
	done;
