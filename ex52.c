#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#define MAX_MESSAGE 512
#define MOVE_RIGHT 'd'
#define MOVE_LEFT 'a'
#define MOVE_DOWN 's'
#define ROTATE 'w'
#define HORIZONTAL 'h'
#define VERTICAL 'v'
#define ROW 0
#define COL 1
#define FRAME_WIDTH 20
#define FRAME_HEIGHT 20

typedef struct Game {
    char frame[FRAME_HEIGHT][FRAME_WIDTH];      // game screen.
    int block[2];                               // current col number of block.
    char orientation;                           // game orientation (horizontal / vertical).
} Game;

// create a game struct and point to it's address.
Game g;
Game* game = &g;

void initFrame();
void initBlock();
void updateBlock();
int blockReachedRight();
int blockReachedLeft();
int blockReachedBottom();
int canMoveBlockDown();
void handleKeyPress();
void displayCurrentFrame();

int main() {
    // initialize the game.
    initFrame();
    initBlock();
    game->orientation = HORIZONTAL;
    // display first frame.
    displayCurrentFrame();
    // start the game flow.
    updateBlock();
    // start handling key press.
     signal(SIGUSR2, handleKeyPress);
     while (pause());
}

/*
 * Initialize the game frame.
 */
void initFrame() {
    // draw the frame.
    int i;
    int j;
    for (i = 0; i < 19; i++) {
        game->frame[i][0] = '*';
        for (j = 1; j < 19; j++) {
            game->frame[i][j] = ' ';
        }
        game->frame[i][19] = '*';
    }
    for (i = 0; i < 20; i++) {
        game->frame[19][i] = '*';
    }
}

/*
 * Initialize the block.
 */
void initBlock() {
    // initialize the block depanding on the game orientation.
    if (game->orientation == HORIZONTAL) {
        game->block[ROW] = 0;
        game->block[COL] = 9;
    } else if (game->orientation == VERTICAL) {
        game->block[ROW] = 1;
        game->block[COL] = 9;
    }
}

/*
 * Create the game flow by updating the block's location each second (in addition to response for key press).
 */
void updateBlock() {
    // set the alarm for 1 second between each update.
    signal(SIGALRM, updateBlock);
    alarm(1);
    // move the block one line down.
    game->block[ROW] += 1;
    // if the block has reached the bottom line, remove it and initialize another one instead.
    if (blockReachedBottom()) {
        initFrame();
        initBlock();
        // else, draw the block on its' new location on the frame.
    } else {
        initFrame();
        if(game->orientation == HORIZONTAL) {
            game->frame[game->block[ROW]][game->block[COL] - 1] = '-';
            game->frame[game->block[ROW]][game->block[COL]] = '-';
            game->frame[game->block[ROW]][game->block[COL] + 1] = '-';
        } else {
            game->frame[game->block[ROW] - 2][game->block[COL] ] = '-';
            game->frame[game->block[ROW] - 1][game->block[COL]] = '-';
            game->frame[game->block[ROW]][game->block[COL] ] = '-';
        }
    }
    // if for some sync reason the block went out of limits, reset the frame and block.
    if ((game->block[COL] >= 19) || (game->block[COL] <= 0)) {
        initFrame();
        initBlock();
    }
    if ((game->block[ROW] >= 19) || (game->block[ROW] <= 0)) {
        initFrame();
        initBlock();
    }
    displayCurrentFrame();
}

/*
 * Update the game frame according to the pressed key.
 */
void handleKeyPress() {
    // receive signals from father process.
    signal(SIGUSR2, handleKeyPress);
    char c = (char) getchar();
    switch (c) {
        case ROTATE:
            // check if the rotation is possible.
            if (game->orientation == VERTICAL) {
                if ((!blockReachedRight()) && (!blockReachedLeft())) {
                    initFrame();
                    if (game->orientation == HORIZONTAL) {
                        game->orientation = VERTICAL;
                        updateBlock();
                    } else if (game->orientation == VERTICAL) {
                        game->orientation = HORIZONTAL;
                        updateBlock();
                    }
                }
            } else {
                initFrame();
                if (game->orientation == HORIZONTAL) {
                    game->orientation = VERTICAL;
                    updateBlock();
                } else if (game->orientation == VERTICAL) {
                    game->orientation = HORIZONTAL;
                    updateBlock();
                }
            }
            displayCurrentFrame();
            break;
        case MOVE_RIGHT:
            // if the block hasn't reached the right border, move it to the right.
            if (!blockReachedRight()) {
                game->block[COL] += 1;
                updateBlock();
            }
            displayCurrentFrame();
            break;
        case MOVE_LEFT:
            // if the block hasn't reached the left border, move it to the left.
            if (!blockReachedLeft()) {
                game->block[COL] -= 1;
                updateBlock();
            }
            displayCurrentFrame();
            break;
        case MOVE_DOWN:
            // if the block hasn't reached the bottom border, move it down.
            if (canMoveBlockDown()) {
                game->block[ROW] += 1;
                updateBlock();
            }
            displayCurrentFrame();
            break;
        default:
            break;
    }
}

/*
 * Print the frame and the block according to its' current row and column values.
 */
void displayCurrentFrame() {
    // clear previous frame.
    system("clear");
    // draw current frame.
    int i;
    int j;
    for (i = 0; i < 20; i++) {
        for (j = 0; j < 20; j++) {
            char c = game->frame[i][j];
            write(STDOUT_FILENO, &c, sizeof(char));
            if (j == 19) {
                write(STDOUT_FILENO, "\n", sizeof(char));
            }
        }
    }
}

/*
 * Check if the block has reached bottom border.
 */
int blockReachedBottom() {
    if (game->block[ROW] == FRAME_HEIGHT - 1) {
        return 1;
    }
    return 0;
}

/*
 * Check if it is possible to move the block down by pressing the 's' key,
 * (since there are two options for the block to be moved down).
 */
int canMoveBlockDown() {
    if (blockReachedBottom()) {
        return 0;
    }
    if (game->orientation == HORIZONTAL) {
        if (game->block[ROW] == FRAME_HEIGHT - 2) {
            return 0;
        }
    } else if (game->orientation == VERTICAL) {
        if (game->block[ROW] + 1 == FRAME_HEIGHT - 2) {
            return 0;
        }
    }
    return 1;
}

/*
 * Check if the block has reached right border.
 */
int blockReachedRight() {
    if (game->orientation == HORIZONTAL) {
        if (game->block[COL] + 1 == FRAME_WIDTH - 2) {
            return 1;
        }
    }
    if (game->orientation == VERTICAL) {
        if (game->block[COL] == FRAME_WIDTH - 2) {
            return 1;
        }
    }
    return 0;
}

/*
 * Check if the block has reached left border.
 */
int blockReachedLeft() {
    if (game->orientation == HORIZONTAL) {
        if (game->block[COL] - 1 == 1) {
            return 1;
        }
    }
    if (game->orientation == VERTICAL) {
        if (game->block[COL] == 1) {
            return 1;
        }
    }
    return 0;
}


