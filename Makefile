# Build details

CXX = g++
CXXFLAGS = -W -Wall -g -std=c++11
BD = ./build

# Compile to objects

$(BD)/%.o: %.cpp
	-$(if $(wildcard $(BD)),,mkdir -p $(BD))
	$(CXX) -c -o $@ $< $(CXXFLAGS)

# Build Executable

bin = sched

all: $(bin)

rebuild: clean all

# Executable 1

obj = sched.o scheduler.o simulator.o event.o random.o process.o
objbd = $(obj:%=$(BD)/%)

$(bin): $(objbd)
	$(CXX) -o $(bin) $(objbd) $(CXXFLAGS)

# Dependencies

$(BD)/scheduler.o: scheduler.h event.h process.h
$(BD)/simulator.o: simulator.h random.h event.h process.h scheduler.h debug.h
$(BD)/event.o: event.h process.h debug.h
$(BD)/sched.o: debug.h simulator.h random.h event.h process.h scheduler.h
$(BD)/random.o: random.h debug.h
$(BD)/process.o: process.h debug.h

# Clean up

clean:
	rm -f "$(bin)" $(objbd)
	rm -fd "$(BD)"

# PHONY

.PHONY: all rebuild clean
