/* 
 * Fire men game for bitbox
 * version 2, with blitting
 */

/* 
	
 TODO
 - splash
 - make use of engine
 - sound

*/

#include <stdint.h>
#include <bitbox.h>
#include <stdlib.h> // rand
#include <string.h>


typedef uint16_t Sprite[][3]; // c1,c2,n

// global functions

int demo; // boolean. in demo mode, player can just press "start" to start, score is kept.
int score, lives; 
int next_emit_frame;  


int bg_index; // blitting index for background sprite
extern int bg_w, bg_h;
extern Sprite bg_sprite;

// score display
extern Sprite digit_0_sprite, digit_1_sprite, digit_2_sprite, digit_3_sprite, digit_4_sprite;
extern Sprite digit_5_sprite, digit_6_sprite, digit_7_sprite, digit_8_sprite, digit_9_sprite;
extern int digit_0_w, digit_0_h;

Sprite *numbers[] = {
	&digit_0_sprite, &digit_1_sprite, &digit_2_sprite, &digit_3_sprite, 
	&digit_4_sprite, &digit_5_sprite, &digit_6_sprite, &digit_7_sprite, 
	&digit_8_sprite, &digit_9_sprite
};
const int pow10[] = {100,10,1};

Sprite *score_digits[3]; // 3 ptr to sprites
int score_digits_index[3]; // blitting index for score blitting



const int AMBULANCE_X = 500;
const int AMBULANCE_Y = 360;

void start_game();
void start_life();
void initialize_guy();

int angel_index[3]; // blit index
extern Sprite angel_sprite;
extern int angel_w, angel_h;

// press start
extern Sprite start_sprite;
extern const int start_w, start_h; 
int start_index;
const int start_x=260, start_y=260;


// Firemen logic ----------------------------------------------------
int firemen_pos; // position of the firemen, 0 1 or 2
int firemen_index;
const int firemen_steps = 3;
const int firemen_x[] = {60,60+130,60+130*2}; // places to display the firemen.
const int firemen_y=415; // fixed ! (start of the sprite)
const uint16_t TRANSPARENT_COLOR=RGB(255,0,255); 

extern Sprite firemen_sprite;
extern int firemen_w, firemen_h;

void move_firemen()
{
	if (demo && GAMEPAD_PRESSED(0,start))	start_game();

	static int need_rel;

    if (GAMEPAD_PRESSED(0,left) && firemen_pos >0) 
	{
		if (!need_rel)
		{ 
	    	firemen_pos--;
	    	need_rel= 1;
	    }
	}
	else if (GAMEPAD_PRESSED(0,right) && firemen_pos<firemen_steps-1)
	{
		if (!need_rel)
		{ 
		    firemen_pos++;
		    need_rel= 1;
		}
	}
	else
	{
		need_rel =0;
	}
	if (GAMEPAD_PRESSED(0,A) && (vga_frame % 16 == 0)) initialize_guy(2);
}


// Guys ----------------------------------------------------------
extern const int guy_w, guy_h;
extern Sprite guy_sprite;

#define MAX_GUYS 8 // should be enough !
const int INACTIVE=-1;

const float start_guy_y = 50.f; // start position
const float start_guy_x = 50.f;
const float guy_vx = 1.15f; // pixels per frame ; 500 pixels => 4s for whole screen 
const float ROAD_Y = 475.f; // guy h
const float gravity=0.2f;
const float firemen_bounce_y = 430.f; // can bounce at this position 
const float bounce_attenuation = 0.f;//0.0025f; 

typedef struct {
    float x,y,vy; // x,y are *8 for precision !
    // set x negative to disable this guy. 
    // vx is not needed as it's a constant.
    int index;
} Guy;

Guy guys[MAX_GUYS];
int nb_guys;

void initialize_guy() // insert new guy
{
	for (int i=0;i<MAX_GUYS;i++)
	{
		if (guys[i].x==INACTIVE) {
			// found empty slot
			guys[i].x = start_guy_x;
			guys[i].y = start_guy_y;
			guys[i].vy = 0.f;
			break;
		}
	}
	nb_guys++;
}

void disable_guy(int guy)
{
    guys[guy].x = INACTIVE;
    nb_guys--;
}


int can_bounce(int guy)
{
    return (guys[guy].x >= firemen_x[firemen_pos] \
       && guys[guy].x <= firemen_x[firemen_pos]+firemen_w);
}


void update_guy(int guy)
{
    if (guys[guy].x == INACTIVE) return; // skip inactive

    // moving the guy
    guys[guy].x += guy_vx; 
    guys[guy].vy += gravity;
    guys[guy].y += guys[guy].vy;

    // bounce
    if (guys[guy].y>=(firemen_bounce_y-guy_h))
    {
    	if (demo)
    	{ // demo : set firemen exactly where it's needed
    		firemen_pos = 2;
    		while (firemen_x[firemen_pos]>guys[guy].x) firemen_pos--;
    	} 

    	if (can_bounce(guy)) // under a fireman
	    {
	        guys[guy].vy = -guys[guy].vy - bounce_attenuation;
	        guys[guy].y=(firemen_bounce_y-guy_h);
	        if (!demo) score ++;
	    }  
	}

	// ambulance
	if (guys[guy].y>=AMBULANCE_Y  && guys[guy].x>AMBULANCE_X )
	{
        // if (!demo) score += 1;
        disable_guy(guy);
	}
    // collision check
	if (guys[guy].y>=(ROAD_Y-guy_h)) 
    {
        if (lives) 
        {
            lives--;
            start_life();
        } else {
        	demo = 1;
        }
    }
} 




