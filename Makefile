PREFIX:=/usr/local/bin/
TARGET:=filterfs
CXX:=g++
CXXFLAGS:=-std=c++17 -O2

all:
	$(CXX) $(CXXFLAGS) filterfs.cpp -lfuse3 -lpthread -o $(TARGET)

segment:
	$(CXX) $(CXXFLAGS) test_segment.cpp -o segment.out

rule:
	$(CXX) $(CXXFLAGS) test_rule.cpp -o rule.out

test: segment rule
	./segment.out
	./rule.out

install:
	cp -i $(TARGET) $(PREFIX)/$(TARGET)
