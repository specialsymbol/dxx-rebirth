/*
THE COMPUTER CODE CONTAINED HEREIN IS THE SOLE PROPERTY OF PARALLAX
SOFTWARE CORPORATION ("PARALLAX").  PARALLAX, IN DISTRIBUTING THE CODE TO
END-USERS, AND SUBJECT TO ALL OF THE TERMS AND CONDITIONS HEREIN, GRANTS A
ROYALTY-FREE, PERPETUAL LICENSE TO SUCH END-USERS FOR USE BY SUCH END-USERS
IN USING, DISPLAYING,  AND CREATING DERIVATIVE WORKS THEREOF, SO LONG AS
SUCH USE, DISPLAY OR CREATION IS FOR NON-COMMERCIAL, ROYALTY OR REVENUE
FREE PURPOSES.  IN NO EVENT SHALL THE END-USER USE THE COMPUTER CODE
CONTAINED HEREIN FOR REVENUE-BEARING PURPOSES.  THE END-USER UNDERSTANDS
AND AGREES TO THE TERMS HEREIN AND ACCEPTS THE SAME BY USE OF THIS FILE.
COPYRIGHT 1993-1998 PARALLAX SOFTWARE CORPORATION.  ALL RIGHTS RESERVED.
*/


#ifdef RCS
static char rcsid[] = "$Id: titles.c,v 1.2 2006/03/18 23:08:13 michaelstather Exp $";
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "types.h"
#include "timer.h"
#include "key.h"
#include "gr.h"
#include "palette.h"
#include "iff.h"
#include "pcx.h"
#include "u_mem.h"
#include "joy.h"
#include "mono.h"
#include "gamefont.h"
#include "cfile.h"
#include "error.h"
#include "polyobj.h"
#include "textures.h"
#include "screens.h"
#include "multi.h"
#include "player.h"
#include "digi.h"
#include "compbit.h"
#include "text.h"
#include "kmatrix.h"
#include "piggy.h"
#include "songs.h"
#include "newmenu.h"
#include "state.h"
#include "gameseq.h"
#ifdef OGL
#include "ogl_init.h"
#endif

void title_save_game();

ubyte New_pal[768];
int New_pal_254_bash;

char * Briefing_text;

//Begin D1X modification
#define MAX_BRIEFING_COLORS 7
//End D1X modification

char Ending_text_filename[13] = "endreg.tex";
char Briefing_text_filename[13] = "briefing.tex";

#define	SHAREWARE_ENDING_FILENAME "ending.tex"

//	Can be set by -noscreens command line option.  Causes bypassing of all briefing screens.
int	Skip_briefing_screens=0;
int	Briefing_foreground_colors[MAX_BRIEFING_COLORS], Briefing_background_colors[MAX_BRIEFING_COLORS];
int	Current_color = 0;
int	Erase_color;

// added by Jan Bobrowski for variable-size menu screen
static int rescale_x(int x)
{
	return x * GWIDTH / 320;
}

static int rescale_y(int y)
{
	return y * GHEIGHT / 200;
}

int local_key_inkey(void)
{
	int rval;
	rval = key_inkey();

	if ( rval==KEY_ALTED+KEY_F2 )	{
	 	title_save_game();
		return 0;
	}

	if (rval == KEY_PRINT_SCREEN) {
		save_screen_shot(0);
		return 0; //say no key pressed
	}

	return rval;
}

int show_title_screen( char * filename, int allow_keys )	
{
	fix timer;
	int pcx_error;
	grs_bitmap title_bm;

	gr_init_bitmap_data (&title_bm);

	if ((pcx_error=pcx_read_bitmap( filename, &title_bm, BM_LINEAR, New_pal ))!=PCX_ERROR_NONE)	{
		printf( "File '%s', PCX load error: %s (%i)\n  (No big deal, just no title screen.)\n",filename, pcx_errormsg(pcx_error), pcx_error);
		mprintf((0, "File '%s', PCX load error: %s (%i)\n  (No big deal, just no title screen.)\n",filename, pcx_errormsg(pcx_error), pcx_error));
		Int3();
		gr_free_bitmap_data (&title_bm);
		return 0;
	}

#ifdef OGL
	gr_palette_load(New_pal);
#else
	gr_palette_clear();	
#endif
	gr_set_current_canvas( NULL );
	show_fullscr(&title_bm);
        //added on 9/13/98 by adb to make update-needing arch's work
        gr_update();
        //end addition - adb

	gr_free_bitmap_data (&title_bm);

	if (allow_keys > 2 || gr_palette_fade_in( New_pal, 32, allow_keys ) || allow_keys > 1) {
		return 1;
	}

	gr_palette_load( New_pal );
	timer = timer_get_fixed_seconds() + i2f(3);
	while (1) {
		if ( local_key_inkey() && allow_keys ) break;
		if ( timer_get_fixed_seconds() > timer ) break;
        }
	if (gr_palette_fade_out( New_pal, 32, allow_keys ))
		return 1;
	return 0;
}

typedef struct {
	char	bs_name[255]; // filename, eg merc01.  Assumes .lbm suffix.
	sbyte	level_num;
	sbyte	message_num;
	short	text_ulx, text_uly; // upper left x,y of text window
	short	text_width, text_height; // width and height of text window
} briefing_screen;

