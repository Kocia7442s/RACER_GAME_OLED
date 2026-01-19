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
    game->player_car.lane = 1;  // Commencer dans la ligne centrale
    
    game->score = 0;
    game->distance = 0;
    game->obstacle_spawn_counter = 0;
    game->game_over = 0;
    game->screen_needs_refresh = 1;

    usleep(500000);
}

void game_reset(GameData* game) {
    game->score = 0;
    game->distance = 0;
    game->obstacle_spawn_counter = 0;
    game->game_over = 0;
    game->player_car.lane = 1;
    
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        game->obstacles[i].active = 0;
    }
    
    game->game_start_time = time(NULL);
}

/* ======================= RENDERING FUNCTIONS ======================= */

void render_startscreen(int oled_fd, GameData* game) {
    goldelox_clear_screen(oled_fd);
    
    // Draw title
    goldelox_move_cursor(oled_fd, 7, 1);
    goldelox_text_foreground_color(oled_fd, COLOR_WHITE);
    goldelox_put_string(oled_fd, "RACER");
    
    // Draw instructions
    goldelox_move_cursor(oled_fd, 6, 3);
    goldelox_text_foreground_color(oled_fd, COLOR_RED);
    goldelox_put_string(oled_fd, "BOUTONS");
    goldelox_move_cursor(oled_fd, 3, 4);
    goldelox_put_string(oled_fd, "GAUCHE/DROITE");
    goldelox_move_cursor(oled_fd, 4, 5);
    goldelox_put_string(oled_fd, "Pour bouger");
    goldelox_move_cursor(oled_fd, 4, 6);
    goldelox_put_string(oled_fd, "la voiture");
    
    goldelox_text_foreground_color(oled_fd, COLOR_GREEN);
    goldelox_move_cursor(oled_fd, 6, 8);
    goldelox_put_string(oled_fd, "Eviter");
    goldelox_move_cursor(oled_fd, 3, 9);
    goldelox_put_string(oled_fd, "les obstacles!");
    
    // Display high score
    char score_text[50];
    sprintf(score_text, "High Score: %u", game->high_score);
    goldelox_text_foreground_color(oled_fd, COLOR_BLUE);
    goldelox_move_cursor(oled_fd, 0, 11);
    goldelox_put_string(oled_fd, score_text);
    
    // Start instruction
    goldelox_text_foreground_color(oled_fd, COLOR_DARK_GREEN);
    goldelox_move_cursor(oled_fd, 1, 13);
    goldelox_put_string(oled_fd, "Appuye sur SELECT");
    goldelox_move_cursor(oled_fd, 2, 14);
    goldelox_put_string(oled_fd, "pour commencer");
}

void render_playing(int oled_fd, GameData* game) {
    goldelox_clear_screen(oled_fd);
    usleep(100000);
    
    // Dessine la route
    draw_road(oled_fd);
    
    // Dessine les lignes de vitesse (effet visuel)
    draw_speed_lines(oled_fd);

    // Dessine les obstacles
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (game->obstacles[i].active) {
            draw_obstacle(oled_fd, &game->obstacles[i]);
        }
    }

    // Dessine la voiture
    draw_car(oled_fd, &game->player_car);

    // Dessine le score et la distance en haut
    draw_score(oled_fd, game);
}

void render_highscore(int oled_fd, GameData* game) {
    goldelox_clear_screen(oled_fd);
    usleep(200000);
    
    goldelox_move_cursor(oled_fd, 4, 3);
    goldelox_text_foreground_color(oled_fd, COLOR_WHITE);
    goldelox_put_string(oled_fd, "GAME OVER !");
    
    char score_text[50];
    sprintf(score_text, "Score: %u", game->score);
    goldelox_move_cursor(oled_fd, 6, 6);
    goldelox_text_foreground_color(oled_fd, COLOR_YELLOW);
    goldelox_put_string(oled_fd, score_text);
    
    sprintf(score_text, "Distance: %u", game->distance);
    goldelox_move_cursor(oled_fd, 4, 8);
    goldelox_text_foreground_color(oled_fd, COLOR_YELLOW);
    goldelox_put_string(oled_fd, score_text);
    
    sprintf(score_text, "High: %u", game->high_score);
    goldelox_move_cursor(oled_fd, 7, 11);
    goldelox_text_foreground_color(oled_fd, COLOR_GREEN);
    goldelox_put_string(oled_fd, score_text);
    
    goldelox_move_cursor(oled_fd, 4, 14);
    goldelox_text_foreground_color(oled_fd, COLOR_CYAN);
    goldelox_put_string(oled_fd, "Press SELECT");
    goldelox_move_cursor(oled_fd, 6, 15);
    goldelox_text_foreground_color(oled_fd, COLOR_CYAN);
    goldelox_put_string(oled_fd, "for menu");
}

