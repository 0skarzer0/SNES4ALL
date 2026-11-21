#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "menu.h"
#include "sdlinterface.h"

#ifdef DREAMCAST
extern int __sdl_dc_wait_vblank;
#endif

static char *text_str_title="  SNES4ALL Beta2 ";
static char *text_str_load="Select ROM (X)";
static char *text_str_save="SaveStates (Y)";
static char *text_str_autosave="Save SRAM";
static char *text_str_hw_render="HW Render";
static char *text_str_throttle="Throttle ";
static char *text_str_frameskip="Frameskip";
static char *text_str_transparency="Transparency";
static char *text_str_raster="Raster";
static char *text_str_25="25";
static char *text_str_50="50";
static char *text_str_75="75";
static char *text_str_100="100";
static char *text_str_0="0";
static char *text_str_1="1";
static char *text_str_2="2";
static char *text_str_3="3";
static char *text_str_4="4";
static char *text_str_5="5";
static char *text_str_auto="Auto";
static char *text_str_sound="Sound";
static char *text_str_on="On";
static char *text_str_off="Off";
static char *text_str_fast="Fast";
static char *text_str_normal="Reg";
static char *text_str_sync="Sync";
static char *text_str_no="No";
static char *text_str_0p="0/";
static char *text_str_25p="25/";
static char *text_str_50p="50/";
static char *text_str_75p="75/";
static char *text_str_100p="100/";
#ifdef NORES256
static char *text_str_separator="------------------------------";
#else
static char *text_str_separator="-----------------------------";
#endif
static char *text_str_reset="Reset (R)";
static char *text_str_run="Run (L)";
static char *text_str_exit="Exit - Reboot";

int mainMenu_case=-1;
extern int	snes4all_raster;
extern int	snes4all_throttle;
extern int	snes4all_frameskip;
extern int	snes4all_transparency;
extern int	snes4all_sound_enable;
extern int	snes4all_autosave;
extern int	snes4all_hw_render;
extern int menu_moving;
extern char *rom_filename;
extern int snes4all_emulating;

enum { MAIN_MENU_CASE_REBOOT, MAIN_MENU_CASE_LOAD, MAIN_MENU_CASE_RUN, MAIN_MENU_CASE_RESET, MAIN_MENU_CASE_CANCEL, MAIN_MENU_CASE_SAVE };

#include<dirent.h>
static int list_files(void)
{
	int found=0;
	DIR *d=opendir("/cd");
	unsigned i;
	while(d!=NULL && !found)
	{
		struct dirent *actual=readdir(d);
		if (actual==NULL)
			break;
		found++;
	}
	closedir(d);
	return found;
}