#define BRIEFING_SECRET_NUM	31 // This must correspond to the first secret level which must come at the end of the list.
#define BRIEFING_OFFSET_NUM	4 // This must correspond to the first level screen (ie, past the bald guy briefing screens)
#define	SHAREWARE_ENDING_LEVEL_NUM	0x7f
#define	REGISTERED_ENDING_LEVEL_NUM	0x7e
#ifdef OGL
#define Briefing_screens_LH ((SWIDTH >= 640 && SHEIGHT >= 480 && cfexist(DESCENT_DATA_PATH HIRES_DIR "brief01h.pcx"))?Briefing_screens_h:Briefing_screens)
#else
#define Briefing_screens_LH Briefing_screens
#endif

briefing_screen Briefing_screens[] = {
	{	"brief01.pcx",   0,  1,  13, 140, 290,  59 },
	{	"brief02.pcx",   0,  2,  27,  34, 257, 177 },
	{	"brief03.pcx",   0,  3,  20,  22, 257, 177 },
	{	"brief02.pcx",   0,  4,  27,  34, 257, 177 },

	{	"moon01.pcx",    1,  5,  10,  10, 300, 170 },	// level 1
	{	"moon01.pcx",    2,  6,  10,  10, 300, 170 },	// level 2
	{	"moon01.pcx",    3,  7,  10,  10, 300, 170 },	// level 3

	{	"venus01.pcx",   4,  8,  15,  15, 300, 200 },	// level 4
	{	"venus01.pcx",   5,  9,  15,  15, 300, 200 },	// level 5

	{	"brief03.pcx",   6, 10,  20,  22, 257, 177 },
	{	"merc01.pcx",    6, 11,  10,  15, 300, 200 },	// level 6
	{	"merc01.pcx",    7, 12,  10,  15, 300, 200 },	// level 7

#ifndef SHAREWARE
	{	"brief03.pcx",   8, 13,  20,  22, 257, 177 },
	{	"mars01.pcx",    8, 14,  10, 100, 300, 200 },	// level 8
	{	"mars01.pcx",    9, 15,  10, 100, 300, 200 },	// level 9
	{	"brief03.pcx",  10, 16,  20,  22, 257, 177 },
	{	"mars01.pcx",   10, 17,  10, 100, 300, 200 },	// level 10

	{	"jup01.pcx",    11, 18,  10,  40, 300, 200 },	// level 11
	{	"jup01.pcx",    12, 19,  10,  40, 300, 200 },	// level 12
	{	"brief03.pcx",  13, 20,  20,  22, 257, 177 },
	{	"jup01.pcx",    13, 21,  10,  40, 300, 200 },	// level 13
	{	"jup01.pcx",    14, 22,  10,  40, 300, 200 },	// level 14

	{	"saturn01.pcx", 15, 23,  10,  40, 300, 200 },	// level 15
	{	"brief03.pcx",  16, 24,  20,  22, 257, 177 },
	{	"saturn01.pcx", 16, 25,  10,  40, 300, 200 },	// level 16
	{	"brief03.pcx",  17, 26,  20,  22, 257, 177 },
	{	"saturn01.pcx", 17, 27,  10,  40, 300, 200 },	// level 17

	{	"uranus01.pcx", 18, 28, 100, 100, 300, 200 },	// level 18
	{	"uranus01.pcx", 19, 29, 100, 100, 300, 200 },	// level 19
	{	"uranus01.pcx", 20, 30, 100, 100, 300, 200 },	// level 20
	{	"uranus01.pcx", 21, 31, 100, 100, 300, 200 },	// level 21

	{	"neptun01.pcx", 22, 32,  10,  20, 300, 200 },	// level 22
	{	"neptun01.pcx", 23, 33,  10,  20, 300, 200 },	// level 23
	{	"neptun01.pcx", 24, 34,  10,  20, 300, 200 },	// level 24

	{	"pluto01.pcx",  25, 35,  10,  20, 300, 200 },	// level 25
	{	"pluto01.pcx",  26, 36,  10,  20, 300, 200 },	// level 26
	{	"pluto01.pcx",  27, 37,  10,  20, 300, 200 },	// level 27

	{	"aster01.pcx",  -1, 38,  10,  90, 300, 200 },	// secret level -1
	{	"aster01.pcx",  -2, 39,  10,  90, 300, 200 },	// secret level -2
	{	"aster01.pcx",  -3, 40,  10,  90, 300, 200 }, 	// secret level -3
#endif

	{	"end01.pcx",   SHAREWARE_ENDING_LEVEL_NUM,   1, 23, 40, 320, 200 }, 	// shareware end
#ifndef SHAREWARE
	{	"end02.pcx",   REGISTERED_ENDING_LEVEL_NUM,  1,  5,  5, 300, 200 }, 	// registered end
	{	"end01.pcx",   REGISTERED_ENDING_LEVEL_NUM,  2, 23, 40, 320, 200 }, 	// registered end
	{	"end03.pcx",   REGISTERED_ENDING_LEVEL_NUM,  3,  5,  5, 300, 200 }, 	// registered end
#endif

};

