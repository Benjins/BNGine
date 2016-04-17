# Makefile for BNGine.

SOURCES := $(shell find src -name '*.cpp')
SOURCES += $(shell find ext -name '*.cpp')
SOURCES := $(filter-out src/app/win_main.cpp,$(SOURCES))

OS_MESA ?= 0

ifeq ($(OS_MESA),1)
    SOURCES := $(filter-out src/app/x11_main.cpp,$(SOURCES))
else
    SOURCES := $(filter-out src/app/osmesa_main.cpp,$(SOURCES))
endif

LIBFLAGS =-lX11 -lGL

ifeq ($(OS_MESA),1)
    LIBFLAGS += -lOSMesa
endif

OBJ_SUFF := .o

LIB_PREFIX := -l

OBJS := $(addsuffix $(OBJ_SUFF), $(basename $(SOURCES)))

# OTHERVAR := $(filter-out SomethingElse,$(VAR))

CXX ?= g++

HEADERS := $(shell find src -name '*.h')
HEADERS += $(shell find ext -name '*.h')

CXXFLAGS=-g -O0 -DBNS_DEBUG -std=c++11 -Wall -pedantic

ifeq ($(OS_MESA),1)
    CXXFLAGS += -DBNS_OS_MESA
endif

%.o: %.c $(HEADERS) Makefile
	$(CXX) -c -o $@ $< $(CFLAGS)
	
BNGine.out: $(OBJS) $(HEADERS) Makefile
	$(CXX) $(OBJS) $(LIBFLAGS) -o BNGine.out
