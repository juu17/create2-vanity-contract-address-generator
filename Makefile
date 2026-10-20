CXX      ?= g++
CXXFLAGS ?= -std=c++11 -Wall -O2
CPPFLAGS ?=
LDFLAGS  :=

SOURCES    := Dispatcher.cpp eradicate2.cpp hexadecimal.cpp ModeFactory.cpp Speed.cpp sha3.cpp
OBJECTS    := $(SOURCES:.cpp=.o)
DEPS       := $(OBJECTS:.o=.d)
EXECUTABLE := ERADICATE2.x64

# OpenCL ships as a framework on macOS, and as -lOpenCL elsewhere.
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	LDLIBS := -framework OpenCL
else
	LDLIBS   := -lOpenCL
	LDFLAGS  += -s
	CXXFLAGS += -mcmodel=large
endif

.PHONY: all clean

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) $(LDLIBS) -o $@

# -MMD -MP emits a per-object .d file of header prerequisites, which we -include
# below. Editing a header (help.hpp, version.hpp, ...) then recompiles exactly the
# objects that include it — no more stale builds / manual `make clean`.
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

clean:
	rm -f $(OBJECTS) $(DEPS) $(EXECUTABLE)