briefing_screen Briefing_screens_h[] = { // ZICO - added for hires screens
	{ HIRES_DIR "brief01h.pcx",   0,  1,  13, 140, 290,  59 },
	{ HIRES_DIR "brief02h.pcx",   0,  2,  27,  34, 257, 177 },
	{ HIRES_DIR "brief03h.pcx",   0,  3,  20,  22, 257, 177 },
	{ HIRES_DIR "brief02h.pcx",   0,  4,  27,  34, 257, 177 },

	{ HIRES_DIR "moon01h.pcx",    1,  5,  10,  10, 300, 170 },	// level 1
	{ HIRES_DIR "moon01h.pcx",    2,  6,  10,  10, 300, 170 },	// level 2
	{ HIRES_DIR "moon01h.pcx",    3,  7,  10,  10, 300, 170 },	// level 3

	{ HIRES_DIR "venus01h.pcx",   4,  8,  15,  15, 300, 200 },	// level 4
	{ HIRES_DIR "venus01h.pcx",   5,  9,  15,  15, 300, 200 },	// level 5

	{ HIRES_DIR "brief03h.pcx",   6, 10,  20,  22, 257, 177 },
	{ HIRES_DIR "merc01h.pcx",    6, 11,  10,  15, 300, 200 },	// level 6
	{ HIRES_DIR "merc01h.pcx",    7, 12,  10,  15, 300, 200 },	// level 7

#ifndef SHAREWARE
	{ HIRES_DIR "brief03h.pcx",   8, 13,  20,  22, 257, 177 },
	{ HIRES_DIR "mars01h.pcx",    8, 14,  10, 100, 300, 200 },	// level 8
	{ HIRES_DIR "mars01h.pcx",    9, 15,  10, 100, 300, 200 },	// level 9
	{ HIRES_DIR "brief03h.pcx",  10, 16,  20,  22, 257, 177 },
	{ HIRES_DIR "mars01h.pcx",   10, 17,  10, 100, 300, 200 },	// level 10

	{ HIRES_DIR "jup01h.pcx",    11, 18,  10,  40, 300, 200 },	// level 11
	{ HIRES_DIR "jup01h.pcx",    12, 19,  10,  40, 300, 200 },	// level 12
	{ HIRES_DIR "brief03h.pcx",  13, 20,  20,  22, 257, 177 },
	{ HIRES_DIR "jup01h.pcx",    13, 21,  10,  40, 300, 200 },	// level 13
	{ HIRES_DIR "jup01h.pcx",    14, 22,  10,  40, 300, 200 },	// level 14

	{ HIRES_DIR "saturn01h.pcx", 15, 23,  10,  40, 300, 200 },	// level 15
	{ HIRES_DIR "brief03h.pcx",  16, 24,  20,  22, 257, 177 },
	{ HIRES_DIR "saturn01h.pcx", 16, 25,  10,  40, 300, 200 },	// level 16
	{ HIRES_DIR "brief03h.pcx",  17, 26,  20,  22, 257, 177 },
	{ HIRES_DIR "saturn01h.pcx", 17, 27,  10,  40, 300, 200 },	// level 17

	{ HIRES_DIR "uranus01h.pcx", 18, 28, 100, 100, 300, 200 },	// level 18
	{ HIRES_DIR "uranus01h.pcx", 19, 29, 100, 100, 300, 200 },	// level 19
	{ HIRES_DIR "uranus01h.pcx", 20, 30, 100, 100, 300, 200 },	// level 20
	{ HIRES_DIR "uranus01h.pcx", 21, 31, 100, 100, 300, 200 },	// level 21

	{ HIRES_DIR "neptun01h.pcx", 22, 32,  10,  20, 300, 200 },	// level 22
	{ HIRES_DIR "neptun01h.pcx", 23, 33,  10,  20, 300, 200 },	// level 23
	{ HIRES_DIR "neptun01h.pcx", 24, 34,  10,  20, 300, 200 },	// level 24

	{ HIRES_DIR "pluto01h.pcx",  25, 35,  10,  20, 300, 200 },	// level 25
	{ HIRES_DIR "pluto01h.pcx",  26, 36,  10,  20, 300, 200 },	// level 26
	{ HIRES_DIR "pluto01h.pcx",  27, 37,  10,  20, 300, 200 },	// level 27

	{ HIRES_DIR "aster01h.pcx",  -1, 38,  10, 90, 300,  200 },	// secret level -1
	{ HIRES_DIR "aster01h.pcx",  -2, 39,  10, 90, 300,  200 },	// secret level -2
	{ HIRES_DIR "aster01h.pcx",  -3, 40,  10, 90, 300,  200 }, 	// secret level -3
#endif

	{ HIRES_DIR "end01h.pcx",   SHAREWARE_ENDING_LEVEL_NUM,   1,  23, 40, 320, 200 }, 	// shareware end
#ifndef SHAREWARE
	{ HIRES_DIR "end02h.pcx",   REGISTERED_ENDING_LEVEL_NUM,  1,   5,  5, 300, 200 }, 	// registered end
	{ HIRES_DIR "end01h.pcx",   REGISTERED_ENDING_LEVEL_NUM,  2,  23, 40, 320, 200 }, 	// registered end
	{ HIRES_DIR "end03h.pcx",   REGISTERED_ENDING_LEVEL_NUM,  3,   5,  5, 300, 200 }, 	// registered end
#endif

};

#define	MAX_BRIEFING_SCREEN (sizeof(Briefing_screens) / sizeof(Briefing_screens[0]))

char * get_briefing_screen( int level_num )
{
	int i, found_level=0, last_level=0;
	for (i = 0; i < MAX_BRIEFING_SCREEN; i++)	{
		if ( found_level && Briefing_screens_LH[i].level_num != level_num )
			return Briefing_screens_LH[last_level].bs_name;
		if (Briefing_screens[i].level_num == level_num )	{
			found_level=1;
			last_level = i;
		}
	}
	return NULL;
}

int	Briefing_text_x, Briefing_text_y;

void init_char_pos(int x, int y)
{
	Briefing_text_x = x;
	Briefing_text_y = y;
}

