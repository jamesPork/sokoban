// SOKOBAN
// Written by JAMES WU (Z5586247),
// on 6/21/24
// Simple puzzle game with boxes

#include <stdio.h>
#include <string.h>

#define ROWS 10
#define COLS 10


#define MAX_UNDO 1000

//Controls
#define UP 'w'
#define DOWN 's'
#define LEFT 'a'
#define RIGHT 'd'
#define RESET 'r'
#define COUNTER_REQUEST 'c'
#define UNDO 'u'
// Every tile on the map has to be one of the following values.
enum base { 
    NONE,
    WALL,
    STORAGE
};

// A single tile of our board.
// box should only contain the value:
// - true (1): there exists a box here
// - false (0): there doesn't exist a box here
struct tile {
    enum base base;
    int box;
};

struct stack {
    struct tile previous_board[ROWS][COLS];
    int pre_row;
    int pre_col;
    int pre_counter;
};

struct player {
    int row;
    int col;
};

enum direction {
    HORIZONTAL,
    VERTICAL
};

struct linked_box {
    int row1;
    int col1;
    int row2;
    int col2;
};


// Function Prototypes
void init_board(struct tile board[ROWS][COLS]);
void print_line(void);
void print_title(void);
void print_board(struct tile board[ROWS][COLS], 
int player_row, int player_col);
int bounds_check(int row, int col);
void simple_place (struct tile board[ROWS][COLS], int row, int col, char command);
void extended_place (struct tile board[ROWS][COLS], enum base type, 
int row, int col, int end_row, int end_col);
int bound_bind(int invalid_pos);
int board_wrap(int row);
int win_check(struct tile board[ROWS][COLS]);
void board_copy(struct tile copy_board[ROWS][COLS], 
struct tile board[ROWS][COLS]);
int move_possible(struct tile board[ROWS][COLS], int row, int col, int value, enum direction direction);
void box_move(struct tile board[ROWS][COLS], int row, int col, int value, enum direction direction);
void link_box_move(struct tile board[ROWS][COLS], int row, int col, int value, 
enum direction direction , struct linked_box linked_box[50], int link_box_no);

// TODO: Your function prototypes go here

