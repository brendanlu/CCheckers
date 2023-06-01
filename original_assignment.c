
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <assert.h>

#include "consttypes.h"

/* my function prototypes ----------------------------------------------------*/
void fill_print_initial(board_t* board);
int char_to_col(char col);
int char_to_row(char row);
char col_to_char(int col);
char row_to_char(int row);
void print_board(board_t board_input);
int check_move_error_1_to_5(board_t board_input, 
	char col1, char row1, char col2, char row2, int black_action);
int outside_of_board(char col, char row);
char piece_at_location(board_t board_input, char col, char row);
void change_board(board_t* board, char col1, char row1, char col2, char row2);
int all_possible_moves(board_t board_input, char col, char row, 
	valid_moves_t valid_moves, int black_action);
int min(int a, int b);
void print_error_message(int error_code);
int board_cost(board_t board_input);
void print_move_information(int generated_move, int black_action, 
	board_t board_input, int col1, int row1, int col2, int row2, int action);
void generate_node_children(tree_node_t* node, int depth);
int movable_checker(int black_action, char checker);
void link_new_node(tree_node_t* parent_node,
	char col1, char row1, char col2, char row2, int depth);
void generate_tree_depth_3(tree_node_t* root_node);
int move_score_forced(tree_node_t* node);
void free_tree(tree_node_t* node);

/* main function controls all the action -------------------------------------*/
int
main(int argc, char *argv[]) {
    // YOUR IMPLEMENTATION OF STAGES 0-2
    // stage 0
    board_t* board = (board_t*)malloc(sizeof(board_t)); // main board pointer
    fill_print_initial(board);
    print_board(*board);
    
    int black_action = 1, action = 1, error; //, board_cost;
    char col1, row1, col2, row2; 
		// col1/row1 is source cell,  col2/row2 is target cell
		// deliberately kept separate, square_t type not used
	
	// scan input moves
	while (scanf("%c%c-%c%c\n", &col1, &row1, &col2, &row2)==4) {
		// check errors 1-5, terminate function if any found
		error = check_move_error_1_to_5(*board, col1, row1, col2, row2, 
			black_action);
		if (error) {
			print_error_message(error);
			return error;
		}
		// check error 6
			// all the possible moves of the source cell are generated
			// and the target cell is checked against those
		static valid_moves_t valid_moves;
		int num_possibles = all_possible_moves(*board, col1, row1, valid_moves, 
			black_action);
		int illegal = 1;
		for (int i=0;i<num_possibles;i++) {
			if ((valid_moves[i][0] == col2) && (valid_moves[i][1] == row2)) {
				illegal = 0;
			}
		}
		// if target_cell input is not in all legal moves, it must be illegal
			// return main function with error exit code 6
		if (illegal) {
			print_error_message(6);
			return 6;
		}
		
		change_board(board, col1, row1, col2, row2);
		
		print_move_information(0, black_action, *board, col1, row1, col2, row2, 
			action);
		
		print_board(*board);
		
		// change the action to opposite colour, iterate the move counts
		black_action = !black_action;
		action += 1;
	}
	
	char next_action = col1;
		// on last scanf call, col1 will pickup any trailing P or A instruction
			// naming isn't great, hence a reassignment to better variable name
	
	// stages 1 and 2
	int repititions=0; 
	if (next_action == 'A') { // stage 1
		repititions = 1;
	}
	if (next_action == 'P') { // stage 2
		repititions = 10;
	}
			
	for (int i=0; i<repititions; i++) {
		// stage 1
			// create the level 0 node first, and fill with relevent info.
		tree_node_t* level_0_node = (tree_node_t*)malloc(sizeof(tree_node_t));
		level_0_node->depth = 0;
		for (int i=0; i<BOARD_SIZE;i++) {
			for (int j=0; j<BOARD_SIZE; j++) {
				(level_0_node->board_state)[i][j] = (*board)[i][j];
			}
		}
		level_0_node->black_action = black_action;
		level_0_node->children_count = 0;
		level_0_node->children_list = initial_children;
		
		generate_tree_depth_3(level_0_node);
		
		// check if there isn't any possible moves, indicating game over
		if (level_0_node->children_count == 0) {
			if (black_action) {
				printf("%s WIN!\n", WHITE);
				return EXIT_SUCCESS;
			}
			else {
				printf("%s WIN!\n", BLACK);
				return EXIT_SUCCESS;
			}
		}
		
		// now we journey into the tree, and implement the minimax decision rule
		int best_score = move_score_forced(level_0_node); 
			// this is the best possible score we can have
		
		// find the choice node which yields this score
		linked_list_member_t* list_member = (level_0_node->children_list).first;
		tree_node_t* choice_node;
		for (int i=0; i<(level_0_node->children_count); i++) {
			choice_node = (tree_node_t*)(list_member->child_node);
			if (move_score_forced(choice_node) == best_score) {
				break;
			}
			list_member = list_member->next;
		}
		
		change_board(board, 
			(choice_node->move_info)[0][0], 
			(choice_node->move_info)[0][1],
			(choice_node->move_info)[1][0],
			(choice_node->move_info)[1][1]);
		
		print_move_information(1, black_action, *board, 
			(choice_node->move_info)[0][0], 
			(choice_node->move_info)[0][1], 
			(choice_node->move_info)[1][0], 
			(choice_node->move_info)[1][1], 
			action);
		
		print_board(*board);
		
		black_action = !black_action;
		action += 1;
		
		free_tree(level_0_node); // free space occupied by the tree
			// no need for it anymore
	}
	
	free(board);
    return EXIT_SUCCESS;            // exit program with the success code
}
// algorithms are fun

