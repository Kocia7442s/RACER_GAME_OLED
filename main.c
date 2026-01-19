#include "game_function.h"
#include "main.h"
#include "goldelox_function.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>
#include <poll.h>
#include <signal.h>
#include <time.h>

sig_atomic_t sigFlagStop = 0;

void signal_handler(int iNumSignal) {
    switch (iNumSignal) {
        case SIGTERM:
        case SIGINT:
            sigFlagStop = 1;
            fprintf(stdout, "Signal received, stopping...\n");
            break;
        default:
            break;
    }
}

/* ======================= GPIO BUTTON MANAGEMENT ======================= */

typedef struct {
    int fd_chip2;
    int fd_chip3;
    struct gpioevent_request evt_left;    // chip2, line 0
    struct gpioevent_request evt_right;   // chip2, line 1
    struct gpioevent_request evt_select;  // chip3, line 0 (or line 1)
    struct pollfd pollfds[3];
    int num_fds;
} GPIO_Buttons;

int init_gpio_buttons(GPIO_Buttons* buttons) {
    
    // Open gpiochip2 (buttons with Falling Edge)
    buttons->fd_chip2 = open("/dev/gpiochip2", O_RDONLY);
    if (buttons->fd_chip2 < 0) {
        perror("open gpiochip2");
        return -1;
    }
    
    // Open gpiochip3 (buttons with Both Edges)
    buttons->fd_chip3 = open("/dev/gpiochip3", O_RDONLY);
    if (buttons->fd_chip3 < 0) {
        perror("open gpiochip3");
        close(buttons->fd_chip2);
        return -1;
    }
    
    // Configure left button (chip2, line 0) - Falling Edge
    memset(&buttons->evt_left, 0, sizeof(struct gpioevent_request));
    buttons->evt_left.lineoffset = 1;
    buttons->evt_left.handleflags = GPIOHANDLE_REQUEST_INPUT;
    buttons->evt_left.eventflags = GPIOEVENT_REQUEST_FALLING_EDGE;
    
    if (ioctl(buttons->fd_chip2, GPIO_GET_LINEEVENT_IOCTL, &buttons->evt_left) < 0) {
        perror("ioctl left button");
        close(buttons->fd_chip2);
        close(buttons->fd_chip3);
        return -1;
    }
    
    // Configure right button (chip2, line 1) - Falling Edge
    memset(&buttons->evt_right, 0, sizeof(struct gpioevent_request));
    buttons->evt_right.lineoffset = 0;
    buttons->evt_right.handleflags = GPIOHANDLE_REQUEST_INPUT;
    buttons->evt_right.eventflags = GPIOEVENT_REQUEST_FALLING_EDGE;
    
    if (ioctl(buttons->fd_chip2, GPIO_GET_LINEEVENT_IOCTL, &buttons->evt_right) < 0) {
        perror("ioctl right button");
        close(buttons->evt_left.fd);
        close(buttons->fd_chip2);
        close(buttons->fd_chip3);
        return -1;
    }
    
    // Configure select button (chip3, line 0) - Both Edges
    memset(&buttons->evt_select, 0, sizeof(struct gpioevent_request));
    buttons->evt_select.lineoffset = 0;
    buttons->evt_select.handleflags = GPIOHANDLE_REQUEST_INPUT;
    buttons->evt_select.eventflags = GPIOEVENT_REQUEST_BOTH_EDGES;
    
    if (ioctl(buttons->fd_chip3, GPIO_GET_LINEEVENT_IOCTL, &buttons->evt_select) < 0) {
        perror("ioctl select button");
        close(buttons->evt_left.fd);
        close(buttons->evt_right.fd);
        close(buttons->fd_chip2);
        close(buttons->fd_chip3);
        return -1;
    }
    
    // Setup poll file descriptors
    buttons->pollfds[0].fd = buttons->evt_left.fd;
    buttons->pollfds[0].events = POLLIN;
    buttons->pollfds[1].fd = buttons->evt_right.fd;
    buttons->pollfds[1].events = POLLIN;
    buttons->pollfds[2].fd = buttons->evt_select.fd;
    buttons->pollfds[2].events = POLLIN;
    buttons->num_fds = 3;
    
    printf("GPIO boutons initialisés avec succès\n");
    return 0;
}

void cleanup_gpio_buttons(GPIO_Buttons* buttons) {
    close(buttons->evt_left.fd);
    close(buttons->evt_right.fd);
    close(buttons->evt_select.fd);
    close(buttons->fd_chip2);
    close(buttons->fd_chip3);
}

void read_gpio_buttons(GPIO_Buttons* gpio_buttons, ButtonState* button_state) {
    struct gpioevent_data event_data;
    
    // Reset button states
    button_state->left_pressed = 0;
    button_state->right_pressed = 0;
    button_state->select_pressed = 0;
    
    // Poll for events with short timeout
    int poll_result = poll(gpio_buttons->pollfds, gpio_buttons->num_fds, 10);
    
    if (poll_result > 0) {
        // Check left button
        if (gpio_buttons->pollfds[0].revents & POLLIN) {
            if (read(gpio_buttons->evt_left.fd, &event_data, sizeof(event_data)) > 0) {
                button_state->left_pressed = 1;
                printf("Left button pressed\n");
            }
        }
        
        // Check right button
        if (gpio_buttons->pollfds[1].revents & POLLIN) {
            if (read(gpio_buttons->evt_right.fd, &event_data, sizeof(event_data)) > 0) {
                button_state->right_pressed = 1;
                printf("Right button pressed\n");
            }
        }
        
        // Check select button
        if (gpio_buttons->pollfds[2].revents & POLLIN) {
            if (read(gpio_buttons->evt_select.fd, &event_data, sizeof(event_data)) > 0) {
                // For both edges, check which edge
                if (event_data.id == GPIOEVENT_EVENT_FALLING_EDGE) {
                    button_state->select_pressed = 1;
                    printf("Select button pressed\n");
                }
            }
        }
    }
}

