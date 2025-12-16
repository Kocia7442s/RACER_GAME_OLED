#include "main.h"

/* ======================= TEXT & STRING COMMANDS ======================= */
void goldelox_move_cursor(int fd, uint8_t col, uint8_t row);

void goldelox_put_character(int fd, char character);

void goldelox_put_string(int fd, const char* text);

void goldelox_text_foreground_color(int fd, uint16_t color);

void goldelox_text_width(int fd, uint16_t multiplier);







/* ======================= GRAPHIC COMMANDS ======================= */
void goldelox_clear_screen(int fd);

void goldelox_change_colour(int fd, uint16_t oldColour, uint16_t newColour);

void goldelox_draw_circle(int fd, uint16_t x, uint16_t y, uint16_t rad, uint16_t color);

void goldelox_draw_filled_circle(int fd, uint16_t x, uint16_t y, uint16_t rad, uint16_t color);

void goldelox_draw_line(int fd, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);

void goldelox_draw_rectangle(int fd, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);

void goldelox_draw_filled_rectangle(int fd, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);

void goldelox_draw_polyline(int fd, const uint16_t* xPoints, const uint16_t* yPoints, size_t numPoints, uint16_t color);

void goldelox_draw_polygon(int fd, const uint16_t* xPoints, const uint16_t* yPoints, size_t numPoints, uint16_t color);

void goldelox_draw_triangle(int fd, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t color);

int goldelox_calculate_orbit(int fd, uint16_t angle, uint16_t distance, uint16_t* x, uint16_t* y);

void goldelox_put_pixel(int fd, uint16_t x, uint16_t y, uint16_t color);

void goldelox_read_pixel(int fd, uint16_t x, uint16_t y, uint16_t* color);

void goldelox_move_origin(int fd, int16_t xOffset, int16_t yOffset);

void goldelox_draw_line_and_move_origin(int fd, uint16_t xpos, uint16_t ypos);

void goldelox_clipping(int fd, uint8_t value);

void goldelox_set_clip_window(int fd, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

void goldelox_extend_clip_region(int fd);

void goldelox_background_color(int fd, uint16_t color);



/* ======================= SOUND & TUNE COMMANDS ======================= */
void goldelox_beep(int fd, uint16_t frequency, uint16_t duration);

/* ======================= SERIAL COMMUNICATIONS COMMANDS ======================= */
void goldelox_set_baudrate(int fd, uint32_t baudrate);