#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_direct3d.h>
#include "main.h"


#define PI 3.141592
#define MAX_PROJECTILES 7
#define MAX_ASTROIDS 3
#define MIN_INTERVAL (1.0 / 100.0)
#define MAX_LATENCY   0.5

#define HEIGHT 1200
#define WIDTH 1920

ALLEGRO_PATH *path;
ALLEGRO_DISPLAY *display;

ALLEGRO_BITMAP *bmp;
ALLEGRO_BITMAP *bmp2;
ALLEGRO_BITMAP *enemy_ship;
ALLEGRO_BITMAP *astroid;
ALLEGRO_BITMAP *projectiles;
ALLEGRO_BITMAP *player;

ALLEGRO_EVENT event;
ALLEGRO_EVENT_QUEUE *queue;	
ALLEGRO_SAMPLE *sample;

int game_is_running = 1;
int map_y = 0;
int map_x = 0;
double start_time;
double current_time;
double target_time;
double last_game_time = 0.0;

float ball_x;
float ball_y;
float ball_velocity_x;
float ball_velocity_y;
int key_up_rel = 0;

struct _projectiles _ptls[MAX_PROJECTILES];
struct _astroids astr[MAX_ASTROIDS];

int init_resources()
{
	al_set_new_bitmap_flags(ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR);
	/* At this point, we are now in the directory assumed to hold our
	   bundled resources. Again, since we are on Windows, this is the 
	   same directory that contains the executable. So we still need
	   to add the "resources" folder to the path. */
	bmp = al_load_bitmap("resources/Map.jpg");
	if (!bmp)
	{
		/* This should not fail given we have changed the working directory
		   and built a relative path to a file that exists. */
		return -1;
	}
	enemy_ship = al_load_bitmap("resources/enem2.png");
	if (!enemy_ship)
		return -1;

	astroid = al_load_bitmap("resources/astroid.png");
	if (!astroid)
		return -1;

	projectiles = al_load_bitmap("resources/projectile.png");
	if (!projectiles)
		return -1;

	bmp2 = al_clone_bitmap(bmp);
	if(!bmp2)
		return -1;

	player= al_load_bitmap("resources/ship.gif");
	if (!player)
		return -1;

	sample = al_load_sample("resources/music.ogg");
	if (sample)
	{
		/* If the sample was loaded successfully, play it once with default settings. */
		//al_play_sample(sample, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, NULL);
	}

	return 1;

}

int init_start_values()
{
	int i;
	ball_x = (WIDTH-al_get_bitmap_width(player)) / 2;
	ball_y = ((HEIGHT+520)-al_get_bitmap_width(player)) /2;

	for (i=0; i<MAX_PROJECTILES; i++){
		_ptls[i].on_game = 0;
		_ptls[i].proj_left_x = ball_x;
		_ptls[i].proj_left_y = ball_y;
		_ptls[i].proj_right_x = ball_x;
		_ptls[i].proj_right_y = ball_y;
	}
	for (i=0; i<MAX_ASTROIDS; i++){
		astr[i].on_game = 0;
		astr[i].x = 0;
		astr[i].y = 0;
		astr[i].vx = 0;
		astr[i].vy = 0;
		astr[i].angle = 0;
	}
	return 1;
}

double get_time(){return GetTickCount() / 1000.0;}