/* ======================= OLED MANAGEMENT ======================== */

int open_serial(const char* port) {
    // Ouverture du port en mode non-bloquant
    int fd = open(port, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        perror("Erreur ouverture");
        return -1;
    } else {
        printf("Port série %s ouvert avec succès.\n", port);
    }

    struct termios tty;
    memset(&tty, 0, sizeof tty);
    if (tcgetattr(fd, &tty) != 0) {
        perror("Erreur tcgetattr");
        return -1;
    }

    // Configuration des vitesses de transmission asynchrone
    cfsetospeed(&tty, B9600); // vitesse de sortie (TXD) - 9600 baud
    cfsetispeed(&tty, B9600); // vitesse d'entrée (RXD) - 9600 baud

    // Configuration des bits de données
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8 bits de données
    
    // Configuration des drapeaux d'entrée
    tty.c_iflag &= ~IGNBRK;                         // Ne pas ignorer les breaks
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);        // Désactiver le contrôle de flux logiciel (XON/XOFF)
    tty.c_iflag &= ~(ISTRIP | INLCR | IGNCR | ICRNL); // Configuration raw input
    
    // Configuration des drapeaux de sortie (mode raw)
    tty.c_oflag = 0;                                 // Mode raw output
    
    // Configuration du mode local (mode non-canonique)
    tty.c_lflag = 0;                                 // Désactiver mode canonique, signaux, echo
    
    // Configuration des temps d'attente (mode bloquant)
    tty.c_cc[VMIN]  = 0;                            // Lecture non-bloquante (pas d'attente de caractère)
    tty.c_cc[VTIME] = 1;                           // Timeout 1000ms (1 seconde)

    // Configuration des signaux de contrôle
    tty.c_cflag |= (CREAD | CLOCAL);                // Active RXD et mode LOCAL
    tty.c_cflag &= ~(PARENB | PARODD);              // Pas de parité
    tty.c_cflag &= ~CSTOPB;                         // 1 bit de stop
    tty.c_cflag &= ~CRTSCTS;                        // Désactiver RTS/CTS (handshake matériel)

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {        // Appliquer les paramètres immédiatement
        perror("Erreur tcsetattr");
        return -1;
    }

    // Redonner l'attribut bloquant au périphérique
    int flags = fcntl(fd, F_GETFL, 0); // Obtenir les flags actuels
    if (flags < 0) {
        perror("Erreur fcntl F_GETFL");
        return -1;
    }
    flags &= ~O_NONBLOCK;  // Retirer le flag non-bloquant
    if (fcntl(fd, F_SETFL, flags) < 0) {
        perror("Erreur fcntl F_SETFL");
        return -1;
    }

    printf("Port série configuré en mode bloquant.\n");
    return fd;
}

/* ======================= MAIN GAME LOOP ======================= */

int main(int argc, char* argv[]) {
    int oled_fd;
    GameData game;
    GPIO_Buttons gpio_buttons;
    ButtonState button_state;
    struct timespec frame_start, frame_end;
    long frame_time_ns;
    
    // Install signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    printf("Game - Starting...\n");
    
    // Open OLED serial port
    oled_fd = open_serial("/dev/ttyAL0");
    if (oled_fd < 0) {
        fprintf(stderr, "Erreur d'ouverture OLED serial port\n");
        return 1;
    }
    
    // Initialize GPIO buttons
    if (init_gpio_buttons(&gpio_buttons) < 0) {
        fprintf(stderr, "Erreur d'initialisation GPIO buttons\n");
        close(oled_fd);
        return 1;
    }
    
    // Seed random number generator
    srand(time(NULL));
    
    // Initialize game
    game_init(&game, oled_fd);
    
    printf("Initialisation du jeu completé. Commencement main loop...\n");
    
    // Main game loop
    while (!sigFlagStop) {
        clock_gettime(CLOCK_MONOTONIC, &frame_start);
        
        // Read button inputs
        read_gpio_buttons(&gpio_buttons, &button_state);
        
        // Update game state
        game_update(&game, &button_state);
        
        // Render game
        game_render(oled_fd, &game);
        
        // Frame rate control (approx 30 FPS)
        clock_gettime(CLOCK_MONOTONIC, &frame_end);
        frame_time_ns = (frame_end.tv_sec - frame_start.tv_sec) * 1000000000 +
                        (frame_end.tv_nsec - frame_start.tv_nsec);
        
        long target_time_ns = GAME_SPEED * 1000000;  // GAME_SPEED in milliseconds
        if (frame_time_ns < target_time_ns) {
            usleep((target_time_ns - frame_time_ns) / 1000);
        }
    }
    
    printf("Shutting down...\n");
    
    // Cleanup
    cleanup_gpio_buttons(&gpio_buttons);
    close(oled_fd);
    
    printf("Game - Stopped\n");
    return 0;
}