/* function definitions ------------------------------------------------------*/

// fill and print information on the initial board configuration
void fill_print_initial(board_t* board) {
	// specify initial setup 
	printf("BOARD SIZE: %dx%d\n", BOARD_SIZE, BOARD_SIZE);
	printf("#BLACK PIECES: %d\n", ORIGINAL_PIECES_COUNT);
	printf("#WHITE PIECES: %d\n", ORIGINAL_PIECES_COUNT);
	
	// fill board with the original board configuration
	for (int i=0; i<BOARD_SIZE; i++) {
		for (int j=0; j<BOARD_SIZE; j++) {
			int filled = 0; 
			for (int k=0; k<ORIGINAL_PIECES_COUNT; k++) {
				if ((char_to_col(white_initial_squares[k][0]) == i + 1) && 
					(char_to_row(white_initial_squares[k][1]) == j + 1)) {
						(*board)[i][j] = CELL_WPIECE;
						filled = 1;
				}
			}
			
			for (int k=0; k<ORIGINAL_PIECES_COUNT; k++) {
				if ((char_to_col(black_initial_squares[k][0]) == i + 1) && 
					(char_to_row(black_initial_squares[k][1]) == j + 1)) {
						(*board)[i][j] = CELL_BPIECE;
						filled = 1;
				}
			}
			
			if (filled == 0) {
				(*board)[i][j] = '0';
			}
		}
	}
}

// convert column coordinate character position into column number
int char_to_col(char col) { 
	int ascii_value;
	ascii_value = (int) col;
	return ascii_value - ASCII_A + 1; // offset from A + 1 to give col #
}

// convert row coordinate character position into row number
int char_to_row(char row) { 
	int ascii_value;
	ascii_value = (int) row;
	return ascii_value - ASCII_1 + 1; // offset from 1 + 1 to give row #
}

// reverse char_to_col
char col_to_char(int col) {
	return (char) (ASCII_A + col - 1);
}

// reverse char_to_row
char row_to_char(int row) {
	return (char) (ASCII_1 + row - 1);
}

