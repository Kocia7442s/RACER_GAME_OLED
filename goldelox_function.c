#include "goldelox_function.h"

static int _goldelox_send_and_wait_ack(int fd, uint8_t* packet, int length) {
    uint8_t response = 0;
    ssize_t bytes_read;

    // Vider le tampon d'entrée avant d'envoyer.
    // Cela supprime tous les vieux ACKs ou bruits qui traînent.
    tcflush(fd, TCIFLUSH);

    // Écriture du paquet
    if (write(fd, packet, length) != length) {
        perror("Erreur write");
        return -1;
    }

    tcdrain(fd);

    // Lecture de l'ACK
    bytes_read = read(fd, &response, 1);

    if (bytes_read > 0) {
        if (response == 0x06) {
            return 0;
        } else if (response == 0x15) {
            fprintf(stderr, "Erreur Goldelox: NAK reçu (0x15)\n");
            return -1;
        } else {
            fprintf(stderr, "Erreur Goldelox: Réponse inconnue (0x%02X)\n", response);
            return -1;
        }
    } else {
        fprintf(stderr, "Erreur Goldelox: Timeout ou pas de réponse\n");
        return -1;
    }
}

/* ======================= TEXT & STRING COMMANDS ======================= */
void goldelox_move_cursor(int fd, uint8_t col, uint8_t row) {
    uint8_t packet[6];

    packet[0] = 0xFF; 
    packet[1] = 0xE4;

    packet[2] = 0x00;
    packet[3] = row;

    packet[4] = 0x00;
    packet[5] = col;

    if(_goldelox_send_and_wait_ack(fd, packet, 6) == 0) {
        //printf("✓ Curseur déplacé (ACK reçu)\n");
    } else {
        printf("✗ Move cursor: pas d'ACK\n");
    }
}

void goldelox_put_character(int fd, char character) {
    uint8_t packet[3];
    packet[0] = 0xFF;
    packet[1] = 0xFE;
    packet[2] = (uint8_t)character;

    if(_goldelox_send_and_wait_ack(fd, packet, 3) == 0) {
        //printf("✓ Ecriture charactere (ACK reçu)\n");
    } else {
        printf("✗ Put character: pas d'ACK\n");
    }
}

void goldelox_put_string(int fd, const char* text) {
    int len = strlen(text);
    int packetSize = 2 + len + 1; 
    
    uint8_t* packet = (uint8_t*)malloc(packetSize);
    if (!packet) return;

    packet[0] = 0x00;
    packet[1] = 0x06;
    memcpy(&packet[2], text, len + 1); 
    
    if(_goldelox_send_and_wait_ack(fd, packet, packetSize) == 0) {
        // printf("✓ Chaîne '%s' envoyée (ACK reçu)\n", text);
    } else {
        printf("✗ Put String: pas d'ACK\n");
    }
    
    free(packet);
}

void goldelox_text_foreground_color(int fd, uint16_t color) {
    uint8_t packet[4] = {
        0xFF, 0x7F,
        color >> 8, color & 0xFF
    };

    if(_goldelox_send_and_wait_ack(fd, packet, 4) == 0) {
        // printf("✓ Couleur texte changée (ACK reçu)\n");
    } else {
        printf("✗ Foreground color: pas d'ACK\n");
    }
}

void goldelox_text_width(int fd, uint16_t multiplier){
    uint8_t packet[4];

    // Sécurité : la doc précise que la valeur doit être entre 1 et 16.
    if (multiplier < 1) multiplier = 1;
    if (multiplier > 16) multiplier = 16;

    // Commande 0xFF7C
    packet[0] = 0xFF;
    packet[1] = 0x7C;

    // Multiplier sur 16 bits (MSB d'abord)
    // Même si la valeur est petite, on respecte le format Word
    packet[2] = (multiplier >> 8) & 0xFF; // MSB
    packet[3] = multiplier & 0xFF;        // LSB

    if(_goldelox_send_and_wait_ack(fd, packet, 4) == 0) {
        // printf("✓ Largeur texte x%d définie (ACK reçu)\n", multiplier);
    } else {
        printf("✗ Text Width: pas d'ACK\n");
    }
}






