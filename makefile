# .PHONY: test
# test: termimage
# 	xxd -p /dev/urandom | tr -d '\n' | head -c 10000 | ./termimage pi.png

all: termimage calcpi

termimage: termimage.cpp
	c++ -g3 -O0 termimage.cpp -o termimage -lpng

calcpi: calcpi.cpp
	c++ -O3 calcpi.cpp -o calcpi