int main(void) {
    struct tile board[ROWS][COLS];
    init_board(board);
    char command;
    int row;
    int col;
    int end_row;
    int end_col;
    struct linked_box linked_box[50];
    int link_no = 0;

    printf("=== Level Setup ===\n");

    while (scanf(" %c", &command) == 1 && command != 'q') {
        if (command >= 'a') {
            //checks if the command is lower case or upper case
            scanf(" %d %d", &row, &col);
            if (bounds_check(row, col)) {
                // checks if input row and column is a valid input
                simple_place(board, row, col, command);
                //places box, wall or storage at row col based on command
                if (command == 'l') {
                    scanf(" %d %d", &linked_box[link_no].row2, &linked_box[link_no].col2);
                    if (board[row][col].box == 1 && 
                    board[linked_box[link_no].row2]
                    [linked_box[link_no].col2].box == 1) {
                        linked_box[link_no].row1 = row;
                        linked_box[link_no].col1 = col;
                        link_no++;
                        print_board(board, -1, -1);
                    } else {
                        printf("Invalid Location(s)\n");
                    }
                }
            } else {
                //if input is invalid:
                printf("Location out of bounds\n");
                print_board(board, -1, -1);
            }
        } else if (command >= 'A') {
            scanf(" %d %d", &row, &col);
            scanf(" %d %d", &end_row, &end_col);
            if (bounds_check(row, col) || bounds_check(end_row, end_col)) {
                row = bound_bind(row);
                col = bound_bind(col);
                end_row = bound_bind(end_row);
                end_col = bound_bind(end_col);
                // resets row/col to 0 or 9 
                // if number is <0 or >9 respectively
                if (command == 'W') {
                    extended_place (board, WALL, row, col, end_row, end_col);
                } else if (command == 'S') {
                    extended_place (board, STORAGE, row, col, end_row, end_col);
                }
            } else {
                //if both input locations are invalid:
                printf("Location out of bounds\n");
                print_board(board, -1, -1); 
            }
        } 
    }
    int i = 0;
    while (i == 0) {
        printf("Enter player starting position: ");
        scanf(" %d %d", &row, &col);
        if (bounds_check(row, col) == 1 && board[row][col].base != WALL 
        && board[row][col].box != 1) {
            printf("\n=== Starting Sokoban! ===\n");
            print_board(board, row, col);
            i++;
        } else {
            // if player location is invalid
            printf("Position (%d, %d) is invalid\n", row, col);
        }
    }
    int start_row = row;
    int start_col = col;
    struct tile initial_board[ROWS][COLS];
    board_copy(initial_board, board);
    //Stores initial state of board to allow the board to reset back
    //to its original state

    struct stack undo_boards[MAX_UNDO];
    //Array of boards to allow undo
    int board_num = 0;

    int counter = 0;
    undo_boards[board_num].pre_counter = counter;
    char control;
    while (scanf(" %c", &control) == 1 && win_check(board) == 0) {
        //For undoing:
        if (control == UP || control == LEFT 
        || control == DOWN || control == RIGHT) {
            //Stores the board memory so it can revert back to it
            //when u is pressed
            board_copy(undo_boards[board_num].previous_board, board);
            undo_boards[board_num].pre_row = row;
            undo_boards[board_num].pre_col = col;
            undo_boards[board_num].pre_counter = counter;
            board_num++;
        }
        if (control == UP) {
            if (move_possible(board, row, col, -1, VERTICAL) == 1) {
                row--;
                counter++;
                box_move(board, row, col, -1, VERTICAL);
            } 
        } else if (control == LEFT) {
            if (move_possible(board, row, col, -1, HORIZONTAL) == 1) {
                col--;
                counter++;
                box_move(board, row, col, -1, HORIZONTAL);
            } 
        } else if (control == DOWN) {
            if (move_possible(board, row, col, +1, VERTICAL) == 1) {
                row++;
                counter++;
                box_move(board, row, col, +1, VERTICAL);
            } 
        } else if (control == RIGHT) {
            if (move_possible(board, row, col, +1, HORIZONTAL) == 1) {
                col++;
                counter++;
                box_move(board, row, col, +1, HORIZONTAL);
            } 
        } else if (control == COUNTER_REQUEST) {
            printf("Number of moves so far: %d\n", counter);
        } else if (control == RESET) {
            printf("=== Resetting Game ===\n");
            board_copy(board, initial_board);
            row = start_row;
            col = start_col;
            counter = 0;
        } else if (control == UNDO && counter > 0) {
            //Undo only works when moves have been made
            board_num--;
            while (counter == undo_boards[board_num].pre_counter) {
                //Counter also acts as valid move made so it goes to the board
                //until the counter changes
                board_num--;
            }
            board_copy(board, undo_boards[board_num].previous_board);
            row = undo_boards[board_num].pre_row;
            col = undo_boards[board_num].pre_col;
            counter = undo_boards[board_num].pre_counter;
        }

        row = board_wrap(row);
        col = board_wrap(col);
        print_board(board, row, col);
        //Detects if all storage locations have been filled:
        if (win_check(board)) {
            if (counter == 1) {
                printf("=== Level Solved in %d Move! ===\n", counter);
            } else {
                printf("=== Level Solved in %d Moves! ===\n", counter);
            }
        }
    }
    


    return 0;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////// YOUR FUNCTIONS //////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


int bounds_check(int row, int col) {
    if (row < 0 || row >= ROWS || col < 0 || col >= COLS) {
        return 0;
    } else {
        return 1;
    }
}
int bound_bind(int invalid_pos) {
    if (invalid_pos < 0) {
        invalid_pos = 0;
    } else if (invalid_pos > 9) {
        invalid_pos = 9;
    }
    return invalid_pos;
}
void simple_place (struct tile board[ROWS][COLS], 
int row, int col, char command) {
    if (command == 'w') {
        board[row][col].base = WALL;
        board[row][col].box = 0;
        print_board(board, -1, -1);
    } else if (command == 's') {
        board[row][col].base = STORAGE;
        print_board(board, -1, -1);
    } else if (command == 'b') {
        if (board[row][col].base == WALL) {
            board[row][col].base = NONE;
        } 
        // reset base to nothing if wall and stays the same if base is storage
        board[row][col].box = 1;
        print_board(board, -1, -1);
    }
}


void extended_place (struct tile board[ROWS][COLS], enum base type, 
int row, int col, int end_row, int end_col) {
    int col_start = col;
    while (row <= end_row) {
        col = col_start;
        while (col < end_col) {
            board[row][col].base = type;
            col++;
        }
        board[row][col].base = type;
        row++;
    } 
    print_board(board, -1, -1);
}
//Ensures rows or columns of player access legal bounds by cycling across 
//the board
int board_wrap(int row) {
    if (row > 9) {
        row = row - ROWS;
    } else if (row < 0) {
        row = row + ROWS;
    }
    return row;
}

//Checks number of storage, boxes and stored boxes. If the number of stored 
//boxes <= storages ('<' to account for extra storages) and no box then return 1
int win_check(struct tile board[ROWS][COLS]) {
    int storage_num = 0;
    int stored_box = 0;
    int box_num = 0;
    int row = 0;
    while (row < ROWS) {
        int col = 0;
        while (col < COLS) {
            if (board[row][col].base == STORAGE) {
                storage_num++;
            } 
            if (board[row][col].base == STORAGE && board[row][col].box) {
                stored_box++;
            }
            if (board[row][col].base != STORAGE && board[row][col].box) {
                box_num++;
            }
            col++;
        }
        row++;
    }
    if (stored_box == 0 && storage_num == 0 && box_num == 0) {
        return 0;
    } else if (stored_box <= storage_num && box_num == 0) {
        return 1;
    } else {
        return 0;
    }
}
//Copies board
void board_copy(struct tile copy_board[ROWS][COLS], 
struct tile board[ROWS][COLS]) {
    int row = 0;
    while (row < ROWS) {
        int col = 0;
        while (col < COLS) {
            copy_board[row][col] = board[row][col];
            col++;
        }
        row++;
    }
}

//UP LEFT value -1
//DOWN RIGHT value +1
//1 if move is possible, 0 if not.
int move_possible(struct tile board[ROWS][COLS], int row, int col, 
int value, enum direction direction) {
    int temp = value;
    int i = 0;
    if (direction == VERTICAL) {
        //If there is a sequence of boxes, it detects boxes until there is
        //a wall
        while (board[board_wrap(row + value)][col].box == 1 && i < 10) {
            if (board[board_wrap(row + value + temp)][col].base == WALL) {
                return 0;
            }
            value += temp;
            i++;
        } 
        value = temp;
        //Detects if wall is in front
        if (board[board_wrap(row + value)][col].base == WALL) {
            return 0;
        } 
        return 1;
    } else if (direction == HORIZONTAL) {
        //If horizontal enum is passed in, the columns are changed instead.
        while (board[row][board_wrap(col + value)].box == 1 && i < 10) {
            if (board[row][board_wrap(col + value + temp)].base == WALL) {
                return 0;
            }
            value += temp;
            i++;
        } 
        value = temp;
        if (board[row][board_wrap(col + value)].base == WALL) {
            return 0;
        } 
        return 1;
    }
    return 1;
}

//UP LEFT value -1
//DOWN RIGHT value +1
void box_move(struct tile board[ROWS][COLS], int row, int col, 
int value, enum direction direction) {
    struct tile copy[ROWS][COLS];
    board_copy(copy, board);
    int i = 0;
    if (direction == VERTICAL) {
        //Detects if there is a row or col of boxes and adds the value
        //to the row of boxes to simulate being pushed
        while (copy[board_wrap(row)][col].box == 1) {
            if (i == 0) {
                //First box is removed because player is on the location
                //and box number stays the same 
                board[board_wrap(row)][col].box = 0;
            }
            board[board_wrap(row + value)][col].box = 1;
            row += value;
            i++;
        } 
    } else if (direction == HORIZONTAL) {
        while (copy[row][board_wrap(col)].box == 1) {
            if (i == 0) {
                board[row][board_wrap(col)].box = 0;
            }
            board[row][board_wrap(col + value)].box = 1;
            col += value;
            i++;
        } 
    }
}





////////////////////////////////////////////////////////////////////////////////
////////////////////////////// PROVIDED FUNCTIONS //////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// initialises the board to default values.
void init_board(struct tile board[ROWS][COLS]) {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            board[i][j].base = NONE;
            board[i][j].box = 0;
        }
    }
}

