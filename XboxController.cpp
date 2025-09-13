#include "XboxController.h"
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

XboxController::XboxController(const std::string& devicePath) :
    mPrevX{0,0},
    mPrevY{0,0},
    mDevicePath(devicePath)

{
    mFd = ::open(mDevicePath.c_str(), O_RDONLY | O_NONBLOCK);
    if (mFd < 0) {
        std::cerr << "Failed to open " << mDevicePath << std::endl;
        return;
    }

    if (::libevdev_new_from_fd(mFd, &mDevice) < 0) {
        std::cerr << "Failed to init libevdev" << std::endl;
        ::close(mFd);
        return;
    }
    std::cout << "Input device name: " << libevdev_get_name(mDevice) << std::endl;
}

XboxController::~XboxController()
{
    mRunning = false;
    mEventThread.join();
    if (mDevice)
        ::libevdev_free(mDevice);
    if (mFd >= 0)
        ::close(mFd);
}

static void printEvent(input_event& ev)
{
    if (ev.type == EV_KEY) {
        std::cout << "Button " << ev.code << (ev.value ? " pressed" : " released") << std::endl;
    } else if (ev.type == EV_ABS) {
        if (ev.code == ABS_X) {
            std::cout << "Left Stick X (" << ev.code << "): " << ev.value << std::endl;
        } else if (ev.code == ABS_Y) {
            std::cout << "Left Stick Y (" << ev.code << "): " << ev.value << std::endl;
        } else if (ev.code == ABS_RX) {
            std::cout << "Right Stick X (" << ev.code  << "): " << ev.value << std::endl;
         }else if (ev.code == ABS_RY) {
            std::cout << "Right Stick Y (" << ev.code  << "): " << ev.value << std::endl;
        }
    }else {
        // std::cerr << "Unkonwn code: " << ev.code << std::endl;
    }
}

void XboxController::startEventThread(const bool blocking)
{
    mEventThread = std::thread( [this, blocking] (){
        input_event ev;
        unsigned int flags = LIBEVDEV_READ_FLAG_NORMAL;
        if (blocking) flags |= LIBEVDEV_READ_FLAG_BLOCKING;

        while (mRunning) {
            int rc = ::libevdev_next_event(mDevice, flags, &ev);
            if (rc == 0) {
                if (0) { printEvent(ev); }
                std::lock_guard<std::mutex> lg(mEventLocker);
                switch(ev.type) {
                    case EV_KEY:
                    // Only process released
                    if (ev.value == 0) {
                        mEventQueue.emplace_back(XboxEvent::Button, ev.code, 0, 0);
                    }
                        break;
                    case EV_ABS:
                    {
                        const int axis = (ev.code == ABS_X || ev.code == ABS_Y) ? XboxEvent::LeftAxis : XboxEvent::RightAxis;
                        const bool isX = (ev.code == ABS_X || ev.code == ABS_RX);
                        if (isX) {
                            mEventQueue.emplace_back(axis, 0, ev.value, mPrevY[axis]);
                            mPrevX[axis] = ev.value;
                        } else {
                            mEventQueue.emplace_back(axis, 0, mPrevX[axis], ev.value);
                            mPrevY[axis] = ev.value;   
                        }
                    }
                        break;
                    default:
                        break;
                }
            //TODO LIBEVDEV_READ_STATUS_SUCCESS
            } else if (rc == LIBEVDEV_READ_STATUS_SYNC) {
                std::fprintf(stderr, "Falling behind processing events\n");
            } else if (rc != -EAGAIN) {
                std::fprintf(stderr, "Failed to read event rc=%d(%s)\n", rc, strerror(rc));
                break; // error or device unplugged
            }
            usleep(1000); // small sleep to avoid busy-wait
        }
    });
}

std::optional<XboxEvent> XboxController::getNextEvent()
{
    std::lock_guard<std::mutex> lg(mEventLocker);
    if (mEventQueue.empty() == false) {
        std::optional<XboxEvent> ret = mEventQueue.front();
        mEventQueue.pop_front();
        return ret;
    }
    return std::nullopt;
}

int __attribute__((weak))  main(int argc, char** argv) {

    if (argc > 1) {
        XboxController xbox(argv[1]);
        xbox.startEventThread();
        while(true) {
            auto ret = xbox.getNextEvent();
            if (ret) {
                fprintf(stderr, "Event: %d\n", ret.value().type);
            }
            usleep(1000);
        }
    } else {
        std::cerr << "Need device path" << std::endl;
    }
    return 0;
}