CXX = g++
FLAGS = -O2
DEBUG = no

ifeq ($(strip $(DEBUG)),yes)
    FLAGS += -DDEBUG
endif

PROG = alimerge

.PHONY: all

all: $(PROG)

$(PROG): $(PROG).o
	$(CXX) -o $@ $^

%.o: %.cpp
	$(CXX) $(FLAGS) -c -o $@ $<

clean:
	rm -f $(PROG) *.o