// print a nice visual representation of the board given a board_t input
void print_board(board_t board_input) {
	printf("     A   B   C   D   E   F   G   H"); 
	// note: main loop iterating through board row, sub loop iterates column
		// hence switched around iterating variables for clarity
	for (int j=0; j<BOARD_SIZE; j++) { 
		for (int i=0; i<BOARD_SIZE; i++) { 
			if (i==0) { 
				printf("\n   +---+---+---+---+---+---+---+---+\n");
				printf(" %d |", j + 1);
			}
			if (board_input[i][j] == CELL_EMPTY) {
				printf(" . |");
			}
			else {
				printf(" %c |", board_input[i][j]);
			}
		}
	}
	printf("\n   +---+---+---+---+---+---+---+---+\n");
}

// lazy evaluation, systematically check for move errors 1-5
	// if any found, print error message and return error code
int check_move_error_1_to_5(board_t board_input, 
	char col1, char row1, char col2, char row2, int black_action) {
	// error 1
	if (outside_of_board(col1, row1)) {
		return 1;
	}
	// error 2
	if (outside_of_board(col2, row2)) {
		return 2;
	}
	// error 3
	if (piece_at_location(board_input, col1, row1) == CELL_EMPTY) {
		return 3;
	}
	// error 4
	if (piece_at_location(board_input, col2, row2) != CELL_EMPTY) {
		return 4;
	}
	// error 5 
	if (black_action) {
		if ((piece_at_location(board_input, col1, row1) == CELL_WPIECE)
			|| 
		(piece_at_location(board_input, col1, row1) == CELL_WTOWER)) {
			return 5;
		}
	}
	else {
		if ((piece_at_location(board_input, col1, row1) == CELL_BPIECE)
			|| 
		(piece_at_location(board_input, col1, row1) == CELL_BTOWER)) {
			return 5;
		}
	}
	return NO_ERRORS_1_TO_5;
}

// check if a col/row input is outside of the board
int outside_of_board(char col, char row) {
	if ((char_to_col(col) < 1) || (char_to_col(col) > BOARD_SIZE) 
		|| (char_to_row(row) < 1) || (char_to_row(row) > BOARD_SIZE)) {
		return 1;
	}
	else {
		return 0;
	}
}

// return piece at a given character col/row input for a board_t input
char piece_at_location(board_t board_input, char col, char row) {
	return board_input[char_to_col(col) - 1][char_to_row(row) - 1];
}

// adjust a board_t variable with a given input move
void change_board(board_t* board, char col1, char row1, char col2, char row2) {
	char being_moved_temp = piece_at_location(*board, col1, row1);
	
	// make source cell empty
	(*board)[char_to_col(col1) - 1][char_to_row(row1) - 1] = CELL_EMPTY;
	
	// consider, if piece reached end, it must be promoted
	if ((being_moved_temp == CELL_BPIECE) && (row2 == END_ROW_BLACK)) {
		(*board)[char_to_col(col2) - 1][char_to_row(row2) - 1]
			= CELL_BTOWER;
	}
	else if ((being_moved_temp == CELL_WPIECE) && (row2 == END_ROW_WHITE)) {
		(*board)[char_to_col(col2) - 1][char_to_row(row2) - 1]
			= CELL_WTOWER;
	}
	// otherwise make target cell the piece being moved
	else {
		(*board)[char_to_col(col2) - 1][char_to_row(row2) - 1] 
			= being_moved_temp;
	}
	// if capturing, captured middle piece must be removed as well
	if (abs(char_to_col(col2) - char_to_col(col1)) == CAPTURE_JUMP) {
		int middle_col = min(char_to_col(col1), char_to_col(col2)) + 1;
		int middle_row = min(char_to_row(row1), char_to_row(row2)) + 1;
		(*board)[middle_col - 1][middle_row - 1] = CELL_EMPTY;
	}
}

// function which returns the number of valid target squares a piece at 
// source defined by col/row can make, given a board_t input. 
// Also writes each into a static variable of type valid_moves_t
	// sorry for lack of abstraction, very confusing  with different static 
	// and local variables when abstracted into a function
	// so ultimately deemed better in this format