void render_gameover(int oled_fd, GameData* game) {
    render_highscore(oled_fd, game);
}

/* ======================= DRAWING HELPERS ======================= */

void draw_car(int oled_fd, Car* car) {
    // Simple car: filled rectangle
    uint16_t x1, x2, y1, y2;
    
    switch(car->lane) {
        case 0:
            x1 = ROAD_X_START + 5;
            break;
        case 1:
            x1 = SCREEN_WIDTH / 2 - CAR_WIDTH / 2;
            break;
        case 2:
            x1 = ROAD_X_START + ROAD_WIDTH - CAR_WIDTH - 5;
            break;
        default:
            x1 = SCREEN_WIDTH / 2 - CAR_WIDTH / 2;
    }
    
    x2 = x1 + CAR_WIDTH;
    y1 = car->y;
    y2 = car->y + CAR_HEIGHT;
    
    goldelox_draw_filled_rectangle(oled_fd, x1, y1, x2, y2, COLOR_RED);
    
    goldelox_draw_line(oled_fd, x1 + 4, y1 + 3, x1 + 12, y1 + 3, COLOR_YELLOW);
}

void draw_road(int oled_fd) {
    uint16_t road_x1 = ROAD_X_START;
    uint16_t road_x2 = ROAD_X_START + ROAD_WIDTH;
    
    goldelox_draw_rectangle(oled_fd, road_x1, 0, road_x2, SCREEN_HEIGHT, COLOR_WHITE);
    
    for (uint16_t y = 0; y < SCREEN_HEIGHT; y += 10) {
        goldelox_draw_line(oled_fd, SCREEN_WIDTH / 2, y, SCREEN_WIDTH / 2, y + 5, COLOR_YELLOW);
    }
    
    uint16_t lane_divider1 = ROAD_X_START + ROAD_WIDTH / 3;
    uint16_t lane_divider2 = ROAD_X_START + 2 * ROAD_WIDTH / 3;
    
    goldelox_draw_line(oled_fd, lane_divider1, 0, lane_divider1, SCREEN_HEIGHT, COLOR_GRAY);
    goldelox_draw_line(oled_fd, lane_divider2, 0, lane_divider2, SCREEN_HEIGHT, COLOR_GRAY);
}

void draw_obstacle(int oled_fd, Obstacle* obs) {
    goldelox_draw_filled_rectangle(oled_fd, obs->x, obs->y, obs->x + obs->width, obs->y + obs->height, COLOR_MAGENTA);
    
    goldelox_draw_line(oled_fd, obs->x + 4, obs->y + 3, obs->x + 12, obs->y + 3, COLOR_WHITE);
}

void draw_score(int oled_fd, GameData* game) {
    char score_text[30];
    
    sprintf(score_text, "Score:%u", game->score);
    goldelox_move_cursor(oled_fd, 0, 0);
    goldelox_text_foreground_color(oled_fd, COLOR_YELLOW);
    goldelox_put_string(oled_fd, score_text);
    
    sprintf(score_text, "Dist:%u", game->distance);
    goldelox_move_cursor(oled_fd, 0, 2);
    goldelox_text_foreground_color(oled_fd, COLOR_GREEN);
    goldelox_put_string(oled_fd, score_text);
}

void draw_speed_lines(int oled_fd) {
    static uint16_t offset = 0;
    offset = (offset + ROAD_SCROLL_SPEED) % 20;
    
    for (uint16_t y = offset; y < SCREEN_HEIGHT; y += 20) {
        goldelox_draw_line(oled_fd, ROAD_X_START + 10, y, ROAD_X_START + 15, y + 3, COLOR_DARK_GREEN);
        goldelox_draw_line(oled_fd, ROAD_X_START + ROAD_WIDTH - 15, y, ROAD_X_START + ROAD_WIDTH - 10, y + 3, COLOR_DARK_GREEN);
    }
}

void update_car_position(Car* car, ButtonState* buttons) {
    if (buttons->left_pressed && car->lane > 0) {
        car->lane--;
    }
    if (buttons->right_pressed && car->lane < 2) {
        car->lane++;
    }
}

