#ifndef GAME_FUNCTION_H
#define GAME_FUNCTION_H

#include <stdint.h>
#include <time.h>


/* ======================= GAME CONSTANTS ======================= */
#define SCREEN_WIDTH    160
#define SCREEN_HEIGHT   128

#define ROAD_WIDTH      80
#define ROAD_X_START    ((SCREEN_WIDTH - ROAD_WIDTH) / 2)

#define CAR_WIDTH       16
#define CAR_HEIGHT      12
#define CAR_Y_POS       (SCREEN_HEIGHT - 20)

#define MAX_OBSTACLES   10
#define OBSTACLE_WIDTH  16
#define OBSTACLE_HEIGHT 12

#define GAME_SPEED      30  // milliseconds per frame
#define ROAD_SCROLL_SPEED 2 // pixels per frame

/* ======================= COLOR DEFINITIONS (RGB565) ======================= */
#define COLOR_BLACK     0x0000
#define COLOR_WHITE     0xFFFF
#define COLOR_RED       0xF800
#define COLOR_GREEN     0x07E0
#define COLOR_BLUE      0x001F
#define COLOR_YELLOW    0xFFE0
#define COLOR_CYAN      0x07FF
#define COLOR_MAGENTA   0xF81F
#define COLOR_GRAY      0x8410
#define COLOR_DARK_GREEN 0x0400

/* ======================= GAME STATE ENUM ======================= */
typedef enum {
    STATE_STARTSCREEN,
    STATE_PLAYING,
    STATE_HIGHSCORE,
    STATE_GAMEOVER
} GameState;

/* ======================= DATA STRUCTURES ======================= */

/* Player car structure */
typedef struct {
    uint16_t x;             // X position (center)
    uint16_t y;             // Y position (should be CAR_Y_POS)
    uint16_t width;
    uint16_t height;
    uint16_t lane;          // 0 = left, 1 = center, 2 = right
} Car;

/* Road obstacle structure */
typedef struct {
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    uint8_t active;         // 1 = visible, 0 = off screen
} Obstacle;

/* Game state structure */
typedef struct {
    GameState current_state;
    Car player_car;
    Obstacle obstacles[MAX_OBSTACLES];
    uint32_t score;
    uint32_t high_score;
    uint32_t distance;
    uint16_t obstacle_spawn_counter;
    time_t game_start_time;
    uint8_t game_over;
    uint8_t screen_needs_refresh;
} GameData;

/* ======================= BUTTON INPUTS ======================= */
typedef struct {
    uint8_t left_pressed;
    uint8_t right_pressed;
    uint8_t select_pressed;
} ButtonState;

/* ======================= FUNCTION DECLARATIONS ======================= */

/* Game initialization */
void game_init(GameData* game, int oled_fd);
void game_reset(GameData* game);

/* Game state management */
void game_update(GameData* game, ButtonState* buttons);
void game_render(int oled_fd, GameData* game);

/* Screen rendering functions */
void render_startscreen(int oled_fd, GameData* game);
void render_playing(int oled_fd, GameData* game);
void render_highscore(int oled_fd, GameData* game);
void render_gameover(int oled_fd, GameData* game);

/* Game logic functions */
void update_car_position(Car* car, ButtonState* buttons);
void update_obstacles(GameData* game);
void spawn_obstacle(GameData* game);
void check_collisions(GameData* game);
uint8_t is_collision(Car* car, Obstacle* obs);

/* Drawing helper functions */
void draw_car(int oled_fd, Car* car);
void draw_road(int oled_fd);
void draw_obstacle(int oled_fd, Obstacle* obs);
void draw_score(int oled_fd, GameData* game);
void draw_speed_lines(int oled_fd);

/* Score management */
void load_high_score(GameData* game);
void save_high_score(GameData* game);

/* Input handling */
void read_buttons(ButtonState* buttons);

#endif // PIXEL_RACER_H