int all_possible_moves(board_t board_input, char col, char row, 
	valid_moves_t valid_moves, int black_action) {
	int num_possibles = 0, is_tower = 0;
	if ((piece_at_location(board_input, col, row) == CELL_BTOWER)
		|| (piece_at_location(board_input, col, row) == CELL_WTOWER)) {
		is_tower = 1;
	}
	
	// iterate through possible tower movement vectors
	if (is_tower) {
		for (int i=0; i<NUM_TOWER_MOVEMENTS; i++) {
			int possible_col1, possible_row1;
			possible_col1 = char_to_col(col) + tower_movements[i][0];
			possible_row1 = char_to_row(row) + tower_movements[i][1];
			char p_col1, p_row1;
			p_col1 = col_to_char(possible_col1);
			p_row1 = row_to_char(possible_row1);
			
			int error_check = check_move_error_1_to_5(board_input, 
				col, row, p_col1, p_row1, black_action);
			
			// if no issue with movement, add to valid_moves_t static variable
			if (error_check == NO_ERRORS_1_TO_5) {
				valid_moves[num_possibles][0] = p_col1;
				valid_moves[num_possibles][1] = p_row1;
				num_possibles += 1;
			}
			// otherwise, if target cell occupied by opposing piece, 
				// capture move possible
			int capture_possible = 0;
			if (error_check == 4) {
				if (black_action) {
					if ((piece_at_location(board_input, p_col1, p_row1) 
						== CELL_WPIECE)
						|| 
					(piece_at_location(board_input, p_col1, p_row1) 
						== CELL_WTOWER)) {
						capture_possible = 1;
					}
				}
				else {
					if ((piece_at_location(board_input, p_col1, p_row1) 
						== CELL_BPIECE)
						|| 
					(piece_at_location(board_input, p_col1, p_row1) 
						== CELL_BTOWER)) {
						capture_possible = 1;
					}
				}
			}
			if (capture_possible) {
				int possible_col2, possible_row2;
				possible_col2 = char_to_col(col) + 
					(CAPTURE_JUMP * tower_movements[i][0]);
				possible_row2 = char_to_row(row) + 
					(CAPTURE_JUMP * tower_movements[i][1]);
				char p_col2, p_row2;
				p_col2 = col_to_char(possible_col2);
				p_row2 = row_to_char(possible_row2);
				
				int error_check_2 = check_move_error_1_to_5(board_input, 
					col, row, p_col2, p_row2, black_action);
				
				if (error_check_2 == NO_ERRORS_1_TO_5) {
					valid_moves[num_possibles][0] = p_col2;
					valid_moves[num_possibles][1] = p_row2;
					num_possibles += 1;
				}
			}
		}
	}
	
	// iterate through possible black piece movement vectors
	if (piece_at_location(board_input, col, row) == CELL_BPIECE) {
		for (int i=0; i<NUM_PIECE_MOVEMENTS; i++) {
			int possible_col1, possible_row1;
			possible_col1 = char_to_col(col) + black_piece_movements[i][0];
			possible_row1 = char_to_row(row) + black_piece_movements[i][1];
			char p_col1, p_row1;
			p_col1 = col_to_char(possible_col1);
			p_row1 = row_to_char(possible_row1);
			
			int error_check = check_move_error_1_to_5(board_input, 
				col, row, p_col1, p_row1, black_action);
			
			// if no issue with movement, add to valid_moves_t static variable
			if (error_check == NO_ERRORS_1_TO_5) {
				valid_moves[num_possibles][0] = p_col1;
				valid_moves[num_possibles][1] = p_row1;
				num_possibles += 1;
			}
			
			int capture_possible = 0;
			if (error_check == 4) {
				if (black_action) {
					if ((piece_at_location(board_input, p_col1, p_row1) 
						== CELL_WPIECE)
						|| 
					(piece_at_location(board_input, p_col1, p_row1) 
						== CELL_WTOWER)) {
						capture_possible = 1;
					}
				}
				else {
					if ((piece_at_location(board_input, p_col1, p_row1) 
						== CELL_BPIECE)
						|| 
					(piece_at_location(board_input, p_col1, p_row1) 
						== CELL_BTOWER)) {
						capture_possible = 1;
					}
				}
			}
			if (capture_possible) {
				int possible_col2, possible_row2;
				possible_col2 = char_to_col(col) + 
					(CAPTURE_JUMP * black_piece_movements[i][0]);
				possible_row2 = char_to_row(row) + 
					(CAPTURE_JUMP * black_piece_movements[i][1]);
				char p_col2, p_row2;
				p_col2 = col_to_char(possible_col2);
				p_row2 = row_to_char(possible_row2);
				
				int error_check_2 = check_move_error_1_to_5(board_input, 
					col, row, p_col2, p_row2, black_action);
				
				if (error_check_2 == NO_ERRORS_1_TO_5) {
					valid_moves[num_possibles][0] = p_col2;
					valid_moves[num_possibles][1] = p_row2;
					num_possibles += 1;
				}
			}
		}
	}
	
	// iterate through possible white piece movement vectors
	if (piece_at_location(board_input, col, row) == CELL_WPIECE) {
		for (int i=0; i<NUM_PIECE_MOVEMENTS; i++) {
			int possible_col1, possible_row1;
			possible_col1 = char_to_col(col) + white_piece_movements[i][0];
			possible_row1 = char_to_row(row) + white_piece_movements[i][1];
			char p_col1, p_row1;
			p_col1 = col_to_char(possible_col1);
			p_row1 = row_to_char(possible_row1);
			
			int error_check = check_move_error_1_to_5(board_input, 
				col, row, p_col1, p_row1, black_action);
			
			// if no issue with movement, add to valid_moves_t static variable
			if (error_check == NO_ERRORS_1_TO_5) {
				valid_moves[num_possibles][0] = p_col1;
				valid_moves[num_possibles][1] = p_row1;
				num_possibles += 1;
			}
			
			int capture_possible = 0;
			if (error_check == 4) {
				if (black_action) {
					if ((piece_at_location(board_input, p_col1, p_row1) 
						== CELL_WPIECE)
						|| 
					(piece_at_location(board_input, p_col1, p_row1) 
						== CELL_WTOWER)) {
						capture_possible = 1;
					}
				}
				else {
					if ((piece_at_location(board_input, p_col1, p_row1) 
						== CELL_BPIECE)
						|| 
					(piece_at_location(board_input, p_col1, p_row1) 
						== CELL_BTOWER)) {
						capture_possible = 1;
					}
				}
			}
			if (capture_possible) {
				int possible_col2, possible_row2;
				possible_col2 = char_to_col(col) + 
					(CAPTURE_JUMP * white_piece_movements[i][0]);
				possible_row2 = char_to_row(row) + 
					(CAPTURE_JUMP * white_piece_movements[i][1]);
				char p_col2, p_row2;
				p_col2 = col_to_char(possible_col2);
				p_row2 = row_to_char(possible_row2);
				
				int error_check_2 = check_move_error_1_to_5(board_input, 
					col, row, p_col2, p_row2, black_action);
				
				if (error_check_2 == NO_ERRORS_1_TO_5) {
					valid_moves[num_possibles][0] = p_col2;
					valid_moves[num_possibles][1] = p_row2;
					num_possibles += 1;
				}
			}
		}
	}
	
	return num_possibles;
}