/* ======================= GRAPHIC COMMANDS ======================= */
void goldelox_clear_screen(int fd) {
    uint8_t packet[2] = {0xFF, 0xD7};
    if(_goldelox_send_and_wait_ack(fd, packet, 2) == 0) {
        // printf("✓ Écran effacé (ACK reçu)\n");
    } else {
        printf("✗ Clear Screen: pas d'ACK\n");
    }
}

void goldelox_change_colour(int fd, uint16_t oldColor, uint16_t newColor) {
    uint8_t packet[6] = {
        0xFF, 0xBE,
        oldColor >> 8, oldColor & 0xFF,
        newColor >> 8, newColor & 0xFF
    };
    if(_goldelox_send_and_wait_ack(fd, packet, 3) == 0) {
        // printf("✓ Couleur changée (ACK reçu)\n");
    } else {
        printf("✗ Change colour: pas d'ACK\n");
    }
}

void goldelox_draw_circle(int fd, uint16_t x, uint16_t y, uint16_t rad, uint16_t color) {
    uint8_t packet[10] = {
        0xFF, 0xCD,
        x >> 8, x & 0xFF,
        y >> 8, y & 0xFF,
        rad >> 8, rad & 0xFF,
        color >> 8, color & 0xFF
    };
    if(_goldelox_send_and_wait_ack(fd, packet, 10) == 0) {
        printf("✓ Cercle dessiné\n");
    } else {
        fprintf(stderr, "✗ Draw Circle: pas d'ACK\n");
    }
}

void goldelox_draw_filled_circle(int fd, uint16_t x, uint16_t y, uint16_t rad, uint16_t color) {
    uint8_t packet[10] = {
        0xFF, 0xCC,
        x >> 8, x & 0xFF,
        y >> 8, y & 0xFF,
        rad >> 8, rad & 0xFF,
        color >> 8, color & 0xFF
    };
    if(_goldelox_send_and_wait_ack(fd, packet, 10) == 0) {
        printf("✓ Cercle plein dessiné\n");
    } else {
        fprintf(stderr, "✗ Draw Filled Circle: pas d'ACK\n");
    }
}

void goldelox_draw_line(int fd, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) {
    uint8_t packet[12] = {
        0xFF, 0xD2,
        x1 >> 8, x1 & 0xFF,
        y1 >> 8, y1 & 0xFF,
        x2 >> 8, x2 & 0xFF,
        y2 >> 8, y2 & 0xFF,
        color >> 8, color & 0xFF
    };
    if(_goldelox_send_and_wait_ack(fd, packet, 12) == 0) {
        // printf("✓ Ligne dessiné\n");
    } else {
        fprintf(stderr, "✗ Draw line: pas d'ACK\n");
    }
}

void goldelox_draw_rectangle(int fd, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) {
    uint8_t packet[12] = {
        0xFF, 0xCF,
        x1 >> 8, x1 & 0xFF,
        y1 >> 8, y1 & 0xFF,
        x2 >> 8, x2 & 0xFF,
        y2 >> 8, y2 & 0xFF,
        color >> 8, color & 0xFF
    };
    if(_goldelox_send_and_wait_ack(fd, packet, 12) == 0) {
        // printf("✓ Rectangle dessiné\n");
    } else {
        fprintf(stderr, "✗ Draw rectangle: pas d'ACK\n");
    }
}

void goldelox_draw_filled_rectangle(int fd, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) {
    uint8_t packet[12] = {
        0xFF, 0xCE,
        x1 >> 8, x1 & 0xFF,
        y1 >> 8, y1 & 0xFF,
        x2 >> 8, x2 & 0xFF,
        y2 >> 8, y2 & 0xFF,
        color >> 8, color & 0xFF
    };
    if(_goldelox_send_and_wait_ack(fd, packet, 12) == 0) {
        // printf("✓ Rectangle plein dessiné\n");
    } else {
        fprintf(stderr, "✗ Draw Filled Rectangle: pas d'ACK\n");
    }
}

