# Makefile for BNGine.

SOURCES := $(shell find src -name '*.cpp')
SOURCES += $(shell find ext -name '*.cpp')
SOURCES := $(filter-out src/app/win_main.cpp,$(SOURCES))

OBJ_SUFF := .o

LIB_PREFIX := -l

OBJS := $(addsuffix $(OBJ_SUFF), $(basename $(SOURCES)))

# OTHERVAR := $(filter-out SomethingElse,$(VAR))

CXX ?= g++

HEADERS := $(shell find src -name '*.h')
HEADERS += $(shell find ext -name '*.h')

CXXFLAGS=-g -DBNS_DEBUG -std=c++11 -Wall -pedantic

%.o: %.c $(HEADERS)
	$(CXX) -c -o $@ $< $(CFLAGS)
	
BNGine.out: $(OBJS)
	$(CXX) $(OBJS) -lX11 -lGL -o BNGine.out