grs_canvas *Robot_canv = NULL;
vms_angvec Robot_angles;

char	Bitmap_name[32] = "";
#define	EXIT_DOOR_MAX	14
#define	OTHER_THING_MAX	10	// Adam: This is the number of frames in your new animating thing.
#define	DOOR_DIV_INIT	6
sbyte	Door_dir=1, Door_div_count=0, Animating_bitmap_type=0;

//	-----------------------------------------------------------------------------
void show_bitmap_frame(void)
{
        grs_canvas      *curcanv_save, *bitmap_canv=0;
	grs_bitmap	*bitmap_ptr;

	// Only plot every nth frame.
	if (Door_div_count) {
		Door_div_count--;
		return;
	}

	Door_div_count = DOOR_DIV_INIT;

	if (Bitmap_name[0] != 0) {
		char		*pound_signp;
		int		num, dig1, dig2;

		// Set supertransparency color to black
		if (!New_pal_254_bash) {
			New_pal_254_bash = 1;
			New_pal[254*3] = 0;
			New_pal[254*3+1] = 0;
			New_pal[254*3+2] = 0;
		}

	switch (Animating_bitmap_type) {
		case 0:		bitmap_canv = gr_create_sub_canvas(grd_curcanv, rescale_x(220), rescale_y(45), 64, 64);	break;
		case 1:		bitmap_canv = gr_create_sub_canvas(grd_curcanv, rescale_x(220), rescale_y(45), 94, 94);	break; // Adam: Change here for your new animating bitmap thing. 94, 94 are bitmap size.
		default:	Int3(); // Impossible, illegal value for Animating_bitmap_type
		}

		curcanv_save = grd_curcanv;
		grd_curcanv = bitmap_canv;

		pound_signp = strchr(Bitmap_name, '#');
		Assert(pound_signp != NULL);

		dig1 = *(pound_signp+1);
		dig2 = *(pound_signp+2);
		if (dig2 == 0)
			num = dig1-'0';
		else
			num = (dig1-'0')*10 + (dig2-'0');

		switch (Animating_bitmap_type) {
			case 0:
				num += Door_dir;
				if (num > EXIT_DOOR_MAX) {
					num = EXIT_DOOR_MAX;
					Door_dir = -1;
				} else if (num < 0) {
					num = 0;
					Door_dir = 1;
				}
				break;
			case 1:
				num++;
				if (num > OTHER_THING_MAX)
					num = 0;
				break;
		}

		Assert(num < 100);
		if (num >= 10) {
			*(pound_signp+1) = (num / 10) + '0';
			*(pound_signp+2) = (num % 10) + '0';
			*(pound_signp+3) = 0;
		} else {
			*(pound_signp+1) = (num % 10) + '0';
			*(pound_signp+2) = 0;
		}

		bitmap_index bi;
		bi = piggy_find_bitmap(Bitmap_name);
		bitmap_ptr = &GameBitmaps[bi.index];
		PIGGY_PAGE_IN( bi );
#ifdef OGL
		ogl_ubitmapm_cs(0,0,(bitmap_ptr->bm_w*(SWIDTH/320)),(bitmap_ptr->bm_h*(SHEIGHT/200)),bitmap_ptr,255,F1_0);
#else
		gr_bitmapm(0, 0, bitmap_ptr);
#endif
		grd_curcanv = curcanv_save;
		free(bitmap_canv);

		switch (Animating_bitmap_type) {
			case 0:
				if (num == EXIT_DOOR_MAX) {
					Door_dir = -1;
					Door_div_count = 64;
				} else if (num == 0) {
					Door_dir = 1;
					Door_div_count = 64;
				}
				break;
			case 1:
				break;
		}
	}

}

//	-----------------------------------------------------------------------------
void show_briefing_bitmap(grs_bitmap *bmp)
{
	grs_canvas	*curcanv_save, *bitmap_canv;
	if (!(Briefing_screens_LH == Briefing_screens_h)) {
		bitmap_canv = gr_create_sub_canvas(grd_curcanv, 220*((double)SWIDTH/320), 45*((double)SHEIGHT/200), 166, 138);
		curcanv_save = grd_curcanv;
		grd_curcanv = bitmap_canv;	
		gr_bitmapm(0, 0, bmp);
		grd_curcanv = curcanv_save;
		free(bitmap_canv);
	}
}

//	-----------------------------------------------------------------------------
void show_spinning_robot_frame(int robot_num)
{
	grs_canvas	*curcanv_save;

	if (robot_num != -1) {
		
		Robot_angles.h += 150;
		curcanv_save = grd_curcanv;
		grd_curcanv = Robot_canv;
		Assert(Robot_info[robot_num].model_num != -1);
		draw_model_picture(Robot_info[robot_num].model_num, &Robot_angles);
		grd_curcanv = curcanv_save;
		
	}

}



//	-----------------------------------------------------------------------------
void init_spinning_robot(void)
{
	int x = rescale_x(138);
	int y = rescale_y(55);
	int w = rescale_x(162);
	int h = rescale_y(134);

	Robot_canv = gr_create_sub_canvas(grd_curcanv, x, y, w, h);
}

//	-----------------------------------------------------------------------------
void init_briefing_bitmap(void)
{
	Robot_canv = gr_create_sub_canvas(grd_curcanv, 138, 55, 166, 138);
}

