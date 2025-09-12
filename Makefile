CFLAGS := -Wall -Werror -std=c++17 -I.
LDFLAGS := -lgpiod -L.
CC := g++

stepper: libgpiod.so bgt_tcm2209.cpp bgt_tcm2209.h main.cpp
	$(CC) $(CFLAGS) bgt_tcm2209.cpp main.cpp -o stepper $(LDFLAGS)

dual_stepper: libgpiod.so dual_stepper.cpp
	$(CC) $(CFLAGS) dual_stepper.cpp -o dual-step $(LDFLAGS)

libgpiod.so: libgpiod_stub.c
	$(CC) -fPIC -shared -o libgpiod.so libgpiod_stub.c

default: libgpiod.so stepper dual_stepper

.PHONY: clean
clean:
	rm -f libgpiod.so stepper dual_stepper