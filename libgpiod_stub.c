#include <stdio.h>
#include <stdlib.h>

struct gpiod_chip { const char* p;};
struct gpiod_line { int n;};

struct gpiod_chip* gpiod_chip_open(const char *path) {
    fprintf(stderr, "STUB: gpiod_chip_open(%s)\n", path);
    return (struct gpiod_chip*)malloc(sizeof(struct gpiod_chip));
}

void gpiod_chip_close(struct gpiod_chip *chip)
{
    fprintf(stderr, "STUB: gpiod_chip_close()\n");
    if (chip)
        free(chip);
}


int gpiod_line_set_value(struct gpiod_line *line, int value)
{
    fprintf(stderr, "STUB: %s pin:%d val: %d\n", __func__, line->n, value);
    return 0;
}

struct gpiod_line* gpiod_chip_get_line(struct gpiod_chip*, int n )
{
    struct gpiod_line* p =
        (struct gpiod_line*) malloc(sizeof(struct gpiod_line));
    p->n = n;
    return p; 
}

int gpiod_line_request_output(struct gpiod_line*, const char*, int)
{
    return 0;
}

void gpiod_line_release(struct gpiod_line* line)
{
    if (line)
        free(line);
}