//	-----------------------------------------------------------------------------
//	Returns char width.
//	If show_robot_flag set, then show a frame of the spinning robot.
int show_char_delay(char the_char, int delay, int robot_num, int cursor_flag)
{
	int	w, h, aw;
	char	message[2];
	fix	start_time;
	int	i;

	start_time = timer_get_fixed_seconds();

	message[0] = the_char;
	message[1] = 0;

	gr_get_string_size(message, &w, &h, &aw );

	Assert((Current_color >= 0) && (Current_color < MAX_BRIEFING_COLORS));

// Draw cursor if there is some delay and caller says to draw cursor
// added/changed on 9/13/98 by adb to fix messy code and add gr_update
     if (delay)
      {
	if (cursor_flag && delay) {
		gr_set_fontcolor(Briefing_foreground_colors[Current_color], -1);
		gr_printf(Briefing_text_x+1, Briefing_text_y, "_" );
	}

	for (i=0; i<2; i++) {
                if (robot_num != -1)
			show_spinning_robot_frame(robot_num);
                show_bitmap_frame();

                gr_update();

		while (timer_get_fixed_seconds() < start_time + delay/2);

		start_time = timer_get_fixed_seconds();
	}

	// Erase cursor
        if (cursor_flag) {
		gr_set_fontcolor(Erase_color, -1);
		gr_printf(Briefing_text_x+1, Briefing_text_y, "_" );
	}
      }
// end additions/changed - adb

	// Draw the character
	gr_set_fontcolor(Briefing_background_colors[Current_color], -1);
	gr_printf(Briefing_text_x, Briefing_text_y, message );

	gr_set_fontcolor(Briefing_foreground_colors[Current_color], -1);
	gr_printf(Briefing_text_x+1, Briefing_text_y, message );

	return w;
}

//	-----------------------------------------------------------------------------
int load_briefing_screen( int screen_num )
{
	int	pcx_error;
	if ((pcx_error=pcx_read_fullscr( Briefing_screens_LH[screen_num].bs_name, New_pal ))!=PCX_ERROR_NONE) {
		printf( "File '%s', PCX load error: %s\n  (It's a briefing screen.  Does this cause you pain?)\n",Briefing_screens_LH[screen_num].bs_name, pcx_errormsg(pcx_error));
		Int3();
		return 0;
	}

	return 0;
}

#define	KEY_DELAY_DEFAULT ((F1_0*28)/1000)

//	-----------------------------------------------------------------------------
int get_message_num(char **message)
{
	int	num=0;

	while (**message == ' ')
		(*message)++;

	while ((**message >= '0') && (**message <= '9')) {
		num = 10*num + **message-'0';
		(*message)++;
	}

	while (*(*message)++ != 10) // Get and drop eoln
		;

	return num;
}

void title_save_game()
{
	grs_canvas * save_canv;
	grs_canvas * save_canv_data;
	grs_font * save_font;
	ubyte palette[768];

	if ( Next_level_num == 0 ) return;
	
	save_canv = grd_curcanv;
	save_font = grd_curcanv->cv_font;

	save_canv_data = gr_create_canvas( grd_curcanv->cv_bitmap.bm_w, grd_curcanv->cv_bitmap.bm_h );
	gr_set_current_canvas(save_canv_data);
	gr_ubitmap(0,0,&save_canv->cv_bitmap);
	gr_set_current_canvas(save_canv);
	gr_clear_canvas(gr_find_closest_color_current( 0, 0, 0));
	gr_palette_read( palette );
	gr_palette_load( gr_palette );
#ifndef SHAREWARE
	state_save_all(1);
#endif
	gr_palette_clear();
	
	gr_set_current_canvas(save_canv);
	gr_ubitmap(0,0,&save_canv_data->cv_bitmap);
	gr_palette_load( palette );
	gr_set_curfont(save_font);
}


//	-----------------------------------------------------------------------------
void get_message_name(char **message, char *result)
{
	while (**message == ' ')
		(*message)++;

	while ((**message != ' ') && (**message != 10)) {
		if (**message != 13)
			*result++ = **message;
		(*message)++;
	}

	if (**message != 10)
		while (*(*message)++ != 10) // Get and drop eoln
			;

	*result = 0;
}

//	-----------------------------------------------------------------------------
void flash_cursor(int cursor_flag)
{
	if (cursor_flag == 0)
		return;

	if ((timer_get_fixed_seconds() % (F1_0/2) ) > (F1_0/4))
		gr_set_fontcolor(Briefing_foreground_colors[Current_color], -1);
	else
		gr_set_fontcolor(Erase_color, -1);

	gr_printf(Briefing_text_x+1, Briefing_text_y, "_" );

}

