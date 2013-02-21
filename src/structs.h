#ifndef _STRUKTURER_H
#define _STRUKTURER_H

#define MAX_PROJECTILES 7



struct _projectiles 
{
	int on_game;
	
	// ------ Left projectile ------
	float proj_left_x, proj_left_y;
	float _ptls_left_nx_x, _ptls_left_nx_y;
	
	// ------ Right projectile ------
	float proj_right_x, proj_right_y;
	float _ptls_right_nx_x, _ptls_right_nx_y;
	
	// ------ Velocity of both projectiles -----
	float proj_velocity_x, proj_velocity_y;
};

struct main_player
{
	int    x;  // ?
	int    y;  // ?
	double aimAngle;
	int    strength;
	int    shoots;
	int    maxShoots; // antall prosjektiler han kan skyte samtidig
	int    frame;
	int    antFrames;
	double animtime;
	BITMAP *sprite;
	struct _projectiles projectiles[MAX_PROJECTILES];
};

struct _astroids
{
	int on_game;
	float x;
	float y;
	float next_x;
	float next_y;
	float vx;
	float vy;
	float angle;
};




#endif