static void draw_mainMenu(int c)
{
	static int b=0;
	int bb=(b&3); //(b%6)/3;

	text_draw_background();
	text_draw_window(40,24,260,200+8,text_str_title);
	write_text(6,4,text_str_separator);
	if ((c==0)&&(bb))
		write_text_sel(6,5,text_str_load);
	else
		write_text(6,5,text_str_load);

	write_text(6,6,text_str_separator);
	if ((c==1)&&(bb))
		write_text_sel(6,7,text_str_save);
	else
		write_text(6,7,text_str_save);
	write_text(6,8,text_str_separator);

	write_text(6,9,text_str_throttle);
	if ((snes4all_throttle==0)&&((c!=2)||(bb)))
		write_text_sel3(19,9,text_str_0);
	else
		write_text(19,9,text_str_0);
	if ((snes4all_throttle==1)&&((c!=2)||(bb)))
		write_text_sel3(21,9,text_str_25);
	else
		write_text(21,9,text_str_25);
	if ((snes4all_throttle==2)&&((c!=2)||(bb)))
		write_text_sel3(24,9,text_str_50);
	else
		write_text(24,9,text_str_50);
	if ((snes4all_throttle==3)&&((c!=2)||(bb)))
		write_text_sel3(27,9,text_str_75);
	else
		write_text(27,9,text_str_75);
	if ((snes4all_throttle==4)&&((c!=2)||(bb)))
		write_text_sel3(30,9,text_str_100);
	else
		write_text(30,9,text_str_100);

	write_text(6,11,text_str_frameskip);
	if ((snes4all_frameskip==0)&&((c!=3)||(bb)))
		write_text_sel3(19,11,text_str_0);
	else
		write_text(19,11,text_str_0);
	if ((snes4all_frameskip==1)&&((c!=3)||(bb)))
		write_text_sel3(21,11,text_str_1);
	else
		write_text(21,11,text_str_1);
	if ((snes4all_frameskip==2)&&((c!=3)||(bb)))
		write_text_sel3(23,11,text_str_2);
	else
		write_text(23,11,text_str_2);
	if ((snes4all_frameskip==3)&&((c!=3)||(bb)))
		write_text_sel3(25,11,text_str_3);
	else
		write_text(25,11,text_str_3);
	if ((snes4all_frameskip==4)&&((c!=3)||(bb)))
		write_text_sel3(27,11,text_str_4);
	else
		write_text(27,11,text_str_4);
	if ((snes4all_frameskip==5)&&((c!=3)||(bb)))
		write_text_sel3(29,11,text_str_5);
	else
		write_text(29,11,text_str_5);
	if ((snes4all_frameskip==-1)&&((c!=3)||(bb)))
		write_text_sel3(31,11,text_str_auto);
	else
		write_text(31,11,text_str_auto);

	write_text(6,13,text_str_transparency);
	if ((snes4all_transparency==0)&&((c!=4)||(bb)))
		write_text_sel3(19,13,text_str_off);
	else
		write_text(19,13,text_str_off);
	if ((snes4all_transparency!=0)&&((c!=4)||(bb)))
		write_text_sel3(25,13,text_str_on);
	else
		write_text(25,13,text_str_on);

	write_text(6,15,text_str_raster);
	if ((snes4all_raster==0)&&((c!=5)||(bb)))
		write_text_sel2(19,15,text_str_0p);
	else
		write_text(19,15,text_str_0p);
	if ((snes4all_raster==1)&&((c!=5)||(bb)))
		write_text_sel2(21,15,text_str_25p);
	else
		write_text(21,15,text_str_25p);
	if ((snes4all_raster==2)&&((c!=5)||(bb)))
		write_text_sel2(24,15,text_str_50p);
	else
		write_text(24,15,text_str_50p);
	if ((snes4all_raster==3)&&((c!=5)||(bb)))
		write_text_sel2(27,15,text_str_75p);
	else
		write_text(27,15,text_str_75p);
	if ((snes4all_raster==4)&&((c!=5)||(bb)))
		write_text_sel2(30,15,text_str_100p);
	else
		write_text(30,15,text_str_100p);

	write_text(6,17,text_str_sound);
	if ((snes4all_sound_enable==0)&&((c!=6)||(bb)))
		write_text_sel3(19,17,text_str_no);
	else
		write_text(19,17,text_str_no);
	if ((snes4all_sound_enable==1)&&((c!=6)||(bb)))
		write_text_sel3(22,17,text_str_fast);
	else
		write_text(22,17,text_str_fast);
	if ((snes4all_sound_enable==2)&&((c!=6)||(bb)))
		write_text_sel3(27,17,text_str_normal);
	else
		write_text(27,17,text_str_normal);
	if ((snes4all_sound_enable==3)&&((c!=6)||(bb)))
		write_text_sel3(31,17,text_str_sync);
	else
		write_text(31,17,text_str_sync);

	write_text(6,19,text_str_autosave);
	if ((!snes4all_autosave)&&((c!=7)||(bb)))
		write_text_sel3(19,19,text_str_off);
	else
		write_text(19,19,text_str_off);
	if ((snes4all_autosave)&&((c!=7)||(bb)))
		write_text_sel3(25,19,text_str_on);
	else
		write_text(25,19,text_str_on);


#ifdef USE_GL
	write_text(6,21,text_str_hw_render);
	if ((!snes4all_hw_render)&&((c!=8)||(bb)))
		write_text_sel3(19,21,text_str_off);
	else
		write_text(19,21,text_str_off);
	if ((snes4all_hw_render)&&((c!=8)||(bb)))
		write_text_sel3(25,21,text_str_on);
	else
		write_text(25,21,text_str_on);

	write_text(6,22,text_str_separator);
	if ((c==9)&&(bb))
		write_text_sel(6,23,text_str_reset);
	else
		write_text(6,23,text_str_reset);

	write_text(6,24,text_str_separator);
	if ((c==10)&&(bb))
		write_text_sel(6,25,text_str_run);
	else
		write_text(6,25,text_str_run);

	write_text(6,26,text_str_separator);
	if ((c==11)&&(bb))
		write_text_sel(6,27,text_str_exit);
	else
		write_text(6,27,text_str_exit);
#else
	write_text(6,20,text_str_separator);
	if ((c==8)&&(bb))
		write_text_sel(6,21,text_str_reset);
	else
		write_text(6,21,text_str_reset);

	write_text(6,22,text_str_separator);
	if ((c==9)&&(bb))
		write_text_sel(6,23,text_str_run);
	else
		write_text(6,23,text_str_run);
	write_text(6,24,text_str_separator);

	write_text(6,25,text_str_separator);
	if ((c==10)&&(bb))
		write_text_sel(6,26,text_str_exit);
	else
		write_text(6,26,text_str_exit);
	write_text(6,27,text_str_separator);
#endif

	text_flip();
	b++;
}

