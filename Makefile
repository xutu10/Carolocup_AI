CPPFLAG= -std=c++11
#RM= rm -f

.PHONY: all test clean distclean

all: park_test

park_test: park_test.cpp park.cpp
	$(CXX) $(CPPFLAG) $^ -o  $@

clean:
	rm -f park_test

distclean: clean
