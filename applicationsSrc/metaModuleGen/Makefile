mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
current_dir := $(notdir $(patsubst %/,%,$(dir $(mkfile_path))))
APPDIR = ../../applicationsBin/$(current_dir)

SRCS = metaModuleGen.cpp metaModuleGenBlockCode.cpp metaModuleCore.cpp metaModuleSearch.cpp
OUT = $(APPDIR)/metaModuleGen
MODULELIB = -lsimCatoms3D
TESTS =

OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.depends)

OS = $(shell uname -s)
SIMULATORLIB = $(MODULELIB:-l%=../../simulatorCore/lib/lib%.a)

ifeq ($(GLOBAL_INCLUDES), )
INCLUDES = -I. -I../../simulatorCore/src -I/usr/local/include -I/opt/local/include -I/usr/X11/include
else
INCLUDES = -I. -I../../simulatorCore/src $(GLOBAL_INCLUDES)
endif

INCLUDES += $(LOCAL_INCLUDES)

ifeq ($(GLOBAL_LIBS), )
	ifeq ($(OS),Darwin)
LIBS = -L./ -L../../simulatorCore/lib -L/usr/local/lib -lGLEW -lglut -framework GLUT -framework OpenGL -L/usr/X11/lib /usr/local/lib/libglut.dylib /usr/local/lib/libmuparser.dylib $(MODULELIB)
	else
LIBS = -L./ -L../../simulatorCore/lib -L/usr/local/lib -L/opt/local/lib -L/usr/X11/lib -lglut -lGL -lGLU -lGLEW -lpthread -lm -ldl -lmuparser $(MODULELIB)
	endif
else
LIBS = $(GLOBAL_LIBS) -L../../simulatorCore/lib
endif

ifeq ($(GLOBAL_CCFLAGS),)
CCFLAGS = -g -Wall -std=c++17 -Wsuggest-override -fno-stack-protector
	ifeq ($(OS), Darwin)
	CCFLAGS += -DGL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED -Wno-deprecated-declarations -Wno-overloaded-virtual
	endif
else
CCFLAGS = $(GLOBAL_CCFLAGS)
endif

CC = g++

.PHONY: clean all test

.cpp.o:
	$(CC) $(INCLUDES) $(CCFLAGS) -c $< -o $@

%.depends: %.cpp
	$(CC) -M $(CCFLAGS) $(INCLUDES) $< > $@

all: $(OUT)
	@:

test:
	@:

autoinstall: $(OUT)
	cp $(OUT)  $(APPDIR)

$(APPDIR)/$(OUT): $(OUT)

$(OUT): $(SIMULATORLIB) $(OBJS)
	$(CC) -o $(OUT) $(OBJS) $(LIBS)

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS)
endif

clean:
	rm -f *~ $(OBJS) $(OUT) $(DEPS)