void update_obstacles(GameData* game) {
    // Move all obstacles down (scrolling effect)
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (game->obstacles[i].active) {
            game->obstacles[i].y += ROAD_SCROLL_SPEED;
            
            // Deactivate if off screen
            if (game->obstacles[i].y > SCREEN_HEIGHT) {
                game->obstacles[i].active = 0;
                game->score += 10;
                game->distance += 5;
            }
        }
    }
}

void spawn_obstacle(GameData* game) {
    game->obstacle_spawn_counter++;
    
    if (game->obstacle_spawn_counter > 30) {
        game->obstacle_spawn_counter = 0;
        
        for (int i = 0; i < MAX_OBSTACLES; i++) {
            if (!game->obstacles[i].active) {
                uint8_t lane = rand() % 3;
                uint16_t x;
                
                switch(lane) {
                    case 0:
                        x = ROAD_X_START + 5;
                        break;
                    case 1:
                        x = SCREEN_WIDTH / 2 - OBSTACLE_WIDTH / 2;
                        break;
                    case 2:
                        x = ROAD_X_START + ROAD_WIDTH - OBSTACLE_WIDTH - 5;
                        break;
                    default:
                        x = SCREEN_WIDTH / 2 - OBSTACLE_WIDTH / 2;
                }
                
                game->obstacles[i].x = x;
                game->obstacles[i].y = 0;
                game->obstacles[i].width = OBSTACLE_WIDTH;
                game->obstacles[i].height = OBSTACLE_HEIGHT;
                game->obstacles[i].active = 1;
                break;
            }
        }
    }
}

uint8_t is_collision(Car* car, Obstacle* obs) {
    uint16_t car_x1, car_x2, car_y1, car_y2;
    
    switch(car->lane) {
        case 0:
            car_x1 = ROAD_X_START + 5;
            break;
        case 1:
            car_x1 = SCREEN_WIDTH / 2 - CAR_WIDTH / 2;
            break;
        case 2:
            car_x1 = ROAD_X_START + ROAD_WIDTH - CAR_WIDTH - 5;
            break;
        default:
            car_x1 = SCREEN_WIDTH / 2 - CAR_WIDTH / 2;
    }
    
    car_x2 = car_x1 + CAR_WIDTH;
    car_y1 = car->y;
    car_y2 = car->y + CAR_HEIGHT;
    
    return !(car_x2 < obs->x || car_x1 > obs->x + obs->width ||
             car_y2 < obs->y || car_y1 > obs->y + obs->height);
}

void check_collisions(GameData* game) {
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (game->obstacles[i].active) {
            if (is_collision(&game->player_car, &game->obstacles[i])) {
                game->game_over = 1;
                
                if (game->score > game->high_score) {
                    game->high_score = game->score;
                }
                break;
            }
        }
    }
}

void game_update(GameData* game, ButtonState* buttons) {
    switch(game->current_state) {
        case STATE_STARTSCREEN:
            if (buttons->select_pressed) {
                game_reset(game);
                game->current_state = STATE_PLAYING;
                game->screen_needs_refresh = 1;
            }
            break;
            
        case STATE_PLAYING:
            update_car_position(&game->player_car, buttons);
            update_obstacles(game);
            spawn_obstacle(game);
            check_collisions(game);
            
            if (game->game_over) {
                game->current_state = STATE_HIGHSCORE;
                game->screen_needs_refresh = 1;
            }
            break;
            
        case STATE_HIGHSCORE:
        case STATE_GAMEOVER:
            if (buttons->select_pressed) {
                game->current_state = STATE_STARTSCREEN;
                game->screen_needs_refresh = 1;
            }
            break;
            
        default:
            break;
    }
}

void game_render(int oled_fd, GameData* game) {
    switch(game->current_state) {
        case STATE_STARTSCREEN:
            if (game->screen_needs_refresh) {
                render_startscreen(oled_fd, game);
                game->screen_needs_refresh = 0;
            }
            break;
            
        case STATE_PLAYING:
            render_playing(oled_fd, game);
            break;
            
        case STATE_HIGHSCORE:
        case STATE_GAMEOVER:
            if (game->screen_needs_refresh) {
                render_highscore(oled_fd, game);
                game->screen_needs_refresh = 0;
            }
            break;
            
        default:
            break;
    }
}