// simple function to return the smaller of two integers, 
	// if it is the same, it returns the number
int min(int a, int b) {
	if (a < b) {
		return a;
	}
	else {
		return b;
	}
}

// simple function to print various error messages
void print_error_message(int error_code) {
	if (error_code == 1) {
		printf("ERROR: Source cell is outside of the board.\n");
	}
	if (error_code == 2) {
		printf("ERROR: Target cell is outside of the board.\n");
	}
	if (error_code == 3) {
		printf("ERROR: Source cell is emtpy.\n");
	}
	if (error_code == 4) {
		printf("ERROR: Target cell is not empty.\n");
	}
	if (error_code == 5) {
		printf("ERROR: Source cell holds opponent's piece/tower.\n");
	}
	if (error_code == 6) {
		printf("ERROR: Illegal action.\n");
	}
}

// function to return board cost for board_t input
int board_cost(board_t board_input) {
	int b_count=0, B_count=0, w_count=0, W_count=0;
	for (int i=0; i<BOARD_SIZE;i++) {
		for (int j=0; j<BOARD_SIZE; j++) {
			if (board_input[i][j] == CELL_BPIECE) {
				b_count += 1;
			}
			if (board_input[i][j] == CELL_BTOWER) {
				B_count += 1;
			}
			if (board_input[i][j] == CELL_WPIECE) {
				w_count += 1;
			}
			if (board_input[i][j] == CELL_WTOWER) {
				W_count += 1;
			}
		}
	}
	int cost = b_count + (3 * B_count) - w_count - (3 * W_count);
	return cost;
}

