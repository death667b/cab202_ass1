/*
Pong game with four levels

CAB202 Assignment 1 Semester 2 2016
Author:  Ian William Daniel
Student ID:  n5372828
*/


// Includes
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <cab202_graphics.h>
#include <cab202_sprites.h>
#include <cab202_timers.h>

// Configuration
#define DELAY 10
#define MIN_HEIGHT 10
#define MIN_WIDTH 60
#define HUMAN_PADDLE 0
#define COMPUTER_PADDLE 1
#define T_MINUTES 0
#define T_SECONDS 1
#define MOVE_DOWN 1
#define MOVE_UP -1

// Initial game settings
bool game_over = false, start_ball = true, count_down_timer = true;
char anykey_help_text[9];
int key, lives, score, level, timer, timer_old;
int seconds_counter = -1, minutes_counter = 0;
int paddle_max_y, paddle_min_y, paddle_height;
int starting_ball_x, starting_ball_y;
int start_level_three_time = 0;
int rails_width;

sprite_id paddle[2];
sprite_id ball;
sprite_id singularity;
sprite_id *rails_upper = NULL;
sprite_id *rails_lower = NULL;


// Function prototypes
void setup(void);
void process_loop(void);
void draw_levels(void);
void draw_help_screen(void);
void draw_boarder(bool display_menu);
void draw_info_panel(void);
void draw_singularity(void);
void screen_size_test(void);
void count_time(int * time_return);
void move_ball(void);
void move_paddle(sprite_id player, int direction);
void bounce_on_paddle_contact(int player);
void check_for_human_lose(void);
void move_computer_paddle(void);
void game_count_down(void);
void show_exit_screen(void);
void listen_keyboard(void);
void restart_round(void);
void reset_game(void);


void setup() {
	strcpy(anykey_help_text, "start");
	srand( get_current_time() );
	key = 'h';

	timer_old = 0;
	lives = 10;
	score = 0;
	level = 1;

	screen_size_test();

	//  Setup Humand and Computer Paddle

	char * paddle_img =
	/**/  "|"
	/**/  "|"
	/**/  "|"
	/**/  "|"
	/**/  "|"
	/**/  "|"
	/**/  "|";

	int panel_height = 4, paddle_width = 1;
	
	if (screen_height() >= 21) {
		paddle_height = 7;
	} else {
		paddle_height = (screen_height() - panel_height - 1) /2;
	}

	int human_paddle_x = screen_width() - 4, computer_paddle_x = 3;
	int paddle_start_y = (screen_height() - paddle_height) /2 + 1;

	paddle_min_y = 3;
	paddle_max_y = screen_height() - 1 - paddle_height;
	
	paddle[HUMAN_PADDLE] = sprite_create(
		human_paddle_x, 
		paddle_start_y, 
		paddle_width, 
		paddle_height, 
		paddle_img
	);
	paddle[COMPUTER_PADDLE] = sprite_create(
		computer_paddle_x, 
		paddle_start_y, 
		paddle_width, 
		paddle_height, 
		paddle_img
	);

	// Setup Ball

	char * ball_img = "o";

	starting_ball_x = screen_width() / 2;
	starting_ball_y = screen_height() / 2;

	int ball_width = 1,	ball_height = 1;

	ball = sprite_create( 
		starting_ball_x, 
		starting_ball_y, 
		ball_width, 
		ball_height,
		ball_img 
	);

	// Setup Singularity

	int sing_width = 9, sing_height = 5;
	int sing_x = (screen_width() - sing_width ) /2;
	int sing_y = screen_height() / 2 - 1;
	
	char * sing_img =
	/**/  " \\  |  / "
	/**/  "  \\ | /  "
	/**/  "--     --"
	/**/  "  / | \\  "
	/**/  " /  |  \\ ";

	singularity = sprite_create(
		sing_x, 
		sing_y, 
		sing_width, 
		sing_height, 
		sing_img
	);

	// Setup Rails

	rails_width = screen_width() /2;
	int rails_starting_x = rails_width - (rails_width/2);
	int single_rail_width = 1, single_rail_height = 1;
	int rails_y = round((screen_height() +4)/ 3);

	rails_upper = malloc(sizeof(sprite_id) * rails_width);
	rails_lower = malloc(sizeof(sprite_id) * rails_width);

	char * rails_img = "=";

	for (int i = 0; i < rails_width; i++) {
		rails_upper[i] = sprite_create(
			rails_starting_x+i,
			rails_y,
			single_rail_width,
			single_rail_height,
			rails_img
		);

		rails_lower[i] = sprite_create(
			rails_starting_x+i,
			(rails_y - single_rail_height) * 2,
			single_rail_width,
			single_rail_height,
			rails_img
		);
	}
}


