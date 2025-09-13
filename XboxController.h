#pragma once

#include <atomic>
#include <deque>
#include <mutex>
#include <iostream>
#include <optional>
#include <thread>


#ifndef MOCK_EVDEV
#include <libevdev/libevdev.h>

#else
#define LIBEVDEV_READ_FLAG_NORMAL 0 
#define LIBEVDEV_READ_STATUS_SYNC ENOTSUP
#define ABS_X 0
#define ABS_Y 1
#define ABS_RX 2
#define ABS_RY 3
#define EV_KEY 4
#define EV_ABS 5

typedef struct {
    int fd;
} libevdev;

typedef struct {
    int type;
    int value;
    int code;
} input_event;



libevdev dummy;
__attribute__((weak)) int libevdev_new_from_fd(int, libevdev** dev) { *dev = &dummy ; return 0; }
__attribute__((weak)) const char* libevdev_get_name(libevdev*) { return "dummy"; }
__attribute__((weak)) void libevdev_free(libevdev*) {}
__attribute__((weak)) int libevdev_next_event(libevdev*, int, input_event*) { return 0; }

#endif

struct XboxEvent {
    XboxEvent(int t, int b, float x, float y) { type = t; button = b; xpos = x; ypos = y; }
    enum {
        Button,
        LeftAxis,
        RightAxis
    };
    int type;
    int button;
    float xpos;
    float ypos;
};

class XboxController {
public:
    XboxController(const std::string& devicePath);
    virtual ~XboxController();

    void startEventThread();
    std::optional<XboxEvent> getNextEvent();

private:
    std::thread mEventThread;
    std::atomic<bool> mRunning{true};
    std::deque<XboxEvent> mEventQueue;
    std::mutex mEventLocker;

    // EvDev device information
    const std::string& mDevicePath;
    libevdev* mDevice = nullptr;
    int mFd;
};

