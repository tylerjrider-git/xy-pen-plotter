#include "bgt_tmc2209.h"
#include <gpiod.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h> // sleep



static const float PI = 3.14159;
static const int STEPS_PER_DEGREE = 256;
static const int MICROSTEP_SETTING = 8; //8x microsteps.


// TCM PINOUTS, platform specific.
#define TMC2209_CHIP_NAME "/dev/gpiochip0"
#define TMC2209_PIN_EN 4
#define TMC2209_PIN_DIR 27
#define TMC2209_PIN_STEP 17

struct pins {
    int en;
    int dir;
    int step;
};

struct pins get_stepper_pinout(int n) {
    if (n == 0) {
        return {TMC2209_PIN_EN, TMC2209_PIN_DIR,TMC2209_PIN_STEP };
    } else {
        //TODO
        return {TMC2209_PIN_EN, TMC2209_PIN_DIR,TMC2209_PIN_STEP };
    }
}

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

struct tmc2209_handle* tmc2209_create(void)
{
    return (struct tmc2209_handle*) ::malloc(sizeof(struct tmc2209_handle));
}

void tmc2209_destroy(struct tmc2209_handle** handle)
{
    if (handle) {
        free(*handle);
        *handle = NULL;
    }
}

int tmc2209_init(struct tmc2209_handle* handle, int n_stepper)
{
    const char* chipname = TMC2209_CHIP_NAME;
    struct pins pins = {0};
    int rc = 0;

    pins = get_stepper_pinout(handle->nStep);

    handle->nStep = n_stepper;
    handle->pins.chip = gpiod_chip_open(chipname);
    if (!handle->pins.chip) {
        fprintf(stderr, "Failed to open chip\n");
        return 1;
    }

    handle->pins.enb = gpiod_chip_get_line(handle->pins.chip, pins.en);
    if (!handle->pins.enb) {
        fprintf(stderr, "Failed to get enb pin\n");
        return 1;
    }

    // active low.
    if ((rc = gpiod_line_request_output(handle->pins.enb, "tmc2209-enb", 1))) {
        fprintf(stderr, "Failed to set output, rc:%d", rc);
        return 1;
    }

    handle->pins.dir = gpiod_chip_get_line(handle->pins.chip, pins.dir);
    if (!handle->pins.dir) {
        fprintf(stderr, "Failed to get dir pin\n");
        return 1;
    }

    if (gpiod_line_request_output(handle->pins.dir, "tmc2209-dir", 0)) {
        fprintf(stderr,"Failed to set dir 0");
        return 1;
    }

    handle->pins.step = gpiod_chip_get_line(handle->pins.chip, pins.step);
    if (!handle->pins.step) {
        fprintf(stderr, "Failed to get step pin\n");
        return 1;
    }

    if (gpiod_line_request_output(handle->pins.step, "tmc2209-step", 0)) {
        fprintf(stderr, "Failed to set step 0");
        return 0;
    }

    return 0;
}

void tmc2209_teardown(struct tmc2209_handle* handle)
{
    gpiod_line_release(handle->pins.step);
    gpiod_line_release(handle->pins.dir);
    gpiod_line_release(handle->pins.enb);
    gpiod_chip_close(handle->pins.chip);
}

void tmc2209_enable(struct tmc2209_handle* handle, bool on)
{
    if (on) {
        gpiod_line_set_value(handle->pins.enb, 0);
    } else {
        gpiod_line_set_value(handle->pins.enb, 1);
    }
}

void tmc2209_setdir(struct tmc2209_handle* handle, bool direction)
{
    if (!direction) {
        // Counter
       gpiod_line_set_value(handle->pins.dir, 0);
    } else {
        // Clockwise
        gpiod_line_set_value(handle->pins.dir, 1);
    }
}

#define DEGREE_PER_STEP ((double) 1.8f / MICROSTEP_SETTING)
#define STEPS_PER_REVOLUTION (360.f / DEGREE_PER_STEP)
#define USEC_PER_SEC (1e6)
#define MAX(A, B) ((A) > (B) ? A : B)

static const inline unsigned speed2udelay(float rps)
{
    unsigned int usec_per_step = USEC_PER_SEC / (rps * STEPS_PER_REVOLUTION);
    return MAX(usec_per_step, 2);
}

// rps: revolutions per second.
void tmc2209_step(struct tmc2209_handle* handle, int nstep, float rps)
{
    unsigned int usec_per_step = speed2udelay(rps); // RPS:1.0, nstep:~360deg => steps(360) /s.

    while (nstep-- > 0) {
       gpiod_line_set_value(handle->pins.step, 1);
       usleep(usec_per_step /2); 
       gpiod_line_set_value(handle->pins.step, 0);
       usleep(usec_per_step/2);
    }
}

void tmc2209_angle_step(struct tmc2209_handle* handle, double degrees, float speed)
{
    double num_steps = (degrees / (double)DEGREE_PER_STEP);
    printf("To get %lf degrees, need : %lf steps\n", degrees, num_steps);
    tmc2209_step(handle, num_steps, speed);
}

