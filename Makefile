TEST_DIR:=/tmp/fuser-filter

all:
	g++ -std=c++17 passthrough.cpp -lfuse3 -lpthread -o passthrough

filter:
	g++ filter.cpp -o filter.out
segment:
	g++ test_segment.cpp -o segment.out
rule:
	g++ test_rule.cpp -o rule.out

close:
	fusermount3 -u $(TEST_DIR)
