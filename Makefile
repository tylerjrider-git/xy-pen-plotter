CFLAGS := -Wall -Werror -std=c++17
CFLAGS += $(shell pkg-config --cflags libevdev)
LDFLAGS += $(shell pkg-config --libs libevdev libgpiod)
CC := g++

default: stepper dual_stepper xbox

stepper: bgt_tmc2209.cpp main.cpp
	$(CC) $(CFLAGS) $^ -o $@ -o stepper $(LDFLAGS)

dual_stepper: StepperController.cpp XboxController.cpp bgt_tmc2209.cpp dual_stepper.cpp 
	$(CC) $(CFLAGS)  $^ -o $@ $(LDFLAGS)

libgpiod.so: libgpiod_stub.c
	$(CC) $(CFLAGS) -fPIC -shared -o libgpiod.so libgpiod_stub.c $(LDFLAGS)

xbox_dummy: XboxController.cpp
	$(CC) $(CFLAGS) -DMOCK_EVDEV $^ -o $@ $(LDFLAGS)

xbox: XboxController.cpp
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f libgpiod.so stepper dual_stepper xbox