//	-----------------------------------------------------------------------------
//	Return true if message got aborted by user (pressed ESC), else return false.
int show_briefing_message(int screen_num, char *message)
{
	int	prev_ch=-1;
	int	ch, done=0;
	briefing_screen	*bsp = &Briefing_screens[screen_num];
	int	delay_count = KEY_DELAY_DEFAULT;
	int	key_check;
	int	robot_num=-1;
	int	rval=0;
	int	tab_stop=0;
	int	flashing_cursor=0;
	int	new_page=0;
	int text_ulx = rescale_x(bsp->text_ulx);
	int text_uly = rescale_y(bsp->text_uly);

	Bitmap_name[0] = 0;

	Current_color = 0;

	gr_set_curfont( GAME_FONT );

	init_char_pos(text_ulx, text_uly);

	while (!done) {
		ch = *message++;
		if (ch == '$') {
			ch = *message++;
			if (ch == 'C') {
				Current_color = get_message_num(&message)-1;
				Assert((Current_color >= 0) && (Current_color < MAX_BRIEFING_COLORS));
				prev_ch = 10;
			} else if (ch == 'F') { // toggle flashing cursor
				flashing_cursor = !flashing_cursor;
				prev_ch = 10;
				while (*message++ != 10)
					;
			} else if (ch == 'T') {
				tab_stop = get_message_num(&message);
				prev_ch = 10; // read to eoln
			} else if (ch == 'R') {
				if (Robot_canv != NULL)
					{free(Robot_canv); Robot_canv=NULL;}

				init_spinning_robot();
				robot_num = get_message_num(&message);
				prev_ch = 10; // read to eoln
			} else if (ch == 'N') {
				if (Robot_canv != NULL)
					{free(Robot_canv); Robot_canv=NULL;}

				get_message_name(&message, Bitmap_name);
				strcat(Bitmap_name, "#0");
				Animating_bitmap_type = 0;
				prev_ch = 10;
			} else if (ch == 'O') {
				if (Robot_canv != NULL)
					{free(Robot_canv); Robot_canv=NULL;}

				get_message_name(&message, Bitmap_name);
				strcat(Bitmap_name, "#0");
				Animating_bitmap_type = 1;
				prev_ch = 10;
			} else if (ch == 'B') {
				char		bitmap_name[32];
				grs_bitmap	guy_bitmap;
				ubyte		temp_palette[768];
				int		iff_error;

				if (Robot_canv != NULL)
					{free(Robot_canv); Robot_canv=NULL;}

				get_message_name(&message, bitmap_name);
				strcat(bitmap_name, ".bbm");
				gr_init_bitmap_data (&guy_bitmap);
				iff_error = iff_read_bitmap(bitmap_name, &guy_bitmap, BM_LINEAR, temp_palette);
				Assert(iff_error == IFF_NO_ERROR);

				show_briefing_bitmap(&guy_bitmap);
				gr_free_bitmap_data (&guy_bitmap);
				prev_ch = 10;
			} else if (ch == 'S') {
				int	keypress;
				fix	start_time;
				fix 	time_out_value;

				start_time = timer_get_fixed_seconds();
				start_time = timer_get_approx_seconds();
				time_out_value = start_time + i2f(60*5); // Wait 1 minute...

//added on 9/13/98 by adb to make arch's requiring updates work
				gr_update();
//end changes by adb

				while ( (keypress = local_key_inkey()) == 0 ) { // Wait for a key
					if ( timer_get_approx_seconds() > time_out_value ) {
						keypress = 0;
						break; // Time out after 1 minute..
					}
					while (timer_get_fixed_seconds() < start_time + KEY_DELAY_DEFAULT/2)
						;
					flash_cursor(flashing_cursor);
					show_spinning_robot_frame(robot_num);
					show_bitmap_frame();

//added on 9/13/98 by adb to make arch's requiring updates work
					gr_update();
//end changes by adb

					start_time += KEY_DELAY_DEFAULT/2;
				}

#ifndef NDEBUG
				if (keypress == KEY_BACKSP)
					Int3();
#endif
				if (keypress == KEY_ESC)
					rval = 1;

				flashing_cursor = 0;
				done = 1;
			} else if (ch == 'P') { // New page.
				new_page = 1;
				while (*message != 10) {
					message++; // drop carriage return after special escape sequence
				}
				message++;
				prev_ch = 10;
//Begin D1X addition
			} else if (ch == '$' || ch == ';') { // Print a $/;
				prev_ch = ch;
				Briefing_text_x += show_char_delay(ch, delay_count, robot_num, flashing_cursor);
//End D1X addition
			}
		} else if (ch == '\t') {		//	Tab
			if (Briefing_text_x - text_ulx < tab_stop)
				Briefing_text_x = text_ulx + tab_stop;
		} else if ((ch == ';') && (prev_ch == 10)) {
			while (*message++ != 10)
				;
			prev_ch = 10;
		} else if (ch == '\\') {
			prev_ch = ch;
		} else if (ch == 10) {
			if (prev_ch != '\\') {
				prev_ch = ch;
				Briefing_text_y += FONTSCALE_Y(GAME_FONT->ft_h)+FONTSCALE_Y(GAME_FONT->ft_h)*3/5;
				Briefing_text_x = text_ulx;
				if (Briefing_text_y > text_uly + rescale_y(bsp->text_height)) {
					load_briefing_screen(screen_num);
					Briefing_text_x = text_ulx;
					Briefing_text_y = text_uly;
				}
			} else {
				if (ch == 13)
					Int3();
				prev_ch = ch;
			}
		} else {
			prev_ch = ch;
			Briefing_text_x += show_char_delay(ch, delay_count, robot_num, flashing_cursor);
		}

// added/changed on 9/13/98 by adb to speed up briefings after pressing a key with SDL
// Check for Esc -> abort.
		if(delay_count)
			key_check=local_key_inkey();
		else
			key_check=0;
//end change - adb
		if ( key_check == KEY_ESC ) {
			rval = 1;
			done = 1;
		}

		if ( key_check == KEY_ALTED+KEY_F2 )	
			title_save_game();

		if ((key_check == KEY_SPACEBAR) || (key_check == KEY_ENTER))
			delay_count = 0;

		if (Briefing_text_x > text_ulx + rescale_x(bsp->text_width)) {
			Briefing_text_x = text_ulx;
			Briefing_text_y += GAME_FONT->ft_h+GAME_FONT->ft_h*3/5;
		}

		if ((new_page) || (Briefing_text_y > text_uly + rescale_y(bsp->text_height))) {
			fix	start_time = 0;
			fix	time_out_value = 0;
			int	keypress;

			new_page = 0;
			start_time = timer_get_approx_seconds();
			time_out_value = start_time + i2f(60*5); // Wait 1 minute...

//added on 9/13/98 by adb to make arch's requiring updates work
			gr_update();
//end changes by adb

			while ( (keypress = local_key_inkey()) == 0 ) { // Wait for a key
				if ( timer_get_approx_seconds() > time_out_value ) {
					keypress = 0;
					break; // Time out after 1 minute..
				}
				while (timer_get_approx_seconds() < start_time + KEY_DELAY_DEFAULT/2)
					;
				flash_cursor(flashing_cursor);
				show_spinning_robot_frame(robot_num);
				show_bitmap_frame();

//added on 9/13/98 by adb to make arch's requiring updates work
				gr_update();
//end changes by adb

				start_time += KEY_DELAY_DEFAULT/2;
			}

			robot_num = -1;

#ifndef NDEBUG
			if (keypress == KEY_BACKSP)
				Int3();
#endif
			if (keypress == KEY_ESC) {
				rval = 1;
				done = 1;
			}

			load_briefing_screen(screen_num);
			Briefing_text_x = text_ulx;
			Briefing_text_y = text_uly;
			delay_count = KEY_DELAY_DEFAULT;
		}
	}

	if (Robot_canv != NULL)
		{free(Robot_canv); Robot_canv=NULL;}

	return rval;
}