int main (void){
	setup_screen();
	setup();

	while(!game_over){
		process_loop();

		timer_pause(DELAY);
	}

	show_exit_screen();

	return 0;
}

void process_loop(){
	clear_screen();

	if (key == 'h' || key == 'H'){
		draw_help_screen();
	}

	key = get_char();
	
	bool display_menu = true;
	draw_boarder(display_menu);
	draw_info_panel();
	draw_levels();
	game_count_down();

	sprite_draw( ball );
	move_ball();

	listen_keyboard();

	show_screen();
}


/**
* Draw Levels
* - Draw the the characteristics for the 4 different
*   levels
*
* @return void
*/
void draw_levels() {
	sprite_draw( paddle[HUMAN_PADDLE] );

	if (level > 1) {
		sprite_draw( paddle[COMPUTER_PADDLE] );
	}

	if (level == 3) {
		draw_singularity();
	}

	if (level == 4) {
		for (int i = 0; i < rails_width; i++) {
			sprite_draw(rails_upper[i]);
			sprite_draw(rails_lower[i]);
		}

	}
} // END draw_levels


/**
* Listen Keyboard
* - React to user input, game controls and quiting
*   
* @return void
*/
void listen_keyboard() {
	if (key == 's' || key == 'S') {
		move_paddle(paddle[HUMAN_PADDLE], MOVE_DOWN);
	}

	if (key == 'w' || key == 'W') {
		move_paddle(paddle[HUMAN_PADDLE], MOVE_UP);
	}

	if (key == 'l' || key == 'L') {
		if (level < 4){
			level++;			
		} else {
			level = 1;
		}

		restart_round();
	} 

	if (key == 'q' || key == 'Q') {
		game_over = true;
	}
} // END listen_keyboard


/**
* Draw Singularity
* - Draws the singularity sprite
* - Displays after a 5 second delay
*
* @return void
*/
void draw_singularity() {
	int time[2] = {0, 0};
	int sum_time, show_after = 5;
	count_time(time);

	sum_time = (time[T_MINUTES] * 60) + time[T_SECONDS];

	if (start_level_three_time == 0) {
		start_level_three_time = sum_time;
	} else {
		if (sum_time - start_level_three_time > show_after){
			sprite_draw(singularity);
		}
	}
} // END draw_singularity


/**
* Bounce on Paddle Contact
* - Calculate the location of the ball with respect to the
*   player's paddle.
* - If the ball touches the paddle's tips the ball will
*   bounce vertically
* - Otherwise it will bounce horizontally
* - If the paddle is next to or 1 space to the top or bottom
*   the ball will bounce hozizontally instead of vertically
* - If the ball touches the human paddle at all - increment  
*   score by one 
*
* @param int player number's paddle to check
*
* @return void
*/
void bounce_on_paddle_contact(int player) {
	int ball_x = round( sprite_x(ball) );
	int ball_y = round( sprite_y(ball) );

	int paddle_x = round( sprite_x(paddle[player]) );
	int paddle_y = round( sprite_y(paddle[player]) );

	// Test for any paddle contact
	if (ball_x == paddle_x && 
			ball_y <= paddle_y+paddle_height-1 &&
			ball_y >= paddle_y) {

		// Human score increase
		if (player == HUMAN_PADDLE) {
			score++;
		}

		double ball_dx = sprite_dx(ball);
		double ball_dy = sprite_dy(ball);

		int top_gap = 5, bottom_gap = screen_height() - 4;
		
		// Test for boundary paddle contact
		if ((ball_y == paddle_y && ball_y >= top_gap) || 
				(ball_y <= bottom_gap &&
				 ball_y == paddle_y+paddle_height-1)) {

			int move_ball_y = 1;
			ball_dy = -ball_dy;

			if (ball_y == paddle_y) {
				move_ball_y = -1;
			}

			sprite_back(ball);
			sprite_move(ball, 0, move_ball_y);

		// General contact
		} else {
			ball_dx = -ball_dx;

			sprite_back(ball);	
		}

		sprite_turn_to(ball, ball_dx, ball_dy );
	}
} // END bounce_on_paddle_contact


