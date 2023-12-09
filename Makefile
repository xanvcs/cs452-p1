#Make file to build an executable and a testing harness
#using address sanitizer and google test

ASAN ?= -fsanitize=address -fsanitize=bounds -fsanitize=undefined -fno-omit-frame-pointer
CPPFLAGS ?= -std=c++17 -Wall -O1 -g -MMD -MP
LDFLAGS ?= -std=c++17 -pthread -lreadline
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Linux)
GTEST ?= -lgtest_main -lgtest
endif

ifeq ($(UNAME_S),Darwin)
INCLUDE ?= -I /opt/homebrew/include/
GTEST ?= -L/opt/homebrew/lib/ -lgtest_main -lgtest
endif

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(ASAN) $(INCLUDE) -c $< -o $@

all: myprogram test-lab

myprogram: lab.o main.o
	$(CXX) $(ASAN) $(LDFLAGS) $^ -o $@

test-lab: test-lab.o lab.o
	$(CXX) $(ASAN) $(LDFLAGS) $(GTEST) $^ -o $@

check: test-lab
	ASAN_OPTIONS=detect_leaks=1 ./$<

.PHONY: clean
clean:
	$(RM) *.o *.d myprogram test-lab

-include lab.d test-lab.d