void game_tick(double delta_time)
{
	int i;
	int ground = 0;
	int jump = 0;
	static int _time_wait = 0;
	float next_x, next_y;
	
	/* New input */
	al_get_next_event(queue, &event);
	if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE ||(event.type == ALLEGRO_EVENT_KEY_CHAR && event.keyboard.keycode == ALLEGRO_KEY_ESCAPE))
		game_is_running = 0;
	if (event.type == ALLEGRO_EVENT_KEY_CHAR) {
		if (event.keyboard.keycode == ALLEGRO_KEY_UP || event.keyboard.keycode == ALLEGRO_KEY_W)
			ball_velocity_y -= 280;
		if (event.keyboard.keycode == ALLEGRO_KEY_DOWN || event.keyboard.keycode == ALLEGRO_KEY_S)
			ball_velocity_y += 280;
		if (event.keyboard.keycode == ALLEGRO_KEY_LEFT || event.keyboard.keycode == ALLEGRO_KEY_A)
			ball_velocity_x -= 280;
		if (event.keyboard.keycode == ALLEGRO_KEY_RIGHT || event.keyboard.keycode == ALLEGRO_KEY_D)
			ball_velocity_x += 280;
		if (event.keyboard.keycode == ALLEGRO_KEY_SPACE){
			_time_wait++;
			for (i=0; i<MAX_PROJECTILES; i++){
				if (!_ptls[i].on_game && (_time_wait%15 == 0)){
					_ptls[i].on_game = 1;
					_ptls[i].proj_left_y = ball_y+13;
					_ptls[i].proj_left_x = ball_x+12;
					_ptls[i].proj_right_y = ball_y+13;
					_ptls[i].proj_right_x = ball_x+40;
					_ptls[i].proj_velocity_y -= 254;
					_time_wait=0;
					break;
				}
			}
		}
	}

	// Gravity (If we are in space, then screw the gravity)
	//ball_velocity_y += 981.1f * delta_time;
	
	/* Slow down effect
	if (ball_velocity_x > 1.0)
		ball_velocity_x -= 130.0f * delta_time;
	if (ball_velocity_x < -1.0)
		ball_velocity_x += 130.0f * delta_time;
	*/

	/* Velocity constraints
	if (ball_velocity_x > 300)
		ball_velocity_x = 300;
	if (ball_velocity_x < -300)
		ball_velocity_x = -300;
	*/
	for (i=0; i<MAX_ASTROIDS; i++){
		if (!astr[i].on_game){
			astr[i].on_game = 1;
			astr[i].x = rand()%(WIDTH-70);
			astr[i].y = 0;
			astr[i].angle=rand()%360;
			astr[i].vy += 76+rand()%66;
		}
	}

	/* Calculate next ball move */
	next_x = ball_x + ball_velocity_x * delta_time;
    next_y = ball_y + ball_velocity_y * delta_time;
	
	/* Calculate next projectile move */
	for (i=0; i<MAX_PROJECTILES; i++){
		if (_ptls[i].on_game){
			_ptls[i]._ptls_left_nx_y = _ptls[i].proj_left_y + _ptls[i].proj_velocity_y * delta_time;
			_ptls[i]._ptls_right_nx_y = _ptls[i].proj_right_y + _ptls[i].proj_velocity_y * delta_time;
		}
	}

	/* Calculate next astroid movement */
	for (i=0; i<MAX_ASTROIDS; i++){
		if (astr[i].on_game){
			astr[i].next_y = astr[i].y + astr[i].vy * delta_time;
		}
	}

	if (next_y > (HEIGHT-60.01))
	{
		next_y = (HEIGHT-60); //Reset position if we colide ith the bottom
		ground = 1;
	}
	else
		ground = 0;
	
	// Calculate boundrys 
	for (i=0; i<MAX_ASTROIDS; i++){
		if (astr[i].on_game){
			if (astr[i].next_y > HEIGHT){
				astr[i].vy = 0;
				astr[i].on_game = 0;
				astr[i].angle = 0.34;
			}
		}
	}
	
	// if Ball moves out of screen horizontaly
	if (next_x < 0)
		next_x = 0;
	if (next_x > WIDTH-60)
		next_x = WIDTH-62;

	//If Projectile exits the screen reset the on_game var
	for (i=0; i<MAX_PROJECTILES; i++){
		if(_ptls[i].on_game){
			if(_ptls[i]._ptls_left_nx_y < 0 || _ptls[i]._ptls_right_nx_y < 0 ){
				_ptls[i].on_game = 0;
				_ptls[i].proj_velocity_y = _ptls[i].proj_velocity_x = 0;
			}
		}
	}

	/* Set next ball move */
	ball_x = next_x;
    ball_y = next_y;

	/* Set next projectile move */
	for (i=0; i<MAX_PROJECTILES; i++){
		if (_ptls[i].on_game){
			_ptls[i].proj_left_y = _ptls[i]._ptls_left_nx_y;
			_ptls[i].proj_right_y = _ptls[i]._ptls_right_nx_y;
		}
	}

	/* Set next astroid move */
	for (i=0; i<MAX_ASTROIDS; i++){
		if(astr[i].on_game){
			astr[i].y = astr[i].next_y;
		}
	}

	if (map_y++ >= 640)
		map_y = 0;

	ball_velocity_x = ball_velocity_y = 0; 
}

void draw_frame()
{
	/* */
	int i;
	al_clear_to_color(al_map_rgb(0, 0, 0));
	al_draw_tinted_bitmap( bmp, al_map_rgba_f(1,0.5,0,0.3f),0, (-640+map_y), /*flags*/ 0);
	al_draw_tinted_bitmap( bmp, al_map_rgba_f(1,0.5,0,0.3f),0, 0+map_y, /*flags*/ 0);

	al_draw_bitmap(player, ball_x, ball_y, 0);

	for (i=0; i<MAX_ASTROIDS; i++){
		if (astr[i].on_game){
			al_draw_rotated_bitmap(astroid, al_get_bitmap_width(astroid)/2, al_get_bitmap_height(astroid)/2, astr[i].x, astr[i].y, astr[i].angle+=0.00026, 0);
		}
	}

	for (i=0; i<MAX_PROJECTILES; i++){
		if (_ptls[i].on_game){
			al_draw_bitmap(projectiles, _ptls[i].proj_left_x, _ptls[i].proj_left_y, 0);
			al_draw_bitmap(projectiles, _ptls[i].proj_right_x, _ptls[i].proj_right_y, 0);
		}
	}
}

