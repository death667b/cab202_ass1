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
#define DELAY (10)
#define MIN_HEIGHT (10)
#define MIN_WIDTH (60)
#define HUMAN_PADDLE (0)
#define COMPUTER_PADDLE (1)
#define T_MINUTES (0)
#define T_SECONDS (1)
#define MOVE_DOWN (1)
#define MOVE_UP (-1)
#define NO_REFLECT (0)
#define REFLECT_X (1)
#define REFLECT_Y (2)


// Global Variables
bool game_over = false, initialize_ball = true, count_down_timer = true;
bool singularity_active = false;
char anykey_help_text[9];
int key, lives, score, level, timer, timer_old;
int seconds_counter = -1, minutes_counter = 0;
int paddle_max_y, paddle_min_y, paddle_height;
int human_paddle_x, computer_paddle_x, paddle_start_y;
int starting_ball_x, starting_ball_y;
int start_level_three_time = 0;
int sing_x, sing_y, sing_width, sing_height;
int rails_width;

sprite_id paddle[2];
sprite_id ball;
sprite_id counter_box;
sprite_id singularity;
sprite_id *rails_upper = NULL;
sprite_id *rails_lower = NULL;


// Function prototypes
void setup(void);
void init_paddles(void);
void init_ball(void);
void init_singularity(void);
void init_rails(void);
void init_countdown_box(void);
void process_loop(void);
void draw_levels(void);
void draw_help_screen(void);
void draw_boarder(bool display_menu);
void draw_info_panel(void);
void draw_singularity(void);
void singularity_effect(void);
void screen_size_test(void);
void count_time(int * time_return);
void move_ball(void);
void reset_rails(void);
void check_for_wall_bounce(void);
void move_paddle(sprite_id player, int direction);
void bounce_on_paddle_contact(int player);
void check_for_human_lose(void);
void move_computer_paddle(void);
void game_count_down(void);
void show_exit_screen(void);
void ball_acceleration(void);
void listen_keyboard(void);
void level_change(void);
void restart_round(void);
void reset_game(void);
void realign_ball_with_paddle();
void rail_collision(void);
void hide_rail(sprite_id rail[], int contact);
void game_lost(void);
bool is_end_of_rail (sprite_id rail[], int contact);
int rail_ball_collision_status(sprite_id rail[]);


int main (void){
	setup_screen();
	setup();

	while(!game_over){
		process_loop();

		timer_pause(DELAY);
	}

	show_exit_screen();

	return 0;
} // END Main


/**
* Setup
* - High level function to setup the game
*
* @return void
*/
void setup() {
	strcpy(anykey_help_text, "start");
	srand( get_current_time() );
	key = 'h';

	timer_old = 0;
	lives = 10;
	score = 0;
	level = 1;

	screen_size_test();

	init_paddles();
	init_ball();
	init_singularity();
	init_rails();
	init_countdown_box();
} // END setup


/**
* Process Loop
* - High level function that runs the game loop
*
* @return void
*/
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

	singularity_effect();
	rail_collision();

	listen_keyboard();

	show_screen();
} // END process_loop


/**
* Rail Collision
* - Calculate the ball location with respect to the rails
* - Reflect ball in the required direction
*
* @return void
*/
void rail_collision(void) {
	if (level == 4) {
		int reflect_direction_upper = NO_REFLECT;
		int reflect_direction_lower = NO_REFLECT;

		double ball_dx = sprite_dx(ball);
		double ball_dy = sprite_dy(ball);

		reflect_direction_upper = rail_ball_collision_status(rails_upper);
		reflect_direction_lower = rail_ball_collision_status(rails_lower);

		if (reflect_direction_upper == REFLECT_Y || reflect_direction_lower == REFLECT_Y) {
			ball_dy = -ball_dy;
			sprite_back(ball);
			sprite_turn_to(ball, ball_dx, ball_dy );
		} else if (reflect_direction_upper == REFLECT_X || reflect_direction_lower == REFLECT_X) {
			ball_dx = -ball_dx;
			sprite_back(ball);
			sprite_turn_to(ball, ball_dx, ball_dy );
		}
	}
} // END rail_collision


