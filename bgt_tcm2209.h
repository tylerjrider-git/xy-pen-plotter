#ifndef BGT_TCM2209
#define BGT_TCM2209

#include <stdbool.h>
#include <gpiod.h>

#define TCM2209_MAX_SPEED 5.0
#define TCM2209_STEP_TMIN_NS (100)

constexpr bool CCW = true;
constexpr bool CW = false;

struct tcm2209_pins {
    struct gpiod_chip* chip;
    struct gpiod_line* enb;
    struct gpiod_line* dir;
    struct gpiod_line* step;
};

struct tcm2209_handle {
    int nStep;
    struct tcm2209_pins pins;
};

int tcm2209_init(struct tcm2209_handle* handle);

void tcm2209_teardown(struct tcm2209_handle* handle);

void tcm2209_enable(struct tcm2209_handle* handle, bool on);

void tcm2209_setdir(struct tcm2209_handle* handle, bool direction);

void tcm2209_step(struct tcm2209_handle* handle, int nstep, float rps);

void tcm2209_angle_step(struct tcm2209_handle* handle, double degrees, float speed);


#endif