void goldelox_draw_polyline(int fd, const uint16_t* xPoints, const uint16_t* yPoints, size_t numPoints, uint16_t color) {
    size_t packetSize = 2 + 2 + (numPoints * 2) * 2 + 2; // cmd + n + xPoints + yPoints + color
    uint8_t* packet = (uint8_t*)malloc(packetSize);
    if (!packet) {
        fprintf(stderr, "Erreur d'allocation mémoire pour polyline\n");
        return;
    }
    size_t idx = 0;
    // cmd
    packet[idx++] = 0x00;
    packet[idx++] = 0x05;
    // n
    packet[idx++] = (numPoints >> 8) & 0xFF;
    packet[idx++] = numPoints & 0xFF;
    // vx1...vxN
    for (size_t i = 0; i < numPoints; i++) {
        packet[idx++] = (xPoints[i] >> 8) & 0xFF;
        packet[idx++] = xPoints[i] & 0xFF;
    }
    // vy1...vyN
    for (size_t i = 0; i < numPoints; i++) {
        packet[idx++] = (yPoints[i] >> 8) & 0xFF;
        packet[idx++] = yPoints[i] & 0xFF;
    }
    // couleur
    packet[idx++] = (color >> 8) & 0xFF;
    packet[idx++] = color & 0xFF;

    write(fd, packet, packetSize);
    tcdrain(fd);
    usleep(200000);
    uint8_t response[1];
    ssize_t bytes_read = read(fd, response, 1);
    if (bytes_read > 0 && response[0] == 0x06)
        printf("✓ Polyline dessinée (ACK reçu)\n");
    else
        printf("✗ Draw Polyline: pas d'ACK\n");

    free(packet);
}

void goldelox_draw_polygon(int fd, const uint16_t* xPoints, const uint16_t* yPoints, size_t numPoints, uint16_t color) {
    size_t packetSize = 2 + 2 + (numPoints * 2) * 2 + 2; // cmd + n + xPoints + yPoints + color
    uint8_t* packet = (uint8_t*)malloc(packetSize);
    if (!packet) {
        fprintf(stderr, "Erreur d'allocation mémoire pour polygon\n");
        return;
    }
    size_t idx = 0;
    // cmd
    packet[idx++] = 0x00;
    packet[idx++] = 0x04;
    // n
    packet[idx++] = (numPoints >> 8) & 0xFF;
    packet[idx++] = numPoints & 0xFF;
    // vx1...vxN
    for (size_t i = 0; i < numPoints; i++) {
        packet[idx++] = (xPoints[i] >> 8) & 0xFF;
        packet[idx++] = xPoints[i] & 0xFF;
    }
    // vy1...vyN
    for (size_t i = 0; i < numPoints; i++) {
        packet[idx++] = (yPoints[i] >> 8) & 0xFF;
        packet[idx++] = yPoints[i] & 0xFF;
    }
    // couleur
    packet[idx++] = (color >> 8) & 0xFF;
    packet[idx++] = color & 0xFF;

    write(fd, packet, packetSize);
    tcdrain(fd);
    usleep(200000);
    uint8_t response[1];
    ssize_t bytes_read = read(fd, response, 1);
    if (bytes_read > 0 && response[0] == 0x06)
        printf("✓ Polygon dessiné (ACK reçu)\n");
    else
        printf("✗ Draw Polygon: pas d'ACK\n");

    free(packet);
}

void goldelox_draw_triangle(int fd, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t color) {
    uint8_t packet[16] = {
        0xFF, 0xC9,
        x1 >> 8, x1 & 0xFF,
        y1 >> 8, y1 & 0xFF,
        x2 >> 8, x2 & 0xFF,
        y2 >> 8, y2 & 0xFF,
        x3 >> 8, x3 & 0xFF,
        y3 >> 8, y3 & 0xFF,
        color >> 8, color & 0xFF
    };
    write(fd, packet, 16);
    tcdrain(fd);
    usleep(200000);
    uint8_t response[1];
    ssize_t bytes_read = read(fd, response, 1);
    if (bytes_read > 0 && response[0] == 0x06)
        printf("✓ Triangle dessiné (ACK reçu)\n");
    else
        printf("✗ Draw Triangle: pas d'ACK\n");
}