/**
* Rail Ball Collision Status
* - Test if the ball has come into contact with visible rails
*
* @return bool
*/
int rail_ball_collision_status(sprite_id rail[]) {
	int reflect_direction = NO_REFLECT;
	int rail_to_left = -1, rail_to_right = 1;

	int ball_x = round( sprite_x(ball) );
	int ball_y = round( sprite_y(ball) );

	int left_x_rail = sprite_x(rail[0]);
	int y_rail = sprite_y(rail[0]);
	int right_x_rail = sprite_x(rail[rails_width-1]);

	if (ball_x <= right_x_rail && 
		ball_x >= left_x_rail && 
		ball_y == y_rail) {

		int contact = ball_x - left_x_rail;

		if (sprite_visible(rail[contact])){
			if (is_end_of_rail(rail, contact)) {
				reflect_direction = REFLECT_X;
			} else {
				reflect_direction = REFLECT_Y;
			}

			hide_rail(rail, contact);
		}
	}

	return reflect_direction;
} // END rail_ball_collision_status


/**
* Hide Rail
* - Hides rails on either side of the ball contact
* - If the rail exist and visible, it will hide it
* Note:  This is required to prevent seg faults
*
* @return void
*/
void hide_rail(sprite_id rail[], int contact) {
	int min_element = 0;
	int max_element = rails_width-1;

	int from_rail_number = (contact <= min_element) ? min_element : contact -1;
	int to_rail_number = (contact >= max_element) ? max_element: contact +1;

	for (; from_rail_number <= to_rail_number; from_rail_number++) {
		if (sprite_visible(rail[from_rail_number])) {
			sprite_hide(rail[from_rail_number]);
		}
	}
} // END hide_rail


/**
* Reset Rails
* - Reset the visibilty to visible on all rails
*
* @return void
*/
void reset_rails() {
	for (int i = 0; i < rails_width; i++) {
		sprite_show(rails_upper[i]);
		sprite_show(rails_lower[i]);
	}
} // END reset_rails


/**
* Is End Of Rail
* - Test to see if the ball has collided with the last visible 
*   section of the rail.  This is to prevent the ball getting stuck
*   in the rail and removing every piece.  
*-  May have not work perfectly if the ball hits the bottom or top of
*   the last section.  Same problem as the paddle
*
* @return bool True if in contact with the last section of the rail row
*/
bool is_end_of_rail(sprite_id rail[], int contact) {
	bool last_rail_left = true, last_rail_right = true;

	for (int i = contact; i < rails_width-1; i++) {
		if (sprite_visible(rail[contact])) {
			last_rail_right = false;
		}
	}

	for (int i = contact; i > 0; i--) {
		if (sprite_visible(rail[contact])) {
			last_rail_left = false;
		}		
	}

	return (last_rail_left || last_rail_right);
} // END is_end_of_rail


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
		level_change();
	} 

	if (key == 'q' || key == 'Q') {
		game_over = true;
	}
} // END listen_keyboard


/**
* Level Change
* - Cycle though levels
*
* @return void
*/
void level_change() {
	if (level < 4){
		level++;			
	} else {
		level = 1;
	}

	if (level == 4) {
		reset_rails();
	}

	restart_round();
} // END level_change


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
			singularity_active = true;
		}
	}
} // END draw_singularity


/**
* Singularity Effect
* - When active calculate the area of effect for the 
*   singularity
*
* @return void
*/
void singularity_effect(void) {
	if (singularity_active) {
		int ball_x = round(sprite_x(ball));
		int ball_y = round(sprite_y(ball));

		if (ball_x <= sing_x + sing_width &&
				ball_x >= sing_x &&
				ball_y <= sing_y + sing_height &&
				ball_y >= sing_y) {

			ball_acceleration();
		}

		
	}
} // END singularity_effect