// Helper function for print_board().
// Prints a line the width of the sokoban board.
void print_line(void) {
    for (int i = 0; i < COLS * 4 + 1; i++) {
        printf("-");
    }
    printf("\n");
}


// Helper function for print_board().
// Prints out the title for above the sokoban board.
void print_title(void) {
    print_line();
    char *title = "S O K O B A N";
    int len = COLS * 4 + 1;
    int n_white = len - strlen(title) - 2;
    printf("|");
    for (int i = 0; i < n_white / 2; i++) {
        printf(" ");
    }
    printf("%s", title);
    for (int i = 0; i < (n_white + 1) / 2; i++) {
        printf(" ");
    }
    printf("|\n");
}

// Prints out the current state of the sokoban board.
// It will place the player on the board at position player_row, player_col.
// If player position is out of bounds, it won't place a player anywhere.
void print_board(
    struct tile board[ROWS][COLS], int player_row, int player_col) {
    print_title();
    for (int i = 0; i < ROWS; i++) {
        print_line();
        for (int j = 0; j < COLS; j++) {
            printf("|");

            struct tile curr = board[i][j];
            if (i == player_row && j == player_col) {
                printf("^_^");
            } else if (curr.base == WALL) {
                printf("===");
            } else if (curr.box && curr.base == STORAGE) {
                printf("[o]");
            } else if (curr.box) {
                printf("[ ]");
            } else if (curr.base == STORAGE) {
                printf(" o ");
            } else {
                printf("   ");
            }
        }
        printf("|\n");
    }
    print_line();
    printf("\n");
}