/**
* Reset Game
* - Reset the game after a human lose or level change
*
* @return void
*/
void reset_game() {
	timer_old = 0, timer = 0, lives = 10, score = 0, level = 1;
	seconds_counter = -1, minutes_counter = 0;
	restart_round();
} // END reset_game


/**
* Restart Round
* - Restarts the game after a human lose or level change
*
* @return void
*/
void restart_round() {
	sprite_move_to(ball, starting_ball_x, starting_ball_y);
	count_down_timer = true;
	start_ball = true;
	start_level_three_time = 0;
} // END restart_round


/**
* Check for Humun Losa
* - If the ball reachs the fall right screen
* 	remove one live
*
* @return void
*/
void check_for_human_lose() {
	int x = round( sprite_x(ball) );
	int right_wall_x = screen_width() - 1;

	// If the human has lost, restart round
	if (x == right_wall_x) {
		lives--;
		restart_round();
	}
} // END check_for_human_lose


void move_computer_paddle() {
	int ball_x = 3;
	int paddle_y = round( sprite_y(ball) - (paddle_height /2));

	if (paddle_min_y <= paddle_y && paddle_y <= paddle_max_y) {
		sprite_move_to(
			paddle[COMPUTER_PADDLE], 
			ball_x, 
			paddle_y);
	}
} // END move_computer_paddle


/**
* Game Count Down
* - Draws a box with a 3 half second count down at the beginning 
*   of a round
*
* @return void
*/
void game_count_down() {
	if (count_down_timer) {
		int counter_steps = 3, counter_delay = 300;
		int counter_x = (screen_width()  /2) - 2, counter_y;

		if (screen_height() >= 15) {
			counter_y = screen_height() /3;
		} else {
			counter_y = 5;
		}
		
		int box_width = 10, box_height = 5;

		sprite_id counter_box;

		char * counter_box_img =
		/**/  "*--------*"
		/**/  "|        |"
		/**/  "|        |"
		/**/  "|        |"
		/**/  "*--------*";

		counter_box = sprite_create(
			counter_x -3, 
			counter_y -2, 
			box_width, 
			box_height, 
			counter_box_img
		);

		do {
			sprite_draw(counter_box);
			draw_formatted(counter_x,counter_y,"%d...", counter_steps--);
			show_screen();
			timer_pause(counter_delay);
			get_char();
		} while (counter_steps > 0);

		count_down_timer = false;

		// Align the clock with true game time
		seconds_counter--;
	}
} // END game_count_down


/**
* Move Ball
* - Setup ball for the initial direction 
* - Move the ball and bounce off walls or paddles
*
* @return void
*/
void move_ball() {
	if (start_ball) {
		start_ball = false;
		int start_angle = 90;
		double angle = (rand() % start_angle) - start_angle /2;

		sprite_turn_to(ball, 0.2, 0 );
		sprite_turn(ball, angle);
	}

	sprite_step(ball);

	bounce_on_paddle_contact(HUMAN_PADDLE);
	check_for_human_lose();

	if (level > 1) {
		bounce_on_paddle_contact(COMPUTER_PADDLE);
		move_computer_paddle();
	}

	// General ball movement
	int ball_width = 1,	ball_height = 1;
	int panel_height = 2;

	int x = round( sprite_x(ball) );
	int y = round( sprite_y(ball) );

	double dx = sprite_dx(ball);
	double dy = sprite_dy(ball);

	bool dir_changed = false;

	if ( x == 0 || x == screen_width() - ball_width ) {
		dx = -dx;
		dir_changed = true;
	}

	if ( y == panel_height  || y == screen_height() - ball_height ) {
		dy = -dy;
		dir_changed = true;
	}

	if ( dir_changed ) {
		sprite_back(ball);
		sprite_turn_to(ball, dx, dy );
	}
} // END move_ball