int goldelox_calculate_orbit(int fd, uint16_t angle, uint16_t distance, uint16_t* x, uint16_t* y) {
    uint8_t packet[6];
    // cmd
    packet[0] = 0x00;
    packet[1] = 0x03;
    // angle
    packet[2] = (angle >> 8) & 0xFF;
    packet[3] = angle & 0xFF;
    // distance
    packet[4] = (distance >> 8) & 0xFF;
    packet[5] = distance & 0xFF;

    write(fd, packet, 6);
    tcdrain(fd);
    usleep(200000);
    uint8_t response[5];
    ssize_t bytes_read = read(fd, response, 5);
    if (bytes_read == 5 && response[0] == 0x06) {
        *x = (response[1] << 8) | response[2];
        *y = (response[3] << 8) | response[4];
        printf("✓ Orbit calculé : X=%u, Y=%u (ACK reçu)\n", *x, *y);
        return 0;
    } else {
        printf("✗ Orbit: pas d'ACK ou réponse invalide\n");
        return -1;
    }
}

void goldelox_put_pixel(int fd, uint16_t x, uint16_t y, uint16_t color) {
    uint8_t packet[8] = {
        0xFF, 0xCB,
        x >> 8, x & 0xFF,
        y >> 8, y & 0xFF,
        color >> 8, color & 0xFF
    };
    write(fd, packet, 8);
    tcdrain(fd);
    usleep(200000);
    uint8_t response[1];
    ssize_t bytes_read = read(fd, response, 1);
    if (bytes_read > 0 && response[0] == 0x06)
        printf("✓ Pixel dessiné (ACK reçu)\n");
    else
        printf("✗ Put Pixel: pas d'ACK\n");
}

void goldelox_read_pixel(int fd, uint16_t x, uint16_t y, uint16_t* color) {
    uint8_t packet[6] = {
        0xFF, 0xCA,
        x >> 8, x & 0xFF,
        y >> 8, y & 0xFF
    };
    write(fd, packet, 6);
    tcdrain(fd);
    usleep(200000);
    uint8_t response[3];
    ssize_t bytes_read = read(fd, response, 3);
    if (bytes_read == 3 && response[0] == 0x06) {
        *color = (response[1] << 8) | response[2];
        printf("✓ Pixel lu : Couleur=0x%04X (ACK reçu)\n", *color);
    } else {
        printf("✗ Read Pixel: pas d'ACK ou réponse invalide\n");
    }
}

void goldelox_move_origin(int fd, int16_t xOffset, int16_t yOffset) {
    uint8_t packet[6] = {
        0xFF, 0xD6,
        (xOffset >> 8) & 0xFF, xOffset & 0xFF,
        (yOffset >> 8) & 0xFF, yOffset & 0xFF
    };
    write(fd, packet, 6);
    tcdrain(fd);
    usleep(200000);
    uint8_t response[1];
    ssize_t bytes_read = read(fd, response, 1);
    if (bytes_read > 0 && response[0] == 0x06)
        printf("✓ Origine déplacée (ACK reçu)\n");
    else
        printf("✗ Move Origin: pas d'ACK\n");
}

void goldelox_draw_line_and_move_origin(int fd, uint16_t xpos, uint16_t ypos) {
    uint8_t packet[6] = {
        0xFF, 0xD4,
        xpos >> 8, xpos & 0xFF,
        ypos >> 8, ypos & 0xFF
    };
    write(fd, packet, 6);
    tcdrain(fd);
    usleep(200000);
    uint8_t response[1];
    ssize_t bytes_read = read(fd, response, 1);
    if (bytes_read > 0 && response[0] == 0x06)
        printf("✓ Ligne dessinée et origine déplacée (ACK reçu)\n");
    else
        printf("✗ Draw Line and Move Origin: pas d'ACK\n");
}

