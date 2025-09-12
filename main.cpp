#include "bgt_tcm2209.h"
#include <stdio.h>
#include <string.h> // strlen
#include <stdlib.h> // strtod


typedef struct {
    double angle_step;
    bool direction;
    unsigned int num_steps;
    bool interactive;
    double speed;
    bool continuous;
} Args;


Args parse_args(int argc, char **argv) {
   Args args = {0};
    args.speed  = 1.0;

    for (int i = 1; i < argc; i++) {
        const char* arg = argv[i];

        if (i+1 < argc) {
            if (strncmp(arg, "--degrees", strlen("--degrees")) == 0) {
                args.angle_step = strtod(argv[i+1], NULL);
                continue;
            } else if (strncmp(arg, "--direction", strlen("--direction")) == 0) {
                args.direction = strncmp(argv[i+1], "clockwise", strlen("clockwise")) == 0;
                continue;
            } else if (strncmp(arg, "--steps", strlen("--steps")) == 0) {
                args.num_steps = atoi(argv[i+1]);
                continue;
            } else if (strncmp(arg, "--speed", strlen("--speed")) == 0) {
                args.speed = strtod(argv[i+1], NULL);
                if (args.speed > TCM2209_MAX_SPEED) {
                    args.speed = TCM2209_MAX_SPEED;
                }
                continue;
            }
        }
        if (strncmp(arg, "-i", strlen("-i")) == 0) {
            args.interactive = true;
        } else if (strncmp(arg, "-c", strlen("-c")) == 0) {
            args.continuous = true;
        }
    }
    printf("Args:\n");
    printf("Angle: %.02lf Direction: %s, Steps: %u Interactive: %s Continuous : %s Speed: %.02lf\n",
        args.angle_step, args.direction ? "clockwise" : "counterclockwise", args.num_steps,
        args.interactive ? "yes" : "no", args.continuous ? "yes" : "no", args.speed);
    return args;
}


bool process_control_key() {
    getchar(); // skip '['
    char c = getchar();
    printf("Got char : %d, %c\n", (int) c,c );
    return c == 'C';
}

void run(Args args, struct tcm2209_handle* handle)
{
    tcm2209_enable(handle, true);
    if (args.interactive) {
        while(1) {
            char c = getchar();
            if (c == 10)
                continue;
            if (c == '\033') {
                bool dir = process_control_key();
                tcm2209_setdir(handle, dir);
                tcm2209_angle_step(handle, 360, 1);
            } else {
                printf("Unrecoginized control key %d\n", (int)c);
                continue;
            }
        }
    } else if (args.continuous) {
        tcm2209_setdir(handle, args.direction);
        while(1) {
            tcm2209_angle_step(handle, 360, args.speed);
        }
    } else {
        tcm2209_setdir(handle, args.direction);
        if (args.angle_step != 0)
            tcm2209_angle_step(handle, args.angle_step, args.speed);
        else if (args.num_steps != 0)
            tcm2209_step(handle, args.num_steps, args.speed);

    } 
     tcm2209_enable(handle, false);
}


int main(int argc, char ** argv)
{
    int rc;
    struct tcm2209_handle* handle = (struct tcm2209_handle*)
        ::malloc(sizeof (struct tcm2209_handle));

    rc = tcm2209_init(handle);
    if (rc) {
        fprintf(stderr, "Failed to init\n");
        return -1;
    }

    run(parse_args(argc, argv), handle);
    tcm2209_teardown(handle);
    return 0;
}