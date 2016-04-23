# Makefile for BNGine.

SOURCES := $(shell find src -name '*.cpp')
SOURCES += $(shell find ext -name '*.cpp')

OS_MESA ?= 0

#uh....
SOURCES := $(filter-out src/app/win_main.cpp,$(SOURCES))
SOURCES := $(filter-out src/app/x11_main.cpp,$(SOURCES))
SOURCES := $(filter-out src/app/osmesa_main.cpp,$(SOURCES))
SOURCES := $(filter-out src/app/metagen_main.cpp,$(SOURCES))

ifeq ($(OS_MESA),1)
    SOURCES += src/app/osmesa_main.cpp
else
    SOURCES += src/app/x11_main.cpp
endif

LIBFLAGS =-lX11 -lGL

ifeq ($(OS_MESA),1)
    LIBFLAGS += -lOSMesa
endif

OBJ_SUFF := .o

LIB_PREFIX := -l

OBJS := $(addsuffix $(OBJ_SUFF), $(basename $(SOURCES)))

CPP_UTIL_SOURCES := $(shell find ext/CppUtils -name '*.cpp')
CPP_UTILS_OBJS := $(addsuffix $(OBJ_SUFF), $(basename $(CPP_UTIL_SOURCES)))

# OTHERVAR := $(filter-out SomethingElse,$(VAR))

CXX ?= g++

HEADERS := $(shell find src -name '*.h')
HEADERS += $(shell find ext -name '*.h')

GEN_FILES := $(shell find gen -name '*.cpp')

GEN_OBJS := $(addsuffix $(OBJ_SUFF), $(basename $(GEN_FILES)))

GEN_FILES += $(shell find gen -name '*.h')

CXXFLAGS=-g -O0 -DBNS_DEBUG -std=c++11 -Wall -pedantic

ifeq ($(OS_MESA),1)
    CXXFLAGS += -DBNS_OS_MESA
endif

METAGEN_EXE_SOURCES := $(shell find src/metagen -name '*.cpp')
METAGEN_EXE_SOURCES += src/app/metagen_main.cpp

METAGEN_EXE_OBJS := $(addsuffix $(OBJ_SUFF), $(basename $(METAGEN_EXE_SOURCES)))


%.o: %.cpp $(HEADERS) Makefile
	$(CXX) -c -o $@ $< $(CXXFLAGS)

BNGine.out: $(OBJS) $(HEADERS) $(GEN_OBJS)
	$(CXX) $(OBJS) $(GEN_OBJS) $(LIBFLAGS) -o BNGine.out


genCode: BNSMetaGen.out $(HEADERS)
	$(CXX) $(METAGEN_EXE_OBJS) $(CPP_UTILS_OBJS) -o BNSMetaGen.out
	./BNSMetaGen.out

.PHONY : genCode