/**
* Ball Acceleration
* - Calculate the balls acceleration when under effect from the
*   singularity and adjust the balls speed and direction
*
* @return void
*/
void ball_acceleration() {
	int ball_x = round(sprite_x(ball));
	int ball_y = round(sprite_y(ball));

	// Smaller the number, the higher intensity
	double inner_intensity = 1e-5;

	// Bigger the number, faster acceleration increase
	double gravity_and_mass = 0.075;

	int speed_limit_per_redraw = 1;

	double x_difference = (screen_width()/2-1) - ball_x;
	double y_difference = (screen_height()/2+1) - ball_y;

	double distance_squared = pow(y_difference, 2) + pow(x_difference, 2);

	if (distance_squared < inner_intensity) {
		distance_squared = inner_intensity;
	}

	double distance = sqrt(distance_squared);

	double ball_dx = sprite_dx(ball);
	double ball_dy = sprite_dy(ball);

	double acceleration = gravity_and_mass / distance_squared;

	ball_dx = ball_dx + (acceleration * x_difference / distance);
	ball_dy = ball_dy + (acceleration * y_difference / distance);

	double velocity = sqrt(pow(ball_dx, 2) + pow(ball_dy, 2));

	if (velocity > speed_limit_per_redraw){
		ball_dx = ball_dx / velocity;
		ball_dy = ball_dy / velocity;
	}

	sprite_turn_to(ball, ball_dx, ball_dy);
} // END ball_acceleration


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
* Prevent Ball Getting Stuck
* - When the human moves the paddle into the ball the ball
*   can not move.  
* - Checks if the ball exists in the same space as the paddle and 
*   moves the ball.
*
* @return void
*/
void realign_ball_with_paddle() {
	int ball_x = round( sprite_x(ball) );
	int ball_y = round( sprite_y(ball) );

	int paddle_x = round( sprite_x(paddle[HUMAN_PADDLE]) );
	int paddle_y = round( sprite_y(paddle[HUMAN_PADDLE]) );

	if (ball_x == paddle_x && 
			ball_y <= paddle_y+paddle_height-1 &&
			ball_y >= paddle_y) {

		// Ball stuck
		sprite_move(ball, -1, 0);
	}
} // END realign_ball_with_paddle


/**
* Reset Game
* - Reset the game after a human lose or level change
*
* @return void
*/
void reset_game() {
	sprite_move_to(paddle[HUMAN_PADDLE], human_paddle_x, paddle_start_y);
	sprite_move_to(paddle[COMPUTER_PADDLE], computer_paddle_x, paddle_start_y);
	
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

	singularity_active = false;
	count_down_timer = true;
	initialize_ball = true;
	start_level_three_time = 0;
} // END restart_round


/**
* Check for Humun Lose
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
		if (lives > 0) {
			restart_round();
		} else {
			game_lost();
		}
		
	}
} // END check_for_human_lose


/**
* Game Lost
* - Human has lost the game
* - Ask human to play again
*     Y: resets game
*     N: quits game
*
* @return void
*/
void game_lost() {
	clear_screen();

	bool show_panel = false;
	draw_boarder(show_panel);

	int title_text_x = (screen_width() /2) - 6, title_text_y = screen_height() /3;
	int question_text_x = (screen_width() /2) - 22, question_text_y = screen_height() /2;

	draw_string(title_text_x, title_text_y, "You have lost");
	draw_string(question_text_x, question_text_y, "Would you like to play again?  (Y)es or (N)o");

	show_screen();

	bool correct_key = false;
	int loop_key;
	do {
		loop_key = wait_char();

		if (loop_key == 'y' || loop_key == 'Y') {
			correct_key = true;
			key = 'h';
			reset_game();

		} else if (loop_key == 'n' || loop_key == 'N') {
			game_over = true;
			correct_key = true;
		}

	} while ( !correct_key );
} // END game_lost


