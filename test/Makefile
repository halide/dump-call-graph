CXX ?= g++
MKDIR=mkdir -p
RM=rm -f

BIN_DIR=../bin

HALIDE_DIR ?= ../../Halide
HALIDE_LIB=$(HALIDE_DIR)/bin/$(BUILD_PREFIX)/libHalide.a
HALIDE_INC=$(HALIDE_DIR)/include/
HALIDE_SUP=$(HALIDE_DIR)/apps/support/

PNGFLAGS=$(shell libpng-config --ldflags) $(shell libpng-config --cflags)

TEST_SRCS := $(wildcard *.cpp)
TEST_APPS := $(patsubst %.cpp,%,$(TEST_SRCS))
TEST_CALLS := $(patsubst %.cpp,%.calls.json,$(TEST_SRCS))

all: $(TEST_CALLS)

DumpCallGraph.o: ../DumpCallGraph.cpp ../DumpCallGraph.h $(HALIDE_INC)
	$(CXX) -fno-rtti -c -I$(HALIDE_INC) $< -o $@

%: %.cpp DumpCallGraph.o $(HALIDE_INC) $(HALIDE_LIB)
	$(CXX) $< DumpCallGraph.o -I$(HALIDE_INC) $(HALIDE_LIB) -o $@ -ldl -lpthread

%.calls.json: %
	./$<

clean:
	$(RM) DumpCallGraph.o bilateral_grid bilateral_grid.calls.json