int main()
{

	ALLEGRO_MONITOR_INFO ainfo;

	int mode_flags[2] = {
	   ALLEGRO_FULLSCREEN | ALLEGRO_OPENGL , 
	   ALLEGRO_FULLSCREEN | ALLEGRO_OPENGL_3_0
	};

	const char* mode_driver_text[3] = {
	  "ALLEGRO_DIRECT3D",
	  "ALLEGRO_OPENGL",
	  "ALLEGRO_OPENGL_3_0"
	};
	
	int j=0;
	int num_modes = 0;
	for (int i = 0 ; i < 3 ; i++) {
		al_set_new_display_flags(mode_flags[i]);
		//output_log << "Modes available for " << mode_driver_text[i] << " :" << endl;
//		num_modes = al_get_num_display_modes();
//		for (j = 0 ; j < num_modes ; ++j) {
			//ALLEGRO_DISPLAY_MODE admode;
			//if (al_get_display_mode(j , &admode) == &admode) {
				// Do something with info
				//printf("%d X %d",admode.width,admode.height);
				//printf("%d Hz using",admode.refresh_rate);
			//}
		//}
	}

	if (int adapts = al_get_num_video_adapters() == 1){
		printf("Screw this shit\n");
		return -1;
	} 

	//bool al_get_monitor_info(int adapter, ALLEGRO_MONITOR_INFO *info)
	
	al_get_monitor_info(0, &ainfo);
	int width = ainfo.x2 - ainfo.x1;
	int height = ainfo.y2 - ainfo.y1;


	srand(time(NULL));

	/* These properties are used to help Allegro come up with reasonable
	   default values for a few things like the window title and application
	   data directory. */

	al_set_org_name("Allegro.cc");
	al_set_app_name("Demonstration");
	
	/* You must initialize Allegro before calling most functions.
	   Be sure to initialize any addon that you are using too.
	   (You also need to include the header files for each addon.) */
	
	al_init();
	al_set_new_display_flags(ALLEGRO_NOFRAME);
	//al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);
	al_init_image_addon();
	al_install_keyboard();

	/* When playing audio files, you'll typically call the following
	   three functions to set everything up. The 16 refers to the number
	   of samples we will be playing simultaneously. Obviously for this
	   demo, a value of 1 would be sufficient. */

	al_install_audio();
	al_init_acodec_addon();
	al_reserve_samples(16);
	
	display = al_create_display(WIDTH, HEIGHT);

	if (!display)
	{
		/* It's very unlikely that this would fail here, but always
		   check the return values for errors. */
		return -1;
	}

	/* When the program starts, there's no guarantee that the working
	   directory will be the same as the executable. For instance, when 
	   running from within the MSVC IDE, it will be the base solution
	   directory above the bin folder.

	   Also, some platforms (e.g., OS X) have application bundles which
	   are structured in a special way with distinct resource directories.
	   In order to easily support multiple platforms, you can ask Allegro
	   to find the directory where bundled data is located.

	   Note that in most cases you will actually use a sub directory
	   of the resource location. In this example, we query the resources
	   path, and change the working directory to it. At this point we
	   can load files with relative path names.
	*/

	path = al_get_standard_path(ALLEGRO_RESOURCES_PATH);
	al_change_directory(al_path_cstr(path, '/'));
	al_destroy_path(path);
	
	if (!init_resources()){
		MessageBox(NULL, L"Failed to initialize startup, game will quit", L"Startup init failed",0);
		return -1;
	}


	/* To use the event system, we must first create a queue, and then attach
	   the various event generating objects to it. In this case, we want to
	   know when the window is closed or a key is pressed. */

	queue = al_create_event_queue();
	al_register_event_source(queue, al_get_display_event_source(display));
	al_register_event_source(queue, al_get_keyboard_event_source());
	
	init_start_values();

	while (game_is_running)
	{
		current_time = get_time();
		target_time = current_time - start_time;

		/* If the computer's clock has been adjusted backwards,
		 * compensate */
		if (target_time < last_game_time)
			start_time = current_time - last_game_time;

		/* If the game time lags too much, for example if the computer
		 * has been suspended, avoid trying to catch up */
		if (target_time > last_game_time + MAX_LATENCY)
			last_game_time = target_time - MAX_LATENCY;

		/* If more than MIN_INTERVAL has passed since last update, run
		 * game_tick() again. */
		if (target_time >= last_game_time + MIN_INTERVAL)
		{
			game_tick(target_time - last_game_time);
			last_game_time = target_time;
		}

		/* The default target bitmap is the backbuffer of the display.
	       We'll draw the image to it and "flip" the backbuffer to make it
	       visible. It's better to draw the bitmap inside this loop because
		   certain events may cause the display to get "lost," such as minimizing
		   and restoring the window. By drawing within the loop, we are 
		   safeguarding against that. */
		draw_frame();
		al_flip_display();

	}

	return 0;
}
