TEST_DIR:=/tmp/fuser-filter

all:
	g++ -std=c++17 filterfs.cpp -lfuse3 -lpthread -o filterfs

segment:
	g++ test_segment.cpp -o segment.out
rule:
	g++ test_rule.cpp -o rule.out

close:
	fusermount3 -u $(TEST_DIR)