// Game logic
void start_game()
{
	demo =0;
    score = 0;
    lives = 3;
    message("New game\n");
    start_life();
}

void start_life()
{
    // reset elements state once a life has been lost
    firemen_pos = 0;
    for (int guy=0;guy<MAX_GUYS;guy++)
    {
        guys[guy].x  = INACTIVE;
    }
    nb_guys=0;

    next_emit_frame = vga_frame+60; // issue a small pause.
    message("Lost ! \n");
}



// Graphic engine ---------------------------------
int blit (Sprite sprite, int w, int h, int x, int y, int blit_index)
{
	// shoud we blit ? note that line <0 or >screen size won't be called at all
	if (vga_line>=y && vga_line<y+h )
	{
		for (int dx=0; dx<w ;)
		{ 
			uint16_t c1 = sprite[blit_index][0];
			uint16_t c2 = sprite[blit_index][1];

			
			// fill line, note that we must be sure that x is >0 (no crop)
			// XXX align / word by word / unroll loops
			if (sprite[blit_index][2] == 1) 
			{				
				if (c1 != TRANSPARENT_COLOR) draw_buffer[x+dx  ]=c1;
				if (c2 != TRANSPARENT_COLOR) draw_buffer[x+dx+1]=c2;
			}
			else 
			{
				if (c1 != TRANSPARENT_COLOR) // in that case we will never have c1 and not c2
				{
					uint32_t c = c1<<16 | c2;
					int nb = sprite[blit_index][2];

					for (int i=0;i<nb;i++)
					{
						*(uint32_t*)&draw_buffer[x+dx+i*2]=c;
					}
				}
			}
			dx += sprite[blit_index][2]*2;
			blit_index += 1;
			#ifdef EMULATED
				if(x+dx>640)
					printf("vga_line : %d, sprite w %d\n",vga_line,w);
			#endif
		}
	}

	return blit_index; // return the new index in the sprite
}

// --------------------------------------------- callbacks

void game_init()
{
	demo = 1; // start as demo mode
	// inactivate all guys
	for (int i=0;i<MAX_GUYS;i++) guys[i].x = INACTIVE;
	nb_guys=0;
	score_digits[0] = score_digits[1] =	score_digits[2] = numbers[0];
}

// check if a guy can be emitted i.e. that another one is not at the same position
int isaguy_high()
{
	for (int i=0;i<MAX_GUYS;i++) {
			if (guys[i].x!=INACTIVE && guys[i].y<start_guy_y+30 )
				return 1;
	}
	return 0;
}

void game_frame()
{

	move_firemen();

	// moving active guys 
	for (int guy=0;guy<MAX_GUYS;guy++)
	{
		update_guy(guy);
	}

	// creating new ?

	if (vga_frame >= next_emit_frame && !isaguy_high() )
	{
		if (next_emit_frame) initialize_guy();

		// how many should be active now ? 1 ts les 10, 5max
		if (nb_guys<1+score/20 && nb_guys<5 ) 
			// not enough, prepare one 
			next_emit_frame=vga_frame+15+rand()%60; // in 0.5-1.5 seconds.
		else // ok now, don't send a new one
			next_emit_frame=0;
	};
}


void graph_vsync(void) 
{
	// Do all on same line
	if (vga_line != VGA_V_PIXELS+3) return;


	// rewind all sprites for the next frame
	bg_index = firemen_index = 0;
	for (int i=0;i<MAX_GUYS;i++) 
		guys[i].index = 0;

	// score sprites
	for (int i=0;i<3;i++) {
		score_digits[i] = numbers[(score/pow10[i])%10];
		score_digits_index[i]=0;
	}

	// angels 
	for (int i=0;i<3;i++) {
		angel_index[i]=0;
	}

	// index
	start_index = 0;
}


void graph_line()
{	
	// Background 
	bg_index = blit(bg_sprite, bg_w, bg_h, 0, 0, bg_index); 


	// blit score
	for (int i=0;i<3;i++)
	{
		score_digits_index[i] = blit(
			*score_digits[i],
			digit_0_w,digit_0_h,
			500+i*20,20,
			score_digits_index[i]
			);
	}

	// blit angels or press start
	if (!demo) 
	{
		for (int i=0;i<3-lives;i++)
		{
			angel_index[i] = blit(
				angel_sprite,
				angel_w,angel_h,
				500+i*32,52,
				angel_index[i]
				);
		}
	} 
	else if ((vga_frame/32)%2)
	{ // press start (blinking)
		start_index = blit(
			start_sprite, 
			start_w, start_h,
			start_x, start_y,
			start_index
			);
	}

	// guys
	for (int i=0;i<MAX_GUYS;i++)
	{
		if (guys[i].x != INACTIVE)
		{
			guys[i].index = blit(	
				guy_sprite, guy_w, guy_h,
				guys[i].x, guys[i].y,
				guys[i].index
			);
		}
	}

	// blit firemen
	firemen_index =blit(
		firemen_sprite, 
		firemen_w, firemen_h, 
		firemen_x[firemen_pos], firemen_y, 
		firemen_index
	); 

}

void game_snd_buffer(uint16_t *buffer, int len) {};