#include "game_function.h"
#include "goldelox_function.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


/* ======================= INITIALIZATION ======================= */

void game_init(GameData* game, int oled_fd) {
    memset(game, 0, sizeof(GameData));
    
    game->current_state = STATE_STARTSCREEN;
    game->player_car.x = SCREEN_WIDTH / 2;
    game->player_car.y = CAR_Y_POS;
    game->player_car.width = CAR_WIDTH;
    game->player_car.height = CAR_HEIGHT;
    game->player_car.lane = 1;  // Start in center lane
    
    game->score = 0;
    game->distance = 0;
    game->obstacle_spawn_counter = 0;
    game->game_over = 0;
    
    // Clear screen
    goldelox_clear_screen(oled_fd);
    usleep(500000);
}

void game_reset(GameData* game) {
    game->score = 0;
    game->distance = 0;
    game->obstacle_spawn_counter = 0;
    game->game_over = 0;
    game->player_car.lane = 1;  // Center lane
    
    // Clear all obstacles
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        game->obstacles[i].active = 0;
    }
    
    game->game_start_time = time(NULL);
}

/* ======================= RENDERING FUNCTIONS ======================= */

void render_startscreen(int oled_fd, GameData* game) {
    goldelox_clear_screen(oled_fd);
    
    // Draw title
    // goldelox_text_foreground_color(oled_fd, COLOR_WHITE);
    // goldelox_put_string(oled_fd, "RACER");
    
    // Draw instructions
    // goldelox_move_cursor(oled_fd, 0, 16);
    // goldelox_text_foreground_color(oled_fd, COLOR_RED);
    // goldelox_put_string(oled_fd, "LEFT/RIGHT BUTTONS");
    // goldelox_put_string(oled_fd, "to move car");
    
    // goldelox_put_string(oled_fd, "Avoid obstacles!");
    
    // Display high score
    // char score_text[50];
    // sprintf(score_text, "High Score: %u", game->high_score);
    // goldelox_put_string(oled_fd, score_text);
    
    // Start instruction
    // goldelox_put_string(oled_fd, "PRESS SELECT");
    // goldelox_put_string(oled_fd, "to start");
}

void game_update(GameData* game, ButtonState* buttons) {
    switch(game->current_state) {
        case STATE_STARTSCREEN:
            if (buttons->select_pressed) {
                game_reset(game);
                game->current_state = STATE_PLAYING;
            }
            break;
            
        case STATE_PLAYING:
            // update_car_position(&game->player_car, buttons);
            // update_obstacles(game);
            // spawn_obstacle(game);
            // check_collisions(game);
            
            if (game->game_over) {
                game->current_state = STATE_HIGHSCORE;
            }
            break;
            
        case STATE_HIGHSCORE:
        case STATE_GAMEOVER:
            if (buttons->select_pressed) {
                game->current_state = STATE_STARTSCREEN;
            }
            break;
            
        default:
            break;
    }
}

void game_render(int oled_fd, GameData* game) {
    switch(game->current_state) {
        case STATE_STARTSCREEN:
            render_startscreen(oled_fd, game);
            break;
            
        case STATE_PLAYING:
            // render_playing(oled_fd, game);
            break;
            
        case STATE_HIGHSCORE:
        case STATE_GAMEOVER:
            // render_highscore(oled_fd, game);
            break;
            
        default:
            break;
    }
}