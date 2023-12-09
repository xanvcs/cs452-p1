#Make file to build an executable and a testing harness
#using address sanitizer and google test

ASAN ?= -fsanitize=address -fsanitize=bounds -fsanitize=undefined
CPPFLAGS ?= -std=c++17 -Wall -Wextra -O1 -fno-omit-frame-pointer -g -MMD -MP
LDFLAGS ?= -pthread -lreadline
GTEST ?= -lgtest
UNAME_S := $(shell uname -s)


ifeq ($(UNAME_S),Darwin)
MAC_INCLUDE ?= -I /opt/homebrew/include/
MAC_LIB ?= -L/opt/homebrew/lib/
endif

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(ASAN) $(MAC_INCLUDE) -c $< -o $@

all: myprogram test-lab

myprogram: lab.o main.o
	$(CXX) $(CPPFLAGS) $(ASAN) $(LDFLAGS) $^ -o $@

test-lab: test-lab.o lab.o
	$(CXX) $(CPPFLAGS) $(ASAN) $(LDFLAGS) $(MAC_LIB) $(GTEST) $^ -o $@

check: test-lab
	ASAN_OPTIONS=detect_leaks=1 ./$<

.PHONY: clean
clean:
	$(RM) *.o *.d myprogram test-lab

-include lab.d test-lab.d