//	-----------------------------------------------------------------------------
//	Return a pointer to the start of text for screen #screen_num.
char * get_briefing_message(int screen_num)
{
	char	*tptr = Briefing_text;
	int	cur_screen=0;
	int	ch;

	Assert(screen_num >= 0);

	while ( (*tptr != 0 ) && (screen_num != cur_screen)) {
		ch = *tptr++;
		if (ch == '$') {
			ch = *tptr++;
			if (ch == 'S')
				cur_screen = get_message_num(&tptr);
		}
	}

	return tptr;
}

// -----------------------------------------------------------------------------
//	Load Descent briefing text.
void load_screen_text(char *filename, char **buf)
{
	CFILE	*tfile;
	CFILE *ifile;
	int	len, i;
	int	have_binary = 0;

	if ((tfile = cfopen(filename,"rb")) == NULL) {
		char nfilename[30], *ptr;

		strcpy(nfilename, filename);
		if ((ptr = strrchr(nfilename, '.')))
			*ptr = '\0';
		strcat(nfilename, ".txb");
		if ((ifile = cfopen(nfilename, "rb")) == NULL)
			Error("Cannot open file %s or %s", filename, nfilename);
		have_binary = 1;

		len = cfilelength(ifile);
		MALLOC(*buf,char, len);
		cfread(*buf, 1, len, ifile);
		cfclose(ifile);
	} else {
		len = cfilelength(tfile);
		MALLOC(*buf, char, len);
		cfread(*buf, 1, len, tfile);
		cfclose(tfile);
	}

	if (have_binary) {
		char *ptr;

		for (i = 0, ptr = *buf; i < len; i++, ptr++) {
			if (*ptr != '\n') {
				encode_rotate_left(ptr);
				*ptr = *ptr ^ BITMAP_TBL_XOR;
				encode_rotate_left(ptr);
			}
		}
	}
}

//	-----------------------------------------------------------------------------
//	Return true if message got aborted, else return false.
int show_briefing_text(int screen_num)
{
	char	*message_ptr;

	//green
	Briefing_foreground_colors[0] = gr_find_closest_color_current( 0, 54, 0);
	Briefing_background_colors[0] = gr_find_closest_color_current( 0, 19, 0);
	//white
	Briefing_foreground_colors[1] = gr_find_closest_color_current( 42, 38, 32);
	Briefing_background_colors[1] = gr_find_closest_color_current( 14, 14, 14);
//Begin D1X addition
	//red
	Briefing_foreground_colors[2] = gr_find_closest_color_current( 63, 0, 0);
	Briefing_background_colors[2] = gr_find_closest_color_current( 31, 0, 0);
	//blue
	Briefing_foreground_colors[3] = gr_find_closest_color_current( 0, 0, 54);
	Briefing_background_colors[3] = gr_find_closest_color_current( 0, 0, 19);
	//gray
	Briefing_foreground_colors[4] = gr_find_closest_color_current( 14, 14, 14);
	Briefing_background_colors[4] = gr_find_closest_color_current( 0, 0, 0);
	//yellow
	Briefing_foreground_colors[5] = gr_find_closest_color_current( 54, 54, 0);
	Briefing_background_colors[5] = gr_find_closest_color_current( 19, 19, 0);
	//purple
	Briefing_foreground_colors[6] = gr_find_closest_color_current( 0, 54, 54);
	Briefing_background_colors[6] = gr_find_closest_color_current( 0, 19, 19);
//End D1X addition

	Erase_color = gr_find_closest_color_current(0, 0, 0);

	message_ptr = get_briefing_message(Briefing_screens[screen_num].message_num);

	return show_briefing_message(screen_num, message_ptr);
}

