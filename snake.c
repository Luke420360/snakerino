#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <termios.h>
#include <sys/select.h>

/* CONSTS */
const int     FIELD_HEIGHT         = 10;
const int     FIELD_WIDTH          = 10;
const int     SNAKE_START_POINT[2] = {8, 8};
const int     SNAKE_START_LENGTH   = 3;
const double  SNAKE_SPEED          = 1;
const int     FOOD_AMOUNT          = 40;
const char    SYMBOL_FIELD_WALL    = '#';
const char    SYMBOL_FIELD_EMPTY[] = "○ ";
const char    SYMBOL_SNAKEHEAD[]   = "🐲";
const char    SYMBOL_SNAKEBODY[]   = "🟢";
const char    SYMBOL_FOOD[]        = "🍫";

/* VARIABLES */
int is_game_over = 0;
int is_paused = 0;
int snake[FIELD_HEIGHT * FIELD_WIDTH][2];
int snake_length = SNAKE_START_LENGTH;
int snake_looking_direction = 2;
int food[FOOD_AMOUNT][2];

/*
    SNAKE_LOOKING_DIRECTION
    1: GOING RIGHT
    2: GOING UP
    -1: GOING LEFT
    -2: GOING DOWN
    0: Nothing
*/

void snake_init () {
    int start_x = SNAKE_START_POINT[0];
    int start_y = SNAKE_START_POINT[1];

    // HEAD
    snake[0][0] = start_x;
    snake[0][1] = start_y;

    // BODY
    for (int bodypart = 1; bodypart <= snake_length; bodypart++)
    {
        snake[bodypart][0] = start_x + bodypart;
        snake[bodypart][1] = start_y;
    }
}

int food_placeable (int x, int y) {
    for (int snake_part = 0; snake_part <= snake_length; snake_part++)
    {
        if(snake[snake_part][0] == x && snake[snake_part][1] == y) return 1;
    }
    for (int f = 0; f < FOOD_AMOUNT; f++)
    {
        if(food[f][0] == x && food[f][1] == y) return 1;
    }

    return 0;
}

int randomNr (int min, int max) {
    int rd_num = (rand() % (max - min + 1)) + min;
    return rd_num;
}

void food_respawn_one(int index) {
    int new_x;
    int new_y;
    unsigned int seed = time(0);
    do {
        new_x = randomNr(0, FIELD_WIDTH - 1);
        new_y = randomNr(0, FIELD_HEIGHT - 1);
    } while(food_placeable(new_x, new_y) == 1);

    food[index][0] = new_x;
    food[index][1] = new_y;
}

void food_spawn () {
    for (int f = 0; f < FOOD_AMOUNT; f++)
    {
        food_respawn_one(f);
    }
}

int check_collide (int snake_head_next_x, int snake_head_next_y) {
    // Colliding with Wall
    if (snake_head_next_x < 0 || snake_head_next_x > FIELD_WIDTH -1) return 2;
    if (snake_head_next_y < 0 || snake_head_next_y > FIELD_HEIGHT -1) return 2;
    
    // Colliding with the body of the snake
    for (int snake_part = 1; snake_part <= snake_length; snake_part++)
    {
        if (snake[snake_part][0] == snake_head_next_x && snake[snake_part][1] == snake_head_next_y) {
            return 2;
        }
    }

    for (int f = 0; f < FOOD_AMOUNT; f++)
    {
        if (food[f][0] == snake_head_next_x && food[f][1] == snake_head_next_y) {
            if( ( (snake_length + 1) + FOOD_AMOUNT) >= (FIELD_WIDTH * FIELD_HEIGHT)) {
                food[f][0] = -1;
                food[f][1] = -1;
            } else {
                food_respawn_one(f);
            }
            snake_length++;
        }
    }

    return 0;
}

void snake_move () {
    int last_x = snake[0][0];
    int last_y = snake[0][1];

    switch (snake_looking_direction) {
        case 1: { 
            if (check_collide(snake[0][0] + SNAKE_SPEED, snake[0][1]) == 2) {
                is_game_over = 1;
                return;
            } else {
                snake[0][0] = snake[0][0] + SNAKE_SPEED;
            }
        }
        break;
        case 2: {
            if (check_collide(snake[0][0], snake[0][1] - SNAKE_SPEED) == 2) {
                is_game_over = 1;
                return;
            } else {
                snake[0][1] = snake[0][1] - SNAKE_SPEED;
            }
        }
        break;
        case -1: { 
            if (check_collide(snake[0][0] - SNAKE_SPEED, snake[0][1]) == 2) {
                is_game_over = 1;
                return;
            } else {
                snake[0][0] = snake[0][0] - SNAKE_SPEED;
            }
        }
        break;
        case -2: {
            if (check_collide(snake[0][0], snake[0][1] + SNAKE_SPEED) == 2) {
                is_game_over = 1;
                return;
            } else {
                snake[0][1] = snake[0][1] + SNAKE_SPEED;
            }
        }
    }

    for (int snake_part = 1; snake_part <= snake_length; snake_part++)
    {
        int temp_x = snake[snake_part][0];
        int temp_y = snake[snake_part][1];

        snake[snake_part][0] = last_x;
        snake[snake_part][1] = last_y;

        last_x = temp_x;
        last_y = temp_y;
    }
}

void render_field () {
    const char* field_symbol = SYMBOL_FIELD_EMPTY;
    for (int y = 0; y < FIELD_HEIGHT; y++)
    {
        for (int x = 0; x < FIELD_WIDTH; x++)
        {
            // SNAKE
            for (int i = 0; i <= snake_length; i++)
            {
                if (snake[i][0] == x && snake[i][1] == y) {
                    switch (i) {
                        case 0: field_symbol = SYMBOL_SNAKEHEAD;
                        break;
                        default: field_symbol = SYMBOL_SNAKEBODY;
                    }
                    break;
                }
            }
            
            // FOOD
            for (int f = 0; f < FOOD_AMOUNT; f++)
            {
                if(food[f][0] == x && food[f][1] ==y ) {
                    field_symbol = SYMBOL_FOOD;
                } 
            }

            printf("%s", field_symbol);
            field_symbol = SYMBOL_FIELD_EMPTY;
        }
        printf("\n");
    }
    printf("\e[1;1H\e[2J");
}

void enableRawMode() {
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

void disableRawMode() {
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

int kbhit() {
    struct timeval tv = {0, 0};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
}

void on_move () {
    switch(getchar()) {
        case 'w': {
                if(snake_looking_direction != -2) snake_looking_direction = 2;
            }
        break;
        case 'a': {
                if(snake_looking_direction != 1) snake_looking_direction = -1;
            }
        break;
        case 's': {
                if(snake_looking_direction != 2) snake_looking_direction = -2;
            }
        break;
        case 'd': {
                if(snake_looking_direction != -1) snake_looking_direction = 1;
            }
        break;
        default: snake_looking_direction = 0;
    }
}

int main () {
    printf("\e[1;1H\e[2J");
    enableRawMode();
    snake_init();
    food_spawn();

    while(is_game_over != 1) {
        render_field();
        if(kbhit()) on_move();
        usleep(150000);
        if(kbhit()) on_move();
        snake_move();
    }

    disableRawMode();
    printf("%s %d", "Your Score: ", snake_length + 1);
    return 0;
}