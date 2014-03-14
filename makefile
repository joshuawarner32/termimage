# .PHONY: test
# test: termimage
# 	xxd -p /dev/urandom | tr -d '\n' | head -c 10000 | ./termimage pi.png

termimage: termimage.cpp
	c++ -g3 -O0 termimage.cpp -o termimage -lpng