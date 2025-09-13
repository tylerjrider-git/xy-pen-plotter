CFLAGS := -Wall -Werror -std=c++17
CFLAGS += $(shell pkg-config --cflags libevdev)
LDFLAGS += $(shell pkg-config --libs libevdev libgpiod)
CC := g++


default: stepper dual_stepper xbox

stepper: bgt_tcm2209.cpp bgt_tcm2209.h main.cpp
	$(CC) $(CFLAGS) bgt_tcm2209.cpp main.cpp -o stepper $(LDFLAGS)

dual_stepper: StepperController.cpp bgt_tcm2209.cpp
	$(CC) $(CFLAGS)  $^ -o $@ $(LDFLAGS)

libgpiod.so: libgpiod_stub.c
	$(CC) $(CFLAGS) -fPIC -shared -o libgpiod.so libgpiod_stub.c $(LDFLAGS)

xbox_dummy: XboxController.cpp
	$(CC) $(CFLAGS) -DMOCK_EVDEV XboxController.cpp -o xbox $(LDFLAGS)

xbox: XboxController.cpp
	$(CC) $(CFLAGS) XboxController.cpp -o xbox $(LDFLAGS)

.PHONY: clean
clean:
	rm -f libgpiod.so stepper dual_stepper xbox