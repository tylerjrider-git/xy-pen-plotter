#ifndef BGT_TMC2209
#define BGT_TMC2209

#include <stdbool.h>
#include <gpiod.h>

#define TMC2209_MAX_SPEED 5.0
#define TMC2209_STEP_TMIN_NS (100)

constexpr bool CCW = true;
constexpr bool CW = false;

struct tmc2209_pins {
    struct gpiod_chip* chip;
    struct gpiod_line* enb;
    struct gpiod_line* dir;
    struct gpiod_line* step;
};

struct tmc2209_handle {
    int nStep;
    struct tmc2209_pins pins;
};

int tmc2209_init(struct tmc2209_handle* handle);

void tmc2209_teardown(struct tmc2209_handle* handle);

void tmc2209_enable(struct tmc2209_handle* handle, bool on);

void tmc2209_setdir(struct tmc2209_handle* handle, bool direction);

void tmc2209_step(struct tmc2209_handle* handle, int nstep, float rps);

void tmc2209_angle_step(struct tmc2209_handle* handle, double degrees, float speed);


#endif