/**
* Move Computer Paddle
* - Computer paddle just tracks the y location of the ball
*
* @return void
*/
void move_computer_paddle() {
	int paddle_y = round( sprite_y(ball) - (paddle_height /2));

	if (paddle_min_y <= paddle_y && paddle_y <= paddle_max_y) {
		sprite_move_to(
			paddle[COMPUTER_PADDLE], 
			computer_paddle_x, 
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

		// Clear background before drawing the counter down box
		for (int i = 0; i < sprite_width(counter_box); i++) {
			for (int j = 0; j < sprite_height(counter_box); j++) {
				draw_char(i+sprite_x(counter_box), j+sprite_y(counter_box), ' ');
			}
		}

		// Count down
		do {
			sprite_draw(counter_box);
			draw_formatted(counter_x,counter_y,"%d...", counter_steps--);
			show_screen();
			timer_pause(counter_delay);
			key = get_char();
			if (key == 'l' || key == 'L') {
				level_change();
				draw_info_panel();
			}
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
	if (initialize_ball) {
		initialize_ball = false;
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

	check_for_wall_bounce();
} // END move_ball


/**
* Check for Wall Bounce
* - After each ball step check if it has hit a wall
* - If true, change direction
*
* @return void
*/
void check_for_wall_bounce() {
	int ball_width = 1,	ball_height = 1;
	int panel_height = 2;

	int ball_x = round( sprite_x(ball) );
	int ball_y = round( sprite_y(ball) );

	double ball_dx = sprite_dx(ball);
	double ball_dy = sprite_dy(ball);

	bool dir_changed = false;

	if ( ball_x == 0 || ball_x == screen_width() - ball_width ) {
		ball_dx = -ball_dx;
		dir_changed = true;
	}

	if ( ball_y == panel_height  || ball_y == screen_height() - ball_height ) {
		ball_dy = -ball_dy;
		dir_changed = true;
	}

	if ( dir_changed ) {
		sprite_back(ball);
		sprite_turn_to(ball, ball_dx, ball_dy );
	}
} // END check_for_wall_bounce


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

	realign_ball_with_paddle();
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
	int anykey_text_y = screen_height() - border_padding, anykey_move_y = -1, anykey_move_x = -3;
	
	if (screen_height() < 15) {
		title_move_x = -14;
		controls_move_x = 16;
		anykey_move_y = 0;
		anykey_move_x = 11;
	} else {
		title_move_y = -1 * screen_height() / 5;
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
	} while (loop_key < 0);

	if (loop_key == 'q' || loop_key =='Q') {
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

	bool show_panel = false;
	draw_boarder(show_panel);

	draw_string(screen_width() /2-12, (screen_height() /2),   "Thank you for playing...");

	show_screen();
	timer_pause(3000);
} //END show_exit_screen


/**
* Initialize Paddles
* - Sets up two paddles, one for human and the other for computer
*
* @return void
*/
void init_paddles() {
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

	human_paddle_x = screen_width() - 4, computer_paddle_x = 3;
	paddle_start_y = (screen_height() - paddle_height) /2 + 1;

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
} // END init_paddles


/**
* Initialize Ball
* - Creates the ball in the game
*
* @return void
*/
void init_ball() {
	char * ball_img = "O";

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
} // END init_ball


/**
* Initialize Singularity
* - Sets up the singularity on level 3
*
* @return void
*/
void init_singularity() {
	sing_width = 9, sing_height = 5;
	sing_x = (screen_width() - sing_width ) /2;
	sing_y = screen_height() / 2 - 1;
	
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
} // END init_singularity


/**
* Initialize Rails
* - Sets up the two rails, both upper and lower.
* - Memory allocated dynamically based on the user's screen width
* - Each rail is an array with length half the screen width
*
* @return void
*/
void init_rails() {
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
} // END init_rails


/**
* Initialize Countdown Box
* - Sets up the outer box for the countdown
*
* @return void
*/
void init_countdown_box() {
	int counter_x = (screen_width()  /2) - 2, counter_y;

	if (screen_height() >= 15) {
		counter_y = screen_height() /3;
	} else {
		counter_y = 5;
	}

	int box_width = 10, box_height = 5;

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
} // END init_countdown_box