static int key_mainMenu(int *cp)
{
	int back_c=-1;
	int c=(*cp);
	int end=0;
	int left=0, right=0, up=0, down=0, hit0=0, hit1=0, hit2=0, hit3=0, hit4=0, hit5=0;
	SDL_Event event;

	while (SDL_PollEvent(&event) > 0)
	{
		if (event.type == SDL_QUIT)
		{
			mainMenu_case=MAIN_MENU_CASE_REBOOT;
			end=-1;
		}
		else
		if (event.type == SDL_KEYDOWN)
		{
			sound_play_beep();
			switch(event.key.keysym.sym)
			{
				case SDLK_RIGHT: right=1; break;
				case SDLK_LEFT: left=1; break;
				case SDLK_UP: up=1; break;
				case SDLK_DOWN: down=1; break;
				case SDLK_RETURN:
				case SDLK_LCTRL: hit0=1; break;
				case SDLK_LALT: hit1=1; break;
				case SDLK_TAB: hit2=1; break;
				case SDLK_LSHIFT: hit3=1; break;
				case SDLK_SPACE: hit4=1; break;
				case SDLK_BACKSPACE: hit5=1; break;
			}
			if (hit1)
			{
				mainMenu_case=MAIN_MENU_CASE_CANCEL;
				end=1;
			}
			else if (hit2)
			{
				back_c=c;
				hit0=1;
#ifdef USE_GL
				c=9;
#else
				c=8;
#endif
			}
			else if (hit3)
			{
				mainMenu_case=MAIN_MENU_CASE_LOAD;
				end=1;
			}
			else if (hit4)
			{
				mainMenu_case=MAIN_MENU_CASE_SAVE;
				end=1;
			}
			else if (hit5)
			{
				back_c=c;
				hit0=1;
#ifdef USE_GL
				c=10;
#else
				c=9;
#endif
			}
			if (up)
			{
#ifdef USE_GL
				if (c>0) c=(c-1)%12;
				else c=11;
#else
				if (c>0) c=(c-1)%11;
				else c=10;
#endif
			}
			else if (down)
#ifdef USE_GL
				c=(c+1)%12;
#else
				c=(c+1)%11;
#endif
			else
			switch(c)
			{
				case 0:	// LOAD ROM 
					if (hit0)
					{
						mainMenu_case=MAIN_MENU_CASE_LOAD;
						end=1;
					}
					break;
				case 1: // SAVESTATE
					if (hit0)
					{
						mainMenu_case=MAIN_MENU_CASE_SAVE;
						end=1;
					}
					break;
				case 2: // THROTTLE
					if (left)
					{
						if (snes4all_throttle>0)
							snes4all_throttle--;
						else
							snes4all_throttle=4;
					}
					else if (right)
					{
						if (snes4all_throttle<4)
							snes4all_throttle++;
						else
							snes4all_throttle=0;
					}
					break;
				case 3:	// FRAMESKIP
					if (left)
					{
						if (snes4all_frameskip>-1)
							snes4all_frameskip--;
						else
							snes4all_frameskip=5;
					}
					else if (right)
					{
						if (snes4all_frameskip<5)
							snes4all_frameskip++;
						else
							snes4all_frameskip=-1;
					}
					break;
				case 4: // TRANSPARENCY
					if ((left)||(right))
						 snes4all_transparency=~snes4all_transparency;
					break;
				case 5: // RASTER
					if (left) {
						if (!snes4all_raster) snes4all_raster=4;
						else snes4all_raster--;
					} else if (right) {
						if (snes4all_raster==4) snes4all_raster=0;
						else snes4all_raster++;
					}
					break;
				case 6:	// SOUND
					if (left) {
						if (!snes4all_sound_enable) snes4all_sound_enable=3;
						else snes4all_sound_enable--;
					} else if (right) {
						if (snes4all_sound_enable==3) snes4all_sound_enable=0;
						else snes4all_sound_enable++;
					}
					break;
				case 7: // SRAM
					if ((left)||(right))
						 snes4all_autosave=~snes4all_autosave;
					break;
				case 8:	// RESET / HW_RENDER
#ifdef USE_GL
					if ((left)||(right))
						 snes4all_hw_render=~snes4all_hw_render;
					
					break;
				case 9:
#endif
					if (hit0)
					{
						mainMenu_case=MAIN_MENU_CASE_RESET;
						end=1;
					}
					break;
#ifdef USE_GL
				case 10:// RUN
#else
				case 9:	// RUN
#endif
					if (hit0)
					{
						mainMenu_case=MAIN_MENU_CASE_RUN;
						end=1;
					}
					break;
#ifdef USE_GL
				case 11:// EXIT
#else
				case 10:// EXIT
#endif
					if (hit0)
					{
						mainMenu_case=MAIN_MENU_CASE_REBOOT;
						end=1;
					}
					break;
			}
			if (back_c>=0)
			{
				c=back_c;
				back_c=-1;
			}
		}
	}


	(*cp)=c;
	return end;
}

