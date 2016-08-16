/*
Pong game with four levels

CAB202 Assignment 1 Semester 2 2016
Author:  Ian William Daniel
Student ID:  n5372828
*/


// Includes
#include <string.h>
#include <math.h>
#include <cab202_graphics.h>
#include <cab202_sprites.h>
#include <cab202_timers.h>

// Configuration
#define DELAY (10)
#define MIN_HEIGHT (10)
#define MIN_WIDTH (60)
#define HUMAN_PADDLE (0)
#define COMPUTER_PADDLE (1)

// Initial game settings
bool game_over = false;
char anykey_help_text[9];
int key, screen_h, screen_w;
int lives, score, level;
sprite_id paddle[2];


// Function prototypes
void setup(void);
void process_loop(void);
void draw_help_screen(void);
void draw_boarder(bool display_menu);
void draw_info_panel(void);
void screen_size_test(void);


/**********************************************************************   REMOVE screen_h and screen_w  ******************/
void setup(){
	strcpy(anykey_help_text, "start");
	key = 'h';
	screen_h = screen_height();
	screen_w = screen_width();

	lives = 10;
	score = 0;
	level = 1;

	screen_size_test();

	char * paddle_img =
	/**/  "|"
	/**/  "|"
	/**/  "|"
	/**/  "|"
	/**/  "|"
	/**/  "|"
	/**/  "|";

	int paddle_height = 7, panel_height = 2;
	int human_paddle_x = screen_width() - 4;
	int computer_paddle_x = 3;
	int paddle_start_y = (screen_height() - paddle_height) /2 + panel_height;
	
	paddle[HUMAN_PADDLE] = sprite_create(
		human_paddle_x, 
		paddle_start_y, 
		1, 
		paddle_height, 
		paddle_img
	);
	paddle[COMPUTER_PADDLE] = sprite_create(
		computer_paddle_x, 
		paddle_start_y, 
		1, 
		paddle_height, 
		paddle_img
	);
}


int main (void){
	setup_screen();
	setup();

	while(!game_over){
		process_loop();

		timer_pause(DELAY);
	}

	return 0;
}

void process_loop(){
	clear_screen();

	if (key == 'h' || key == 'H'){
		// pause game time
		draw_help_screen();
		// resume game time
	}

	key = get_char();
	
	bool display_menu = true;
	draw_boarder(display_menu);
	draw_info_panel();

	sprite_draw(paddle[HUMAN_PADDLE]);
	sprite_draw(paddle[COMPUTER_PADDLE]);

	// if(key) level toggle
	//  - reset game time if level changed

	// if(key) human paddle up/down

	// *****************
	// draw human paddle
	// draw computer paddle
	// - comp paddle's y value is equal to ball y value
	// paddles need to be created as an array
	// define human 0 and computer 1 for paddle selection
	// *****************

	// draw ball

	show_screen();
}


/**
* Draw Info Panel
* - Add lives, score, current level and time passed at 1/4 increments
*   across the screen.
*
* @return void
*/
void draw_info_panel() {
	int panel_y = 1;
	int lives_x = 2;
	int score_x = round((screen_width() / 4));
	int level_x = round((screen_width() / 4) * 2);
	int timer_x = round((screen_width() / 4) * 3);
/************************************************************* Setup the timer *****************************************/
	draw_formatted(lives_x, panel_y, "Lives = %d", lives);
	draw_formatted(score_x, panel_y, "* Score = %d", score_x);
	draw_formatted(level_x, panel_y, "* Level = %d", level);
	draw_string(timer_x, panel_y, "* Time = 0:49");
} // END draw_info_panel


/**
* Draw Boarder
* - Draws a board for both the help screen and an active game
*
* @param bool display_menu: true will add a section for game details
*
* @return void
*/
void draw_boarder(bool display_menu){
	int left = 0, right = screen_w-1;
	int top = 0, bottom = screen_h-1;

	draw_line(left,  top,    right,  top,    '*');
	draw_line(left,  top,    left,   bottom, '*');
	draw_line(left,  bottom, right,  bottom, '*');
	draw_line(right, top,    right,  bottom, '*');

	if (display_menu){
		draw_line(left, top + 2, right, top + 2, '*');
	}
}// END draw_boarder


/**
*	Draw help screen
* - Changed the location of the title text and help text depending 
*   the height of the initial window.
*
* @return void
*/
void draw_help_screen(){
	// Init variables
	sprite_id sprite_title_help_text, sprite_controls_help_text;

	char * title_help_text =
	/**/	"Multi Level Pong v1.0"
	/**/	"by Ian William Daniel"
	/**/	"Student ID: n5372828 ";

	char* controls_help_text =
	/**/	"Controls:               "
	/**/	"W - Moves paddle up     "
	/**/	"S - Moves paddle down   "
	/**/	"L - Cycles though Levels"
	/**/	"H - For this help screen";

  int border_padding = 2, loop_key;
	int title_text_w = 21, title_text_h = 3, title_move_x = 0, title_move_y = 0;
	int controls_text_w = 24, controls_text_h = 5, controls_move_x = 0, controls_move_y = 0;
	int anykey_text_x = (screen_w - border_padding - title_text_w / 2) - 2, anykey_move_x = -3;
	int anykey_text_y = screen_h - border_padding, anykey_move_y = -2;
	
	if (screen_h < 15) {
		title_move_x = -14;
		controls_move_x = 14;
		anykey_move_y = 0;
		anykey_move_x = 11;
	} else {
		title_move_y = -2;
		controls_move_y = 2;
	}

	int title_display_y = ((screen_h - border_padding - title_text_h) /2) + title_move_y;
	int title_display_x = ((screen_w - border_padding - title_text_w) /2) + title_move_x;

	int controls_display_y = ((screen_h - border_padding - title_text_h) /2) + controls_move_y;
	int controls_display_x = ((screen_w - border_padding - title_text_w) /2) + controls_move_x;

	// Create Sprites
	sprite_title_help_text = sprite_create(
		title_display_x, 
		title_display_y, 
		title_text_w, 
		title_text_h, 
		title_help_text
	);
	sprite_controls_help_text = sprite_create(
		controls_display_x, 
		controls_display_y, 
		controls_text_w, 
		controls_text_h, 
		controls_help_text
	);

	// Draw spite text, and key to proceed and border with out menu
	sprite_draw(sprite_title_help_text);
	sprite_draw(sprite_controls_help_text);
	draw_formatted(title_display_x + anykey_move_x, anykey_text_y + anykey_move_y, "Press any key to %s game...", anykey_help_text);
	bool display_menu = false;
	draw_boarder(display_menu);
	show_screen();

	// Keep function in a loop until the user is ready to play game
	do{
		// Note:  no key press returns a negitive number
		loop_key = get_char();

		if (loop_key > 0) {
			key = loop_key;
		}
	} while (key == 'h' || key == 'H');

	strcpy(anykey_help_text, "continue");
	clear_screen();
}  // END draw_help_screen


/**
* Screen Size Test
* - The minimum allowable screen size is width of 60 and height of 10
*   If failing this, quits the program
*
* @return void
*/
void screen_size_test(){
	if (screen_width() < MIN_WIDTH || screen_height() < MIN_HEIGHT){
		draw_string(screen_width() /2-22, (screen_height() /2)-2, "You screen size is too small");
		draw_string(screen_width() /2-22, (screen_height() /2)-1, "It needs to be a minium 60 wide and 10 high");
		draw_string(screen_width() /2-22, (screen_height() /2),   "Press any key to exit");
		show_screen();

		game_over = true;
		key = wait_char();
	}
} // END screen_size_test