//	-----------------------------------------------------------------------------
//	Return true if screen got aborted by user, else return false.
int show_briefing_screen( int screen_num, int allow_keys)
{
	int	rval=0;
	int	pcx_error;
	grs_bitmap briefing_bm;

	New_pal_254_bash = 0;

	if (Skip_briefing_screens) {
		mprintf((0, "Skipping briefing screen [%s]\n", &Briefing_screens[screen_num].bs_name));
		return 0;
	}

	gr_init_bitmap_data (&briefing_bm);

	if ((pcx_error=pcx_read_bitmap( Briefing_screens_LH[screen_num].bs_name, &briefing_bm, BM_LINEAR, New_pal ))!=PCX_ERROR_NONE) {
		printf( "PCX load error: %s.  File '%s'\n\n", pcx_errormsg(pcx_error), Briefing_screens_LH[screen_num].bs_name);
		mprintf((0, "File '%s', PCX load error: %s (%i)\n  (It's a briefing screen.  Does this cause you pain?)\n",Briefing_screens_LH[screen_num].bs_name, pcx_errormsg(pcx_error), pcx_error));
		Int3();
		return 0;
	}
// 	
#ifdef OGL
	gr_palette_load(New_pal);
#else
	gr_palette_clear();
#endif

		
        show_fullscr(&briefing_bm );

//added on 9/13/98 by adb to make arch's requiring updates work
	gr_update();
//end changes by adb

	gr_free_bitmap_data (&briefing_bm);

	if (gr_palette_fade_in( New_pal, 32, allow_keys ))	
		return 1;

	rval = show_briefing_text(screen_num);

	if (gr_palette_fade_out( New_pal, 32, allow_keys ))
		return 1;

	return rval;
}


//	-----------------------------------------------------------------------------
void do_briefing_screens(int level_num)
{
	int	abort_briefing_screens = 0;
	int	cur_briefing_screen = 0;

	if (Skip_briefing_screens) {
		mprintf((0, "Skipping all briefing screens.\n"));
		return;
	}

	if (!Briefing_text_filename[0]) //no filename?
		return;

	songs_play_song( SONG_BRIEFING, 1 );

	set_screen_mode( SCREEN_MENU );
	gr_set_current_canvas(NULL);

	key_flush();

	load_screen_text(Briefing_text_filename, &Briefing_text);

	if (level_num == 1) {
		while ((!abort_briefing_screens) && (Briefing_screens[cur_briefing_screen].level_num == 0)) {
			abort_briefing_screens = show_briefing_screen(cur_briefing_screen, 0);
			cur_briefing_screen++;
		}
	}

	if (!abort_briefing_screens) {
		for (cur_briefing_screen = 0; cur_briefing_screen < MAX_BRIEFING_SCREEN; cur_briefing_screen++)
			if (Briefing_screens[cur_briefing_screen].level_num == level_num)
				if (show_briefing_screen(cur_briefing_screen, 0))
					break;
	}

	free(Briefing_text);

	key_flush();
}

void show_order_form()
{
	show_title_screen(
#ifdef SHAREWARE
	"order01.pcx",
#else
	"warning.pcx",
#endif
	1);
}

#ifndef SHAREWARE
void do_registered_end_game(void)
{
	int	cur_briefing_screen;

	if (!Ending_text_filename[0]) //no filename?
		return;

	if ((Game_mode & GM_MULTI) && !(Game_mode & GM_MULTI_COOP))
	{
		// Special ending for deathmatch!!
		int len = 40;
		
		MALLOC(Briefing_text, char, len);
		sprintf(Briefing_text, "Test");
	}
		
	load_screen_text(Ending_text_filename, &Briefing_text);

	for (cur_briefing_screen = 0; cur_briefing_screen < MAX_BRIEFING_SCREEN; cur_briefing_screen++)
		if (Briefing_screens[cur_briefing_screen].level_num == REGISTERED_ENDING_LEVEL_NUM)
			if (show_briefing_screen(cur_briefing_screen, 0))
				break;

}
#endif

void do_shareware_end_game(void)
{
	int	cur_briefing_screen;

#ifdef NETWORK
	if ((Game_mode & GM_MULTI) && !(Game_mode & GM_MULTI_COOP))
	{
		MALLOC(Briefing_text, char, 4); // Dummy
		kmatrix_view();
		return;
	}
	else 
#endif
	{
#ifdef DEST_SAT
		load_screen_text(Ending_text_filename, &Briefing_text);
#else
		load_screen_text(SHAREWARE_ENDING_FILENAME, &Briefing_text);
#endif
	}

	for (cur_briefing_screen = 0; cur_briefing_screen < MAX_BRIEFING_SCREEN; cur_briefing_screen++)
		if (Briefing_screens[cur_briefing_screen].level_num == SHAREWARE_ENDING_LEVEL_NUM)
			if (show_briefing_screen(cur_briefing_screen, 0))
				break;

}

void do_end_game(void)
{
	set_screen_mode( SCREEN_MENU );
	gr_set_current_canvas(NULL);

	key_flush();

	#ifdef SHAREWARE
	do_shareware_end_game(); //hurrah! you win!
	#else
		#ifdef DEST_SAT
			do_shareware_end_game(); //hurrah! you win!
		#else
			do_registered_end_game(); //hurrah! you win!
		#endif
	#endif

	if (Briefing_text) {
		free(Briefing_text);
		Briefing_text = NULL;
	}

	key_flush();

	Function_mode = FMODE_MENU;

	if ((Game_mode & GM_SERIAL) || (Game_mode & GM_MODEM))
		Game_mode |= GM_GAME_OVER;
	else
		Game_mode = GM_GAME_OVER;

#ifdef DEST_SAT
	show_order_form();
#endif

#ifdef SHAREWARE
	show_order_form();
#endif

}