/**
* Move Paddle
* - Move the provided paddle in the direction supplied
*   +1 to move down the page
*   -1 to move up the page
* - Movement will stop on the up and lower bounds
*
* @param sprite_id player Paddle to move up or down
* @param int direction Direction to move paddle +down -up
*
* @return void
*/
void move_paddle(sprite_id player, int direction) {
	int move_x = 0;
	int current_y = round(sprite_y(player)) + direction;

	if (paddle_min_y <= current_y && current_y <= paddle_max_y) {
		sprite_move(player, move_x, direction);
	}
} // END move_paddle


/**
*	Count Time
* - Simple function to measure game time
* - Only counts each second when being called
*
* @param *int time_return Int array with two elements 
* - int[0]  is seconds and int[1] is minutes
*/
void count_time(int * time_return) {
  timer = get_current_time();
  if (timer != timer_old){
  	timer_old = timer;
  	seconds_counter++;
  }

  if (seconds_counter == 60){
  	seconds_counter = 0;
  	minutes_counter++;
  }

  time_return[T_SECONDS] = seconds_counter;
  time_return[T_MINUTES] = minutes_counter;
} // END count_time


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
	int time[2] = {0, 0};
	
	count_time(time);

	draw_formatted(lives_x, panel_y, "Lives = %d", lives);
	draw_formatted(score_x, panel_y, "* Score = %d", score);
	draw_formatted(level_x, panel_y, "* Level = %d", level);
	draw_formatted(timer_x, panel_y, "* Time = %02d:%02d", time[T_MINUTES], time[T_SECONDS]);
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
	int left = 0, right = screen_width()-1;
	int top = 0, bottom = screen_height()-1;

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
	/**/	"CAB202 Assignment 1 - Pong"
	/**/	"    Ian William Daniel    "
	/**/	"   Student ID: n5372828   ";

	char* controls_help_text =
	/**/	"Controls:               "
	/**/	"W - Moves paddle up     "
	/**/	"S - Moves paddle down   "
	/**/	"L - Cycles though Levels"
	/**/	"H - For this help screen"
	/**/    "Q - Quit game           ";

  	int border_padding = 2, loop_key;
	int title_text_w = 26, title_text_h = 3, title_move_x = 0, title_move_y = 0;
	int controls_text_w = 24, controls_text_h = 6, controls_move_x = 0, controls_move_y = 0;
	int anykey_text_x = (screen_width() - border_padding - title_text_w / 2) - 2, anykey_move_x = -3;
	int anykey_text_y = screen_height() - border_padding, anykey_move_y = -1;
	
	if (screen_height() < 15) {
		title_move_x = -14;
		controls_move_x = 16;
		anykey_move_y = 0;
		anykey_move_x = 11;
	} else {
		title_move_y = -2;
		controls_move_y = 2;
	}

	int title_display_y = ((screen_height() - border_padding - title_text_h) /2) + title_move_y;
	int title_display_x = ((screen_width() - border_padding - title_text_w) /2) + title_move_x;

	int controls_display_y = ((screen_height() - border_padding - title_text_h) /2) + controls_move_y;
	int controls_display_x = ((screen_width() - border_padding - title_text_w) /2) + controls_move_x;

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
	draw_formatted(
		title_display_x + anykey_move_x, 
		anykey_text_y + anykey_move_y, 
		"Press any key to %s game...", 
		anykey_help_text
	);
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

	if (key == 'q' || key =='Q') {
		game_over = true;
		count_down_timer = false;
	}

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


/**
* Show Exit Screen
* - Simple function to display exiting screen before game exit
*
* @return void
*/
void show_exit_screen() {
	clear_screen();
	draw_string(screen_width() /2-12, (screen_height() /2),   "Thank you for playing...");
	show_screen();
	timer_pause(3000);
} //END show_exit_screen