// Marche pas donc a vérifier
void goldelox_clipping(int fd, uint8_t value) {
    uint8_t packet[3] = {
        0xFF, 0x6C,
        value ? 0x01 : 0x00
    };
    write(fd, packet, 3);
    tcdrain(fd);
    usleep(200000);
    uint8_t response[1];
    ssize_t bytes_read = read(fd, response, 1);
    if (bytes_read > 0 && response[0] == 0x06)
        printf("✓ Clipping %s (ACK reçu)\n", value ? "activé" : "désactivé");
    else
        printf("✗ Clipping: pas d'ACK\n");
}

// A tester
void goldelox_set_clip_window(int fd, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    uint8_t packet[10] = {
        0xFF, 0xBF,
        (x1 >> 8) & 0xFF, x1 & 0xFF,
        (y1 >> 8) & 0xFF, y1 & 0xFF,
        (x2 >> 8) & 0xFF, x2 & 0xFF,
        (y2 >> 8) & 0xFF, y2 & 0xFF
    };
    write(fd, packet, 10);
    tcdrain(fd);
    usleep(200000);
    uint8_t response[1];
    ssize_t bytes_read = read(fd, response, 1);
    if (bytes_read > 0 && response[0] == 0x06)
        printf("✓ Clip Window défini (ACK reçu)\n");
    else
        printf("✗ Clip Window: pas d'ACK\n");
}

// A tester
void goldelox_extend_clip_region(int fd) {
    uint8_t packet[2] = {0xFF, 0xBC};
    write(fd, packet, 2);
    tcdrain(fd);
    usleep(200000);
    uint8_t response[1];
    ssize_t bytes_read = read(fd, response, 1);
    if (bytes_read > 0 && response[0] == 0x06)
        printf("✓ Clip Region étendu (ACK reçu)\n");
    else
        printf("✗ Extend Clip Region: pas d'ACK\n");
}

void goldelox_background_color(int fd, uint16_t color) {
    uint8_t packet[4] = {
        0xFF, 0x6E,
        color >> 8, color & 0xFF
    };
    write(fd, packet, 4);
    tcdrain(fd);
    usleep(200000);
    uint8_t response[1];
    ssize_t bytes_read = read(fd, response, 1);
    if (bytes_read > 0 && response[0] == 0x06)
        printf("✓ Couleur de fond définie (ACK reçu)\n");
    else
        printf("✗ Background Color: pas d'ACK\n");
}



/* ======================= SOUND & TUNE COMMANDS ======================= */
void goldelox_beep(int fd, uint16_t frequency, uint16_t duration) {
    uint8_t packet[6] = {
        0xFF, 0xDA,
        frequency >> 8, frequency & 0xFF,
        duration >> 8, duration & 0xFF
    };
    write(fd, packet, 6);
    tcdrain(fd);
    usleep(200000);
    uint8_t response[1];
    ssize_t bytes_read = read(fd, response, 1);
    if (bytes_read > 0 && response[0] == 0x06)
        printf("✓ Beep émis (ACK reçu)\n");
    else
        printf("✗ Beep: pas d'ACK\n");
}

/* ======================= SERIAL COMMUNICATIONS COMMANDS ======================= */
// A tester
void goldelox_set_baudrate(int fd, uint32_t baudrate) {
    uint8_t packet[6] = {
        0x00, 0x0B,
        (baudrate >> 24) & 0xFF,
        (baudrate >> 16) & 0xFF,
        (baudrate >> 8) & 0xFF,
        baudrate & 0xFF
    };
    if(_goldelox_send_and_wait_ack(fd, packet, 6) == 0) {
        printf("✓ Baudrate changée (ACK reçu)\n");
    } else {
        printf("✗ Set Baudrate: pas d'ACK\n");
    }
}