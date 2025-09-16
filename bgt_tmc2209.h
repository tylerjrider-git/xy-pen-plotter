#ifndef BGT_TMC2209
#define BGT_TMC2209

#include <stdbool.h>

#define TMC2209_MAX_SPEED 7.0
#define TMC2209_MIN_SPEED 0.01
#define TMC2209_STEP_TMIN_NS (100)

constexpr bool CCW = true;
constexpr bool CW = false;

struct tmc2209_handle;

struct tmc2209_handle* tmc2209_create();

void tmc2209_destroy(struct tmc2209_handle** handle);

int tmc2209_init(struct tmc2209_handle* handle, int n_step /* which stepper*/);

void tmc2209_teardown(struct tmc2209_handle* handle);

void tmc2209_enable(struct tmc2209_handle* handle, bool on);

void tmc2209_setdir(struct tmc2209_handle* handle, bool direction);

int tmc2209_step(struct tmc2209_handle* handle, unsigned int nstep, float rps);

int tmc2209_angle_step(struct tmc2209_handle* handle, double degrees, float speed);

double tmc2209_degrees_to_steps(double degrees);

#endif