static void raise_mainMenu()
{
	int i;

	text_draw_background();
	text_flip();
	for(i=0;i<10;i++)
	{
		text_draw_background();
		text_draw_window(40,(10-i)*24,260,200,text_str_title);
		text_flip();
	}
}

static void unraise_mainMenu()
{
	int i;

	for(i=9;i>=0;i--)
	{
		text_draw_background();
		text_draw_window(40,(10-i)*24,260,200,text_str_title);
		text_flip();
	}
	text_draw_background();
	text_flip();
}

void drawNoROM(void)
{
	int i;
	for(i=0;i<200;i++)
	{
		text_draw_background();
		text_draw_window(64,60,160,32,"ERROR");
		write_text(10,8,"No ROM selected");
		text_flip();
	}
}

int run_mainMenu()
{
	static int c=0;
	int end;
	mainMenu_case=-1;

	menu_moving=1;
	while(mainMenu_case<0)
	{
		raise_mainMenu();
		end=0;
		while(!end)
		{
			draw_mainMenu(c);
			end=key_mainMenu(&c);
		}
		unraise_mainMenu();
		switch(mainMenu_case)
		{
			case MAIN_MENU_CASE_SAVE:
#ifndef NO_SAVE_MENU
				run_menuSave();
				extern int savestate_state;
				if (savestate_state)
					mainMenu_case=1;
				else
					mainMenu_case=-1;
#else
				mainMenu_case=-1;
#endif
				break;
			case MAIN_MENU_CASE_LOAD:
				run_menuLoad();	
				mainMenu_case=-1;
				break;
			case MAIN_MENU_CASE_RESET:
				if (snes4all_emulating)
				{
					mainMenu_case=2;
					break;
				}
			case MAIN_MENU_CASE_RUN:
				if (!rom_filename[0] && !snes4all_emulating)
				{
					mainMenu_case=-1;
					drawNoROM();
					break;
				}
				mainMenu_case=1;
				break;
			case MAIN_MENU_CASE_REBOOT:
				menu_unraise();
				SDL_Delay(333);
				SDL_Quit();
#if defined(DREAMCAST) && defined(REBOOT_DREAMCAST)
				arch_reboot();
#else
				exit(0);
#endif
				break;
			default:
				mainMenu_case=-1;
		}
	}

	menu_moving=0;
	text_draw_background();
	text_draw_window(96,64,140,32,"-------");
	write_text(14,9,"Please wait");
	text_flip();

	return mainMenu_case;
}
