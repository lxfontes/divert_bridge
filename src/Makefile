CC = g++
CFLAGS = -O2 -ggdb
CFLAGS += -I../deps/libev/ -DEV_STANDALONE=1 -DEV_MULTIPLICITY=1

CFLAGS += -I ../deps/v8/include
LDFLAGS = ../deps/v8/libv8.a -lpthread -lm
OBJFILES := $(patsubst %.cc,%.o,$(wildcard *.cc))
PROG=divert_bridge

COMPILE = $(CC) $(CFLAGS) -c
all: $(PROG)


$(PROG): $(OBJFILES)
	        $(CC) -o $(PROG) $(OBJFILES) $(LDFLAGS)

%.o: %.cc
	        $(COMPILE) -o $@ $<
clean:
	/bin/rm -f $(OBJFILES) $(PROG)