//print some headers regarding move information
void print_move_information(int generated_move, int black_action, 
	board_t board_input, int col1, int row1, int col2, int row2, int action) {
	printf("=====================================\n");
	if (generated_move) {
		printf("*** ");
	}
	if (black_action) {
		printf("%s ACTION #%d: %c%c-%c%c\n", 
			BLACK, action, col1, row1, col2, row2);
	}
	else {
		printf("%s ACTION #%d: %c%c-%c%c\n", 
			WHITE, action, col1, row1, col2, row2);
	}
	printf("BOARD COST: %d\n", board_cost(board_input));
}

// find and link node children for a given tree/subtree root 
void generate_node_children(tree_node_t* node, int depth) {
	// iterate through board to find checkers belonging to the player 
		// note: done in row-major order
	int generated_children = 0;
	for (int j=0; j<BOARD_SIZE;j++) {
		for (int i=0; i<BOARD_SIZE; i++) {
			char checker = (node->board_state)[i][j];
			if (movable_checker(node->black_action, checker)) {
				// if a checker belonging to the colour with action is found
					// generate all valid moves and create a tree node for each
				static valid_moves_t valid_moves; 
				char col = col_to_char(i+1);
				char row = row_to_char(j+1);
				int num_possibles = all_possible_moves((node->board_state), 
					col, row, valid_moves, (node->black_action));
				for (int k=0; k<num_possibles; k++) {
					// define the source cell and target cell
					char col1 = col;
					char row1 = row;
					char col2 = valid_moves[k][0];
					char row2 = valid_moves[k][1];
					
					link_new_node(node, col1, row1, col2, row2, depth);
					generated_children += 1;
				}
			}
		}
	}
	node->children_count = generated_children;
		// update children count; useful for accessing tree later
}

// check whether a given square contains a movable checker depending on the turn
int movable_checker(int black_action, char checker) {
	if ((black_action) && ((checker == CELL_BPIECE) || 
			(checker == CELL_BTOWER))) {
		return 1;
	}
	else if ((!black_action) && ((checker == CELL_WPIECE) || 
			(checker == CELL_WTOWER))) {
		return 1;
	}
	else {
		return 0;
	}
}

void link_new_node(tree_node_t* parent_node,
	char col1, char row1, char col2, char row2, int depth) {
	// create a new tree node ready to link
		// first copy over new board after move is made
	tree_node_t* new_node = (tree_node_t*)malloc(sizeof(tree_node_t));
	new_node->depth = depth;
	for (int i=0; i<BOARD_SIZE; i++) {
		for (int j=0; j<BOARD_SIZE; j++) {
			(new_node->board_state)[i][j] = (parent_node->board_state)[i][j];
		}
	}
	change_board((board_t*)(&(new_node->board_state)), col1, row1, col2, row2);
	
	new_node->move_info[0][0] = col1;
	new_node->move_info[0][1] = row1;
	new_node->move_info[1][0] = col2;
	new_node->move_info[1][1] = row2;
	new_node->black_action = !(parent_node->black_action);
		// change of turn
	new_node->children_list = initial_children;
	new_node->children_count = 0;
	
	// create a list member to contain the address of the new node, 
		// which is dynamically linked to children_list of the parent node
	linked_list_member_t* new_list_member
		= (linked_list_member_t*)malloc(sizeof(linked_list_member_t));
	new_list_member->child_node = (void*)new_node; // raw address of new node
	new_list_member->next = NULL;
	
	// link the new list member to the parent node children_list
	if ((parent_node->children_list).first == NULL) {
		(parent_node->children_list).first = new_list_member;
	}
	
	if ((parent_node->children_list).last == NULL) {
		(parent_node->children_list).last = new_list_member;
	}
	else {
		((parent_node->children_list).last)->next = new_list_member;
		(parent_node->children_list).last = new_list_member;
	}
}

