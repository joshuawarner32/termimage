# .PHONY: test
# test: termimage
# 	xxd -p /dev/urandom | tr -d '\n' | head -c 10000 | ./termimage pi.png

all: termimage calcpi

example: all
	./calcpi 100000 | ./termimage pi.png

termimage: termimage.cpp
	c++ -O3 termimage.cpp -o termimage -lpng

calcpi: calcpi.cpp
	c++ -O3 calcpi.cpp -o calcpi