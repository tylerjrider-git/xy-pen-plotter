
struct gpiod_line;
struct gpiod_chip* gpiod_chip_open(const char* );

void gpiod_chip_close(struct gpiod_chip*);

int gpiod_line_set_value(struct gpiod_line *line, int value);

struct gpiod_line* gpiod_chip_get_line(struct gpiod_chip*, int);

int gpiod_line_request_output(struct gpiod_line*, const char*, int);

void gpiod_line_release(struct gpiod_line*);