// generate a tree of depth 3 containing the tree_node's 
void generate_tree_depth_3(tree_node_t* level_0_node) {
	// generate the first generation children
	generate_node_children(level_0_node, 1);
	
	// generate second generation children for each first generation child
	linked_list_member_t* list_member_1 = (level_0_node->children_list).first;
	tree_node_t* level_1_node;
	for (int i=0; i<(level_0_node->children_count); i++) {
		level_1_node = (tree_node_t*)(list_member_1->child_node);
		generate_node_children(level_1_node, 2);
		
		// generate third generation children for each second generation child
		linked_list_member_t* list_member_2 
			= (level_1_node->children_list).first;
		tree_node_t* level_2_node;
		for (int j=0; j<(level_1_node->children_count); j++) {
			level_2_node = (tree_node_t*)(list_member_2->child_node);
			generate_node_children(level_2_node, 3);
			list_member_2 = list_member_2->next;
		}
		
		list_member_1 = list_member_1->next;
	}
}

// recursively find the forced best score of an option branch
	// applies the minimax decision rule and assumes player rationality
	// according to this rule
int move_score_forced(tree_node_t* node) {
	// base case, we have reached a leaf
	if (node->children_count == 0) {
		// note: if the leaf is less than depth 3, we return int_min/max
			// as specified in the assignment
		if (node->depth != 3) {
			if (node->black_action) {
				return INT_MIN;
			}
			else {
				return INT_MAX;
			}
		}
		
		int leaf_cost = board_cost(node->board_state);
		return leaf_cost;
	}
	
	// recursive case
	else {
		int max_score = INT_MIN, min_score = INT_MAX;
		linked_list_member_t* list_member = (node->children_list).first;
		tree_node_t* child_node;
		// iterate through children to find scores
		for (int i=0; i<(node->children_count); i++) {
			child_node = (tree_node_t*)(list_member->child_node);
			int child_score = move_score_forced(child_node);
			if (child_score > max_score) {
				max_score = child_score;
			}
			if (child_score < min_score) {
				min_score = child_score;
			}
			list_member = list_member->next;
		}
		
		// black and white make rational actions!!
		if (node->black_action) {
			return max_score;
		}
		else {
			return min_score;
		}
	}
}

// free up the space taken by the tree, and the linked lists in the nodes
	// done recursively
void free_tree(tree_node_t* node) {
	// base case, node is leaf
	if (node->children_count == 0) {
		free(node);
	}
	// recursive case, have to recurse down to the leaf of the node first
	else {
		// we need to kill each child node...that sounds horrible...
			// 'free' each child node...better...
		linked_list_member_t* list_member = (node->children_list).first;
			// list_member is used as an iterating item
		tree_node_t* child_node;
		for (int i=0; i<(node->children_count); i++) {
			child_node = (tree_node_t*)(list_member->child_node);
			free_tree(child_node);
			
			// once we free the subtree fathered by the child
				// we are free to unlink the list member too
			linked_list_member_t* redundant_list_member = list_member;
			list_member = list_member->next;
			free(redundant_list_member);
		}
	}
}

// algorithms are fun
/* THE END -------------------------------------------------------------------*/