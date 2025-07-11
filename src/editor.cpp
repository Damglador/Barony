/*-------------------------------------------------------------------------------

	BARONY
	File: editor.cpp
	Desc: main code for the level editor

	Copyright 2013-2016 (c) Turning Wheel LLC, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#include "main.hpp"
#include "draw.hpp"
#include "editor.hpp"
#include "entity.hpp"
#include "items.hpp"
#include "player.hpp"
#include "interface/interface.hpp"
#include "files.hpp"
#include "init.hpp"
#include "mod_tools.hpp"
#include <sys/stat.h>
#ifdef LINUX
#include <filesystem>
#endif
#ifndef EDITOR
#define EDITOR
#endif

#ifdef STEAMWORKS
#include <steam/steam_api.h>
#include "steam.hpp"
#endif // STEAMWORKS


//#include "player.hpp"

std::map<int, std::string> modelFileNames;
Entity* selectedEntity[MAXPLAYERS] = { nullptr };
Entity* lastSelectedEntity[MAXPLAYERS] = { nullptr };
Sint32 mousex = 0, mousey = 0;
Sint32 omousex = 0, omousey = 0;
Sint32 mousexrel = 0, mouseyrel = 0;
int itemSelect = 0;
int itemSlotSelected = -1;
char itemName[128];
real_t prev_x = 0;
real_t prev_y = 0;
bool duplicatedSprite = false;
int game = 0;
// function prototypes
Uint32 timerCallback(Uint32 interval, void* param);
bool handleEvents(void);
void mainLogic(void);
std::vector<Entity*> groupedEntities;
bool moveSelectionNegativeX = false;
bool moveSelectionNegativeY = false;
std::vector<std::string> mapNames;
std::list<std::string> modFolderNames;
std::string physfs_saveDirectory = BASE_DATA_DIR;
std::string physfs_openDirectory = BASE_DATA_DIR;
float limbs[NUMMONSTERS][20][3]; // dummy variable for files.cpp limbs reloading in Barony.
void buttonStartSingleplayer(button_t* my) {} // dummy function for mod_tools.cpp
void steamStatisticUpdate(int statisticNum, ESteamStatTypes type, int value) {} // dummy function for mod_tools.cpp
//AchievementObserver achievementObserver; // dummy function for mod_tools.cpp
void initClass(int i) {} // dummy function for mod_tools.cpp
//void AchievementObserver::updateGlobalStat(int index, int value) {}
std::vector<std::pair<SDL_Surface**, std::string>> systemResourceImages; // dummy variable for files.cpp system resource reloading in Barony.
void initMenuOptions() {} // dummy
int textInsertCaratPosition = -1;
GenericGUIMenu GenericGUI[MAXPLAYERS];

void actGib(Entity* my) {} // dummy for draw.cpp
void actHudArm(Entity* my) {} // dummy for draw.cpp
void actHudWeapon(Entity* my) {} // dummy for draw.cpp
void actHUDMagicParticle(Entity* my) {} // dummy for draw.cpp
void actHUDMagicParticleCircling(Entity* my) {} // dummy for draw.cpp
void actHudShield(Entity* my) {} // dummy for draw.cpp
void actHudAdditional(Entity* my) {} // dummy for draw.cpp
void actHudArrowModel(Entity* my) {} // dummy for draw.cpp
void actLeftHandMagic(Entity* my) {} // dummy for draw.cpp
void actRightHandMagic(Entity* my) {} // dummy for draw.cpp
bool messagePlayer(int player, Uint32 type, char const * const message, ...) {return true;} // dummy

map_t copymap;

int errorMessage = 0;
int errorArr[12] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

char monsterPropertyNames[14][16] = 
{
	"Name:",
	"MAX HP:",
	"HP:",
	"MAX MP:",
	"MP:",
	"LEVEL:",
	"GOLD:",
	"STR:",
	"DEX:",
	"CON:",
	"INT:",
	"PER:",
	"CHR:",
	"Is NPC:"
};

char chestPropertyNames[4][40] =
{
	"Orientation: (0-3)",
	"Chest Type: (0-7)",
	"Locked Chance: (0-100%)",
	"Mimic Chance: (0-100%)"
};

char summonTrapPropertyNames[7][44] =
{
	"Monster To Spawn: (-1 to 32)",
	"Quantity Per Spawn: (1-9)",
	"Time Between Spawns: (1-999s)",
	"Amount of Spawn Instances: (1-99)",
	"Requires Power to Disable: (0-1)",
	"Chance to Stop Working Each Spawn: (0-100%)",
	"Autospawn Next To Player (0-16)"
};

char itemPropertyNames[6][36] =
{
	"Item ID: (1-255)",
	"Status: (0-5)",
	"Blessing: (-9 to +9)",
	"Quantity: (1-99)",
	"Identified: (0-2)",
	"Category: (0-16, if random_item)"
};

char itemCategoryNames[17][32] =
{
	"random",
	"weapon",
	"armor",
	"amulet",
	"potion",
	"scroll",
	"magicstaff",
	"ring",
	"spellbook",
	"gem",
	"thrown",
	"tool",
	"food",
	"book",
	"equipment",
	"jewelry",
	"magical"
};

char powerCrystalPropertyNames[4][39] =
{
	"Orientation: (0-3)",
	"Powered Distance (0-99)",
	"Rotation Direction: (0-1)",
	"Require Unlock Spell to Activate (0-1)"
};

char monsterItemPropertyNames[7][36] =
{
	"Item ID: (0-255)",
	"Status: (0-5)",
	"Blessing: (-9 to +9)",
	"Quantity: (1-99)",
	"Identified: (0-2)",
	"Chance (1-100)",
	"Category: (0-16, if default_random)"
};

char leverTimerPropertyNames[1][26] =
{
	"Powered Duration (1-999s)"
};

char boulderTrapPropertyNames[3][42] =
{
	"Amount of times to re-fire (-1 - 99)",
	"Delay between re-fire (2-999s)",
	"Pre-delay for first time trigger (0-999s)"
};

char pedestalPropertyNames[5][35] =
{
	"Orb Type (0-3)",
	"Pre-loaded with Orb (0-1)",
	"Inverted power generation (0-1)",
	"Pedestal start beneath ground(0-1)",
	"Lock orb when placed(0-1)"
};

char teleporterPropertyNames[3][25] =
{
	"X Coordinate to teleport",
	"Y Coordinate to teleport",
	"Type of sprite (0-2)"
};

char ceilingTilePropertyNames[4][30] =
{
	"Model texture to use (0-9999)",
	"Direction (0-3)",
	"Allow boulder trap gen (0-2)",
	"Allow minotaur break (0-1)"
};

char spellTrapPropertyNames[5][38] =
{
	"Spell Type: (-1 - 9)",
	"Amount of times to refire (-1 - 99)",
	"Power once to continuously fire (0-1)",
	"Ceiling model to use (0-9999)",
	"Trap refire rate (1-999s)",
};

char shrineTeleportPropertyNames[4][48] =
{
	"Direction (-1 - 3)",
	"Height Offset (Qtrs of a voxel, +ive is higher)",
	"Destination X Offset",
	"Destination Y Offset"
};

char furniturePropertyNames[1][19] =
{
	"Direction (-1 - 7)"
};

char floorDecorationPropertyNames[9][59] =
{
	"Model texture to use (0-9999)",
	"Direction (-1 - 7)",
	"Height Offset (Qtrs of a voxel, +ive is higher)",
	"X Offset (L/R, Qtrs of a voxel, +ive is right)",
	"Y Offset (U/D, Qtrs of a voxel, +ive is down)",
	"Interact Text",
	"",
	"",
	""
};

char colliderDecorationPropertyNames[11][59] =
{
	"Model texture to use (0-9999)",
	"Direction (-1 - 7)",
	"Height Offset (Qtrs of a voxel, +ive is higher)",
	"X Offset (L/R, Qtrs of a voxel, +ive is right)",
	"Y Offset (U/D, Qtrs of a voxel, +ive is down)",
	"Has Collision (0 - no collision, 1 - collision",
	"Collision X (8 - full width, 4 - half width)",
	"Collision Y (8 - full width, 4 - half width)",
	"HP (0 - not damageable)",
	"Diggable (0-1)",
	"Damage Types"
};

char soundSourcePropertyNames[5][59] =
{
	"Sound source line number to play from sounds.txt (0-999)",
	"Volume (0-255)",
	"Play once only (0-1)",
	"Activation delay (0-9999 ticks, 50 ticks / sec)",
	"Sound origin (0 = this entity, 1 = global)"
};

char lightSourcePropertyNames[7][48] =
{
	"Light always on (0-1)",
	"Brightness (0-255)",
	"Invert power (0-1)",
	"Light/unlight once only (0-1)",
	"Tile radius of light source (0-64)",
	"Light flicker enable (0-1)",
	"Activation delay (0-9999 ticks, 50 ticks / sec)"
};

char textSourcePropertyNames[10][45] =
{
	"Color (R, G, B) (0-255):",
	"",
	"",
	"Text:",
	"",
	"",
	"",
	"",
	"Message delay (0-9999 ticks, 50 ticks / sec)",
	"Send message once only (0 - 1)"
};

char customPortalPropertyNames[7][54] =
{
	"Model texture to use (0-9999)",
	"Animation frames (0-9)",
	"Model Height Offset (Qtrs of a voxel, +ive is higher)",
	"Levels to advance (-99 - 99)",
	"Level name to jump to (Can be used with above option)",
	"Requires power to be visible (0-1)",
	"Exit toggle between secret levels file (0-1)"
};

char signalTimerPropertyNames[6][55] =
{
	"Input signal direction (0 - 3)",
	"Output activation delay (0-9999 ticks, 50 ticks / sec)",
	"Output pulse time (0 - 9999 ticks, 50 ticks / sec)",
	"Output repeat count (0 - 9999)",
	"Latch input and keep powered (0 - 1)",
	"Invert Output (0 - 1)"
};

char ANDGatePropertyNames[6][55] =
{
	"Output signal direction (0 - 3)",
	"Output activation delay (0-9999 ticks, 50 ticks / sec)",
	"Output pulse time (0 - 9999 ticks, 50 ticks / sec)",
	"Output repeat count (0 - 9999)",
	"Latch input and keep powered (0 - 1)",
	"Invert Output (0 - 1)"
};

char pressurePlatePropertyNames[1][34] =
{
	"Trigger Type: (0-7)"
};

char tablePropertyNames[3][34] =
{
	"Direction (-1 - 7)",
	"Spawn chairs (-1 - 4)",
	"Spawn item on table % (-1 - 100)",
};

char readableBookPropertyNames[4][32] =
{
	"Status: (0-5)",
	"Blessing: (-9 to +9)",
	"Identified: (0-2)",
	"Book Title:"
};

char doorPropertyNames[3][42] =
{
	"Force Door Locked/Unlocked (0-2)",
	"Disable unlocking with lockpick (0-1)",
	"Disable unlocking with spell (0-1)"
};

char gatePropertyNames[1][35] =
{
	"Disable unlocking with spell (0-1)"
};

char playerSpawnPropertyNames[1][35] =
{
	"Spawn Facing Direction (-1 - 7)"
};

char statuePropertyNames[2][16] =
{
	"Direction (0-3)",
	"Statue ID",
};

const char* playerClassLangEntry(int classnum, int playernum)
{
	if ( classnum >= CLASS_BARBARIAN && classnum <= CLASS_JOKER )
	{
		return Language::get(1900 + classnum);
	}
	else if ( classnum >= CLASS_CONJURER )
	{
		return Language::get(3223 + classnum - CLASS_CONJURER);
	}
	else if ( classnum >= CLASS_SEXTON && classnum <= CLASS_MONK )
	{
		return Language::get(2550 + classnum - CLASS_SEXTON);
	}
	else
	{
		return "undefined classname";
	}
}

/*-------------------------------------------------------------------------------

mouseInBounds

Returns true if the mouse is within the rectangle specified, otherwise
returns false

-------------------------------------------------------------------------------*/

bool mouseInBounds(int x1, int x2, int y1, int y2)
{
	if ( omousey >= y1 && omousey < y2 )
		if ( omousex >= x1 && omousex < x2 )
		{
			return true;
		}

	return false;
}

int recentUsedTiles[9][9] = { 0 };
int recentUsedTilePalette = 0;
int lockTilePalette[9] = { 0 };
int lastPaletteTileSelected = 0;

void closeNetworkInterfaces()
{
	//Because Dennis.
}

/*-------------------------------------------------------------------------------

	mainLogic

	handles time dependent procedures

-------------------------------------------------------------------------------*/

view_t camera_vel;
view_t camera;

void mainLogic(void)
{
	// messages
	if ( messagetime > 0 )
	{
		messagetime--;
	}

	if ( errorMessage > 0 )
	{
		errorMessage--;
		if ( errorMessage == 0 )
		{
			for ( int i = 0; i < sizeof(errorArr) / sizeof(int); i++ )
				errorArr[i] = 0;
		}
	}
	

	// basic editing functions are not available under these cases
	if ( subwindow || tilepalette || spritepalette )
	{
		return;
	}

	// scroll camera on minimap
	if ( mousestatus[SDL_BUTTON_LEFT] && toolbox )
	{
		if ( omousex >= xres - 120 && omousex < xres - 8 )
		{
			if ( omousey >= 24 && omousey < 136 )
			{
				camx = ((long)map.width << TEXTUREPOWER) * (real_t)(mousex - xres + 120) / 112 - xres / 2;
				camy = ((long)map.height << TEXTUREPOWER) * (real_t)(mousey - 24) / 112 - yres / 2;
			}
		}
	}
	camx -= camx % TEXTURESIZE; // make sure the camera is a multiple of 32 for hover text to work.
	camy -= camy % TEXTURESIZE; // make sure the camera is a multiple of 32 for hover text to work.

	// basic editor functions
	if ( mode3d == false )
	{
		camx += (keystatus[SDLK_RIGHT] - keystatus[SDLK_LEFT]) * TEXTURESIZE;
		camy += (keystatus[SDLK_DOWN] - keystatus[SDLK_UP]) * TEXTURESIZE;
	}
	else
	{
		// camera velocity
		camera_vel.x += cos(camera.ang) * (keystatus[SDLK_UP] - keystatus[SDLK_DOWN]) * .05;
		camera_vel.y += sin(camera.ang) * (keystatus[SDLK_UP] - keystatus[SDLK_DOWN]) * .05;
		camera_vel.z += (keystatus[SDLK_PAGEDOWN] - keystatus[SDLK_PAGEUP]) * .25;
		camera_vel.ang += (keystatus[SDLK_RIGHT] - keystatus[SDLK_LEFT]) * .04;

		// camera position
		camera.x += camera_vel.x;
		camera.y += camera_vel.y;
		camera.z += camera_vel.z;
		camera.ang += camera_vel.ang;
		while ( camera.ang >= PI * 2 )
		{
			camera.ang -= PI * 2;
		}
		while ( camera.ang < 0 )
		{
			camera.ang += PI * 2;
		}

		// friction
		camera_vel.x *= .65;
		camera_vel.y *= .65;
		camera_vel.z *= .65;
		camera_vel.ang *= .5;
	}
	if ( camx < -xres / 2 )
	{
		camx = -xres / 2;
	}
	if ( camx > ((long)map.width << TEXTUREPOWER) - ((long)xres / 2) )
	{
		camx = ((long)map.width << TEXTUREPOWER) - ((long)xres / 2);
	}
	if ( camy < -yres / 2 )
	{
		camy = -yres / 2;
	}
	if ( camy > ((long)map.height << TEXTUREPOWER) - ((long)yres / 2) )
	{
		camy = ((long)map.height << TEXTUREPOWER) - ((long)yres / 2);
	}

	if (scroll < 0 )   // mousewheel up
	{
		if ( keystatus[SDLK_LCTRL] || keystatus[SDLK_RCTRL] )
		{
			recentUsedTilePalette++; //scroll through palettes 1-9
			if ( recentUsedTilePalette == 9 )
			{
				recentUsedTilePalette = 0;
			}
		}
		else if ( keystatus[SDLK_LSHIFT] || keystatus[SDLK_RSHIFT] )
		{
			drawlayer = std::min(drawlayer + 1, MAPLAYERS - 1);
		}
		else
		{
			lastPaletteTileSelected++; //scroll through tiles 1-9
			if ( lastPaletteTileSelected == 9 )
			{
				lastPaletteTileSelected = 0;
			}
			selectedTile = selectedTile = recentUsedTiles[recentUsedTilePalette][lastPaletteTileSelected];
		}
		scroll = 0;
	}
	if (scroll > 0 )   // mousewheel down
	{
		if ( keystatus[SDLK_LCTRL] || keystatus[SDLK_RCTRL] )
		{
			recentUsedTilePalette--; //scroll through palettes 1-9
			if ( recentUsedTilePalette == -1 )
			{
				recentUsedTilePalette = 8;
			}
		}
		else if ( keystatus[SDLK_LSHIFT] || keystatus[SDLK_RSHIFT] )
		{
			drawlayer = std::max(drawlayer - 1, 0);
		}
		else
		{
			lastPaletteTileSelected--; //scroll through tiles 1-9
			if ( lastPaletteTileSelected == -1 )
			{
				lastPaletteTileSelected = 8;
			}
			selectedTile = selectedTile = recentUsedTiles[recentUsedTilePalette][lastPaletteTileSelected];

		}
		scroll = 0;
	}

	switch ( drawlayer )
	{
		case 0:
			strcpy(layerstatus, "FLOOR");
			break;
		case 1:
			strcpy(layerstatus, "WALLS");
			break;
		case 2:
			strcpy(layerstatus, "CEILING");
			break;
		default:
			strcpy(layerstatus, "UNKNOWN");
			break;
	}
}

/*-------------------------------------------------------------------------------

	handleButtons

	Draws buttons and processes clicks

-------------------------------------------------------------------------------*/

void handleButtons(void)
{
	node_t* node;
	node_t* nextnode;
	button_t* button;
	int w, h;

	// handle buttons
	for ( node = button_l.first; node != NULL; node = nextnode )
	{
		nextnode = node->next;
		button = (button_t*)node->element;
		if ( !subwindow && button->focused )
		{
			list_RemoveNode(button->node);
			continue;
		}
		if ( button->visible == 0 )
		{
			continue;    // invisible buttons are not processed
		}
		w = strlen(button->label) * 8;
		h = 8;
		if ( subwindow && !button->focused )
		{
			// unfocused buttons do not work when a subwindow is active
			drawWindow(button->x, button->y, button->x + button->sizex, button->y + button->sizey);
			printText(font8x8_bmp, button->x + (button->sizex - w) / 2, button->y + (button->sizey - h) / 2, button->label);
		}
		else
		{
			if ( omousex >= button->x && omousex < button->x + button->sizex )
			{
				if ( omousey >= button->y && omousey < button->y + button->sizey )
				{
					if ( button == butFile && menuVisible )
					{
						menuVisible = 1;
					}
					if ( button == butEdit && menuVisible )
					{
						menuVisible = 2;
					}
					if ( button == butView && menuVisible )
					{
						menuVisible = 3;
					}
					if ( button == butMap && menuVisible )
					{
						menuVisible = 4;
					}
					if ( button == butHelp && menuVisible )
					{
						menuVisible = 5;
					}
					if ( mousestatus[SDL_BUTTON_LEFT] )
					{
						button->pressed = true;
					}
				}
			}
			if ( button->pressed )
			{
				if ( omousex >= button->x && omousex < button->x + button->sizex && mousex >= button->x && mousex < button->x + button->sizex )
				{
					if ( omousey >= button->y && omousey < button->y + button->sizey && mousey >= button->y && mousey < button->y + button->sizey )
					{
						drawDepressed(button->x, button->y, button->x + button->sizex, button->y + button->sizey);
						printText(font8x8_bmp, button->x + (button->sizex - w) / 2, button->y + (button->sizey - h) / 2, button->label);
						if ( !mousestatus[SDL_BUTTON_LEFT] )   // releasing the mouse over the button
						{
							button->pressed = false;
							if ( button->action != NULL )
							{
								(*button->action)(button); // run the button's assigned action
								if ( !subwindow && button->focused )
								{
									list_RemoveNode(button->node);
								}
							}
						}
					}
					else
					{
						drawWindow(button->x, button->y, button->x + button->sizex, button->y + button->sizey);
						printText(font8x8_bmp, button->x + (button->sizex - w) / 2, button->y + (button->sizey - h) / 2, button->label);
						if ( !mousestatus[SDL_BUTTON_LEFT] )   // releasing the mouse over nothing
						{
							button->pressed = false;
						}
					}
				}
				else
				{
					drawWindow(button->x, button->y, button->x + button->sizex, button->y + button->sizey);
					printText(font8x8_bmp, button->x + (button->sizex - w) / 2, button->y + (button->sizey - h) / 2, button->label);
					if ( !mousestatus[SDL_BUTTON_LEFT] )   // releasing the mouse over nothing
					{
						button->pressed = false;
					}
				}
			}
			else
			{
				if ( (button != butFile || menuVisible != 1) && (button != butEdit || menuVisible != 2) && (button != butView || menuVisible != 3) && (button != butMap || menuVisible != 4) && (button != butHelp || menuVisible != 5) )
				{
					drawWindow(button->x, button->y, button->x + button->sizex, button->y + button->sizey);
					printText(font8x8_bmp, button->x + (button->sizex - w) / 2, button->y + (button->sizey - h) / 2, button->label);
				}
				else
				{
					drawDepressed(button->x, button->y, button->x + button->sizex, button->y + button->sizey);
					printText(font8x8_bmp, button->x + (button->sizex - w) / 2, button->y + (button->sizey - h) / 2, button->label);
				}
			}
		}
	}
}

/*-------------------------------------------------------------------------------

	handleEvents

	Handles all SDL events; receives input, updates gamestate, etc.

-------------------------------------------------------------------------------*/

bool handleEvents(void)
{
	real_t d;
	int j;


	// calculate app rate
	t = SDL_GetTicks();
	real_t timesync = t - ot;
	ot = t;

	// do timer
	time_diff += timesync;
	constexpr real_t frame = (real_t)1000 / (real_t)TICKS_PER_SECOND;
	while (time_diff >= frame) {
		time_diff -= frame;
		timerCallback(0, NULL);
	}

	// calculate fps
	if ( timesync != 0 )
	{
		frameval[cycles & (AVERAGEFRAMES - 1)] = 1.0 / timesync;
	}
	else
	{
		frameval[cycles & (AVERAGEFRAMES - 1)] = 1.0;
	}
	d = frameval[0];
	for (j = 1; j < AVERAGEFRAMES; j++)
	{
		d += frameval[j];
	}
	fps = d / AVERAGEFRAMES * 1000;

	while ( SDL_PollEvent(&event) )   // poll SDL events
	{
		// Global events
		switch ( event.type )
		{
			case SDL_QUIT: // if SDL receives the shutdown signal
				buttonExit(NULL);
				break;
			case SDL_KEYDOWN: // if a key is pressed...
				if ( SDL_IsTextInputActive() )
				{
#ifdef APPLE
					if ( (event.key.keysym.sym == SDLK_DELETE || event.key.keysym.sym == SDLK_BACKSPACE) && strlen(inputstr) > 0 )
					{
						inputstr[strlen(inputstr) - 1] = 0;
						cursorflash = ticks;
					}
#else
					if ( event.key.keysym.sym == SDLK_BACKSPACE && strlen(inputstr) > 0 )
					{
						if ( textInsertCaratPosition >= 0 && newwindow == 20 && textInsertCaratPosition != strlen(inputstr) )
						{
							if ( textInsertCaratPosition != 0 )
							{
								std::string tmp = inputstr;
								tmp.erase(textInsertCaratPosition - 1, 1);
								strcpy(inputstr, tmp.c_str());
								textInsertCaratPosition = std::max(textInsertCaratPosition - 1, 0);
							}
						}
						else
						{
							inputstr[strlen(inputstr) - 1] = 0;
						}
						cursorflash = ticks;
					}
					else if ( event.key.keysym.sym == SDLK_BACKQUOTE && strlen(inputstr) > 0 )
					{
						if ( textInsertCaratPosition >= 0 && newwindow == 20 && textInsertCaratPosition != strlen(inputstr) )
						{
							if ( textInsertCaratPosition != 0 )
							{
								std::string tmp = inputstr;
								tmp.erase(textInsertCaratPosition - 1, 1);
								strcpy(inputstr, tmp.c_str());
								textInsertCaratPosition = std::max(textInsertCaratPosition - 1, 0);
							}
						}
						else
						{
							inputstr[strlen(inputstr) - 1] = 0;
						}
						cursorflash = ticks;
					}
#endif
					else if ( event.key.keysym.sym == SDLK_c && SDL_GetModState()&KMOD_CTRL )
					{
						SDL_SetClipboardText(inputstr);
						cursorflash = ticks;
					}
					else if ( event.key.keysym.sym == SDLK_v && SDL_GetModState()&KMOD_CTRL )
					{
						strncpy(inputstr, SDL_GetClipboardText(), inputlen);
						cursorflash = ticks;
					}
				}
				lastkeypressed = event.key.keysym.sym;
				keystatus[event.key.keysym.sym] = 1; // set this key's index to 1
				break;
			case SDL_KEYUP: // if a key is unpressed...
				keystatus[event.key.keysym.sym] = 0; // set this key's index to 0
				break;
			case SDL_TEXTINPUT:
				if ( (event.text.text[0] != 'c' && event.text.text[0] != 'C') || !(SDL_GetModState()&KMOD_CTRL) )
				{
					if ( (event.text.text[0] != 'v' && event.text.text[0] != 'V') || !(SDL_GetModState()&KMOD_CTRL) )
					{
						if ( event.text.text[0] != '`' )
						{
							if ( textInsertCaratPosition >= 0 && newwindow == 20 )
							{
								if ( textInsertCaratPosition == strlen(inputstr) )
								{
									strncat(inputstr, event.text.text, std::max<size_t>(0, inputlen - strlen(inputstr)));
									textInsertCaratPosition = std::min((int)strlen(inputstr), textInsertCaratPosition + 1);
								}
								else if ( inputlen - ((int)strlen(inputstr) + 1) >= 0)
								{
									std::string tmp = inputstr;
									tmp.insert(textInsertCaratPosition, event.text.text);
									strcpy(inputstr, tmp.c_str());
									textInsertCaratPosition = std::min((int)strlen(inputstr), textInsertCaratPosition + 1);
								}
							}
							else
							{
								strncat(inputstr, event.text.text, std::max<size_t>(0, inputlen - strlen(inputstr)));
							}
							cursorflash = ticks;
						}
					}
				}
				break;
			case SDL_MOUSEMOTION: // if the mouse is moved...
                float factorX;
                float factorY;
                {
                    int w1, w2, h1, h2;
                    SDL_GL_GetDrawableSize(screen, &w1, &h1);
                    SDL_GetWindowSize(screen, &w2, &h2);
                    factorX = (float)w1 / w2;
                    factorY = (float)h1 / h2;
                }
                mousex = event.motion.x * factorX;
                mousey = event.motion.y * factorY;
				mousexrel = event.motion.xrel;
				mouseyrel = event.motion.yrel;
				break;
			case SDL_MOUSEBUTTONDOWN: // if a mouse button is pressed...
				mousestatus[event.button.button] = 1; // set this mouse button to 1
				break;
			case SDL_MOUSEBUTTONUP: // if a mouse button is released...
				mousestatus[event.button.button] = 0; // set this mouse button to 0
				break;
			case SDL_MOUSEWHEEL:
				if ( event.wheel.y > 0 )
				{
					mousestatus[SDL_BUTTON_WHEELUP] = 1;
				}
				else if ( event.wheel.y < 0 )
				{
					mousestatus[SDL_BUTTON_WHEELDOWN] = 1;
				}
				if (mousestatus[4])
				{
					scroll = 1;
				}
				else if (mousestatus[5])
				{
					scroll = -1;
				}
				mousestatus[SDL_BUTTON_WHEELUP] = 0;
				mousestatus[SDL_BUTTON_WHEELDOWN] = 0;
				break;
			case SDL_USEREVENT: // if the game timer elapses
				mainLogic();
				mousexrel = 0;
				mouseyrel = 0;
				break;
			case SDL_WINDOWEVENT: // if the window is resized
				if ( event.window.event == SDL_WINDOWEVENT_RESIZED )
				{
					if (fullscreen || ticks == 0)
					{
						break;
					}
                    float factorX, factorY;
                    {
                        int w1, w2, h1, h2;
                        SDL_GL_GetDrawableSize(screen, &w1, &h1);
                        SDL_GetWindowSize(screen, &w2, &h2);
                        factorX = (float)w1 / w2;
                        factorY = (float)h1 / h2;
                    }
                    const int x = event.window.data1 * factorX;
                    const int y = event.window.data2 * factorY;
					if ( !resizeWindow(x, y) )
					{
						printlog("critical error! Attempting to abort safely...\n");
						mainloop = 0;
					}
					if (palette != NULL)
					{
						free(palette);
					}
					palette = (int*) malloc(sizeof(unsigned int) * xres * yres);
				}
				break;
		}
	}
	if (!mousestatus[SDL_BUTTON_LEFT])
	{
		omousex = mousex;
		omousey = mousey;
		ocamx = camx;
		ocamy = camy;
	}

	return true;
}

/*-------------------------------------------------------------------------------

	timerCallback

	A callback function for the game timer which pushes an SDL event

-------------------------------------------------------------------------------*/

#include "ui/LoadingScreen.hpp"

Uint32 timerCallback(Uint32 interval, void* param)
{
	SDL_Event event;
	SDL_UserEvent userevent;

	userevent.type = SDL_USEREVENT;
	userevent.code = 0;
	userevent.data1 = NULL;
	userevent.data2 = NULL;

	event.type = SDL_USEREVENT;
	event.user = userevent;

	ticks++;
	loadingticks++;
	SDL_PushEvent(&event);
	return (interval);
}

/*-------------------------------------------------------------------------------

	editFill

	Fills a region of the map with a certain tile

-------------------------------------------------------------------------------*/

void editFill(int x, int y, int layer, int type)
{
	int repeat = 1;
	int fillspot;

	if ( type == map.tiles[layer + y * MAPLAYERS + x * MAPLAYERS * map.height] )
	{
		return;
	}

	fillspot = map.tiles[layer + y * MAPLAYERS + x * MAPLAYERS * map.height];
	map.tiles[layer + y * MAPLAYERS + x * MAPLAYERS * map.height] = type + numtiles;

	while ( repeat )
	{
		repeat = 0;
		for ( x = 0; x < map.width; x++ )
		{
			for ( y = 0; y < map.height; y++ )
			{
				if ( map.tiles[layer + y * MAPLAYERS + x * MAPLAYERS * map.height] == type + numtiles )
				{
					if ( x < map.width - 1 )
					{
						if ( map.tiles[layer + y * MAPLAYERS + (x + 1)*MAPLAYERS * map.height] == fillspot )
						{
							map.tiles[layer + y * MAPLAYERS + (x + 1)*MAPLAYERS * map.height] = type + numtiles;
							repeat = 1;
						}
					}
					if ( x > 0 )
					{
						if ( map.tiles[layer + y * MAPLAYERS + (x - 1)*MAPLAYERS * map.height] == fillspot )
						{
							map.tiles[layer + y * MAPLAYERS + (x - 1)*MAPLAYERS * map.height] = type + numtiles;
							repeat = 1;
						}
					}
					if ( y < map.height - 1 )
					{
						if ( map.tiles[layer + (y + 1)*MAPLAYERS + x * MAPLAYERS * map.height] == fillspot )
						{
							map.tiles[layer + (y + 1)*MAPLAYERS + x * MAPLAYERS * map.height] = type + numtiles;
							repeat = 1;
						}
					}
					if ( y > 0 )
					{
						if ( map.tiles[layer + (y - 1)*MAPLAYERS + x * MAPLAYERS * map.height] == fillspot )
						{
							map.tiles[layer + (y - 1)*MAPLAYERS + x * MAPLAYERS * map.height] = type + numtiles;
							repeat = 1;
						}
					}
				}
			}
		}
	}

	for ( x = 0; x < map.width; x++ )
	{
		for ( y = 0; y < map.height; y++ )
		{
			if ( map.tiles[layer + y * MAPLAYERS + x * MAPLAYERS * map.height] == type + numtiles )
			{
				map.tiles[layer + y * MAPLAYERS + x * MAPLAYERS * map.height] = type;
			}
		}
	}
}

/*-------------------------------------------------------------------------------

	makeUndo

	adds an undomap to the undolist

-------------------------------------------------------------------------------*/

#define MAXUNDOS 10

node_t* undospot = nullptr;
node_t* redospot = nullptr;
list_t undolist;
void makeUndo()
{
	node_t* node, *nextnode;

	// eliminate any undo nodes beyond the one we are currently on
	if ( undospot != nullptr )
	{
		for ( node = undospot->next; node != nullptr; node = nextnode )
		{
			nextnode = node->next;
			list_RemoveNode(node);
		}
	}
	else
	{
		if ( redospot )
		{
			list_FreeAll(&undolist);
		}
	}

	// copy all the current map data
	map_t* undomap = (map_t*) malloc(sizeof(map_t));
	strcpy(undomap->author, map.author);
	strcpy(undomap->name, map.name);
	undomap->skybox = map.skybox;
	undomap->width = map.width;
	undomap->height = map.height;
	for ( int c = 0; c < MAPFLAGS; c++ )
	{
		undomap->flags[c] = map.flags[c];
	}
	undomap->tiles = (Sint32*) malloc(sizeof(Sint32) * undomap->width * undomap->height * MAPLAYERS);
	memcpy(undomap->tiles, map.tiles, sizeof(Sint32)*undomap->width * undomap->height * MAPLAYERS);
	undomap->entities = (list_t*) malloc(sizeof(list_t));
	undomap->entities->first = nullptr;
	undomap->entities->last = nullptr;
	undomap->creatures = nullptr;
	undomap->worldUI = nullptr;
	undomap->trapexcludelocations = nullptr;
	undomap->monsterexcludelocations = nullptr;
	undomap->lootexcludelocations = nullptr;
	for ( node = map.entities->first; node != nullptr; node = node->next )
	{
		Entity* entity = newEntity(((Entity*)node->element)->sprite, 1, undomap->entities, nullptr);

		setSpriteAttributes(entity, (Entity*)node->element, (Entity*)node->element);
	}

	// add the new node to the undo list
	node = list_AddNodeLast(&undolist);
	node->element = undomap;
	node->deconstructor = &mapDeconstructor;
	if ( list_Size(&undolist) > MAXUNDOS + 1 )
	{
		list_RemoveNode(undolist.first);
	}
	undospot = node;
	redospot = nullptr;
}

void clearUndos()
{
	list_FreeAll(&undolist);
	undospot = nullptr;
	redospot = nullptr;
}

/*-------------------------------------------------------------------------------

	undo() / redo()

	self explanatory

-------------------------------------------------------------------------------*/

void undo()
{
	node_t* node;
	if ( undospot == NULL )
	{
		return;
	}
	selectedEntity[0] = NULL;
	if ( undospot == undolist.last )
	{
		node_t* tempnode = undospot;
		makeUndo();
		undospot = tempnode;
	}
	free(map.tiles);
	free(camera.vismap);
	map_t* undomap = (map_t*)undospot->element;
	map.width = undomap->width;
	map.height = undomap->height;
	map.tiles = (Sint32*) malloc(sizeof(Sint32) * map.width * map.height * MAPLAYERS);
	camera.vismap = (bool*) malloc(sizeof(bool) * map.height * map.width);
    memset(camera.vismap, 0, sizeof(bool) * map.height * map.width);
	memcpy(map.tiles, undomap->tiles, sizeof(Sint32)*undomap->width * undomap->height * MAPLAYERS);
	list_FreeAll(map.entities);
	for ( node = undomap->entities->first; node != NULL; node = node->next )
	{
		Entity* entity = newEntity(((Entity*)node->element)->sprite, 1, map.entities, nullptr);

		setSpriteAttributes(entity, (Entity*)node->element, (Entity*)node->element);
	}
	if ( redospot != NULL )
	{
		redospot = redospot->prev;
	}
	else
	{
		redospot = undospot->next;
	}
	undospot = undospot->prev;
}

void redo()
{
	node_t* node;

	if ( redospot == NULL )
	{
		return;
	}
	selectedEntity[0] = NULL;
	free(map.tiles);
	free(camera.vismap);
	map_t* undomap = (map_t*)redospot->element;
	map.width = undomap->width;
	map.height = undomap->height;
	map.tiles = (Sint32*) malloc(sizeof(Sint32) * map.width * map.height * MAPLAYERS);
	camera.vismap = (bool*) malloc(sizeof(bool) * map.height * map.width);
    memset(camera.vismap, 0, sizeof(bool) * map.height * map.width);
	memcpy(map.tiles, undomap->tiles, sizeof(Sint32)*undomap->width * undomap->height * MAPLAYERS);
	list_FreeAll(map.entities);
	for ( node = undomap->entities->first; node != NULL; node = node->next )
	{
		Entity* entity = newEntity(((Entity*)node->element)->sprite, 1, map.entities, nullptr);

		setSpriteAttributes(entity, (Entity*)node->element, (Entity*)node->element);
	}
	if ( undospot != NULL )
	{
		undospot = undospot->next;
	}
	else
	{
		undospot = redospot->prev;
	}
	redospot = redospot->next;
}

void processCommandLine(int argc, char** argv)
{
	int c = 0;
	size_t datadirsz = std::min(sizeof(datadir) - 1, strlen(BASE_DATA_DIR));
	strncpy(datadir, BASE_DATA_DIR, datadirsz);
	datadir[datadirsz] = '\0';
	if ( argc > 1 )
	{
		for ( c = 1; c < argc; c++ )
		{
			if ( argv[c] != nullptr )
			{
				if ( !strncmp(argv[c], "-map=", 5) )
				{
					strcpy(maptoload, argv[c] + 5);
					loadingmap = true;
				}
				else if (!strncmp(argv[c], "-datadir=", 9))
				{
					datadirsz = std::min(sizeof(datadir) - 1, strlen(argv[c] + 9));
					strncpy(datadir, argv[c] + 9, datadirsz);
					datadir[datadirsz] = '\0';
				}
				else if ( !strncmp(argv[c], "-xres=", 6) )
				{
					char buf[32];
					size_t len = std::min(sizeof(buf), strlen(argv[c] + 6));
					strncpy(buf, argv[c] + 6, len);
					buf[len] = '\0';
					xres = atoi(buf);
				}
				else if ( !strncmp(argv[c], "-yres=", 6) )
				{
					char buf[32];
					size_t len = std::min(sizeof(buf), strlen(argv[c] + 6));
					strncpy(buf, argv[c] + 6, len);
					buf[len] = '\0';
					yres = atoi(buf);
				}
			}
		}
	}
	printlog("Data path is %s", datadir);
}

/*-------------------------------------------------------------------------------

loadTilePalettes

loads the tile palette file for the editor.

-------------------------------------------------------------------------------*/

int loadTilePalettes()
{
	char filename[128] = { 0 };
	File* fp;
	int c;

	// open log file
	if ( !logfile )
	{
		openLogFile();
	}

	// compose filename
	strcpy(filename, "editor/tilepalettes.txt");

	// check if palette file is valid
	if ( !dataPathExists(filename) )
	{
		// palette file doesn't exist
		printlog("error: unable to locate tile palette file: '%s'", filename);
		return 1;
	}

	// open palette file
	if ( (fp = openDataFile(filename, "rb")) == NULL )
	{
		printlog("error: unable to load tile palette file: '%s'", filename);
		return 1;
	}

	// read file
	int paletteNumber = 0;
	int paletteTile = 0;
	bool lockValueEntry = 0;
	for (; !(fp->eof()); )
	{
		//printlog( "loading line %d...\n", line);
		char data[1024];

		// read line from file
		int i;
		bool fileEnd = false;
		for ( i = 0; ; i++ )
		{
			data[i] = fp->getc();
			if ( fp->eof() )
			{
				fileEnd = true;
				break;
			}

			// blank or comment lines stop reading at a newline
			if ( data[i] == '\n' )
			{
				break;
			}
		}

		if ( fileEnd )
		{
			break;
		}

		// skip blank and comment lines
		if ( data[0] == '\n' || data[0] == '#' )
		{
			continue;
		}

		// process line
		if ( !lockValueEntry )
		{
			recentUsedTiles[paletteNumber][paletteTile] = atoi(data);
			//printlog("read tile number '%d', pattern '%d', data '%d' \n", paletteTile, paletteNumber, atoi(data));
			++paletteTile;
			if ( paletteTile == 9 )
			{
				paletteTile = 0;
				paletteNumber++;
				lockValueEntry = true;
			}
		}
		else
		{
			lockTilePalette[paletteNumber - 1] = atoi(data);
			//printlog("read lock value for palette '%d', data '%d' \n", paletteNumber - 1, atoi(data));
			lockValueEntry = false;
		}
	}

	// close file
	FileIO::close(fp);
	printlog("successfully loaded tile palette file '%s'\n", filename);
	return 0;
}

/*-------------------------------------------------------------------------------

saveTilePalettes()

saves the tile palette file for the editor.

-------------------------------------------------------------------------------*/

int saveTilePalettes()
{
	char filename[128] = { 0 };
	File* fp;
	int c;

	// open log file
	if ( !logfile )
	{
		openLogFile();
	}

	// compose filename
	strcpy(filename, "editor/tilepalettes.txt");

	// check if palette file is valid
	if ( !dataPathExists(filename) )
	{
		// palette file doesn't exist
		printlog("error: unable to locate existing tile palette file: '%s'...\ncreating...", filename);
	}

	// open/create palette file
	if ( (fp = openDataFile(filename, "w")) == NULL )
	{
		printlog("error: unable to save or create tile palette file: '%s'", filename);
		return 1;
	}

	// write file
	Uint32 line;
	int paletteNumber = 0;
	int paletteTile = 0;
	bool lockValueEntry = 0;
	char data[128];

	fp->puts("# Tile palette file\n");
	fp->puts("# lines beginning with pound character are a comment\n");
	fp->puts("# blank lines are ignored\n");
	fp->puts("");

	for ( paletteNumber = 0; paletteNumber < 9; paletteNumber++ )
	{
		paletteTile = 0;
		snprintf(data, sizeof(data), "# palette %d tiles\n", paletteNumber + 1);
		fp->puts(data);
		fp->puts("\n");
		for ( paletteTile = 0; paletteTile < 9; paletteTile++ )
		{
			if ( paletteTile == 3 || paletteTile == 6 )
			{
				fp->puts("\n");
			}
			snprintf(data, sizeof(data), "%d\n", recentUsedTiles[paletteNumber][paletteTile]);
			fp->puts(data);
		}
		fp->puts("\n");
		snprintf(data, sizeof(data), "# palette %d locked (1) or unlocked (0)\n", paletteNumber + 1);
		fp->puts(data);
		fp->puts("\n");
		snprintf(data, sizeof(data), "%d\n", lockTilePalette[paletteNumber]);
		fp->puts(data);
		fp->puts("\n");
	}

	fp->puts("# end\n");

	// close file
	FileIO::close(fp);
	printlog("saved tile palette file '%s'\n", filename);
	return 0;
}

/*-------------------------------------------------------------------------------

updateRecentTileList

Updates the tile palette in the editor if not locked, takes tile as input and either 
inserts into an empty slot, or shifts the palette to accomodate. 

-------------------------------------------------------------------------------*/

void updateRecentTileList(int tile)
{
	int checkEmpty = -1;

	for ( int i = 0; i < 9; i++ )
	{
		if ( recentUsedTiles[recentUsedTilePalette][i] == tile )
		{
			lastPaletteTileSelected = i;
			return; // tile exists in recent list.
		}

		if ( recentUsedTiles[recentUsedTilePalette][i] == 0 && checkEmpty == -1 )
		{
			checkEmpty = i; // index of next empty tile.
			lastPaletteTileSelected = checkEmpty;
		}
	}

	if ( lockTilePalette[recentUsedTilePalette] == 1 )
	{
		return; // palette locked, don't change.
	}

	if ( checkEmpty == -1 )
	{
		for ( int j = 8; j > 0; j-- )
		{
			recentUsedTiles[recentUsedTilePalette][j] = recentUsedTiles[recentUsedTilePalette][j - 1]; // shift array by 1 to insert new tile as the array is full.
		}
		recentUsedTiles[recentUsedTilePalette][0] = tile; // insert tile into array.
		lastPaletteTileSelected = 0;
	}
	else
	{
		recentUsedTiles[recentUsedTilePalette][checkEmpty] = tile; // insert tile into array.
	}

	return;
}

/*-------------------------------------------------------------------------------

	main

	Initializes program resources, harbors main loop, and cleans up
	afterwords

-------------------------------------------------------------------------------*/

bool selectingspace = false;
int selectedarea_x1, selectedarea_x2;
int selectedarea_y1, selectedarea_y2;
bool selectedarea = false;
bool pasting = false;

#ifdef APPLE
#include <mach-o/dyld.h> //For _NSGetExecutablePath()
#endif

int main(int argc, char** argv)
{
#ifdef APPLE
	uint32_t buffsize = 4096;
	char binarypath[buffsize];
	int result = _NSGetExecutablePath(binarypath, &buffsize);
	if (result == 0)   //It worked.
	{
		printlog( "Binary path: %s\n", binarypath);
		char* last = strrchr(binarypath, '/');
		*last = '\0';
		char execpath[buffsize];
		strcpy(execpath, binarypath);
		//char* last = strrchr(execpath, '/');
		//strcat(execpath, '/');
		//strcat(execpath, "/../../../");
		printlog( "Chrooting to directory: %s\n", execpath);
		chdir(execpath);
		///Users/ciprian/barony/barony-sdl2-take2/barony.app/Contents/MacOS/barony
		chdir("..");
		chdir("..");
		chdir("..");
		chdir("barony.app/Contents/Resources");
		//chdir("..");
	}
	else
	{
		printlog( "Failed to get binary path. Program may not work correctly!\n");
	}
#endif

#ifdef LINUX
	(void)chdir(BASE_DATA_DIR); // fixes a lot of headaches...
#endif

	button_t* button;
	node_t* node;
	node_t* nextnode;
	Entity* entity;
	SDL_Rect pos;
	int c;
	int x, y, z;
	int x2, y2;
	//char action[32];
	int oslidery = 0;
	light_t* light = nullptr;
	bool savedundo = false;
	smoothlighting = true;

	Stat* spriteStats = nullptr;

	processCommandLine(argc, argv);

#ifdef WINDOWS
	strcpy(outputdir, "./");
#else
	std::string basepath;
	if (strlen(getenv("XDG_DATA_HOME")) > 0)
	{
		printlog("Picked up XDG_DATA_HOME: %s", getenv("XDG_DATA_HOME"));
		basepath = getenv("XDG_DATA_HOME");
		basepath += "/barony";
	}
	else 
	{
		printlog("XDG_DATA_HOME does not exit, using HOME");
		basepath = getenv("HOME");
		basepath += "/.local/share/barony";
	}
	snprintf(outputdir, sizeof(outputdir), "%s", basepath.c_str());
	std::filesystem::create_directories(outputdir);
#endif

	// load default language file (english)
	if ( Language::loadLanguage("en", true) )
	{
		exit(1);
	}
    
    // init sdl
    Uint32 init_flags = SDL_INIT_VIDEO | SDL_INIT_EVENTS;
    init_flags |= SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC;
    if (SDL_Init(init_flags) == -1)
    {
        printlog("failed to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

	// initialize
	verticalSync = true;
	if ( (x = initApp("Barony Editor", fullscreen)) )
	{
		printlog("Critical error: %d\n", x);
#ifdef STEAMWORKS
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Uh oh",
								"Barony has encountered a critical error and cannot start.\n\n"
								"Please check the log.txt file in the game directory for additional info\n"
								"and verify Steam is running. Alternatively, contact us through our website\n"
								"at http://www.baronygame.com/ for support.",
								screen);
#else
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Uh oh",
								"Barony has encountered a critical error and cannot start.\n\n"
								"Please check the log.txt file in the game directory for additional info,\n"
								"or contact us through our website at http://www.baronygame.com/ for support.",
								screen);
#endif
		deinitApp();
		exit(x);
	}
	
#ifdef STEAMWORKS
	g_SteamStatistics->RequestStats();
#endif // STEAMWORKS


	copymap.tiles = nullptr;
	copymap.entities = nullptr;
	copymap.creatures = nullptr;
	copymap.worldUI = nullptr;
	undolist.first = nullptr;
	undolist.last = nullptr;

	// Load Cursors
	cursorArrow = SDL_GetCursor();
	cursorPencil = newCursor(cursor_pencil);
	cursorPoint = newCursor(cursor_point);
	cursorBrush = newCursor(cursor_brush);
	cursorSelect = cursorArrow;
	cursorFill = newCursor(cursor_fill);

	// create an empty map
	map.width = 32;
	map.height = 24;
	map.entities = (list_t*) malloc(sizeof(list_t));
	map.creatures = nullptr;
	map.worldUI = nullptr;
	map.entities->first = nullptr;
	map.entities->last = nullptr;
	map.tiles = (int*) malloc(sizeof(int) * map.width * map.height * MAPLAYERS);
	camera.vismap = (bool*) malloc(sizeof(bool) * map.height * map.width);
    memset(camera.vismap, 0, sizeof(bool) * map.height * map.width);
	strcpy(map.name, "");
	strcpy(map.author, "");
	map.skybox = 0;
	for ( c = 0; c < MAPFLAGS; c++ )
	{
		map.flags[c] = 0;
	}
	for ( z = 0; z < MAPLAYERS; z++ )
	{
		for ( y = 0; y < map.height; y++ )
		{
			for ( x = 0; x < map.width; x++ )
			{
				if (z == OBSTACLELAYER)
				{
					if (x == 0 || y == 0 || x == map.width - 1 || y == map.height - 1)
					{
						map.tiles[z + y * MAPLAYERS + x * MAPLAYERS * map.height] = 2;
					}
					else
					{
						map.tiles[z + y * MAPLAYERS + x * MAPLAYERS * map.height] = 0;
					}
				}
				else
				{
					map.tiles[z + y * MAPLAYERS + x * MAPLAYERS * map.height] = 1;
				}
			}
		}
	}
    for (int c = 0; c < MAXPLAYERS + 1; ++c) {
        lightmaps[c].clear();
        lightmaps[c].resize(map.width * map.height);
        lightmapsSmoothed[c].clear();
        lightmapsSmoothed[c].resize((map.width + 2) * (map.height + 2));
    }

	// initialize camera position
	camera.x = 4;
	camera.y = 4;
	camera.z = 0;
	camera.ang = 0;
	camera.vang = 0;

	// initialize editor settings
	strcpy(layerstatus, "BACKGROUND");
	palette = (int*) malloc(sizeof(unsigned int) * xres * yres);

	// main interface
	button = butFile = newButton();
	strcpy(button->label, "File");
	button->x = 0;
	button->y = 0;
	button->sizex = 40;
	button->sizey = 16;
	button->action = &buttonFile;

	button = butEdit = newButton();
	strcpy(button->label, "Edit");
	button->x = 40;
	button->y = 0;
	button->sizex = 40;
	button->sizey = 16;
	button->action = &buttonEdit;

	button = butView = newButton();
	strcpy(button->label, "View");
	button->x = 80;
	button->y = 0;
	button->sizex = 40;
	button->sizey = 16;
	button->action = &buttonView;

	button = butMap = newButton();
	strcpy(button->label, "Map");
	button->x = 120;
	button->y = 0;
	button->sizex = 32;
	button->sizey = 16;
	button->action = &buttonMap;

	button = butHelp = newButton();
	strcpy(button->label, "Help");
	button->x = 152;
	button->y = 0;
	button->sizex = 40;
	button->sizey = 16;
	button->action = &buttonHelp;

	button = butX = newButton();
	strcpy(button->label, "X");
	button->x = xres - 16;
	button->y = 0;
	button->sizex = 16;
	button->sizey = 16;
	button->action = &buttonExit;
	button->visible = 0;

	button = but_ = newButton();
	strcpy(button->label, "_");
	button->x = xres - 32;
	button->y = 0;
	button->sizex = 16;
	button->sizey = 16;
	button->action = &buttonIconify;
	button->visible = 0;

	// toolbox
	button = butTilePalette = newButton();
	strcpy(button->label, "Palette ...");
	button->x = xres - 112;
	button->y = 152;
	button->sizex = 96;
	button->sizey = 16;
	button->action = &buttonTilePalette;

	button = butSprite = newButton();
	strcpy(button->label, "Sprite  ...");
	button->x = xres - 112;
	button->y = 168;
	button->sizex = 96;
	button->sizey = 16;
	button->action = &buttonSprite;

	// Pencil Tool Button
	button = butPencil = newButton();
	strcpy(button->label, "Pencil");
	button->x = xres - 96;
	button->y = 204;
	button->sizex = 64;
	button->sizey = 16;
	button->action = &buttonPencil;

	// Point Tool Button
	button = butPoint = newButton();
	strcpy(button->label, "Point");
	button->x = xres - 96;
	button->y = 220;
	button->sizex = 64;
	button->sizey = 16;
	button->action = &buttonPoint;

	// Brush Tool Button
	button = butBrush = newButton();
	strcpy(button->label, "Brush");
	button->x = xres - 96;
	button->y = 236;
	button->sizex = 64;
	button->sizey = 16;
	button->action = &buttonBrush;

	// Select Tool Button
	button = butSelect = newButton();
	strcpy(button->label, "Select");
	button->x = xres - 96;
	button->y = 252;
	button->sizex = 64;
	button->sizey = 16;
	button->action = &buttonSelect;

	// Fill Tool Button
	button = butFill = newButton();
	strcpy(button->label, "Fill");
	button->x = xres - 96;
	button->y = 268;
	button->sizex = 64;
	button->sizey = 16;
	button->action = &buttonFill;

	// file menu
	butNew = button = newButton();
	strcpy(button->label, "New          Ctrl+N");
	button->x = 16;
	button->y = 16;
	button->sizex = 160;
	button->sizey = 16;
	button->action = &buttonNew;
	button->visible = 0;

	butOpen = button = newButton();
	strcpy(button->label, "Open ...     Ctrl+O");
	button->x = 16;
	button->y = 32;
	button->sizex = 160;
	button->sizey = 16;
	button->action = &buttonOpen;
	button->visible = 0;

	butDir = button = newButton();
	strcpy(button->label, "Directory... Ctrl+D");
	button->x = 16;
	button->y = 48;
	button->sizex = 160;
	button->sizey = 16;
	button->action = &buttonOpenDirectory;
	button->visible = 0;

	butSave = button = newButton();
	strcpy(button->label, "Save         Ctrl+S");
	button->x = 16;
	button->y = 64;
	button->sizex = 160;
	button->sizey = 16;
	button->action = &buttonSave;
	button->visible = 0;

	butSaveAs = button = newButton();
	strcpy(button->label, "Save As ...        ");
	button->x = 16;
	button->y = 80;
	button->sizex = 160;
	button->sizey = 16;
	button->action = &buttonSaveAs;
	button->visible = 0;

	butExit = button = newButton();
	strcpy(button->label, "Exit         Alt+F4");
	button->x = 16;
	button->y = 96;
	button->sizex = 160;
	button->sizey = 16;
	button->action = &buttonExit;
	button->visible = 0;

	// edit menu
	butCut = button = newButton();
	strcpy(button->label, "Cut         Ctrl+X");
	button->x = 56;
	button->y = 16;
	button->sizex = 152;
	button->sizey = 16;
	button->action = &buttonCut;
	button->visible = 0;

	butCopy = button = newButton();
	strcpy(button->label, "Copy        Ctrl+C");
	button->x = 56;
	button->y = 32;
	button->sizex = 152;
	button->sizey = 16;
	button->action = &buttonCopy;
	button->visible = 0;

	butPaste = button = newButton();
	strcpy(button->label, "Paste       Ctrl+V");
	button->x = 56;
	button->y = 48;
	button->sizex = 152;
	button->sizey = 16;
	button->action = &buttonPaste;
	button->visible = 0;

	butDelete = button = newButton();
	strcpy(button->label, "Delete      Del   ");
	button->x = 56;
	button->y = 64;
	button->sizex = 152;
	button->sizey = 16;
	button->action = &buttonDelete;
	button->visible = 0;

	butSelectAll = button = newButton();
	strcpy(button->label, "Select All  Ctrl+A");
	button->x = 56;
	button->y = 80;
	button->sizex = 152;
	button->sizey = 16;
	button->action = &buttonSelectAll;
	button->visible = 0;

	butUndo = button = newButton();
	strcpy(button->label, "Undo        Ctrl+Z");
	button->x = 56;
	button->y = 96;
	button->sizex = 152;
	button->sizey = 16;
	button->action = &buttonUndo;
	button->visible = 0;

	butRedo = button = newButton();
	strcpy(button->label, "Redo        Ctrl+Y");
	button->x = 56;
	button->y = 112;
	button->sizex = 152;
	button->sizey = 16;
	button->action = &buttonRedo;
	button->visible = 0;

	// view menu
	butStatusBar = button = newButton();
	strcpy(button->label, "Statusbar   Ctrl+I");
	button->x = 96;
	button->y = 16;
	button->sizex = 152;
	button->sizey = 16;
	button->action = &buttonStatusBar;
	button->visible = 0;

	butToolbox = button = newButton();
	strcpy(button->label, "Toolbox     Ctrl+T");
	button->x = 96;
	button->y = 32;
	button->sizex = 152;
	button->sizey = 16;
	button->action = &buttonToolbox;
	button->visible = 0;

	butAllLayers = button = newButton();
	strcpy(button->label, "All Layers  Ctrl+L");
	button->x = 96;
	button->y = 48;
	button->sizex = 152;
	button->sizey = 16;
	button->action = &buttonAllLayers;
	button->visible = 0;

	butViewSprites = button = newButton();
	strcpy(button->label, "Sprites     Ctrl+E");
	button->x = 96;
	button->y = 64;
	button->sizex = 152;
	button->sizey = 16;
	button->action = &buttonViewSprites;
	button->visible = 0;

	butGrid = button = newButton();
	strcpy(button->label, "Grid        Ctrl+G");
	button->x = 96;
	button->y = 80;
	button->sizex = 152;
	button->sizey = 16;
	button->action = &buttonGrid;
	button->visible = 0;

	but3DMode = button = newButton();
	strcpy(button->label, "3D Mode     Ctrl+F");
	button->x = 96;
	button->y = 96;
	button->sizex = 152;
	button->sizey = 16;
	button->action = &button3DMode;
	button->visible = 0;

	butHoverText = button = newButton();
	strcpy(button->label, "Hover Text  Ctrl+H");
	button->x = 96;
	button->y = 112;
	button->sizex = 152;
	button->sizey = 16;
	button->action = &buttonHoverText;
	button->visible = 0;

	// map menu
	butAttributes = button = newButton();
	strcpy(button->label, "Attributes ...  Ctrl+M      ");
	button->x = 136;
	button->y = 16;
	button->sizex = 232;
	button->sizey = 16;
	button->action = &buttonAttributes;
	button->visible = 0;

	butClearMap = button = newButton();
	strcpy(button->label, "Clear Map       Ctrl+Shift+N");
	button->x = 136;
	button->y = 32;
	button->sizex = 232;
	button->sizey = 16;
	button->action = &buttonClearMap;
	button->visible = 0;

	// help menu
	butAbout = button = newButton();
	strcpy(button->label, "About            F1");
	button->x = 168;
	button->y = 16;
	button->sizex = 160;
	button->sizey = 16;
	button->action = &buttonAbout;
	button->visible = 0;

	// controls menu
	butEditorControls = button = newButton();
	strcpy(button->label, "Editor Help       H");
	button->x = 168;
	button->y = 32;
	button->sizex = 160;
	button->sizey = 16;
	button->action = &buttonEditorControls;
	button->visible = 0;

	if ( loadingmap )
	{
		if ( loadMap(physfsFormatMapName(maptoload).c_str(), &map, map.entities, map.creatures) == -1 )
		{
			strcpy(message, "Failed to open ");
			strcat(message, maptoload);
		}
		else
		{
			strcpy(filename, maptoload);
		}
	}

	ItemTooltips.readItemsFromFile();
	ItemTooltips.readItemLocalizationsFromFile();
	ItemTooltips.readBookLocalizationsFromFile();
	for ( c = 0; c < NUMITEMS; c++ )
	{
		items[c].surfaces.first = nullptr;
		items[c].surfaces.last = nullptr;
		for ( x = 0; x < list_Size(&items[c].images); x++ )
		{
			auto** surface = static_cast<SDL_Surface**>(malloc(sizeof(SDL_Surface*)));
			node_t* node = list_AddNodeLast(&items[c].surfaces);
			node->element = surface;
			node->deconstructor = &defaultDeconstructor;
			node->size = sizeof(SDL_Surface*);

			node_t* node2 = list_Node(&items[c].images, x);
			auto* string = static_cast<string_t*>(node2->element);
			*surface = loadImage(string->data);
		}
	}
	loadTilePalettes();
	EditorEntityData_t::readFromFile();

	bool achievementCartographer = false;

	// main loop
	printlog( "running main loop.\n");
	while (mainloop)
	{
		// game logic
		(void)handleEvents();

#ifdef STEAMWORKS
		SteamAPI_RunCallbacks();
		if ( SteamUser()->BLoggedOn() && !achievementCartographer )
		{
			SteamUserStats()->SetAchievement("BARONY_ACH_CARTOGRAPHER");
			achievementCartographer = true;
			SteamUserStats()->StoreStats();
			//printlog("STEAM ACHIEVEMENT\n");
		}
#endif

		// move buttons
		/*if( !fullscreen ) {
			butX->visible = 0;
			but_->visible = 0;
		} else {
			butX->visible = 1;
			but_->visible = 1;
			butX->x = xres-16;
			but_->x = xres-32;
		}*/
		butTilePalette->x = xres - 112;
		butSprite->x = xres - 112;
		butPencil->x = xres - 96;
		butPoint->x = xres - 96;
		butBrush->x = xres - 96;
		butSelect->x = xres - 96;
		butFill->x = xres - 96;

		bool wasTextEditable = SDL_IsTextInputActive();
		if ( !wasTextEditable )
		{
			textInsertCaratPosition = -1;
		}

		if ( !spritepalette && !tilepalette )
		{
			allowediting = 1;
			if ( (omousex >= xres - 128 && toolbox) || omousey < 16 || (omousey >= yres - 16 && statusbar) || subwindow || menuVisible )
			{
				allowediting = 0;
			}
			if ( mode3d )
			{
				allowediting = 0;
			}
			if ( menuVisible == 1 )
			{
				if ((omousex > 16 + butNew->sizex || omousey > 112 || (omousey < 16 && omousex > 192)) && mousestatus[SDL_BUTTON_LEFT])
				{
					menuVisible = 0;
					menuDisappear = 1;
				}
			}
			else if ( menuVisible == 2 )
			{
				if ((omousex > 56 + butCut->sizex || omousex < 40 || omousey > 128 || (omousey < 16 && omousex > 192)) && mousestatus[SDL_BUTTON_LEFT])
				{
					menuVisible = 0;
					menuDisappear = 1;
				}
			}
			else if ( menuVisible == 3 )
			{
				if ((omousex > 96 + butToolbox->sizex || omousex < 80 || omousey > 128 || (omousey < 16 && omousex > 192)) && mousestatus[SDL_BUTTON_LEFT])
				{
					menuVisible = 0;
					menuDisappear = 1;
				}
			}
			else if ( menuVisible == 4 )
			{
				if ((omousex > 136 + butClearMap->sizex || omousex < 120 || omousey > 48 || (omousey < 16 && omousex > 192)) && mousestatus[SDL_BUTTON_LEFT])
				{
					menuVisible = 0;
					menuDisappear = 1;
				}
			}
			else if ( menuVisible == 5 )
			{
				if ((omousex > 168 + butAbout->sizex || omousex < 152 || omousey > 48 || (omousey < 32 && omousex > 192)) && mousestatus[SDL_BUTTON_LEFT])
				{
					menuVisible = 0;
					menuDisappear = 1;
				}
			}
			if ( !mousestatus[SDL_BUTTON_LEFT] )
			{
				menuDisappear = 0;
			}

			if ( allowediting && !menuDisappear )
			{
				// MAIN LEVEL EDITING
				drawx = (mousex + camx) >> TEXTUREPOWER;
				drawy = (mousey + camy) >> TEXTUREPOWER;
				odrawx = (omousex + ocamx) >> TEXTUREPOWER;
				odrawy = (omousey + ocamy) >> TEXTUREPOWER;

				// Set the Cursor to the corresponding tool
				switch ( selectedTool )
				{
					case 0: // Pencil
						SDL_SetCursor(cursorPencil);
						break;
					case 1: // Point
						SDL_SetCursor(cursorPoint);
						break;
					case 2: // Brush
						SDL_SetCursor(cursorBrush);
						break;
					case 3: // Select
						SDL_SetCursor(cursorSelect);
						break;
					case 4: // Fill
						SDL_SetCursor(cursorFill);
						break;
					default:
						SDL_SetCursor(cursorArrow);
						break;
				}

				// Move Entities
				if ( map.entities->first != NULL && viewsprites && allowediting )
				{
					for ( node = map.entities->first; node != NULL; node = nextnode )
					{
						nextnode = node->next;
						entity = (Entity*)node->element;
						if ( entity == selectedEntity[0] )
						{
							if ( mousestatus[SDL_BUTTON_LEFT] )
							{
								if ( newwindow == 0 )
								{
									// if the entity moved from where it was picked up, or if the sprite was right click duplicated, store an undo.
									if ( selectedEntity[0]->x / 16 != prev_x || selectedEntity[0]->y / 16 != prev_y || duplicatedSprite )
									{
										duplicatedSprite = false;
										makeUndo();
									}
								}
								mousestatus[SDL_BUTTON_LEFT] = 0;
								selectedEntity[0] = NULL;
								break;
							}
							else if ( mousestatus[SDL_BUTTON_RIGHT] )
							{
								if ( newwindow == 0 )
								{
									// if previous sprite was duplicated and another right click is registered, store an undo.
									if ( duplicatedSprite )
									{
										makeUndo();
									}
									duplicatedSprite = true;
								}
								selectedEntity[0] = newEntity(entity->sprite, 0, map.entities, nullptr);
								
								setSpriteAttributes(selectedEntity[0], entity, lastSelectedEntity[0]);

								lastSelectedEntity[0] = selectedEntity[0];

								mousestatus[SDL_BUTTON_RIGHT] = 0;
								break;
							}
							entity->x = (long)(drawx << 4);
							entity->y = (long)(drawy << 4);
						}
						else
						{
							if ( (omousex + camx) >> TEXTUREPOWER == entity->x / 16 && (omousey + camy) >> TEXTUREPOWER == entity->y / 16 )
							{
								if ( mousestatus[SDL_BUTTON_LEFT] && selectedTool == 1 )
								{
									// select sprite
									selectedEntity[0] = entity;
									lastSelectedEntity[0] = selectedEntity[0];
									prev_x = entity->x / 16;
									prev_y = entity->y / 16;
									mousestatus[SDL_BUTTON_LEFT] = 0;
									if ( newwindow == 0 && selectedEntity[0] != NULL )
									{
										makeUndo();
									}
								}
								else if ( mousestatus[SDL_BUTTON_RIGHT] && selectedTool == 1 )
								{
									// duplicate sprite
									duplicatedSprite = true;
									if ( newwindow == 0 )
									{
										makeUndo();
									}
									selectedEntity[0] = newEntity(entity->sprite, 0, map.entities, nullptr);
									lastSelectedEntity[0] = selectedEntity[0];

									setSpriteAttributes(selectedEntity[0], entity, entity);

									mousestatus[SDL_BUTTON_RIGHT] = 0;
								}
							}
						}
					}
				}

				// Modify World
				if ( mousestatus[SDL_BUTTON_LEFT] && selectedEntity[0] == NULL )
				{
					if ( allowediting )
					{
						if ( !savedundo )
						{
							savedundo = true;
							makeUndo();
						}
						if ( !pasting )   // Not Pasting, Normal Editing Mode
						{
							if ( selectedTool == 0 )		// Process Pencil Tool functionality
							{
								if ( drawx >= 0 && drawx < map.width && drawy >= 0 && drawy < map.height )
								{
									map.tiles[drawlayer + drawy * MAPLAYERS + drawx * MAPLAYERS * map.height] = selectedTile;
								}
							}
							else if ( selectedTool == 1 )	// Process Point Tool functionality
							{
								// All functionality of the Point Tool is encapsulated above in the "Move Entities" section
							}
							else if ( selectedTool == 2 )	// Process Brush Tool functionality
							{
								for ( x = drawx - 1; x <= drawx + 1; x++ )
								{
									for ( y = drawy - 1; y <= drawy + 1; y++ )
									{
										if ( (x != drawx - 1 || y != drawy - 1) && (x != drawx + 1 || y != drawy - 1) && (x != drawx - 1 || y != drawy + 1) && (x != drawx + 1 || y != drawy + 1) )
										{
											if ( x >= 0 && x < map.width && y >= 0 && y < map.height )
											{
												map.tiles[drawlayer + y * MAPLAYERS + x * MAPLAYERS * map.height] = selectedTile;
											}
										}
									}
								}
							}
							else if ( selectedTool == 3 )	// Process Select Tool functionality
							{
								if ( selectingspace == false )
								{
									if ( drawx >= 0 && drawy >= 0 && drawx < map.width && drawy < map.height )
									{
										selectingspace = true;
										selectedarea_x1 = drawx;
										selectedarea_x2 = drawx;
										selectedarea_y1 = drawy;
										selectedarea_y2 = drawy;
										selectedarea = true;
									}
									else
									{
										selectedarea = false;
									}
								}
								else
								{
									if ( drawx < odrawx )
									{
										selectedarea_x1 = std::min<unsigned int>(std::max(0, drawx), map.width - 1); //TODO: Why are int and unsigned int being compared?
										selectedarea_x2 = std::min<unsigned int>(std::max(0, odrawx), map.width - 1); //TODO: Why are int and unsigned int being compared?
									}
									else
									{
										selectedarea_x1 = std::min<unsigned int>(std::max(0, odrawx), map.width - 1); //TODO: Why are int and unsigned int being compared?
										selectedarea_x2 = std::min<unsigned int>(std::max(0, drawx), map.width - 1); //TODO: Why are int and unsigned int being compared?
									}
									if ( drawy < odrawy )
									{
										selectedarea_y1 = std::min<unsigned int>(std::max(0, drawy), map.height - 1); //TODO: Why are int and unsigned int being compared?
										selectedarea_y2 = std::min<unsigned int>(std::max(0, odrawy), map.height - 1); //TODO: Why are int and unsigned int being compared?
									}
									else
									{
										selectedarea_y1 = std::min<unsigned int>(std::max(0, odrawy), map.height - 1); //TODO: Why are int and unsigned int being compared?
										selectedarea_y2 = std::min<unsigned int>(std::max(0, drawy), map.height - 1); //TODO: Why are int and unsigned int being compared?
									}
									if ( map.entities->first != nullptr && viewsprites && allowediting )
									{
										reselectEntityGroup();
										moveSelectionNegativeX = false;
										moveSelectionNegativeY = false;
									}
								}
							}
							else if ( selectedTool == 4 )	// Process Fill Tool functionality
							{
								if ( drawx >= 0 && drawx < map.width && drawy >= 0 && drawy < map.height )
								{
									editFill(drawx, drawy, drawlayer, selectedTile);
								}
							}
						}
						else
						{
							// pasting from copymap
							mousestatus[SDL_BUTTON_LEFT] = false;
							for ( x = 0; x < copymap.width; x++ )
							{
								for ( y = 0; y < copymap.height; y++ )
								{
									if ( drawx + x >= 0 && drawx + x < map.width && drawy + y >= 0 && drawy + y < map.height )
									{
										z = copymap.name[0] + y * MAPLAYERS + x * MAPLAYERS * copymap.height;
										if ( copymap.tiles[z] )
										{
											map.tiles[drawlayer + (drawy + y)*MAPLAYERS + (drawx + x)*MAPLAYERS * map.height] = copymap.tiles[z];
										}
									}
								}
							}
							pasting = false;
						}
					}
				}
				else if ( !mousestatus[SDL_BUTTON_LEFT] )
				{
					selectingspace = false;
					savedundo = false;
				}
				if ( mousestatus[SDL_BUTTON_RIGHT] && selectedEntity[0] == NULL )
				{
					if ( selectedTool != 3 )
					{
						if ( drawx >= 0 && drawx < map.width && drawy >= 0 && drawy < map.height )
						{
							selectedTile = map.tiles[drawlayer + drawy * MAPLAYERS + drawx * MAPLAYERS * map.height];
							updateRecentTileList(selectedTile);
						}
					}
					else
					{
						selectedarea = false;
					}
				}
			}
			else
			{
				SDL_SetCursor(cursorArrow);
			}

			// main drawing
			drawClearBuffers();
			if ( mode3d == false )
			{
				if ( alllayers )
					for (c = 0; c <= drawlayer; c++)
					{
						drawLayer(camx, camy, c, &map);
					}
				else
				{
					drawLayer(camx, camy, drawlayer, &map);
				}
				if ( pasting )
				{
					drawLayer(camx - (drawx << TEXTUREPOWER), camy - (drawy << TEXTUREPOWER), copymap.name[0], &copymap);
				}
				if ( selectedarea )
				{
					pos.x = (selectedarea_x1 << TEXTUREPOWER) - camx;
					pos.y = (selectedarea_y1 << TEXTUREPOWER) - camy;
					pos.w = (selectedarea_x2 - selectedarea_x1 + 1) << TEXTUREPOWER;
					pos.h = (selectedarea_y2 - selectedarea_y1 + 1) << TEXTUREPOWER;
					drawRect(&pos, makeColorRGB(255, 255, 255), 127);
				}
				if ( viewsprites )
				{
					drawEntities2D(camx, camy);
				}
				if ( showgrid )
				{
					drawGrid(camx, camy);
				}
			}
			else
			{
				camera.winx = 0;
				camera.winy = 16;
				camera.winw = xres - 128;
				camera.winh = yres - 32;
				light = addLight(camera.x, camera.y, "editor");
				for ( node = map.entities->first; node != NULL; node = node->next )
				{
					entity = (Entity*)node->element;
					entity->flags[SPRITE] = true; // all entities rendered as SPRITES in the editor
					entity->x += 8;
					entity->y += 8;
				}
				occlusionCulling(map, camera);
                beginGraphics();
				glBeginCamera(&camera, false, map);
				glDrawWorld(&camera, REALCOLORS);
				//drawFloors(&camera);
				drawEntities3D(&camera, REALCOLORS);
				glEndCamera(&camera, false, map);
				printTextFormatted(font8x8_bmp, 8, yres - 64, "x = %3.3f\ny = %3.3f\nz = %3.3f\nang = %3.3f\nfps = %3.1f", camera.x, camera.y, camera.z, camera.ang, fps);
				list_RemoveNode(light->node);
				for ( node = map.entities->first; node != NULL; node = node->next )
				{
					entity = (Entity*)node->element;
					entity->x -= 8;
					entity->y -= 8;
				}
			}

			// primary interface
			drawWindowFancy(0, 0, xres, 16);
			if ( toolbox )
			{
				if ( statusbar )
				{
					drawWindowFancy(xres - 128, 16, xres, yres - 16);
				}
				else
				{
					drawWindowFancy(xres - 128, 16, xres, yres);
				}
				drawEditormap(camx, camy);

				// draw selected tile / hovering tile
				pos.x = xres - 48;
				pos.y = 320;
				pos.w = 32;
				pos.h = 32;
				if ( selectedTile >= 0 && selectedTile < numtiles )
				{
					if ( tiles[selectedTile] != NULL )
					{
						drawImage(tiles[selectedTile], NULL, &pos);
					}
					else
					{
						drawImage(sprites[0], NULL, &pos);
					}
				}
				else
				{
					drawImage(sprites[0], NULL, &pos);
				}
				pos.x = xres - 48;
				pos.y = 360;
				pos.w = 32;
				pos.h = 32;
				if ( drawx >= 0 && drawx < map.width && drawy >= 0 && drawy < map.height )
				{
					c = map.tiles[drawlayer + drawy * MAPLAYERS + drawx * MAPLAYERS * map.height];
					if ( c >= 0 && c < numtiles )
					{
						if ( tiles[c] != NULL )
						{
							drawImage(tiles[c], NULL, &pos);
						}
						else
						{
							drawImage(sprites[0], NULL, &pos);
						}
					}
					else
					{
						drawImage(sprites[0], NULL, &pos);
					}
				}
				else
				{
					drawImage(sprites[0], NULL, &pos);
				}
				printTextFormatted(font8x8_bmp, xres - 124, 324, "Selected:\n\n%9d",
					(selectedTile >= 0 && selectedTile < numtiles) ? selectedTile : 0);
				printTextFormatted(font8x8_bmp, xres - 124, 364, " Hovered:\n\n%9d",
					(drawx >= 0 && drawx < map.width&& drawy >= 0 && drawy < map.height) ? c : 0);

				// Print the name of the selected tool below the Tool Buttons
				switch ( selectedTool )
				{
					case 0: // Pencil
						printText(font8x8_bmp, xres - 84, 292, "PENCIL");
						break;
					case 1: // Point
						printText(font8x8_bmp, xres - 84, 292, "POINT");
						break;
					case 2: // Brush
						printText(font8x8_bmp, xres - 84, 292, "BRUSH");
						break;
					case 3: // Select
						printText(font8x8_bmp, xres - 88, 292, "SELECT");
						break;
					case 4: // Fill
						printText(font8x8_bmp, xres - 80, 292, "FILL");
						break;
				}

				int recentTileStartx = xres - 114;
				int recentTileStarty = 420;
				int recentIndex = 0;
				int pad_x = 34;
				int pad_y = 34;
				char tmpStr[32] = "PALETTE: ";
				char tmpStr2[2] = "";
				pos.x = recentTileStartx;
				pos.y = recentTileStarty;
				pos.w = 32;
				pos.h = 32;
				SDL_Rect boxPos;
				snprintf(tmpStr2, sizeof(tmpStr2), "%d", recentUsedTilePalette + 1); //reset
				strcat(tmpStr, tmpStr2);
				printText(font8x8_bmp, xres - 110, recentTileStarty - 16, tmpStr);

				for ( recentIndex = 0; recentIndex < 9; recentIndex++ )
				{
					if ( recentIndex == 3 || recentIndex == 6 )
					{
						pos.x = recentTileStartx;
						pos.y += pad_y;
					}
					

					if ( mousestatus[SDL_BUTTON_LEFT] )
					{
						if ( omousex >= pos.x && omousex < pos.x + 32 && omousey >= pos.y && omousey < pos.y + 32 )
						{
							selectedTile = recentUsedTiles[recentUsedTilePalette][recentIndex];
							lastPaletteTileSelected = recentIndex;
						}
					}
					if ( mousestatus[SDL_BUTTON_RIGHT] )
					{
						if ( omousex >= pos.x && omousex < pos.x + 32 && omousey >= pos.y && omousey < pos.y + 32 )
						{
							if ( lockTilePalette[recentUsedTilePalette] != 1 )
							{
								recentUsedTiles[recentUsedTilePalette][recentIndex] = 0;
							}
						}
					}

					boxPos.x = pos.x - 2;
					boxPos.y = pos.y - 2;
					boxPos.w = pos.w + 4;
					boxPos.h = pos.h + 4;

					if ( lastPaletteTileSelected == recentIndex )
					{
						drawRect(&boxPos, makeColorRGB(255, 0, 0), 255);
					}
					drawImage(tiles[recentUsedTiles[recentUsedTilePalette][recentIndex]], NULL, &pos);
					pos.x += pad_x;
				}

				if ( lockTilePalette[recentUsedTilePalette] == 1 )
				{
					printText(font8x8_bmp, xres - 100, pos.y + 40, "LOCKED");
				}
				else
				{
					printText(font8x8_bmp, xres - 100, pos.y + 40, "UNLOCKED");
				}
			}
			if ( statusbar )
			{
				drawWindowFancy(0, yres - 16, xres, yres);
				printTextFormatted(font8x8_bmp, 4, yres - 12, "X: %4d Y: %4d Z: %d %s", drawx, drawy, drawlayer + 1, layerstatus);
				if ( messagetime )
				{
					printText(font8x8_bmp, xres - 8 * (strlen(message)) - 12, yres - 12, message);
				}
			}

			// handle main menus
			if ( menuVisible == 1 )
			{
				drawWindowFancy(0, 16, 16, 112);
				butNew->visible = 1;
				butOpen->visible = 1;
				butDir->visible = 1;
				butSave->visible = 1;
				butSaveAs->visible = 1;
				butExit->visible = 1;
			}
			else
			{
				butNew->visible = 0;
				butOpen->visible = 0;
				butDir->visible = 0;
				butSave->visible = 0;
				butSaveAs->visible = 0;
				butExit->visible = 0;
			}
			if ( menuVisible == 2 )
			{
				drawWindowFancy(40, 16, 56, 128);
				butPaste->visible = 1;
				butCut->visible = 1;
				butCopy->visible = 1;
				butDelete->visible = 1;
				butSelectAll->visible = 1;
				butUndo->visible = 1;
				butRedo->visible = 1;
			}
			else
			{
				butPaste->visible = 0;
				butCut->visible = 0;
				butCopy->visible = 0;
				butDelete->visible = 0;
				butSelectAll->visible = 0;
				butUndo->visible = 0;
				butRedo->visible = 0;
			}
			if ( menuVisible == 3 )
			{
				drawWindowFancy(80, 16, 96, 128);
				butToolbox->visible = 1;
				butStatusBar->visible = 1;
				butAllLayers->visible = 1;
				butHoverText->visible = 1;
				butViewSprites->visible = 1;
				butGrid->visible = 1;
				but3DMode->visible = 1;
				if ( statusbar )
				{
					printText(font8x8_bmp, 84, 20, "x");
				}
				if ( toolbox )
				{
					printText(font8x8_bmp, 84, 36, "x");
				}
				if ( alllayers )
				{
					printText(font8x8_bmp, 84, 52, "x");
				}
				if ( viewsprites )
				{
					printText(font8x8_bmp, 84, 68, "x");
				}
				if ( showgrid )
				{
					printText(font8x8_bmp, 84, 84, "x");
				}
				if ( mode3d )
				{
					printText(font8x8_bmp, 84, 100, "x");
				}
				if ( hovertext )
				{
					printText(font8x8_bmp, 84, 116, "x");
				}
			}
			else
			{
				butToolbox->visible = 0;
				butStatusBar->visible = 0;
				butAllLayers->visible = 0;
				butHoverText->visible = 0;
				butViewSprites->visible = 0;
				butGrid->visible = 0;
				but3DMode->visible = 0;
			}
			if ( menuVisible == 4 )
			{
				drawWindowFancy(120, 16, 136, 48);
				butAttributes->visible = 1;
				butClearMap->visible = 1;
			}
			else
			{
				butAttributes->visible = 0;
				butClearMap->visible = 0;
			}
			if ( menuVisible == 5 )
			{
				drawWindowFancy(152, 16, 168, 48);
				butAbout->visible = 1;
				butEditorControls->visible = 1;
			}
			else
			{
				butAbout->visible = 0;
				butEditorControls->visible = 0;
			}

			// subwindows
			if ( subwindow )
			{
				drawWindowFancy(subx1, suby1, subx2, suby2);
				if ( subtext[0] != '\0' )
				{
					printText(font8x8_bmp, subx1 + 8, suby1 + 8, subtext);
				}

				// open and save windows
				if ( (openwindow == 1 || savewindow) )
				{
					drawDepressed(subx1 + 4, suby1 + 20, subx2 - 20, suby2 - 52);
					drawDepressed(subx2 - 20, suby1 + 20, subx2 - 4, suby2 - 52);
					if ( !mapNames.empty() )
					{
						slidersize = std::min<int>(((suby2 - 53) - (suby1 + 21)), ((suby2 - 53) - (suby1 + 21)) / ((real_t)mapNames.size() / 20)); //TODO: Why are int and real_t being compared?
						slidery = std::min(std::max(suby1 + 21, slidery), suby2 - 53 - slidersize);
						drawWindowFancy(subx2 - 19, slidery, subx2 - 5, slidery + slidersize);

						// directory list offset from slider
						y2 = ((real_t)(slidery - suby1 - 20) / ((suby2 - 52) - (suby1 + 20))) * (mapNames.size() + 1);
						if ( scroll )
						{
							slidery -= 8 * scroll;
							slidery = std::min(std::max(suby1 + 21, slidery), suby2 - 53 - slidersize);
							y2 = ((real_t)(slidery - suby1 - 20) / ((suby2 - 52) - (suby1 + 20))) * (mapNames.size() + 1);
							selectedFile = std::min<long unsigned int>(std::max(y2, selectedFile), std::min<long unsigned int>(mapNames.size() - 1, y2 + 19)); //TODO: Why are long unsigned int and int being compared? TWICE. On the same line.
							strcpy(filename, mapNames[selectedFile].c_str());
							inputstr = filename;
							scroll = 0;
						}
						if ( mousestatus[SDL_BUTTON_LEFT] && omousex >= subx2 - 20 && omousex < subx2 - 4 && omousey >= suby1 + 20 && omousey < suby2 - 52 )
						{
							slidery = oslidery + mousey - omousey;
							slidery = std::min(std::max(suby1 + 21, slidery), suby2 - 53 - slidersize);
							y2 = ((real_t)(slidery - suby1 - 20) / ((suby2 - 52) - (suby1 + 20))) * (mapNames.size() + 1);
							mclick = 1;
							selectedFile = std::min<long unsigned int>(std::max(y2, selectedFile), std::min<long unsigned int>(mapNames.size() - 1, y2 + 19)); //TODO: Why are long unsigned int and int being compared? TWICE. On the same line.
							strcpy(filename, mapNames[selectedFile].c_str());
							inputstr = filename;
						}
						else
						{
							oslidery = slidery;
						}

						// select a file
						if ( mousestatus[SDL_BUTTON_LEFT] )
						{
							if ( omousex >= subx1 + 8 && omousex < subx2 - 24 && omousey >= suby1 + 24 && omousey < suby2 - 56 )
							{
								selectedFile = y2 + ((omousey - suby1 - 24) >> 3);
								selectedFile = std::min<long unsigned int>(std::max(y2, selectedFile), std::min<long unsigned int>(mapNames.size() - 1, y2 + 19)); //TODO: Why are long unsigned int and int being compared? TWICE. On the same line.
								strcpy(filename, mapNames[selectedFile].c_str());
								inputstr = filename;
							}
						}
						pos.x = subx1 + 8;
						pos.y = suby1 + 24 + (std::max(selectedFile - y2, 0)) * 8;
						pos.w = subx2 - subx1 - 32;
						pos.h = 8;
						drawRect(&pos, makeColorRGB(64, 64, 64), 255);

						// print all the files within the directory
						x = subx1 + 8;
						y = suby1 + 24;
						c = std::min<long unsigned int>(mapNames.size(), 20 + y2); //TODO: Why are long unsigned int and int being compared?
						for (z = y2; z < c; z++)
						{
							printText(font8x8_bmp, x, y, mapNames[z].c_str());
							y += 8;
						}
					}

					// text box to enter file
					drawDepressed(subx1 + 4, suby2 - 48, subx2 - 68, suby2 - 32);
					printText(font8x8_bmp, subx1 + 8, suby2 - 44, filename);

					// enter filename
					if ( !SDL_IsTextInputActive() )
					{
						SDL_StartTextInput();
						inputstr = filename;
					}
					//strncpy(filename,inputstr,28);
					inputlen = 28;
					if ( (ticks - cursorflash) % TICKS_PER_SECOND < TICKS_PER_SECOND / 2 )
					{
						printText(font8x8_bmp, subx1 + 8 + strlen(filename) * 8, suby2 - 44, "\26");
					}
				}
				else if ( openwindow == 2 )
				{
					drawDepressed(subx1 + 4, suby1 + 20, subx2 - 20, suby2 - 112);
					drawDepressed(subx2 - 20, suby1 + 20, subx2 - 4, suby2 - 112);
					if ( !modFolderNames.empty() )
					{
						slidersize = std::min<int>(((suby2 - 113) - (suby1 + 21)), ((suby2 - 113) - (suby1 + 21)) / ((real_t)modFolderNames.size() / 20)); //TODO: Why are int and real_t being compared?
						slidery = std::min(std::max(suby1 + 21, slidery), suby2 - 113 - slidersize);
						drawWindowFancy(subx2 - 19, slidery, subx2 - 5, slidery + slidersize);

						// directory list offset from slider
						y2 = ((real_t)(slidery - suby1 - 20) / ((suby2 - 52) - (suby1 + 20))) * modFolderNames.size();
						if ( scroll )
						{
							slidery -= 8 * scroll;
							slidery = std::min(std::max(suby1 + 21, slidery), suby2 - 113 - slidersize);
							y2 = ((real_t)(slidery - suby1 - 20) / ((suby2 - 112) - (suby1 + 20))) * modFolderNames.size();
							selectedFile = std::min<long unsigned int>(std::max(y2, selectedFile), std::min<long unsigned int>(modFolderNames.size() - 1, y2 + 19)); //TODO: Why are long unsigned int and int being compared? TWICE. On the same line.
							std::list<std::string>::iterator it = modFolderNames.begin();
							std::advance(it, selectedFile);
							strcpy(foldername, it->c_str());
							inputstr = foldername;
							scroll = 0;
						}
						if ( mousestatus[SDL_BUTTON_LEFT] && omousex >= subx2 - 20 && omousex < subx2 - 4 && omousey >= suby1 + 20 && omousey < suby2 - 113 )
						{
							slidery = oslidery + mousey - omousey;
							slidery = std::min(std::max(suby1 + 21, slidery), suby2 - 113 - slidersize);
							y2 = ((real_t)(slidery - suby1 - 20) / ((suby2 - 112) - (suby1 + 20))) * modFolderNames.size();
							mclick = 1;
							selectedFile = std::min<long unsigned int>(std::max(y2, selectedFile), std::min<long unsigned int>(modFolderNames.size() - 1, y2 + 19)); //TODO: Why are long unsigned int and int being compared? TWICE. On the same line.
							std::list<std::string>::iterator it = modFolderNames.begin();
							std::advance(it, selectedFile);
							strcpy(foldername, it->c_str());
							inputstr = foldername;
						}
						else
						{
							oslidery = slidery;
						}

						// select a file
						if ( mousestatus[SDL_BUTTON_LEFT] )
						{
							if ( omousex >= subx1 + 8 && omousex < subx2 - 24 && omousey >= suby1 + 24 && omousey < suby2 - 116 )
							{
								selectedFile = y2 + ((omousey - suby1 - 24) >> 3);
								selectedFile = std::min<long unsigned int>(std::max(y2, selectedFile), std::min<long unsigned int>(modFolderNames.size() - 1, y2 + 19)); //TODO: Why are long unsigned int and int being compared? TWICE. On the same line.
								std::list<std::string>::iterator it = modFolderNames.begin();
								std::advance(it, selectedFile);
								strcpy(foldername, it->c_str());
								inputstr = foldername;
							}
						}
						pos.x = subx1 + 8;
						pos.y = suby1 + 24 + (selectedFile - y2) * 8;
						pos.w = subx2 - subx1 - 32;
						pos.h = 8;
						drawRect(&pos, makeColorRGB(64, 64, 64), 255);

						// print all the files within the directory
						x = subx1 + 8;
						y = suby1 + 24;
						c = std::min<long unsigned int>(modFolderNames.size(), 20 + y2); //TODO: Why are long unsigned int and int being compared?
						for ( z = y2; z < c; z++ )
						{
							std::list<std::string>::iterator it = modFolderNames.begin();
							std::advance(it, z);
							printText(font8x8_bmp, x, y, it->c_str());
							y += 8;
						}
					}

					// text box to enter file
					drawDepressed(subx1 + 4, suby2 - 108, subx2 - 4, suby2 - 92);
					printText(font8x8_bmp, subx1 + 8, suby2 - 104, foldername);

					printTextFormatted(font8x8_bmp, subx1 + 8, suby2 - 32, "Save Dir: %smaps/", physfs_saveDirectory.c_str());
					printTextFormatted(font8x8_bmp, subx1 + 8, suby2 - 16, "Load Dir: %smaps/", physfs_openDirectory.c_str());

					// enter filename
					if ( !SDL_IsTextInputActive() )
					{
						SDL_StartTextInput();
						inputstr = foldername;
					}
					//strncpy(filename,inputstr,28);
					inputlen = 28;
					if ( (ticks - cursorflash) % TICKS_PER_SECOND < TICKS_PER_SECOND / 2 )
					{
						printText(font8x8_bmp, subx1 + 8 + strlen(foldername) * 8, suby2 - 104, "\26");
					}
				}

				// new map and attributes windows
				if ( newwindow == 1 )
				{
					int pad_y1 = 0;
					int start_y = suby1 + 28;
					int rowheight = 16;

					printText(font8x8_bmp, subx1 + 8, start_y + pad_y1, "Map Name:");
					drawDepressed(subx1 + 4, suby1 + 40, subx2 - 4, suby1 + 56);
					printText(font8x8_bmp, subx1 + 8, start_y + 16, nametext);
					pad_y1 += 24;
					printText(font8x8_bmp, subx1 + 8, start_y + pad_y1 + 12, "Author Name:");
					drawDepressed(subx1 + 4, suby1 + 76, subx2 - 4, suby1 + 92);
					printText(font8x8_bmp, subx1 + 8, start_y + 16 + 36, authortext);

					start_y = suby1 + 104;
					pad_y1 = 0;
					int start_x2 = subx1 + 180;
					int start_x3 = subx2 - 32;
					printText(font8x8_bmp, subx1 + 8, start_y + pad_y1, "Map Skybox:");
					drawDepressed(subx1 + 104, start_y + pad_y1 - 4, subx1 + 168, start_y + pad_y1 + rowheight - 4);
					printText(font8x8_bmp, subx1 + 108, start_y + pad_y1, skyboxtext);

					printText(font8x8_bmp, start_x2, start_y + pad_y1, "Disable Traps:");
					printText(font8x8_bmp, start_x3, start_y + pad_y1, mapflagtext[MAP_FLAG_DISABLETRAPS]);
					pad_y1 += 24;
					printText(font8x8_bmp, subx1 + 8, start_y + pad_y1, "Map Ceiling:");
					drawDepressed(subx1 + 104, start_y + pad_y1 - 4, subx1 + 168, start_y + pad_y1 + rowheight - 4);
					printText(font8x8_bmp, subx1 + 108, start_y + pad_y1, mapflagtext[MAP_FLAG_CEILINGTILE]);

					printText(font8x8_bmp, start_x2, start_y + pad_y1, "Disable Monster Spawns:");
					printText(font8x8_bmp, start_x3, start_y + pad_y1, mapflagtext[MAP_FLAG_DISABLEMONSTERS]);
					pad_y1 += 24;
					printText(font8x8_bmp, subx1 + 8, start_y + pad_y1, "Entity Qty:");
					drawDepressed(subx1 + 104, start_y + pad_y1 - 4, subx1 + 128, start_y + pad_y1 + rowheight - 4);
					printText(font8x8_bmp, subx1 + 108, start_y + pad_y1, mapflagtext[MAP_FLAG_GENTOTALMIN]);
					printText(font8x8_bmp, subx1 + 132, start_y + pad_y1, "-");
					drawDepressed(subx1 + 144, start_y + pad_y1 - 4, subx1 + 168, start_y + pad_y1 + rowheight - 4);
					printText(font8x8_bmp, subx1 + 144 + 4, start_y + pad_y1, mapflagtext[MAP_FLAG_GENTOTALMAX]);

					printText(font8x8_bmp, start_x2, start_y + pad_y1, "Disable Loot Spawns:");
					printText(font8x8_bmp, start_x3, start_y + pad_y1, mapflagtext[MAP_FLAG_DISABLELOOT]);
			
					pad_y1 += 24;
					printText(font8x8_bmp, subx1 + 8, start_y + pad_y1, "Monster Qty:");
					drawDepressed(subx1 + 104, start_y + pad_y1 - 4, subx1 + 128, start_y + pad_y1 + rowheight - 4);
					printText(font8x8_bmp, subx1 + 108, start_y + pad_y1, mapflagtext[MAP_FLAG_GENMONSTERMIN]);
					printText(font8x8_bmp, subx1 + 132, start_y + pad_y1, "-");
					drawDepressed(subx1 + 144, start_y + pad_y1 - 4, subx1 + 168, start_y + pad_y1 + rowheight - 4);
					printText(font8x8_bmp, subx1 + 144 + 4, start_y + pad_y1, mapflagtext[MAP_FLAG_GENMONSTERMAX]);

					printText(font8x8_bmp, start_x2, start_y + pad_y1, "Disable Digging:");
					printText(font8x8_bmp, start_x3, start_y + pad_y1, mapflagtext[MAP_FLAG_DISABLEDIGGING]);

					pad_y1 += 24;
					printText(font8x8_bmp, subx1 + 8, start_y + pad_y1, "Item Qty:");
					drawDepressed(subx1 + 104, start_y + pad_y1 - 4, subx1 + 128, start_y + pad_y1 + rowheight - 4);
					printText(font8x8_bmp, subx1 + 108, start_y + pad_y1, mapflagtext[MAP_FLAG_GENLOOTMIN]);
					printText(font8x8_bmp, subx1 + 132, start_y + pad_y1, "-");
					drawDepressed(subx1 + 144, start_y + pad_y1 - 4, subx1 + 168, start_y + pad_y1 + rowheight - 4);
					printText(font8x8_bmp, subx1 + 144 + 4, start_y + pad_y1, mapflagtext[MAP_FLAG_GENLOOTMAX]);

					printText(font8x8_bmp, start_x2, start_y + pad_y1, "Disable Teleportation:");
					printText(font8x8_bmp, start_x3, start_y + pad_y1, mapflagtext[MAP_FLAG_DISABLETELEPORT]);

					pad_y1 += 24;
					printText(font8x8_bmp, subx1 + 8, start_y + pad_y1, "Deco Qty:");
					drawDepressed(subx1 + 104, start_y + pad_y1 - 4, subx1 + 128, start_y + pad_y1 + rowheight - 4);
					printText(font8x8_bmp, subx1 + 108, start_y + pad_y1, mapflagtext[MAP_FLAG_GENDECORATIONMIN]);
					printText(font8x8_bmp, subx1 + 132, start_y + pad_y1, "-");
					drawDepressed(subx1 + 144, start_y + pad_y1 - 4, subx1 + 168, start_y + pad_y1 + rowheight - 4);
					printText(font8x8_bmp, subx1 + 144 + 4, start_y + pad_y1, mapflagtext[MAP_FLAG_GENDECORATIONMAX]);

					printText(font8x8_bmp, start_x2, start_y + pad_y1, "Disable Levitation:");
					printText(font8x8_bmp, start_x3, start_y + pad_y1, mapflagtext[MAP_FLAG_DISABLELEVITATION]);

					pad_y1 += 24;
					printText(font8x8_bmp, subx1 + 8, start_y + pad_y1, "Gen Border:");
					drawDepressed(subx1 + 104, start_y + pad_y1 - 4, subx1 + 168, start_y + pad_y1 + rowheight - 4);
					printText(font8x8_bmp, subx1 + 108, start_y + pad_y1, mapflagtext[MAP_FLAG_PERIMETER_GAP]);

					printText(font8x8_bmp, start_x2, start_y + pad_y1, "Gen Adjacent Rooms:");
					printText(font8x8_bmp, start_x3, start_y + pad_y1, mapflagtext[MAP_FLAG_GENADJACENTROOMS]);

					pad_y1 += 24;
					printText(font8x8_bmp, start_x2, start_y + pad_y1, "Disable Opening Spell:");
					printText(font8x8_bmp, start_x3, start_y + pad_y1, mapflagtext[MAP_FLAG_DISABLEOPENING]);

					pad_y1 += 24;
					printText(font8x8_bmp, start_x2, start_y + pad_y1, "Disable Herx Messages:");
					printText(font8x8_bmp, start_x3, start_y + pad_y1, mapflagtext[MAP_FLAG_DISABLEMESSAGES]);

					pad_y1 += 24;
					printText(font8x8_bmp, start_x2, start_y + pad_y1, "Disable Hunger Loss:");
					printText(font8x8_bmp, start_x3, start_y + pad_y1, mapflagtext[MAP_FLAG_DISABLEHUNGER]);

					start_y = suby2 - 44;
					pad_y1 = 0;
					printText(font8x8_bmp, subx1 + 8, start_y + pad_y1, "Map Width:");
					drawDepressed(subx1 + 104, start_y + pad_y1 - 4, subx1 + 168, start_y + pad_y1 + rowheight - 4);
					printText(font8x8_bmp, subx1 + 108, start_y + pad_y1, widthtext);
					pad_y1 += 24;
					printText(font8x8_bmp, subx1 + 8, start_y + pad_y1, "Map Height:");
					drawDepressed(subx1 + 104, start_y + pad_y1 - 4, subx1 + 168, start_y + pad_y1 + rowheight - 4);
					printText(font8x8_bmp, subx1 + 108, start_y + pad_y1, heighttext);

					if ( keystatus[SDLK_TAB] )
					{
						keystatus[SDLK_TAB] = 0;
						cursorflash = ticks;
						editproperty++;
						if ( editproperty == 15 )
						{
							editproperty = 0;
						}
						switch ( editproperty )
						{
							case 0:
								inputstr = nametext;
								break;
							case 1:
								inputstr = authortext;
								break;
							case 2:
								inputstr = skyboxtext;
								break;
							case 3:
								inputstr = mapflagtext[MAP_FLAG_CEILINGTILE];
								break;
							case 4:
								inputstr = mapflagtext[MAP_FLAG_GENTOTALMIN];
								break;
							case 5:
								inputstr = mapflagtext[MAP_FLAG_GENTOTALMAX];
								break;
							case 6:
								inputstr = mapflagtext[MAP_FLAG_GENMONSTERMIN];
								break;
							case 7:
								inputstr = mapflagtext[MAP_FLAG_GENMONSTERMAX];
								break;
							case 8:
								inputstr = mapflagtext[MAP_FLAG_GENLOOTMIN];
								break;
							case 9:
								inputstr = mapflagtext[MAP_FLAG_GENLOOTMAX];
								break;
							case 10:
								inputstr = mapflagtext[MAP_FLAG_GENDECORATIONMIN];
								break;
							case 11:
								inputstr = mapflagtext[MAP_FLAG_GENDECORATIONMAX];
								break;
							case 12:
								inputstr = mapflagtext[MAP_FLAG_PERIMETER_GAP];
								break;
							case 13:
								inputstr = widthtext;
								break;
							case 14:
								inputstr = heighttext;
								break;
							default:
								break;
						}
					}

					// select a textbox
					if ( mousestatus[SDL_BUTTON_LEFT] )
					{
						if ( omousex >= start_x3 && omousey >= suby1 + 100 && omousex < start_x3 + 24 && omousey < suby1 + 116 )
						{
							if ( !strncmp(mapflagtext[MAP_FLAG_DISABLETRAPS], "[x]", 3) )
							{
								strcpy(mapflagtext[MAP_FLAG_DISABLETRAPS], "[ ]");
							}
							else
							{
								strcpy(mapflagtext[MAP_FLAG_DISABLETRAPS], "[x]");
							}
							mousestatus[SDL_BUTTON_LEFT] = 0;
						}
						if ( omousex >= start_x3 && omousey >= suby1 + 124 && omousex < start_x3 + 24 && omousey < suby1 + 140 )
						{
							if ( !strncmp(mapflagtext[MAP_FLAG_DISABLEMONSTERS], "[x]", 3) )
							{
								strcpy(mapflagtext[MAP_FLAG_DISABLEMONSTERS], "[ ]");
							}
							else
							{
								strcpy(mapflagtext[MAP_FLAG_DISABLEMONSTERS], "[x]");
							}
							mousestatus[SDL_BUTTON_LEFT] = 0;
						}
						if ( omousex >= start_x3 && omousey >= suby1 + 148 && omousex < start_x3 + 24 && omousey < suby1 + 164 )
						{
							if ( !strncmp(mapflagtext[MAP_FLAG_DISABLELOOT], "[x]", 3) )
							{
								strcpy(mapflagtext[MAP_FLAG_DISABLELOOT], "[ ]");
							}
							else
							{
								strcpy(mapflagtext[MAP_FLAG_DISABLELOOT], "[x]");
							}
							mousestatus[SDL_BUTTON_LEFT] = 0;
						}
						if ( omousex >= start_x3 && omousey >= suby1 + 172 && omousex < start_x3 + 24 && omousey < suby1 + 188 )
						{
							if ( !strncmp(mapflagtext[MAP_FLAG_DISABLEDIGGING], "[x]", 3) )
							{
								strcpy(mapflagtext[MAP_FLAG_DISABLEDIGGING], "[ ]");
							}
							else
							{
								strcpy(mapflagtext[MAP_FLAG_DISABLEDIGGING], "[x]");
							}
							mousestatus[SDL_BUTTON_LEFT] = 0;
						}
						if ( omousex >= start_x3 && omousey >= suby1 + 196 && omousex < start_x3 + 24 && omousey < suby1 + 212 )
						{
							if ( !strncmp(mapflagtext[MAP_FLAG_DISABLETELEPORT], "[x]", 3) )
							{
								strcpy(mapflagtext[MAP_FLAG_DISABLETELEPORT], "[ ]");
							}
							else
							{
								strcpy(mapflagtext[MAP_FLAG_DISABLETELEPORT], "[x]");
							}
							mousestatus[SDL_BUTTON_LEFT] = 0;
						}
						if ( omousex >= start_x3 && omousey >= suby1 + 220 && omousex < start_x3 + 24 && omousey < suby1 + 236 )
						{
							if ( !strncmp(mapflagtext[MAP_FLAG_DISABLELEVITATION], "[x]", 3) )
							{
								strcpy(mapflagtext[MAP_FLAG_DISABLELEVITATION], "[ ]");
							}
							else
							{
								strcpy(mapflagtext[MAP_FLAG_DISABLELEVITATION], "[x]");
							}
							mousestatus[SDL_BUTTON_LEFT] = 0;
						}
						if ( omousex >= start_x3 && omousey >= suby1 + 244 && omousex < start_x3 + 24 && omousey < suby1 + 260 )
						{
							if ( !strncmp(mapflagtext[MAP_FLAG_GENADJACENTROOMS], "[x]", 3) )
							{
								strcpy(mapflagtext[MAP_FLAG_GENADJACENTROOMS], "[ ]");
							}
							else
							{
								strcpy(mapflagtext[MAP_FLAG_GENADJACENTROOMS], "[x]");
							}
							mousestatus[SDL_BUTTON_LEFT] = 0;
						}
						if ( omousex >= start_x3 && omousey >= suby1 + 268 && omousex < start_x3 + 24 && omousey < suby1 + 284 )
						{
							if ( !strncmp(mapflagtext[MAP_FLAG_DISABLEOPENING], "[x]", 3) )
							{
								strcpy(mapflagtext[MAP_FLAG_DISABLEOPENING], "[ ]");
							}
							else
							{
								strcpy(mapflagtext[MAP_FLAG_DISABLEOPENING], "[x]");
							}
							mousestatus[SDL_BUTTON_LEFT] = 0;
						}
						if ( omousex >= start_x3 && omousey >= suby1 + 292 && omousex < start_x3 + 24 && omousey < suby1 + 308 )
						{
							if ( !strncmp(mapflagtext[MAP_FLAG_DISABLEMESSAGES], "[x]", 3) )
							{
								strcpy(mapflagtext[MAP_FLAG_DISABLEMESSAGES], "[ ]");
							}
							else
							{
								strcpy(mapflagtext[MAP_FLAG_DISABLEMESSAGES], "[x]");
							}
							mousestatus[SDL_BUTTON_LEFT] = 0;
						}
						if ( omousex >= start_x3 && omousey >= suby1 + 316 && omousex < start_x3 + 24 && omousey < suby1 + 332 )
						{
							if ( !strncmp(mapflagtext[MAP_FLAG_DISABLEHUNGER], "[x]", 3) )
							{
								strcpy(mapflagtext[MAP_FLAG_DISABLEHUNGER], "[ ]");
							}
							else
							{
								strcpy(mapflagtext[MAP_FLAG_DISABLEHUNGER], "[x]");
							}
							mousestatus[SDL_BUTTON_LEFT] = 0;
						}

						start_y = suby1 + 40;
						pad_y1 = 0;
						if ( omousex >= subx1 + 4 && omousey >= start_y + pad_y1 && omousex < subx2 - 4 && omousey < start_y + pad_y1 + 16 )
						{
							inputstr = nametext;
							editproperty = 0;
							cursorflash = ticks;
						}
						pad_y1 += 36;
						if ( omousex >= subx1 + 4 && omousey >= start_y + pad_y1 && omousex < subx2 - 4 && omousey < start_y + pad_y1 + 16 )
						{
							inputstr = authortext;
							editproperty = 1;
							cursorflash = ticks;
						}
						pad_y1 += 24;
						if ( omousex >= subx1 + 104 && omousey >= start_y + pad_y1 && omousex < subx1 + 104 + 64 && omousey < start_y + pad_y1 + 16 )
						{
							inputstr = skyboxtext;
							editproperty = 2;
							cursorflash = ticks;
						}
						pad_y1 += 24;
						if ( omousex >= subx1 + 104 && omousey >= start_y + pad_y1 && omousex < subx1 + 104 + 64 && omousey < start_y + pad_y1 + 16 )
						{
							inputstr = mapflagtext[MAP_FLAG_CEILINGTILE];
							editproperty = 3;
							cursorflash = ticks;
						}
						pad_y1 += 24;
						if ( omousex >= subx1 + 104 && omousey >= start_y + pad_y1 && omousex < subx1 + 104 + 24 && omousey < start_y + pad_y1 + 16 )
						{
							inputstr = mapflagtext[MAP_FLAG_GENTOTALMIN];
							editproperty = 4;
							cursorflash = ticks;
						}
						if ( omousex >= subx1 + 144 && omousey >= start_y + pad_y1 && omousex < subx1 + 144 + 24 && omousey < start_y + pad_y1 + 16 )
						{
							inputstr = mapflagtext[MAP_FLAG_GENTOTALMAX];
							editproperty = 5;
							cursorflash = ticks;
						}
						pad_y1 += 24;
						if ( omousex >= subx1 + 104 && omousey >= start_y + pad_y1 && omousex < subx1 + 104 + 24 && omousey < start_y + pad_y1 + 16 )
						{
							inputstr = mapflagtext[MAP_FLAG_GENMONSTERMIN];
							editproperty = 6;
							cursorflash = ticks;
						}
						if ( omousex >= subx1 + 144 && omousey >= start_y + pad_y1 && omousex < subx1 + 144 + 24 && omousey < start_y + pad_y1 + 16 )
						{
							inputstr = mapflagtext[MAP_FLAG_GENMONSTERMAX];
							editproperty = 7;
							cursorflash = ticks;
						}
						pad_y1 += 24;
						if ( omousex >= subx1 + 104 && omousey >= start_y + pad_y1 && omousex < subx1 + 104 + 24 && omousey < start_y + pad_y1 + 16 )
						{
							inputstr = mapflagtext[MAP_FLAG_GENLOOTMIN];
							editproperty = 8;
							cursorflash = ticks;
						}
						if ( omousex >= subx1 + 144 && omousey >= start_y + pad_y1 && omousex < subx1 + 144 + 24 && omousey < start_y + pad_y1 + 16 )
						{
							inputstr = mapflagtext[MAP_FLAG_GENLOOTMAX];
							editproperty = 9;
							cursorflash = ticks;
						}
						pad_y1 += 24;
						if ( omousex >= subx1 + 104 && omousey >= start_y + pad_y1 && omousex < subx1 + 104 + 24 && omousey < start_y + pad_y1 + 16 )
						{
							inputstr = mapflagtext[MAP_FLAG_GENDECORATIONMIN];
							editproperty = 10;
							cursorflash = ticks;
						}
						if ( omousex >= subx1 + 144 && omousey >= start_y + pad_y1 && omousex < subx1 + 144 + 24 && omousey < start_y + pad_y1 + 16 )
						{
							inputstr = mapflagtext[MAP_FLAG_GENDECORATIONMAX];
							editproperty = 11;
							cursorflash = ticks;
						}
						pad_y1 += 24;
						if ( omousex >= subx1 + 104 && omousey >= start_y + pad_y1 && omousex < subx1 + 104 + 64 && omousey < start_y + pad_y1 + 16 )
						{
							inputstr = mapflagtext[MAP_FLAG_PERIMETER_GAP];
							editproperty = 12;
							cursorflash = ticks;
						}

						if ( omousex >= subx1 + 104 && omousey >= suby2 - 48 && omousex < subx1 + 168 && omousey < suby2 - 32 )
						{
							inputstr = widthtext;
							editproperty = 13;
							cursorflash = ticks;
						}
						if ( omousex >= subx1 + 104 && omousey >= suby2 - 24 && omousex < subx1 + 168 && omousey < suby2 - 8 )
						{
							inputstr = heighttext;
							editproperty = 14;
							cursorflash = ticks;
						}
					}

					start_y = suby1 + 44;
					pad_y1 = 0;

					if ( editproperty == 0 )   // edit map name
					{
						if ( !SDL_IsTextInputActive() )
						{
							SDL_StartTextInput();
							inputstr = nametext;
						}
						//strncpy(nametext,inputstr,31);
						inputlen = 31;
						if ( (ticks - cursorflash) % TICKS_PER_SECOND < TICKS_PER_SECOND / 2 )
						{
							printText(font8x8_bmp, subx1 + 8 + strlen(nametext) * 8, start_y + pad_y1, "\26");
						}
					}
					pad_y1 += 36;
					if ( editproperty == 1 )   // edit author name
					{
						if ( !SDL_IsTextInputActive() )
						{
							SDL_StartTextInput();
							inputstr = authortext;
						}
						//strncpy(authortext,inputstr,31);
						inputlen = 31;
						if ( (ticks - cursorflash) % TICKS_PER_SECOND < TICKS_PER_SECOND / 2 )
						{
							printText(font8x8_bmp, subx1 + 8 + strlen(authortext) * 8, start_y + pad_y1, "\26");
						}
					}
					pad_y1 += 24;
					if ( editproperty == 2 )   // edit map skybox
					{
						if ( !SDL_IsTextInputActive() )
						{
							SDL_StartTextInput();
							inputstr = skyboxtext;
						}
						//strncpy(widthtext,inputstr,3);
						inputlen = 3;
						if ( (ticks - cursorflash) % TICKS_PER_SECOND < TICKS_PER_SECOND / 2 )
						{
							printText(font8x8_bmp, subx1 + 108 + strlen(skyboxtext) * 8, start_y + pad_y1, "\26");
						}
					}
					pad_y1 += 24;
					if ( editproperty == 3 )   // edit map ceiling tiles
					{
						if ( !SDL_IsTextInputActive() )
						{
							SDL_StartTextInput();
							inputstr = mapflagtext[MAP_FLAG_CEILINGTILE];
						}
						//strncpy(widthtext,inputstr,3);
						inputlen = 3;
						if ( (ticks - cursorflash) % TICKS_PER_SECOND < TICKS_PER_SECOND / 2 )
						{
							printText(font8x8_bmp, subx1 + 108 + strlen(mapflagtext[MAP_FLAG_CEILINGTILE]) * 8, start_y + pad_y1, "\26");
						}
					}
					if ( editproperty == 13 )   // edit map width
					{
						if ( !SDL_IsTextInputActive() )
						{
							SDL_StartTextInput();
							inputstr = widthtext;
						}
						//strncpy(widthtext,inputstr,3);
						inputlen = 3;
						if ( (ticks - cursorflash) % TICKS_PER_SECOND < TICKS_PER_SECOND / 2 )
						{
							printText(font8x8_bmp, subx1 + 108 + strlen(widthtext) * 8, suby2 - 44, "\26");
						}
					}
					if ( editproperty == 14 )   // edit map height
					{
						if ( !SDL_IsTextInputActive() )
						{
							SDL_StartTextInput();
							inputstr = heighttext;
						}
						//strncpy(heighttext,inputstr,3);
						inputlen = 3;
						if ( (ticks - cursorflash) % TICKS_PER_SECOND < TICKS_PER_SECOND / 2 )
						{
							printText(font8x8_bmp, subx1 + 108 + strlen(heighttext) * 8, suby2 - 20, "\26");
						}
					}
					pad_y1 += 24;
					if ( editproperty == 4 )   // edit min entity gen
					{
						if ( !SDL_IsTextInputActive() )
						{
							SDL_StartTextInput();
							inputstr = mapflagtext[MAP_FLAG_GENTOTALMIN];
						}
						inputlen = 2;
						if ( (ticks - cursorflash) % TICKS_PER_SECOND < TICKS_PER_SECOND / 2 )
						{
							printText(font8x8_bmp, subx1 + 108 + strlen(mapflagtext[MAP_FLAG_GENTOTALMIN]) * 8, start_y + pad_y1, "\26");
						}
					}
					if ( editproperty == 5 )   // edit max entity gen
					{
						if ( !SDL_IsTextInputActive() )
						{
							SDL_StartTextInput();
							inputstr = mapflagtext[MAP_FLAG_GENTOTALMAX];
						}
						inputlen = 2;
						if ( (ticks - cursorflash) % TICKS_PER_SECOND < TICKS_PER_SECOND / 2 )
						{
							printText(font8x8_bmp, subx1 + 148 + strlen(mapflagtext[MAP_FLAG_GENTOTALMAX]) * 8, start_y + pad_y1, "\26");
						}
					}
					pad_y1 += 24;
					if ( editproperty == 6 )   // edit min monster gen
					{
						if ( !SDL_IsTextInputActive() )
						{
							SDL_StartTextInput();
							inputstr = mapflagtext[MAP_FLAG_GENMONSTERMIN];
						}
						inputlen = 2;
						if ( (ticks - cursorflash) % TICKS_PER_SECOND < TICKS_PER_SECOND / 2 )
						{
							printText(font8x8_bmp, subx1 + 108 + strlen(mapflagtext[MAP_FLAG_GENMONSTERMIN]) * 8, start_y + pad_y1, "\26");
						}
					}
					if ( editproperty == 7 )   // edit max monster gen
					{
						if ( !SDL_IsTextInputActive() )
						{
							SDL_StartTextInput();
							inputstr = mapflagtext[MAP_FLAG_GENMONSTERMAX];
						}
						inputlen = 2;
						if ( (ticks - cursorflash) % TICKS_PER_SECOND < TICKS_PER_SECOND / 2 )
						{
							printText(font8x8_bmp, subx1 + 148 + strlen(mapflagtext[MAP_FLAG_GENMONSTERMAX]) * 8, start_y + pad_y1, "\26");
						}
					}
					pad_y1 += 24;
					if ( editproperty == 8 )   // edit min monster gen
					{
						if ( !SDL_IsTextInputActive() )
						{
							SDL_StartTextInput();
							inputstr = mapflagtext[MAP_FLAG_GENLOOTMIN];
						}
						inputlen = 2;
						if ( (ticks - cursorflash) % TICKS_PER_SECOND < TICKS_PER_SECOND / 2 )
						{
							printText(font8x8_bmp, subx1 + 108 + strlen(mapflagtext[MAP_FLAG_GENLOOTMIN]) * 8, start_y + pad_y1, "\26");
						}
					}
					if ( editproperty == 9 )   // edit max monster gen
					{
						if ( !SDL_IsTextInputActive() )
						{
							SDL_StartTextInput();
							inputstr = mapflagtext[MAP_FLAG_GENLOOTMAX];
						}
						inputlen = 2;
						if ( (ticks - cursorflash) % TICKS_PER_SECOND < TICKS_PER_SECOND / 2 )
						{
							printText(font8x8_bmp, subx1 + 148 + strlen(mapflagtext[MAP_FLAG_GENLOOTMAX]) * 8, start_y + pad_y1, "\26");
						}
					}
					pad_y1 += 24;
					if ( editproperty == 10 )   // edit min decoration gen
					{
						if ( !SDL_IsTextInputActive() )
						{
							SDL_StartTextInput();
							inputstr = mapflagtext[MAP_FLAG_GENDECORATIONMIN];
						}
						inputlen = 2;
						if ( (ticks - cursorflash) % TICKS_PER_SECOND < TICKS_PER_SECOND / 2 )
						{
							printText(font8x8_bmp, subx1 + 108 + strlen(mapflagtext[MAP_FLAG_GENDECORATIONMIN]) * 8, start_y + pad_y1, "\26");
						}
					}
					if ( editproperty == 11 )   // edit max decoration gen
					{
						if ( !SDL_IsTextInputActive() )
						{
							SDL_StartTextInput();
							inputstr = mapflagtext[MAP_FLAG_GENDECORATIONMAX];
						}
						inputlen = 2;
						if ( (ticks - cursorflash) % TICKS_PER_SECOND < TICKS_PER_SECOND / 2 )
						{
							printText(font8x8_bmp, subx1 + 148 + strlen(mapflagtext[MAP_FLAG_GENDECORATIONMAX]) * 8, start_y + pad_y1, "\26");
						}
					}
					pad_y1 += 24;
					if ( editproperty == 12 )   // edit perimeter gap
					{
						if ( !SDL_IsTextInputActive() )
						{
							SDL_StartTextInput();
							inputstr = mapflagtext[MAP_FLAG_PERIMETER_GAP];
						}
						inputlen = 3;
						if ( (ticks - cursorflash) % TICKS_PER_SECOND < TICKS_PER_SECOND / 2 )
						{
							printText(font8x8_bmp, subx1 + 108 + strlen(mapflagtext[MAP_FLAG_PERIMETER_GAP]) * 8, start_y + pad_y1, "\26");
						}
					}
				}
				else if ( newwindow == 2 ) 
				{
					if ( selectedEntity[0] != NULL )
					{
						spriteStats = selectedEntity[0]->getStats();
						if ( spriteStats != nullptr )
						{
							int numProperties = sizeof(monsterPropertyNames) / sizeof(monsterPropertyNames[0]); //find number of entries in property list
							const int lenProperties = sizeof(monsterPropertyNames[0]) / sizeof(char); //find length of entry in property list
							int spacing = 20; // px between each item in the list.
							int pad_y1 = suby1 + 28; // 28 px spacing from subwindow start.
							int pad_x1 = subx1 + 8; // 8px spacing from subwindow start.
							int pad_x2 = 64;
							int pad_y2 = suby1 + 28 + 2 * spacing;
							int pad_x3 = 44; //property field width
							int pad_x4;
							Uint32 color = makeColorRGB(255, 255, 255);
							char tmpPropertyName[lenProperties] = "";
							for ( int i = 0; i < numProperties; i++ )
							{
								strcpy(tmpPropertyName, monsterPropertyNames[i]);
								pad_y1 = suby1 + 28 + i * spacing;

								// value of 0 is name field, should be longer
								if ( i == 0 )
								{
									drawDepressed(pad_x1 - 4, pad_y1 + 16 - 4, subx2 - 4, pad_y1 + 28);
									// print values on top of boxes
									printTextFormattedColor(font8x8_bmp, pad_x1, pad_y1, color, tmpPropertyName);
									printText(font8x8_bmp, pad_x1, pad_y1 + 16, spriteProperties[i]);
								}
								else
								{
									pad_y1 += spacing;
									if ( i < 7 )
									{
										if ( i < 3 ) //hp
										{
											color = makeColorRGB(0, 255, 0);
										}
										else if ( i < 5 ) //mp
										{
											color = makeColorRGB(0, 255, 228);
										}
										else if ( i == 5 ) //level
										{
											color = makeColorRGB(255, 192, 0);
										}
										else if ( i == 6 ) //gold
										{
											color = makeColorRGB(255, 192, 0);
										}
										drawDepressed(pad_x1 + pad_x2 - 4, pad_y1 - 4, pad_x1 + pad_x2 + pad_x3 - 4, pad_y1 + 16 - 4);
										// draw another box side by side, spaced by pad_x3 + 16
										drawDepressed(pad_x1 + pad_x2 - 4 + (pad_x3 + 16), pad_y1 - 4, pad_x1 + pad_x2 + pad_x3 - 4 + (pad_x3 + 16), pad_y1 + 16 - 4);
										// print values on top of boxes
										// print property name
										printTextFormattedColor(font8x8_bmp, pad_x1, pad_y1, color, tmpPropertyName);
										// print dash between boxes
										printTextFormattedColor(font8x8_bmp, pad_x1 + pad_x2 - 4 + (pad_x3 + 4), pad_y1, color, "-");
										// print left text
										printText(font8x8_bmp, pad_x1 + pad_x2, pad_y1, spriteProperties[i]);
										// print right text
										printText(font8x8_bmp, pad_x1 + pad_x2 + (pad_x3 + 16), pad_y1, spriteProperties[i + 12]);
									}
									else if ( i < 13 )
									{
										color = makeColorRGB(255, 255, 255);
										pad_y1 += spacing + 10;
										drawDepressed(pad_x1 + pad_x2 - 4, pad_y1 - 4, pad_x1 + pad_x2 + pad_x3 - 4, pad_y1 + 16 - 4);
										// draw another box side by side, spaced by pad_x3 + 16
										drawDepressed(pad_x1 + pad_x2 - 4 + (pad_x3 + 16), pad_y1 - 4, pad_x1 + pad_x2 + pad_x3 - 4 + (pad_x3 + 16), pad_y1 + 16 - 4);
										// print values on top of boxes
										// print property name
										printTextFormattedColor(font8x8_bmp, pad_x1, pad_y1, color, tmpPropertyName);
										// print dash between boxes
										printTextFormattedColor(font8x8_bmp, pad_x1 + pad_x2 - 4 + (pad_x3 + 4), pad_y1, color, "-");
										// print left text
										printText(font8x8_bmp, pad_x1 + pad_x2, pad_y1, spriteProperties[i]);
										// print right text
										printText(font8x8_bmp, pad_x1 + pad_x2 + (pad_x3 + 16), pad_y1, spriteProperties[i + 12]);
									}
									else if ( i >= 13 )
									{
										if ( i == 13 )
										{
											pad_y1 += 10;
										}
										color = makeColorRGB(255, 255, 255);
										pad_y1 += spacing + 10;
										drawDepressed(pad_x1 + pad_x2 - 4, pad_y1 - 4, pad_x1 + pad_x2 + pad_x3 - 4, pad_y1 + 16 - 4);
										// print property name
										printTextFormattedColor(font8x8_bmp, pad_x1, pad_y1, color, tmpPropertyName);
										// print left text
										printText(font8x8_bmp, pad_x1 + pad_x2, pad_y1, spriteProperties[i + 12]);
										if ( i == 13 && spriteStats->type == SHOPKEEPER )
										{
											char shopTypeText[32] = "";
											color = makeColorRGB(0, 255, 0);
											switch ( atoi(spriteProperties[25]) )
											{
												case 1:
													strcpy(shopTypeText, "Arms and Armor");
													break;
												case 2:
													strcpy(shopTypeText, "Hats and Helmets");
													break;
												case 3:
													strcpy(shopTypeText, "Jewelry");
													break;
												case 4:
													strcpy(shopTypeText, "Bookstore");
													break;
												case 5:
													strcpy(shopTypeText, "Apothecary");
													break;
												case 6:
													strcpy(shopTypeText, "Magistaffs");
													break;
												case 7:
													strcpy(shopTypeText, "Food Store");
													break;
												case 8:
													strcpy(shopTypeText, "Hardware Store");
													break;
												case 9:
													strcpy(shopTypeText, "Hunting Store");
													break;
												case 10:
													strcpy(shopTypeText, "General Store");
													break;
												default:
													strcpy(shopTypeText, "Default Random Store");
													color = makeColorRGB(255, 255, 255);
													break;
											}
											printTextFormattedColor(font8x8_bmp, pad_x1 + pad_x2 + pad_x3 + 8, pad_y1, color, shopTypeText);
											color = makeColorRGB(255, 255, 255);
										}
									}
								}
							}
							
							// Cycle properties with TAB.
							if ( keystatus[SDLK_TAB] )
							{
								keystatus[SDLK_TAB] = 0;
								cursorflash = ticks;
								editproperty++;
								if ( editproperty == numProperties * 2 - 2 )
								{
									// limit of properties is twice the vertical count
									editproperty = 0;
								}
								
								inputstr = spriteProperties[editproperty];
							}
							// select a textbox
							if ( mousestatus[SDL_BUTTON_LEFT] )
							{
								for ( int i = 0; i < numProperties; i++ )
								{
									pad_y1 = suby1 + 28 + i * spacing;
									if ( i == 0 )
									{
										if ( omousex >= pad_x1 - 4 && omousey >= pad_y1 + 16 - 4 && omousex < subx2 - 4 && omousey < pad_y1 + 32 - 4 )
										{
											inputstr = spriteProperties[i];
											editproperty = i;
											cursorflash = ticks;
										}
									}
									else
									{
										pad_y1 += spacing;
										if ( i < 7 )
										{
											// check if mouse is in left property box
											if ( omousex >= pad_x1 + pad_x2 - 4 && omousey >= pad_y1 - 4 && omousex < pad_x1 + pad_x2 + pad_x3 - 4 && omousey < pad_y1 + 16 - 4 )
											{
												inputstr = spriteProperties[i];
												editproperty = i;
												cursorflash = ticks;
											}
											// check if mouse is in right property box (offset from above by pad_x3 + 16)
											else if ( omousex >= pad_x1 + pad_x2 - 4 + (pad_x3 + 16) && omousey >= pad_y1 - 4 && omousex < pad_x1 + pad_x2 + pad_x3 - 4 + (pad_x3 + 16) && omousey < pad_y1 + 16 - 4 )
											{
												inputstr = spriteProperties[i + 12];
												editproperty = i + 12;
												cursorflash = ticks;
											}
										}
										else if ( i < 13 )
										{
											pad_y1 += spacing + 10;
											// check if mouse is in left property box
											if ( omousex >= pad_x1 + pad_x2 - 4 && omousey >= pad_y1 - 4 && omousex < pad_x1 + pad_x2 + pad_x3 - 4 && omousey < pad_y1 + 16 - 4 )
											{
												inputstr = spriteProperties[i];
												editproperty = i;
												cursorflash = ticks;
											}
											// check if mouse is in right property box (offset from above by pad_x3 + 16)
											else if ( omousex >= pad_x1 + pad_x2 - 4 + (pad_x3 + 16) && omousey >= pad_y1 - 4 && omousex < pad_x1 + pad_x2 + pad_x3 - 4 + (pad_x3 + 16) && omousey < pad_y1 + 16 - 4 )
											{
												inputstr = spriteProperties[i + 12];
												editproperty = i + 12;
												cursorflash = ticks;
											}
										}
										else if ( i >= 13 )
										{
											if ( i == 13 )
											{
												pad_y1 += 10;
											}
											pad_y1 += spacing + 10;
											// check if mouse is in left property box
											if ( omousex >= pad_x1 + pad_x2 - 4 && omousey >= pad_y1 - 4 && omousex < pad_x1 + pad_x2 + pad_x3 - 4 && omousey < pad_y1 + 16 - 4 )
											{
												inputstr = spriteProperties[i + 12];
												editproperty = i + 12;
												cursorflash = ticks;
											}
										}
									}
								}
							}

							//items for monster
							pad_y2 = suby1 + 28 + 2 * spacing;
							pad_x3 = 40;
							pad_x4 = subx2 - 112;
							printTextFormattedColor(font8x8_bmp, pad_x4 - 8, pad_y2, color, " Helm");
							
							//pad_y2 += spacing * 2 - 16;
							pad_y2 += spacing * 2;
							printTextFormattedColor(font8x8_bmp, pad_x4 - 8, pad_y2, color, "Amulet");

							//pad_x4 += 64 * 2;
							pad_y2 += spacing * 2;
							printTextFormattedColor(font8x8_bmp, pad_x4 - 4, pad_y2, color, "Armor");

							//pad_x4 -= 64;
							//pad_y2 += spacing * 2 - 16;
							pad_y2 += spacing * 2;
							printTextFormattedColor(font8x8_bmp, pad_x4 - 8 - 4, pad_y2, color, " Boots");

							pad_y2 = suby1 + 28 + 2 * spacing;
							pad_y2 += 16;
							pad_x4 -= 64;
							printTextFormattedColor(font8x8_bmp, pad_x4 - 8 - 4, pad_y2, color, " Cloak");

							pad_x4 += 64 * 2;
							printTextFormattedColor(font8x8_bmp, pad_x4 - 8, pad_y2, color, " Mask");

							pad_x4 -= 64 * 2;
							pad_y2 += spacing * 2;
							printTextFormattedColor(font8x8_bmp, pad_x4 - 8, pad_y2, color, "Weapon");

							pad_x4 += 64 * 2;
							printTextFormattedColor(font8x8_bmp, pad_x4 - 8, pad_y2, color, "Shield");

							pad_x4 -= 64 * 2;
							pad_y2 += spacing * 2;
							printTextFormattedColor(font8x8_bmp, pad_x4 - 8, pad_y2, color, " Ring");

							pad_x4 += 64 * 2;
							printTextFormattedColor(font8x8_bmp, pad_x4 - 8, pad_y2, color, "Gloves");

							pad_x4 -= 64 * 2;

							pad_y2 += 32 + spacing * 2;
							printTextFormattedColor(font8x8_bmp, pad_x4 - 8, pad_y2, color, "Inventory");

							pad_y2 += spacing * 3 + 8;
							if ( !strcmp(spriteProperties[31], "disable") )
							{
								printTextFormattedColor(font8x8_bmp, pad_x4 - 8, pad_y2, color, "Disable Miniboss: [x]");
							}
							else
							{
								printTextFormattedColor(font8x8_bmp, pad_x4 - 8, pad_y2, color, "Disable Miniboss: [ ]");
							}
							if ( mousestatus[SDL_BUTTON_LEFT] )
							{
								int checkbox_x1 = pad_x4 - 8 + strlen("Disable Miniboss: ") * 8;
								int checkbox_x2 = checkbox_x1 + strlen("[ ]") * 8;
								if ( omousex >= checkbox_x1 && omousey >= pad_y2 && omousex < checkbox_x2 && omousey < pad_y2 + 8 )
								{
									mousestatus[SDL_BUTTON_LEFT] = 0;
									if ( !strcmp(spriteProperties[31], "disable") )
									{
										strcpy(spriteProperties[31], "");
									}
									else
									{
										strcpy(spriteProperties[31], "disable");
									}
								}
							}

							if ( editproperty < numProperties * 2 - 2 )   // edit property values
							{
								// limit of properties is twice the vertical count
								if ( !SDL_IsTextInputActive() )
								{
									SDL_StartTextInput();
									inputstr = spriteProperties[0];
								}
								//strncpy(nametext,inputstr,31);

								// value of 0 is the name field, else the input is a number
								if ( editproperty == 0 )
								{
									inputlen = 31;
								}
								else
								{
									inputlen = 4;
								}
								if ( (ticks - cursorflash) % TICKS_PER_SECOND < TICKS_PER_SECOND / 2 )
								{
									pad_y1 = suby1 + 28 + editproperty * spacing;

									if ( editproperty == 0 )
									{
										printText(font8x8_bmp, pad_x1 + strlen(spriteProperties[editproperty]) * 8, pad_y1 + 16, "\26");
									}
									else if ( editproperty < 7 )
									{
										pad_y1 += spacing;
										// left box
										printText(font8x8_bmp, pad_x1 + pad_x2 + strlen(spriteProperties[editproperty]) * 8, pad_y1, "\26");
									}
									else if ( editproperty < 13 )
									{
										pad_y1 += spacing;
										pad_y1 += spacing + 10;
										// left box
										printText(font8x8_bmp, pad_x1 + pad_x2 + strlen(spriteProperties[editproperty]) * 8, pad_y1, "\26");
									}
									else if ( editproperty < 19 )
									{
										pad_y1 = suby1 + 28 + (editproperty - 12) * spacing;
										pad_y1 += spacing;
										// right box
										printText(font8x8_bmp, pad_x1 + pad_x2 + (pad_x3 + 20) + strlen(spriteProperties[editproperty]) * 8, pad_y1, "\26");
									}
									else if ( editproperty < 25 )
									{
										pad_y1 = suby1 + 28 + (editproperty - 12) * spacing;
										pad_y1 += spacing;
										pad_y1 += spacing + 10;
										// right box
										printText(font8x8_bmp, pad_x1 + pad_x2 + (pad_x3 + 20) + strlen(spriteProperties[editproperty]) * 8, pad_y1, "\26");
									}
									else if ( editproperty >= 25 )
									{
										pad_y1 = suby1 + 28 + (editproperty - 12) * spacing;
										pad_y1 += spacing;
										pad_y1 += spacing + 20;
										// left box
										printText(font8x8_bmp, pad_x1 + pad_x2 + strlen(spriteProperties[editproperty]) * 8, pad_y1, "\26");
									}
								}
							}
						}
					}
				}
				else if ( newwindow == 3 )
				{
					if ( selectedEntity[0] != NULL )
					{
						int numProperties = sizeof(chestPropertyNames) / sizeof(chestPropertyNames[0]); //find number of entries in property list
						const int lenProperties = sizeof(chestPropertyNames[0]) / sizeof(char); //find length of entry in property list
						int spacing = 36; // 36 px between each item in the list.
						int pad_y1 = suby1 + 28; // 28 px spacing from subwindow start.
						int pad_x1 = subx1 + 8; // 8px spacing from subwindow start.
						int pad_x2 = 64;
						int pad_x3 = pad_x1 + pad_x2 + 8;
						int pad_y2 = 0;
						char tmpPropertyName[lenProperties] = "";
						Uint32 color = makeColorRGB(0, 255, 0);
						Uint32 colorRandom = makeColorRGB(0, 168, 255);

						for ( int i = 0; i < numProperties; i++ )
						{
							int propertyInt = atoi(spriteProperties[i]);

							strcpy(tmpPropertyName, chestPropertyNames[i]);
							pad_y1 = suby1 + 28 + i * spacing;
							pad_y2 = suby1 + 44 + i * spacing;
							// box outlines then text
							drawDepressed(pad_x1 - 4, suby1 + 40 + i * spacing, pad_x1 - 4 + pad_x2, suby1 + 56 + i * spacing);
							// print values on top of boxes
							printText(font8x8_bmp, pad_x1, suby1 + 44 + i * spacing, spriteProperties[i]);
							printText(font8x8_bmp, pad_x1, pad_y1, tmpPropertyName);

							if ( errorArr[i] != 1 )
							{
								if ( i == 0 )
								{
									if ( propertyInt > 3 || propertyInt < 0 )
									{
										errorMessage = 60;
										errorArr[i] = 1;
										snprintf(spriteProperties[i], sizeof(spriteProperties[i]), "%d", 1); //reset
									}
									else
									{
										color = makeColorRGB(0, 255, 0);
										char tmpStr[32] = "";
										if ( propertyInt == 0 )
										{
											strcpy(tmpStr, "EAST");
										}
										else if ( propertyInt == 1 )
										{
											strcpy(tmpStr, "SOUTH");
										}
										else if ( propertyInt == 2 )
										{
											strcpy(tmpStr, "WEST");
										}
										else if ( propertyInt == 3 )
										{
											strcpy(tmpStr, "NORTH");
										}
										printTextFormattedColor(font8x8_bmp, pad_x3, pad_y2, color, tmpStr);
									}
								} 
								else if ( i == 1 )
								{
									if ( propertyInt > 8 || propertyInt < 0 )
									{
										errorMessage = 60;
										errorArr[i] = 1;
										snprintf(spriteProperties[i], sizeof(spriteProperties[i]), "%d", 0); //reset
									}
								}
								else if ( i == 2 )
								{
									if ( propertyInt > 100 || propertyInt < -1 )
									{
										errorMessage = 60;
										errorArr[i] = 1;
										snprintf(spriteProperties[i], sizeof(spriteProperties[i]), "%d", -1); //reset
									}
									else if ( propertyInt == -1 )
									{
										printTextFormattedColor(font8x8_bmp, pad_x3, pad_y2, colorRandom, "Default 10%");
									}
									else
									{
										color = makeColorRGB(0, 255, 0);
										char tmpStr[32] = "";
										strcpy(tmpStr, spriteProperties[i]); //reset
										strcat(tmpStr, " %%");
										printTextFormattedColor(font8x8_bmp, pad_x3, pad_y2, color, tmpStr);
									}
								}
								else if ( i == 3 )
								{
									if ( propertyInt > 100 || propertyInt < -1 )
									{
										errorMessage = 60;
										errorArr[i] = 1;
										snprintf(spriteProperties[i], sizeof(spriteProperties[i]), "%d", -1); //reset
									}
									else if ( propertyInt == -1 )
									{
										printTextFormattedColor(font8x8_bmp, pad_x3, pad_y2, colorRandom, "Default Rand %");
									}
									else
									{
										color = makeColorRGB(0, 255, 0);
										char tmpStr[32] = "";
										strcpy(tmpStr, spriteProperties[i]); //reset
										strcat(tmpStr, " %%");
										printTextFormattedColor(font8x8_bmp, pad_x3, pad_y2, color, tmpStr);
									}
								}
							}

							if ( errorMessage )
							{
								color = makeColorRGB(255, 0, 0);
								if ( errorArr[i] == 1 )
								{
									printTextFormattedColor(font8x8_bmp, pad_x3, pad_y2, color, "Invalid ID!");
								}
							}

							pad_x1 = subx1 + 8;
						}

						pad_y1 = suby1 + 28 + 4 * spacing + 18;
						pad_x1 += 18;
						spacing = 18;
						//printText(font8x8_bmp, pad_x1 + 16, pad_y1, "Chest Facing");
						pad_y1 = suby1 + 28 + 9 * spacing;
						printText(font8x8_bmp, pad_x1 + 32, pad_y1, "NORTH(3)");
						pad_y1 = suby1 + 28 + 10 * spacing;
						printText(font8x8_bmp, pad_x1, pad_y1, "WEST(2)");
						printText(font8x8_bmp, pad_x1 + 96 - 16, pad_y1, "EAST(0)");
						pad_y1 = suby1 + 28 + 11 * spacing;
						printText(font8x8_bmp, pad_x1 + 32, pad_y1, "SOUTH(1)");

						spacing = 14;
						pad_y1 = suby1 + 14 + 14 * spacing;
						pad_x1 = subx1 + 8 + 192;
						pad_y1 = suby1 + 4 + 1 * spacing;
						printText(font8x8_bmp, pad_x1, pad_y1, "Chest Types");
						pad_y1 += spacing;
						printText(font8x8_bmp, pad_x1, pad_y1, "0 - Random");
						pad_y1 += spacing;
						printText(font8x8_bmp, pad_x1, pad_y1, "1 - Garbage");
						pad_y1 += spacing;
						printText(font8x8_bmp, pad_x1, pad_y1, "2 - Food");
						pad_y1 += spacing;
						printText(font8x8_bmp, pad_x1, pad_y1, "3 - Jewelry");
						pad_y1 += spacing;
						printText(font8x8_bmp, pad_x1, pad_y1, "4 - Equipment");
						pad_y1 += spacing;
						printText(font8x8_bmp, pad_x1, pad_y1, "5 - Tools");
						pad_y1 += spacing;
						printText(font8x8_bmp, pad_x1, pad_y1, "6 - Magical");
						pad_y1 += spacing;
						printText(font8x8_bmp, pad_x1, pad_y1, "7 - Potions");
						pad_y1 += spacing;
						printText(font8x8_bmp, pad_x1, pad_y1, "8 - Empty");

						pad_x1 = subx1 + 8;
						pad_y1 = suby1 + 28;
						spacing = 36;

						// Cycle properties with TAB.
						if ( keystatus[SDLK_TAB] )
						{
							keystatus[SDLK_TAB] = 0;
							cursorflash = ticks;
							editproperty++;
							if ( editproperty == numProperties )
							{
								editproperty = 0;
							}

							inputstr = spriteProperties[editproperty];
						}

						// select a textbox
						if ( mousestatus[SDL_BUTTON_LEFT] )
						{
							for ( int i = 0; i < numProperties; i++ )
							{
								if ( omousex >= pad_x1 - 4 && omousey >= suby1 + 40 + i * spacing && omousex < pad_x1 - 4 + pad_x2 && omousey < suby1 + 56 + i * spacing )
								{
									inputstr = spriteProperties[i];
									editproperty = i;
									cursorflash = ticks;
								}
								pad_x1 = subx1 + 8;
							}
						}
						if ( editproperty < numProperties )   // edit
						{
							if ( !SDL_IsTextInputActive() )
							{
								SDL_StartTextInput();
								inputstr = spriteProperties[0];
							}
							if ( editproperty == 2 )
							{
								inputlen = 3;
							}
							else
							{
								inputlen = 1;
							}
							if ( (ticks - cursorflash) % TICKS_PER_SECOND < TICKS_PER_SECOND / 2 )
							{
								printText(font8x8_bmp, subx1 + 8 + strlen(spriteProperties[editproperty]) * 8, suby1 + 44 + editproperty * spacing, "\26");
							}
						}
					}
				}
				else if ( newwindow == 4 || newwindow == 5 )
				{
					if ( selectedEntity[0] != NULL )
					{
						int numProperties;

						if ( newwindow == 4 )
						{
							numProperties = sizeof(itemPropertyNames) / sizeof(itemPropertyNames[0]); //find number of entries in property list
						}
						else if ( newwindow == 5 )
						{
							numProperties = sizeof(monsterItemPropertyNames) / sizeof(monsterItemPropertyNames[0]); //find number of entries in property list
						}
						const int lenProperties = 32;

						int spacing = 36; // 36 px between each item in the list.
						int verticalOffset = 20;
						int pad_x1 = subx1 + 8 + 96 + 80; // 104 px spacing from subwindow start. handles right side item list
						int pad_y1 = suby1 + 20 + verticalOffset; // 20 px spacing from subwindow start. handles right side item list
						int pad_x2 = 64; // handles right side item list

						int pad_y2 = (suby2 - 52 - 36) + verticalOffset - 4; //handles right side item list
						int pad_x3 = subx1 + 8; //handles left side menu
						int pad_y3 = suby1 + 28; // 28 px spacing from subwindow start, handles left side menu
						int pad_x4 = 64; //handles left side menu-end
						int pad_y4; //handles left side menu-end
						int totalNumItems = (sizeof(itemNameStrings) / sizeof(itemNameStrings[0]));
						int editorNumItems = totalNumItems /* - 1*/;
						switch ( itemSlotSelected )
						{
							case -1:
								break;
							default:
								if ( itemSlotSelected < 10 )
								{
									editorNumItems = 0;
									for ( int i = 0; i < (sizeof(itemStringsByType[itemSlotSelected]) / sizeof(itemStringsByType[itemSlotSelected][0])); i++ )
									{
										if ( strcmp(itemStringsByType[itemSlotSelected][i], "") == 0) //look for the end of the array
										{
											i = totalNumItems;
										}
										editorNumItems++;
									}
								}
								break;
						}
						int propertyInt = 0;
						char tmpPropertyName[lenProperties] = "";
						Uint32 color = makeColorRGB(0, 255, 0);
						Uint32 colorRandom = makeColorRGB(0, 168, 255);
						for ( int i = 0; i < numProperties; i++ )
						{
							if ( newwindow == 4 )
							{
								strcpy(tmpPropertyName, itemPropertyNames[i]);

							}
							else if ( newwindow == 5 )
							{
								strcpy(tmpPropertyName, monsterItemPropertyNames[i]);
							}
							pad_y3 = suby1 + 40 + spacing + i * spacing;
							pad_y4 = suby1 + 44 + 12 + spacing + i * spacing;
							// box outlines then text
							drawDepressed(pad_x3 - 4, pad_y3, pad_x3 - 4 + pad_x2, pad_y4);
							// print values on top of boxes
							printText(font8x8_bmp, pad_x3, pad_y3 - 12, tmpPropertyName);
							printText(font8x8_bmp, pad_x3, pad_y3 + 4, spriteProperties[i]);

							propertyInt = atoi(spriteProperties[i]);

							if ( errorArr[i] != 1 )
							{
								if ( i == 0 )
								{
									if ( newwindow == 4 )
									{
										if ( propertyInt > totalNumItems - 2 || propertyInt < 0 )
										{
											errorMessage = 60;
											errorArr[i] = 1;
											if ( propertyInt < 1 )
											{
												snprintf(spriteProperties[i], sizeof(spriteProperties[i]), "%d", 1);
											}
											else
											{
												snprintf(spriteProperties[i], sizeof(spriteProperties[i]), "%d", editorNumItems - 2);
											}
										}
										else if ( propertyInt == 0 )
										{
											errorMessage = 60;
											errorArr[i] = 1;
										}
									}
									else if ( newwindow == 5 )
									{
										if ( propertyInt > totalNumItems - 2 || propertyInt < 0 )
										{
											errorMessage = 60;
											errorArr[i] = 1;
											if ( propertyInt < 0 )
											{
												snprintf(spriteProperties[i], sizeof(spriteProperties[i]), "%d", 0);
											}
											else
											{
												snprintf(spriteProperties[i], sizeof(spriteProperties[i]), "%d", editorNumItems - 2);
											}
										}
									}
								}
								else if ( i == 1 )
								{
									if ( propertyInt == 2 )
									{
										color = makeColorRGB(200, 128, 0);
										printTextFormattedColor(font8x8_bmp, pad_x3 + pad_x4 + 8, pad_y3 + 4, color, "Decrepit");
									}
									else if ( propertyInt == 3 )
									{
										color = makeColorRGB(255, 255, 0);
										printTextFormattedColor(font8x8_bmp, pad_x3 + pad_x4 + 8, pad_y3 + 4, color, "Worn");
									}
									else if ( propertyInt == 4 )
									{
										color = makeColorRGB(128, 200, 0);
										printTextFormattedColor(font8x8_bmp, pad_x3 + pad_x4 + 8, pad_y3 + 4, color, "Servicable");
									}
									else if ( propertyInt == 5 )
									{
										color = makeColorRGB(0, 255, 0);
										printTextFormattedColor(font8x8_bmp, pad_x3 + pad_x4 + 8, pad_y3 + 4, color, "Excellent");
									}
									else if ( propertyInt == 1 )
									{
										color = makeColorRGB(255, 0, 0);
										printTextFormattedColor(font8x8_bmp, pad_x3 + pad_x4 + 8, pad_y3 + 4, color, "Broken");
									}
									else if ( propertyInt == 0 )
									{
										printTextFormattedColor(font8x8_bmp, pad_x3 + pad_x4 + 8, pad_y3 + 4, colorRandom, "Random");
									}
									else if ( propertyInt < 0 || propertyInt > 5 )
									{
										errorMessage = 60;
										errorArr[i] = 1;
										snprintf(spriteProperties[i], sizeof(spriteProperties[i]), "%d", 0); //reset
									}
								}
								else if ( i == 2 )
								{
									if ( strcmp(spriteProperties[i], "00") == 0 )
									{
										printTextFormattedColor(font8x8_bmp, pad_x3 + pad_x4 + 8, pad_y3 + 4, colorRandom, "Random");
									}
									else if ( propertyInt > 9 || propertyInt < -9 )
									{
										errorMessage = 60;
										errorArr[i] = 1;
										snprintf(spriteProperties[i], sizeof(spriteProperties[i]), "%d", 0); //reset
									}
								}
								else if ( i == 3 )
								{
									if ( propertyInt > 99 || propertyInt < 0 )
									{
										errorMessage = 60;
										errorArr[i] = 1;
										snprintf(spriteProperties[i], sizeof(spriteProperties[i]), "%d", 1); //reset
									}
									else if ( propertyInt == 0 )
									{
										color = makeColorRGB(255, 0, 0);
										printTextFormattedColor(font8x8_bmp, pad_x3 + pad_x4 + 8, pad_y3 + 4, color, "Invalid ID!");
									}
								}
								else if ( i == 4 )
								{
									if ( propertyInt == 1 )
									{
										color = makeColorRGB(0, 255, 0);
										printTextFormattedColor(font8x8_bmp, pad_x3 + pad_x4 + 8, pad_y3 + 4, color, "Identified");
									}
									else if ( propertyInt == 0 )
									{
										color = makeColorRGB(255, 255, 0);
										printTextFormattedColor(font8x8_bmp, pad_x3 + pad_x4 + 8, pad_y3 + 4, color, "Unidentified");
									}
									else if ( propertyInt == 2 )
									{
										printTextFormattedColor(font8x8_bmp, pad_x3 + pad_x4 + 8, pad_y3 + 4, colorRandom, "Random");
									}
									else
									{
										errorMessage = 60;
										errorArr[i] = 1;
										snprintf(spriteProperties[i], sizeof(spriteProperties[i]), "%d", 0);
									}
								}
								else if ( i == 5 && newwindow == 5)
								{
									if ( propertyInt > 100 || propertyInt < 0 )
									{
										errorMessage = 60;
										errorArr[i] = 1;
										snprintf(spriteProperties[i], sizeof(spriteProperties[i]), "%d", 100); //reset
									}
									else if ( propertyInt == 0 )
									{
										color = makeColorRGB(255, 0, 0);
										printTextFormattedColor(font8x8_bmp, pad_x3 + pad_x4 + 8, pad_y3 + 4, color, "Invalid ID!");
									}
									else
									{
										color = makeColorRGB(0, 255, 0);
										char tmpStr[32] = "";
										strcpy(tmpStr, spriteProperties[i]); //reset
										strcat(tmpStr, " %%");
										printTextFormattedColor(font8x8_bmp, pad_x3 + pad_x4 + 8, pad_y3 + 4, color, tmpStr);
									}
								}
								else if ( (i == 5 && newwindow == 4) || (i == 6 && newwindow == 5) )
								{
									if ( propertyInt > 16 || propertyInt < 0 )
									{
										errorMessage = 60;
										errorArr[i] = 1;
										snprintf(spriteProperties[i], sizeof(spriteProperties[i]), "%d", 0); //reset
									}
									else if ( propertyInt >= 0 && propertyInt <= (static_cast<int>(sizeof(itemCategoryNames[propertyInt])) / static_cast<int>(sizeof(itemCategoryNames[propertyInt][0]))) )
									{
										if ( propertyInt == 0 )
										{
											color = colorRandom;
										}
										else
										{
											color = makeColorRGB(200, 64, 220);
										}

										printTextFormattedColor(font8x8_bmp, pad_x3 + pad_x4 + 8, pad_y3 + 4, color, itemCategoryNames[propertyInt]);
									}
								}
							}

							if ( errorMessage )
							{
								color = makeColorRGB(255, 0, 0);
								if ( errorArr[i] == 1 )
								{
									printTextFormattedColor(font8x8_bmp, pad_x3 + pad_x4 + 8, pad_y3 + 4, color, "Invalid ID!");
								}
							}

						}



						drawDepressed(pad_x1, pad_y1, subx2 - 20, pad_y2);
						drawDepressed(subx2 - 20, pad_y1, subx2 - 4, pad_y2);
						slidersize = std::min<int>(((pad_y2 - 1) - (pad_y1 + 1)), ((pad_y2 - 1) - (pad_y1 + 1)) / ((real_t)(editorNumItems + 1) / 20)); //TODO: Why are int and real_t being compared?
						slidery = std::min(std::max(pad_y1, slidery), pad_y2 - 1 - slidersize);
						drawWindowFancy(subx2 - 19, slidery, subx2 - 5, slidery + slidersize);

						// directory list offset from slider
						y2 = ((real_t)(slidery - (pad_y1)) / (pad_y2 - (pad_y1))) * editorNumItems;
						if ( scroll )
						{
							slidery -= 8 * scroll;
							slidery = std::min(std::max(pad_y1, slidery), pad_y2 - 1 - slidersize);
							y2 = ((real_t)(slidery - (pad_y1)) / ((pad_y2) - (pad_y1))) * editorNumItems;
							itemSelect = std::min<long unsigned int>(std::max(y2, itemSelect), std::min<long unsigned int>(editorNumItems - 1, y2 + 19)); //TODO: Why are long unsigned int and int being compared? TWICE. On the same line.
							scroll = 0;
						}
						if ( mousestatus[SDL_BUTTON_LEFT] && omousex >= subx2 - 20 && omousex < subx2 - 4 && omousey >= (pad_y1) && omousey < pad_y2 )
						{
							slidery = oslidery + mousey - omousey;
							slidery = std::min(std::max(pad_y1, slidery), pad_y2 - 1 - slidersize);
							y2 = ((real_t)(slidery - (pad_y1)) / ((pad_y2) - (pad_y1))) * editorNumItems;
							mclick = 1;
							itemSelect = std::min<long unsigned int>(std::max(y2, itemSelect), std::min<long unsigned int>(editorNumItems - 1, y2 + 19)); //TODO: Why are long unsigned int and int being compared? TWICE. On the same line.
						}
						else
						{
							oslidery = slidery;
						}

						pos.x = pad_x1 ;
						pos.y = pad_y1 + 4 + (itemSelect - y2) * 8;
						pos.w = subx2 - pad_x1 - 24;
						pos.h = 8;
						drawRect(&pos, makeColorRGB(64, 64, 64), 255);

						// print all the items
						x = pad_x1;
						y = pad_y1 + 4;
						if ( newwindow == 4 )
						{
							c = std::min<long unsigned int>(editorNumItems, 20 + y2); //TODO: Why are long unsigned int and int being compared?
						}
						else
						{
							c = std::min<long unsigned int>(editorNumItems, 24 + y2); //TODO: Why are long unsigned int and int being compared?
						}
						for ( z = y2; z < c; z++ )
						{
							if ( newwindow == 5 && z == 1 )
							{
								printText(font8x8_bmp, x, y, "default_random");
							}
							else
							{
								switch ( itemSlotSelected )
								{
									case -1:
										printText(font8x8_bmp, x, y, itemNameStrings[z]);
										break;
									default:
										if ( itemSlotSelected < 10 )
										{
											printText(font8x8_bmp, x, y, itemStringsByType[itemSlotSelected][z]);
										}
										else
										{
											printText(font8x8_bmp, x, y, itemNameStrings[z]);
										}
										break;
								}
									
							}
							y += 8;
						}

						// item selection box
						pad_y3 = suby1 + 40;
						pad_y4 = suby1 + 44 + 12;
						// box outlines then text
						drawDepressed(pad_x3 - 4, pad_y3, pad_x3 - 4 + pad_x2 + 112, pad_y4);
						// print values on top of boxes
						printText(font8x8_bmp, pad_x3, pad_y3 - 12, "Item Name");
						printText(font8x8_bmp, pad_x1, pad_y3 - 12, "Click to select item");
						printText(font8x8_bmp, pad_x3, pad_y3 + 4, itemName);
						//drawDepressed(pad_x1, suby2 - 48, subx2 - 4, suby2 - 32);
						//printText(font8x8_bmp, pad_x1, suby2 - 44, itemName);

						// select a file
						if ( mousestatus[SDL_BUTTON_LEFT] )
						{
							if ( omousex >= pad_x1 && omousex < subx2 - 24 && omousey >= pad_y1 + 4 && omousey < pad_y2 - 4 )
							{
								itemSelect = y2 + ((omousey - (pad_y1 + 4)) >> 3);
								if ( newwindow == 4 )
								{
									itemSelect = std::min<long unsigned int>(std::max(y2, itemSelect), std::min<long unsigned int>(editorNumItems - 2, y2 + 19)); //TODO: Why are long unsigned int and int being compared? TWICE. On the same line.
								}
								else
								{
									itemSelect = std::min<long unsigned int>(std::max(y2, itemSelect), std::min<long unsigned int>(editorNumItems - 2, y2 + 23)); //TODO: Why are long unsigned int and int being compared? TWICE. On the same line.
								}
								switch ( itemSlotSelected )
								{
									case -1:
										strcpy(itemName, itemNameStrings[itemSelect]);
										break;
									default:
										if ( itemSlotSelected < 10 )
										{
											strcpy(itemName, itemStringsByType[itemSlotSelected][itemSelect]);
										}
										else
										{
											strcpy(itemName, itemNameStrings[itemSelect]);
										}
										break;
								}
								//inputstr = itemName;
								editorNumItems = (sizeof(itemNameStrings) / sizeof(itemNameStrings[0]));
								for ( z = 0; z < editorNumItems; z++ )
								{
									if ( strcmp(itemName, itemNameStrings[z]) == 0 )
									{
										char tmpStr[5];
										snprintf(tmpStr, sizeof(tmpStr), "%d", z);
										strcpy(spriteProperties[0], tmpStr);
										
										z = editorNumItems;
									}
								}
							}

							for ( int i = 0; i < numProperties; i++ )
							{
								if ( omousex >= pad_x3 - 4 && omousey >= suby1 + 40 + spacing + i * spacing && omousex < pad_x3 - 4 + pad_x4 && omousey < suby1 + 56 + spacing + i * spacing )
								{
									inputstr = spriteProperties[i];
									editproperty = i;
									cursorflash = ticks;
								}
							}
						}

						// Cycle properties with TAB.
						if ( keystatus[SDLK_TAB] )
						{
							keystatus[SDLK_TAB] = 0;
							cursorflash = ticks;
							editproperty++;
							if ( editproperty == numProperties )
							{
								editproperty = 0;
							}

							inputstr = spriteProperties[editproperty];
						}
						
						if ( editproperty < numProperties )   // edit
						{
							if ( !SDL_IsTextInputActive() )
							{
								SDL_StartTextInput();
								inputstr = spriteProperties[0];
							}

							if ( editproperty == 0 )
							{
								inputlen = 3;
								//update the item name when the ID changes.
								if ( newwindow == 5 && atoi(spriteProperties[0]) == 1 )
								{
									strcpy(itemName, "default_random");
								}
								else
								{
									strcpy(itemName, itemNameStrings[atoi(spriteProperties[0])]);
								}
							}
							else if( editproperty == 2 || editproperty == 3 )
							{
								inputlen = 2;
							}
							else if ( editproperty == 5 )
							{
								inputlen = 3;
							}
							else if ( editproperty == 6 && newwindow == 5 )
							{
								inputlen = 3;
							}
							else
							{
								inputlen = 1;
							}

							if ( (ticks - cursorflash) % TICKS_PER_SECOND < TICKS_PER_SECOND / 2 )
							{
								printText(font8x8_bmp, pad_x3 + strlen(spriteProperties[editproperty]) * 8, suby1 + 44 + spacing + editproperty * spacing, "\26");
							}
						}
						
					}
				}
				else if ( newwindow == 6 )
				{
					if ( selectedEntity[0] != NULL )
					{
						int numProperties = sizeof(summonTrapPropertyNames) / sizeof(summonTrapPropertyNames[0]); //find number of entries in property list
						const int lenProperties = sizeof(summonTrapPropertyNames[0]) / sizeof(char); //find length of entry in property list
						int spacing = 36; // 36 px between each item in the list.
						int pad_y1 = suby1 + 28; // 28 px spacing from subwindow start.
						int pad_x1 = subx1 + 8; // 8px spacing from subwindow start.
						int pad_x2 = 64;
						int pad_x3 = pad_x1 + pad_x2 + 8;
						int pad_y2 = 0;
						char tmpPropertyName[lenProperties] = "";
						Uint32 color = makeColorRGB(0, 255, 0);
						Uint32 color2 = makeColorRGB(255, 255, 0);
						Uint32 colorBad = makeColorRGB(255, 0, 0);
						Uint32 colorRandom = makeColorRGB(0, 168, 255);

						for ( int i = 0; i < numProperties; i++ )
						{
							int propertyInt = atoi(spriteProperties[i]);

							strcpy(tmpPropertyName, summonTrapPropertyNames[i]);
							pad_y1 = suby1 + 28 + i * spacing;
							pad_y2 = suby1 + 44 + i * spacing;
							// box outlines then text
							drawDepressed(pad_x1 - 4, suby1 + 40 + i * spacing, pad_x1 - 4 + pad_x2, suby1 + 56 + i * spacing);
							// print values on top of boxes
							printText(font8x8_bmp, pad_x1, suby1 + 44 + i * spacing, spriteProperties[i]);
							printText(font8x8_bmp, pad_x1, pad_y1, tmpPropertyName);

							if ( errorArr[i] != 1 )
							{
								if ( i == 0 ) //check input for valid entries, correct or notify the user if out of bounds.
								{
									if ( propertyInt > 32 || propertyInt < -1 )
									{
										errorMessage = 60;
										errorArr[i] = 1;
										snprintf(spriteProperties[i], sizeof(spriteProperties[i]), "%d", 0); //reset
									}
									else if ( propertyInt == 0 )
									{
										printTextFormattedColor(font8x8_bmp, pad_x3, pad_y2, colorRandom, "Random monster to match level curve");
									}
									else if ( propertyInt == -1 )
									{
										printTextFormattedColor(font8x8_bmp, pad_x3, pad_y2, colorRandom, "Completely random monster");
									}
									else if ( propertyInt == 6 || propertyInt == 12 )
									{
										printTextFormattedColor(font8x8_bmp, pad_x3, pad_y2, colorBad, "Error: Unused monster ID, will reset to 0");
									}
									else
									{
										color = makeColorRGB(0, 255, 0);
										char tmpStr[32] = "";
										strcpy(tmpStr, monsterEditorNameStrings[atoi(spriteProperties[i])]);
										printTextFormattedColor(font8x8_bmp, pad_x3, pad_y2, color, tmpStr);
									}
								}
								else if ( i == 1 )
								{
									if ( propertyInt > 9 || propertyInt < 0 )
									{
										errorMessage = 60;
										errorArr[i] = 1;
										snprintf(spriteProperties[i], sizeof(spriteProperties[i]), "%d", 1); //reset
									}
									else if ( propertyInt == 0 )
									{
										printTextFormattedColor(font8x8_bmp, pad_x3, pad_y2, colorBad, "Error: Must be > 0");
									}
								}
								else if ( i == 2 )
								{
									if ( propertyInt > 999 || propertyInt < 0 )
									{
										errorMessage = 60;
										errorArr[i] = 1;
										snprintf(spriteProperties[i], sizeof(spriteProperties[i]), "%d", 1); //reset
									}
									else if ( propertyInt == 0 )
									{
										printTextFormattedColor(font8x8_bmp, pad_x3, pad_y2, colorBad, "Error: Must be > 0");
									}
									else
									{
										printTextFormattedColor(font8x8_bmp, pad_x3, pad_y2, color, "%d seconds", atoi(spriteProperties[i]));
									}
								}
								else if ( i == 3 )
								{
									if ( propertyInt > 99 || propertyInt < 0 )
									{
										errorMessage = 60;
										errorArr[i] = 1;
										snprintf(spriteProperties[i], sizeof(spriteProperties[i]), "%d", 1); //reset
									}
									else if ( propertyInt == 0 )
									{
										printTextFormattedColor(font8x8_bmp, pad_x3, pad_y2, colorBad, "Error: Must be > 0");
									}
									else
									{
										printTextFormattedColor(font8x8_bmp, pad_x3, pad_y2, color, "%d instances", atoi(spriteProperties[i]));
									}
								}
								else if ( i == 4 )
								{
									if ( propertyInt > 1 || propertyInt < 0 )
									{
										errorMessage = 60;
										errorArr[i] = 1;
										snprintf(spriteProperties[i], sizeof(spriteProperties[i]), "%d", 0); //reset
									}
									else if ( propertyInt == 0 )
									{
										printTextFormattedColor(font8x8_bmp, pad_x3, pad_y2, color2, "No - power to enable");
									}
									else if ( propertyInt == 1 )
									{
										printTextFormattedColor(font8x8_bmp, pad_x3, pad_y2, color, "Yes - power to disable");
									}
								}
								else if ( i == 5 )
								{
									if ( propertyInt > 100 || propertyInt < 0 )
									{
										errorMessage = 60;
										errorArr[i] = 1;
										snprintf(spriteProperties[i], sizeof(spriteProperties[i]), "%d", 0); //reset
									}
									else
									{
										color = makeColorRGB(0, 255, 0);
										char tmpStr[32] = "";
										strcpy(tmpStr, spriteProperties[i]); //reset
										strcat(tmpStr, " %%");
										printTextFormatted(font8x8_bmp, pad_x3, pad_y2, tmpStr);
									}
								}
								else if ( i == 6 )
								{
									if ( propertyInt > 16 || propertyInt < 0 )
									{
										errorMessage = 60;
										errorArr[i] = 1;
										snprintf(spriteProperties[i], sizeof(spriteProperties[i]), "%d", 0); //reset
									}
									else if ( propertyInt == 0 )
									{
										printTextFormattedColor(font8x8_bmp, pad_x3, pad_y2, color2, "");
									}
									else if ( propertyInt > 0 )
									{
										printTextFormattedColor(font8x8_bmp, pad_x3, pad_y2, color, "Auto activate near player (x tiles)");
									}
								}
							}

							if ( errorMessage )
							{
								color = makeColorRGB(255, 0, 0);
								if ( errorArr[i] == 1 )
								{
									printTextFormattedColor(font8x8_bmp, pad_x3, pad_y2, color, "Invalid ID!");
								}
							}

							pad_x1 = subx1 + 8;
						}
						// Cycle properties with TAB.
						if ( keystatus[SDLK_TAB] )
						{
							keystatus[SDLK_TAB] = 0;
							cursorflash = ticks;
							editproperty++;
							if ( editproperty == numProperties )
							{
								editproperty = 0;
							}

							inputstr = spriteProperties[editproperty];
						}

						// select a textbox
						if ( mousestatus[SDL_BUTTON_LEFT] )
						{
							for ( int i = 0; i < numProperties; i++ )
							{
								if ( omousex >= pad_x1 - 4 && omousey >= suby1 + 40 + i * spacing && omousex < pad_x1 - 4 + pad_x2 && omousey < suby1 + 56 + i * spacing )
								{
									inputstr = spriteProperties[i];
									editproperty = i;
									cursorflash = ticks;
								}
								pad_x1 = subx1 + 8;
							}
						}
						if ( editproperty < numProperties )   // edit
						{
							if ( !SDL_IsTextInputActive() )
							{
								SDL_StartTextInput();
								inputstr = spriteProperties[0];
							}
							if ( editproperty == 0 || editproperty == 3) //length of text field allowed to enter
							{
								inputlen = 2;
							}
							else if ( editproperty == 2 || editproperty == 5 || editproperty == 6 )
							{
								inputlen = 3;
							}
							else
							{
								inputlen = 1;
							}
							if ( (ticks - cursorflash) % TICKS_PER_SECOND < TICKS_PER_SECOND / 2 )
							{
								printText(font8x8_bmp, subx1 + 8 + strlen(spriteProperties[editproperty]) * 8, suby1 + 44 + editproperty * spacing, "\26");
							}
						}
					}
				}
				else if ( newwindow == 7 )
				{
					if ( selectedEntity[0] != NULL )
					{
						int numProperties = sizeof(powerCrystalPropertyNames) / sizeof(powerCrystalPropertyNames[0]); //find number of entries in property list
						const int lenProperties = sizeof(powerCrystalPropertyNames[0]) / sizeof(char); //find length of entry in property list
						int spacing = 36; // 36 px between each item in the list.
						int pad_y1 = suby1 + 28; // 28 px spacing from subwindow start.
						int pad_x1 = subx1 + 8; // 8px spacing from subwindow start.
						int pad_x2 = 64;
						int pad_x3 = pad_x1 + pad_x2 + 8;
						int pad_y2 = 0;
						char tmpPropertyName[lenProperties] = "";
						Uint32 color = makeColorRGB(0, 255, 0);
						Uint32 colorRandom = makeColorRGB(0, 168, 255);

						for ( int i = 0; i < numProperties; i++ )
						{
							int propertyInt = atoi(spriteProperties[i]);

							strcpy(tmpPropertyName, powerCrystalPropertyNames[i]);
							pad_y1 = suby1 + 28 + i * spacing;
							pad_y2 = suby1 + 44 + i * spacing;
							// box outlines then text
							drawDepressed(pad_x1 - 4, suby1 + 40 + i * spacing, pad_x1 - 4 + pad_x2, suby1 + 56 + i * spacing);
							// print values on top of boxes
							printText(font8x8_bmp, pad_x1, suby1 + 44 + i * spacing, spriteProperties[i]);
							printText(font8x8_bmp, pad_x1, pad_y1, tmpPropertyName);

							if ( errorArr[i] != 1 )
							{
								if ( i == 0 )
								{
									if ( propertyInt > 3 || propertyInt < 0 )
									{
										errorMessage = 60;
										errorArr[i] = 1;
										snprintf(spriteProperties[i], sizeof(spriteProperties[i]), "%d", 0); //reset
									}
									else
									{
										color = makeColorRGB(0, 255, 0);
										char tmpStr[32] = "";
										if ( propertyInt == 0 )
										{
											strcpy(tmpStr, "EAST");
										}
										else if ( propertyInt == 1 )
										{
											strcpy(tmpStr, "SOUTH");
										}
										else if ( propertyInt == 2 )
										{
											strcpy(tmpStr, "WEST");
										}
										else if ( propertyInt == 3 )
										{
											strcpy(tmpStr, "NORTH");
										}
										printTextFormattedColor(font8x8_bmp, pad_x3, pad_y2, color, tmpStr);
									}
								}
								else if ( i == 1 )
								{
									if ( propertyInt > 99 || propertyInt < 0 )
									{
										errorMessage = 60;
										errorArr[i] = 1;
										snprintf(spriteProperties[i], sizeof(spriteProperties[i]), "%d", 1); //reset
									}
									else
									{
										color = makeColorRGB(0, 255, 0);
										// 32 is WAY TOO SMALL for this, wtf?
										// spriteProperties is a string table, each entry 128 bytes long
										// " Tiles to power in facing direction" = 35 bytes
										// do not forget the null terminator
										char tmpStr[128 + 35 + 1] = "";
										strcpy(tmpStr, spriteProperties[i]); //reset
										strcat(tmpStr, " Tiles to power in facing direction");
										printTextFormattedColor(font8x8_bmp, pad_x3, pad_y2, color, tmpStr);
									}
								}
								else if ( i == 2 )
								{
									if ( propertyInt > 1 || propertyInt < 0 )
									{
										errorMessage = 60;
										errorArr[i] = 1;
										snprintf(spriteProperties[i], sizeof(spriteProperties[i]), "%d", 0); //reset
									}
									else if ( propertyInt == 0 )
									{
										printTextFormattedColor(font8x8_bmp, pad_x3, pad_y2, colorRandom, "Clockwise");
									}
									else if ( propertyInt == 1 )
									{
										printTextFormattedColor(font8x8_bmp, pad_x3, pad_y2, color, "Counter-Clockwise");
									}
								}
								else if ( i == 3 )
								{
									if ( propertyInt > 1 || propertyInt < 0 )
									{
										errorMessage = 60;
										errorArr[i] = 1;
										snprintf(spriteProperties[i], sizeof(spriteProperties[i]), "%d", 0); //reset
									}
									else if ( propertyInt == 0 )
									{
										printTextFormattedColor(font8x8_bmp, pad_x3, pad_y2, colorRandom, "Always on");
									}
									else if ( propertyInt == 1 )
									{
										printTextFormattedColor(font8x8_bmp, pad_x3, pad_y2, color, "Requires spell to activate");
									}
								}
							}

							if ( errorMessage )
							{
								color = makeColorRGB(255, 0, 0);
								if ( errorArr[i] == 1 )
								{
									printTextFormattedColor(font8x8_bmp, pad_x3, pad_y2, color, "Invalid ID!");
								}
							}

							pad_x1 = subx1 + 8;
						}

						// print out directions
						pad_x1 += 54;
						spacing = 18;
						pad_y1 = suby1 + 28 + 8 * spacing;
						printText(font8x8_bmp, pad_x1 + 32, pad_y1, "NORTH(3)");
						pad_y1 = suby1 + 28 + 9 * spacing;
						printText(font8x8_bmp, pad_x1, pad_y1, "WEST(2)");
						printText(font8x8_bmp, pad_x1 + 96 - 16, pad_y1, "EAST(0)");
						pad_y1 = suby1 + 28 + 10 * spacing;
						printText(font8x8_bmp, pad_x1 + 32, pad_y1, "SOUTH(1)");
						spacing = 36;

						// Cycle properties with TAB.
						if ( keystatus[SDLK_TAB] )
						{
							keystatus[SDLK_TAB] = 0;
							cursorflash = ticks;
							editproperty++;
							if ( editproperty == numProperties )
							{
								editproperty = 0;
							}

							inputstr = spriteProperties[editproperty];
						}

						// select a textbox
						if ( mousestatus[SDL_BUTTON_LEFT] )
						{
							for ( int i = 0; i < numProperties; i++ )
							{
								pad_x1 = subx1 + 8;
								if ( omousex >= pad_x1 - 4 && omousey >= suby1 + 40 + i * spacing && omousex < pad_x1 - 4 + pad_x2 && omousey < suby1 + 56 + i * spacing )
								{
									inputstr = spriteProperties[i];
									editproperty = i;
									cursorflash = ticks;
								}
							}
						}
						if ( editproperty < numProperties )   // edit
						{
							if ( !SDL_IsTextInputActive() )
							{
								SDL_StartTextInput();
								inputstr = spriteProperties[0];
							}
							if ( editproperty == 1 )
							{
								inputlen = 2;
							}
							else
							{
								inputlen = 1;
							}
							if ( (ticks - cursorflash) % TICKS_PER_SECOND < TICKS_PER_SECOND / 2 )
							{
								printText(font8x8_bmp, subx1 + 8 + strlen(spriteProperties[editproperty]) * 8, suby1 + 44 + editproperty * spacing, "\26");
							}
						}
					}
				}
				else if ( newwindow == 8 )
				{
					if ( selectedEntity[0] != NULL )
					{
						int numProperties = sizeof(leverTimerPropertyNames) / sizeof(leverTimerPropertyNames[0]); //find number of entries in property list
						const int lenProperties = sizeof(leverTimerPropertyNames[0]) / sizeof(char); //find length of entry in property list
						int spacing = 36; // 36 px between each item in the list.
						int inputFieldHeader_y = suby1 + 28; // 28 px spacing from subwindow start.
						int inputField_x = subx1 + 8; // 8px spacing from subwindow start.
						int inputField_y = inputFieldHeader_y + 16;
						int inputFieldWidth = 64; // width of the text field
						int inputFieldFeedback_x = inputField_x + inputFieldWidth + 8;
						char tmpPropertyName[lenProperties] = "";
						Uint32 color = makeColorRGB(0, 255, 0);
						Uint32 colorRandom = makeColorRGB(0, 168, 255);
						Uint32 colorError = makeColorRGB(255, 0, 0);

						for ( int i = 0; i < numProperties; i++ )
						{
							int propertyInt = atoi(spriteProperties[i]);

							strcpy(tmpPropertyName, leverTimerPropertyNames[i]);
							inputFieldHeader_y = suby1 + 28 + i * spacing;
							inputField_y = inputFieldHeader_y + 16;
							// box outlines then text
							drawDepressed(inputField_x - 4, inputField_y - 4, inputField_x - 4 + inputFieldWidth, inputField_y + 16 - 4);
							// print values on top of boxes
							printText(font8x8_bmp, inputField_x, suby1 + 44 + i * spacing, spriteProperties[i]);
							printText(font8x8_bmp, inputField_x, inputFieldHeader_y, tmpPropertyName);

							if ( errorArr[i] != 1 )
							{
								if ( i == 0 )
								{
									if ( propertyInt > 999 || propertyInt < 0 )
									{
										propertyPageError(i, 5); // reset to default 5 seconds.
									}
									else
									{
										char tmpStr[32] = "";
										if ( propertyInt == 0 )
										{
											strcpy(tmpStr, "Value must be > 0!");
											printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, colorError, tmpStr);
										}
										else
										{
											if ( propertyInt == 1 )
											{
												strcpy(tmpStr, "second");
											}
											else
											{
												strcpy(tmpStr, "seconds");
											}
											printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, tmpStr);
										}
									}
								}
								else
								{
									// enter other row entries here
								}
							}

							if ( errorMessage )
							{
								if ( errorArr[i] == 1 )
								{
									printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, colorError, "Invalid ID!");
								}
							}
						}

						propertyPageTextAndInput(numProperties, inputFieldWidth);

						if ( editproperty < numProperties )   // edit
						{
							if ( !SDL_IsTextInputActive() )
							{
								SDL_StartTextInput();
								inputstr = spriteProperties[0];
							}

							// set the maximum length allowed for user input
							if ( editproperty == 0 )
							{
								inputlen = 4;
							}
							else
							{
								inputlen = 4;
							}
							propertyPageCursorFlash(spacing);
						}
					}
				}
				else if ( newwindow == 9 )
				{
					if ( selectedEntity[0] != NULL )
					{
						int numProperties = sizeof(boulderTrapPropertyNames) / sizeof(boulderTrapPropertyNames[0]); //find number of entries in property list
						const int lenProperties = sizeof(boulderTrapPropertyNames[0]) / sizeof(char); //find length of entry in property list
						int spacing = 36; // 36 px between each item in the list.
						int inputFieldHeader_y = suby1 + 28; // 28 px spacing from subwindow start.
						int inputField_x = subx1 + 8; // 8px spacing from subwindow start.
						int inputField_y = inputFieldHeader_y + 16;
						int inputFieldWidth = 64; // width of the text field
						int inputFieldFeedback_x = inputField_x + inputFieldWidth + 8;
						char tmpPropertyName[lenProperties] = "";
						Uint32 color = makeColorRGB(0, 255, 0);
						Uint32 colorRandom = makeColorRGB(0, 168, 255);
						Uint32 colorError = makeColorRGB(255, 0, 0);

						for ( int i = 0; i < numProperties; i++ )
						{
							int propertyInt = atoi(spriteProperties[i]);

							strcpy(tmpPropertyName, boulderTrapPropertyNames[i]);
							inputFieldHeader_y = suby1 + 28 + i * spacing;
							inputField_y = inputFieldHeader_y + 16;
							// box outlines then text
							drawDepressed(inputField_x - 4, inputField_y - 4, inputField_x - 4 + inputFieldWidth, inputField_y + 16 - 4);
							// print values on top of boxes
							printText(font8x8_bmp, inputField_x, suby1 + 44 + i * spacing, spriteProperties[i]);
							printText(font8x8_bmp, inputField_x, inputFieldHeader_y, tmpPropertyName);

							if ( errorArr[i] != 1 )
							{
								if ( i == 0 )
								{
									if ( propertyInt > 99 || propertyInt < -1 )
									{
										propertyPageError(i, 0); // reset to default 0 re-fire.
									}
									else
									{
										char tmpStr[32] = "";
										if ( propertyInt == 1 )
										{
											strcpy(tmpStr, "time");
										}
										else if ( propertyInt == -1 )
										{
											strcpy(tmpStr, "infinite reload");
										}
										else
										{
											strcpy(tmpStr, "times");
										}
										printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, tmpStr);
									}
								}
								else if ( i == 1 )
								{
									if ( propertyInt > 999 || propertyInt < 0 )
									{
										propertyPageError(i, 1); // reset to default 1 seconds.
									}
									else
									{
										char tmpStr[32] = "";
										if ( propertyInt < 2 )
										{
											strcpy(tmpStr, "Value must be > 1!");
											printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, colorError, tmpStr);
										}
										else
										{
											if ( propertyInt == 1 )
											{
												strcpy(tmpStr, "second");
											}
											else
											{
												strcpy(tmpStr, "seconds");
											}
											printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, tmpStr);
										}
									}
								}
								else if ( i == 2 )
								{
									if ( propertyInt > 999 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 1 seconds.
									}
									else
									{
										char tmpStr[32] = "";
										if ( propertyInt == 1 )
										{
											strcpy(tmpStr, "second");
										}
										else
										{
											strcpy(tmpStr, "seconds");
										}
										printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, tmpStr);
									}
								}
								else
								{
									// enter other row entries here
								}
							}

							if ( errorMessage )
							{
								if ( errorArr[i] == 1 )
								{
									printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, colorError, "Invalid ID!");
								}
							}
						}

						propertyPageTextAndInput(numProperties, inputFieldWidth);

						if ( editproperty < numProperties )   // edit
						{
							if ( !SDL_IsTextInputActive() )
							{
								SDL_StartTextInput();
								inputstr = spriteProperties[0];
							}

							// set the maximum length allowed for user input
							if ( editproperty == 0 )
							{
								inputlen = 4;
							}
							else
							{
								inputlen = 4;
							}
							propertyPageCursorFlash(spacing);
						}
					}
				}
				else if ( newwindow == 10 )
				{
					if ( selectedEntity[0] != NULL )
					{
						int numProperties = sizeof(pedestalPropertyNames) / sizeof(pedestalPropertyNames[0]); //find number of entries in property list
						const int lenProperties = sizeof(pedestalPropertyNames[0]) / sizeof(char); //find length of entry in property list
						int spacing = 36; // 36 px between each item in the list.
						int inputFieldHeader_y = suby1 + 28; // 28 px spacing from subwindow start.
						int inputField_x = subx1 + 8; // 8px spacing from subwindow start.
						int inputField_y = inputFieldHeader_y + 16;
						int inputFieldWidth = 64; // width of the text field
						int inputFieldFeedback_x = inputField_x + inputFieldWidth + 8;
						char tmpPropertyName[lenProperties] = "";
						Uint32 color = makeColorRGB(0, 255, 0);
						Uint32 colorRandom = makeColorRGB(0, 168, 255);
						Uint32 colorError = makeColorRGB(255, 0, 0);

						for ( int i = 0; i < numProperties; i++ )
						{
							int propertyInt = atoi(spriteProperties[i]);

							strcpy(tmpPropertyName, pedestalPropertyNames[i]);
							inputFieldHeader_y = suby1 + 28 + i * spacing;
							inputField_y = inputFieldHeader_y + 16;
							// box outlines then text
							drawDepressed(inputField_x - 4, inputField_y - 4, inputField_x - 4 + inputFieldWidth, inputField_y + 16 - 4);
							// print values on top of boxes
							printText(font8x8_bmp, inputField_x, suby1 + 44 + i * spacing, spriteProperties[i]);
							printText(font8x8_bmp, inputField_x, inputFieldHeader_y, tmpPropertyName);

							if ( errorArr[i] != 1 )
							{
								if ( i == 0 )
								{
									if ( propertyInt > 3 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0 blue.
									}
									else
									{
										char tmpStr[32] = "";
										if ( propertyInt == 0 )
										{
											strcpy(tmpStr, "blue");
										}
										else if ( propertyInt == 1 )
										{
											strcpy(tmpStr, "red");
										}
										else if ( propertyInt == 2 )
										{
											strcpy(tmpStr, "purple");
										}
										else if ( propertyInt == 3 )
										{
											strcpy(tmpStr, "green");
										}
										printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, tmpStr);
									}
								}
								else if ( i == 1 )
								{
									if ( propertyInt > 1 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0 (no orb)
									}
									else
									{
										char tmpStr[32] = "";
										if ( propertyInt == 1 )
										{
											strcpy(tmpStr, "pre-load with orb");
										}
										else
										{
											strcpy(tmpStr, "no orb pre-loaded");
										}
										printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, tmpStr);
									}
								}
								else if ( i == 2 )
								{
									if ( propertyInt > 1 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0 non-inverted.
									}
									else
									{
										char tmpStr[32] = "";
										if ( propertyInt == 1 )
										{
											strcpy(tmpStr, "inverted (orb to de-power)");
										}
										else
										{
											strcpy(tmpStr, "non-inverted (orb to power)");
										}
										printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, tmpStr);
									}
								}
								else if ( i == 3 )
								{
									if ( propertyInt > 1 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0 normal height.
									}
									else
									{
										char tmpStr[32] = "";
										if ( propertyInt == 1 )
										{
											strcpy(tmpStr, "true");
										}
										else
										{
											strcpy(tmpStr, "false");
										}
										printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, tmpStr);
									}
								}
								else if ( i == 4 )
								{
									if ( propertyInt > 1 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0 no lock
									}
									else
									{
										char tmpStr[32] = "";
										if ( propertyInt == 0 )
										{
											strcpy(tmpStr, "able to retreive");
										}
										else
										{
											strcpy(tmpStr, "locked when placed");
										}
										printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, tmpStr);
									}
								}
								else
								{
									// enter other row entries here
								}
							}

							if ( errorMessage )
							{
								if ( errorArr[i] == 1 )
								{
									printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, colorError, "Invalid ID!");
								}
							}
						}

						propertyPageTextAndInput(numProperties, inputFieldWidth);

						if ( editproperty < numProperties )   // edit
						{
							if ( !SDL_IsTextInputActive() )
							{
								SDL_StartTextInput();
								inputstr = spriteProperties[0];
							}

							// set the maximum length allowed for user input
							inputlen = 2;
							propertyPageCursorFlash(spacing);
						}
					}
				}
				else if ( newwindow == 11 )
				{
					if ( selectedEntity[0] != NULL )
					{
						int numProperties = sizeof(teleporterPropertyNames) / sizeof(teleporterPropertyNames[0]); //find number of entries in property list
						const int lenProperties = sizeof(teleporterPropertyNames[0]) / sizeof(char); //find length of entry in property list
						int spacing = 36; // 36 px between each item in the list.
						int inputFieldHeader_y = suby1 + 28; // 28 px spacing from subwindow start.
						int inputField_x = subx1 + 8; // 8px spacing from subwindow start.
						int inputField_y = inputFieldHeader_y + 16;
						int inputFieldWidth = 64; // width of the text field
						int inputFieldFeedback_x = inputField_x + inputFieldWidth + 8;
						char tmpPropertyName[lenProperties] = "";
						Uint32 color = makeColorRGB(0, 255, 0);
						Uint32 colorRandom = makeColorRGB(0, 168, 255);
						Uint32 colorError = makeColorRGB(255, 0, 0);

						for ( int i = 0; i < numProperties; i++ )
						{
							int propertyInt = atoi(spriteProperties[i]);

							strcpy(tmpPropertyName, teleporterPropertyNames[i]);
							inputFieldHeader_y = suby1 + 28 + i * spacing;
							inputField_y = inputFieldHeader_y + 16;
							// box outlines then text
							drawDepressed(inputField_x - 4, inputField_y - 4, inputField_x - 4 + inputFieldWidth, inputField_y + 16 - 4);
							// print values on top of boxes
							printText(font8x8_bmp, inputField_x, suby1 + 44 + i * spacing, spriteProperties[i]);
							printText(font8x8_bmp, inputField_x, inputFieldHeader_y, tmpPropertyName);

							if ( errorArr[i] != 1 )
							{
								if ( i == 0 )
								{
									if ( propertyInt > 999 || propertyInt < 0 )
									{
										propertyPageError(i, 1); // reset to default 1.
									}
								}
								else if ( i == 1 )
								{
									if ( propertyInt > 999 || propertyInt < 0 )
									{
										propertyPageError(i, 1); // reset to default 1.
									}
								}
								else if ( i == 2 )
								{
									if ( propertyInt > 2 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0 up.
									}
									else
									{
										char tmpStr[32] = "";
										if ( propertyInt == 2 )
										{
											strcpy(tmpStr, "portal");
										}
										else if ( propertyInt == 1 )
										{
											strcpy(tmpStr, "ladder down");
										}
										else
										{
											strcpy(tmpStr, "ladder up");
										}
										printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, tmpStr);
									}
								}
								else
								{
									// enter other row entries here
								}
							}

							if ( errorMessage )
							{
								if ( errorArr[i] == 1 )
								{
									printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, colorError, "Invalid ID!");
								}
							}
						}

						propertyPageTextAndInput(numProperties, inputFieldWidth);

						if ( editproperty < numProperties )   // edit
						{
							if ( !SDL_IsTextInputActive() )
							{
								SDL_StartTextInput();
								inputstr = spriteProperties[0];
							}

							// set the maximum length allowed for user input
							if ( editproperty == 2 )
							{
								inputlen = 2;
							}
							else
							{
								inputlen = 4;
							}
							propertyPageCursorFlash(spacing);
						}
					}
				}
				else if ( newwindow == 12 )
				{
					if ( selectedEntity[0] != nullptr )
					{
						int numProperties = sizeof(ceilingTilePropertyNames) / sizeof(ceilingTilePropertyNames[0]); //find number of entries in property list
						const int lenProperties = sizeof(ceilingTilePropertyNames[0]) / sizeof(char); //find length of entry in property list
						int spacing = 36; // 36 px between each item in the list.
						int inputFieldHeader_y = suby1 + 28; // 28 px spacing from subwindow start.
						int inputField_x = subx1 + 8; // 8px spacing from subwindow start.
						int inputField_y = inputFieldHeader_y + 16;
						int inputFieldWidth = 64; // width of the text field
						int inputFieldFeedback_x = inputField_x + inputFieldWidth + 8;
						char tmpPropertyName[lenProperties] = "";
						Uint32 color = makeColorRGB(0, 255, 0);
						Uint32 colorRandom = makeColorRGB(0, 168, 255);
						Uint32 colorError = makeColorRGB(255, 0, 0);

						for ( int i = 0; i < numProperties; i++ )
						{
							int propertyInt = atoi(spriteProperties[i]);

							strcpy(tmpPropertyName, ceilingTilePropertyNames[i]);
							inputFieldHeader_y = suby1 + 28 + i * spacing;
							inputField_y = inputFieldHeader_y + 16;
							// box outlines then text
							drawDepressed(inputField_x - 4, inputField_y - 4, inputField_x - 4 + inputFieldWidth, inputField_y + 16 - 4);
							// print values on top of boxes
							printText(font8x8_bmp, inputField_x, suby1 + 44 + i * spacing, spriteProperties[i]);
							printText(font8x8_bmp, inputField_x, inputFieldHeader_y, tmpPropertyName);

							if ( errorArr[i] != 1 )
							{
								if ( i == 0 )
								{
									if ( propertyInt > 9999 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
									else
									{
										if ( modelFileNames.find(propertyInt) != modelFileNames.end() )
										{
											std::string tmpStr = modelFileNames[propertyInt];
											printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, tmpStr.c_str());
										}
										else
										{
											printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, makeColorRGB(255, 0, 0), "Unknown Model!");
										}
									}
								}
								else if ( i == 1 )
								{
									if ( propertyInt > 3 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
									else
									{
										char tmpStr[32] = "";
										switch ( propertyInt )
										{
											case 0:
												strcpy(tmpStr, "East");
												break;
											case 1:
												strcpy(tmpStr, "South");
												break;
											case 2:
												strcpy(tmpStr, "West");
												break;
											case 3:
												strcpy(tmpStr, "North");
												break;
											default:
												break;
										}
										printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, tmpStr);
									}
								}
								else if ( i == 2 )
								{
									if ( propertyInt > 2 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
									else
									{
										char tmpStr[32] = "";
										switch ( propertyInt )
										{
											case 0:
												strcpy(tmpStr, "No trap spawn");
												printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, tmpStr);
												break;
											case 1:
												strcpy(tmpStr, "Allow spawn and destroy tile");
												printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, colorError, tmpStr);
												break;
											case 2:
												strcpy(tmpStr, "Allow spawn and keep tile");
												printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, colorRandom, tmpStr);
												break;
											default:
												break;
										}
									}
								}
								else if ( i == 3 )
								{
									if ( propertyInt > 1 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
									else
									{
										char tmpStr[32] = "";
										if ( propertyInt == 0 )
										{
											strcpy(tmpStr, "Indestructible");
											printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, tmpStr);
										}
										else if ( propertyInt == 1 )
										{
											strcpy(tmpStr, "Minotaur breaks");
											printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, colorError, tmpStr);
										}
									}
								}
								else
								{
									// enter other row entries here
								}
							}

							if ( errorMessage )
							{
								if ( errorArr[i] == 1 )
								{
									printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, colorError, "Invalid ID!");
								}
							}
						}

						propertyPageTextAndInput(numProperties, inputFieldWidth);

						if ( editproperty < numProperties )   // edit
						{
							if ( !SDL_IsTextInputActive() )
							{
								SDL_StartTextInput();
								inputstr = spriteProperties[0];
							}

							// set the maximum length allowed for user input
							if ( editproperty == 0 )
							{
								inputlen = 4;
							}
							else
							{
								inputlen = 3;
							}
							propertyPageCursorFlash(spacing);
						}
					}
				}
				else if ( newwindow == 13 )
				{
					if ( selectedEntity[0] != nullptr )
					{
						int numProperties = sizeof(spellTrapPropertyNames) / sizeof(spellTrapPropertyNames[0]); //find number of entries in property list
						const int lenProperties = sizeof(spellTrapPropertyNames[0]) / sizeof(char); //find length of entry in property list
						int spacing = 36; // 36 px between each item in the list.
						int inputFieldHeader_y = suby1 + 28; // 28 px spacing from subwindow start.
						int inputField_x = subx1 + 8; // 8px spacing from subwindow start.
						int inputField_y = inputFieldHeader_y + 16;
						int inputFieldWidth = 64; // width of the text field
						int inputFieldFeedback_x = inputField_x + inputFieldWidth + 8;
						char tmpPropertyName[lenProperties] = "";
						Uint32 color = makeColorRGB(0, 255, 0);
						Uint32 colorRandom = makeColorRGB(0, 168, 255);
						Uint32 colorError = makeColorRGB(255, 0, 0);

						for ( int i = 0; i < numProperties; i++ )
						{
							int propertyInt = atoi(spriteProperties[i]);

							strcpy(tmpPropertyName, spellTrapPropertyNames[i]);
							inputFieldHeader_y = suby1 + 28 + i * spacing;
							inputField_y = inputFieldHeader_y + 16;
							// box outlines then text
							drawDepressed(inputField_x - 4, inputField_y - 4, inputField_x - 4 + inputFieldWidth, inputField_y + 16 - 4);
							// print values on top of boxes
							printText(font8x8_bmp, inputField_x, suby1 + 44 + i * spacing, spriteProperties[i]);
							printText(font8x8_bmp, inputField_x, inputFieldHeader_y, tmpPropertyName);

							if ( errorArr[i] != 1 )
							{
								if ( i == 0 )
								{
									if ( propertyInt > 99 || propertyInt < -1 )
									{
										propertyPageError(i, -1); // reset to default -1.
									}
								}
								else if ( i == 1 )
								{
									if ( propertyInt > 99 || propertyInt < -1 )
									{
										propertyPageError(i, -1); // reset to default -1.
									}
								}
								else if ( i == 2 )
								{
									if ( propertyInt > 1 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0 continuous.
									}
									else
									{
										char tmpStr[32] = "";
										if ( propertyInt == 0 )
										{
											strcpy(tmpStr, "must re-trigger power to fire");
										}
										else if ( propertyInt == 1 )
										{
											strcpy(tmpStr, "continous fire first power up");
										}
										printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, tmpStr);
									}
								}
								else if ( i == 3 )
								{
									if ( propertyInt > 9999 || propertyInt < 0 )
									{
										propertyPageError(i, 1); // reset to default 1.
									}
								}
								else if ( i == 4 )
								{
									if ( propertyInt > 999 || propertyInt < 0 )
									{
										propertyPageError(i, 1); // reset to default 1.
									}
									else
									{
										char tmpStr[32] = "";
										if ( propertyInt == 1 )
										{
											strcpy(tmpStr, "second");
										}
										else if ( propertyInt > 1 )
										{
											strcpy(tmpStr, "seconds");
										}
										printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, tmpStr);
									}
								}
								else
								{
									// enter other row entries here
								}
							}

							if ( errorMessage )
							{
								if ( errorArr[i] == 1 )
								{
									printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, colorError, "Invalid ID!");
								}
							}
						}

						propertyPageTextAndInput(numProperties, inputFieldWidth);

						if ( editproperty < numProperties )   // edit
						{
							if ( !SDL_IsTextInputActive() )
							{
								SDL_StartTextInput();
								inputstr = spriteProperties[0];
							}

							// set the maximum length allowed for user input
							if ( editproperty == 0 || editproperty == 1 )
							{
								inputlen = 3;
							}
							else if ( editproperty == 2 )
							{
								inputlen = 2;
							}
							else
							{
								inputlen = 4;
							}
							propertyPageCursorFlash(spacing);
						}
					}
				}
				else if ( newwindow == 14 )
				{
					if ( selectedEntity[0] != nullptr )
					{
						int numProperties = sizeof(furniturePropertyNames) / sizeof(furniturePropertyNames[0]); //find number of entries in property list
						const int lenProperties = sizeof(furniturePropertyNames[0]) / sizeof(char); //find length of entry in property list
						int spacing = 36; // 36 px between each item in the list.
						int inputFieldHeader_y = suby1 + 28; // 28 px spacing from subwindow start.
						int inputField_x = subx1 + 8; // 8px spacing from subwindow start.
						int inputField_y = inputFieldHeader_y + 16;
						int inputFieldWidth = 64; // width of the text field
						int inputFieldFeedback_x = inputField_x + inputFieldWidth + 8;
						char tmpPropertyName[lenProperties] = "";
						Uint32 color = makeColorRGB(0, 255, 0);
						Uint32 colorRandom = makeColorRGB(0, 168, 255);
						Uint32 colorError = makeColorRGB(255, 0, 0);

						for ( int i = 0; i < numProperties; i++ )
						{
							int propertyInt = atoi(spriteProperties[i]);

							strcpy(tmpPropertyName, furniturePropertyNames[i]);
							inputFieldHeader_y = suby1 + 28 + i * spacing;
							inputField_y = inputFieldHeader_y + 16;
							// box outlines then text
							drawDepressed(inputField_x - 4, inputField_y - 4, inputField_x - 4 + inputFieldWidth, inputField_y + 16 - 4);
							// print values on top of boxes
							printText(font8x8_bmp, inputField_x, suby1 + 44 + i * spacing, spriteProperties[i]);
							printText(font8x8_bmp, inputField_x, inputFieldHeader_y, tmpPropertyName);

							if ( errorArr[i] != 1 )
							{
								if ( i == 0 )
								{
									if ( propertyInt > 9 || propertyInt < -1 )
									{
										propertyPageError(i, -1); // reset to default -1.
									}
									else
									{
										char tmpStr[32] = "";
										switch ( propertyInt )
										{
											case -1:
												strcpy(tmpStr, "random");
												break;
											case 0:
												strcpy(tmpStr, "East");
												break;
											case 1:
												strcpy(tmpStr, "Southeast");
												break;
											case 2:
												strcpy(tmpStr, "South");
												break;
											case 3:
												strcpy(tmpStr, "Southwest");
												break;
											case 4:
												strcpy(tmpStr, "West");
												break;
											case 5:
												strcpy(tmpStr, "Northwest");
												break;
											case 6:
												strcpy(tmpStr, "North");
												break;
											case 7:
												strcpy(tmpStr, "Northeast");
												break;
											default:
												break;
										}
										printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, tmpStr);
									}
								}
								else
								{
									// enter other row entries here
								}
							}

							if ( errorMessage )
							{
								if ( errorArr[i] == 1 )
								{
									printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, colorError, "Invalid ID!");
								}
							}
						}

						propertyPageTextAndInput(numProperties, inputFieldWidth);

						if ( editproperty < numProperties )   // edit
						{
							if ( !SDL_IsTextInputActive() )
							{
								SDL_StartTextInput();
								inputstr = spriteProperties[0];
							}

							// set the maximum length allowed for user input
							inputlen = 2;
							propertyPageCursorFlash(spacing);
						}
					}
				}
				else if ( newwindow == 15 )
				{
					if ( selectedEntity[0] != nullptr )
					{
						int numProperties = sizeof(floorDecorationPropertyNames) / sizeof(floorDecorationPropertyNames[0]); //find number of entries in property list
						const int lenProperties = sizeof(floorDecorationPropertyNames[0]) / sizeof(char); //find length of entry in property list
						int spacing = 36; // 36 px between each item in the list.
						int inputFieldHeader_y = suby1 + 28; // 28 px spacing from subwindow start.
						int inputField_x = subx1 + 8; // 8px spacing from subwindow start.
						int inputField_y = inputFieldHeader_y + 16;
						int inputFieldWidth = 64; // width of the text field
						int inputFieldFeedback_x = inputField_x + inputFieldWidth + 8;
						char tmpPropertyName[lenProperties] = "";
						Uint32 color = makeColorRGB(0, 255, 0);
						Uint32 colorRandom = makeColorRGB(0, 168, 255);
						Uint32 colorError = makeColorRGB(255, 0, 0);

						for ( int i = 0; i < numProperties; i++ )
						{
							int propertyInt = atoi(spriteProperties[i]);

							if ( i >= 5 && i <= 8 )
							{
								inputFieldWidth = subx2 - inputField_x; // width of the text field
								if ( i > 5 )
								{
									spacing = 18;
								}
								else
								{
									spacing = 36;
								}
							}
							else
							{
								inputFieldWidth = 64; // width of the text field
								spacing = 36;
							}

							inputFieldHeader_y = suby1 + 28 + i * spacing;
							inputField_y = inputFieldHeader_y + 16;

							if ( i > 5 && i <= 8 )
							{
								int offsetBoxes = 2.5 * 36;
								inputFieldHeader_y = suby1 + 28 + i * spacing + offsetBoxes;
								inputField_y = inputFieldHeader_y + 16;
								// box outlines then text
								drawDepressed(inputField_x - 4, inputField_y - 4, inputField_x - 4 + inputFieldWidth, inputField_y + 16 - 4);
								// print values on top of boxes
								printText(font8x8_bmp, inputField_x, suby1 + 44 + i * spacing + offsetBoxes, spriteProperties[i]);
							}
							else
							{
								// box outlines then text
								drawDepressed(inputField_x - 4, inputField_y - 4, inputField_x - 4 + inputFieldWidth, inputField_y + 16 - 4);
								// print values on top of boxes
								printText(font8x8_bmp, inputField_x, suby1 + 44 + i * spacing, spriteProperties[i]);
								strcpy(tmpPropertyName, floorDecorationPropertyNames[i]);
								printText(font8x8_bmp, inputField_x, inputFieldHeader_y, tmpPropertyName);
							}

							if ( errorArr[i] != 1 )
							{
								if ( i == 0 )
								{
									if ( propertyInt > 9999 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
									else
									{
										if ( modelFileNames.find(propertyInt) != modelFileNames.end() )
										{
											std::string tmpStr = modelFileNames[propertyInt];
											printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, tmpStr.c_str());
										}
										else
										{
											printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, makeColorRGB(255, 0, 0), "Unknown Model!");
										}
									}
								}
								else if ( i == 1 )
								{
									if ( propertyInt > 7 || propertyInt < -1 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
									else
									{
										char tmpStr[32] = "";
										switch ( propertyInt )
										{
											case -1:
												strcpy(tmpStr, "random");
												break;
											case 0:
												strcpy(tmpStr, "East");
												break;
											case 1:
												strcpy(tmpStr, "Southeast");
												break;
											case 2:
												strcpy(tmpStr, "South");
												break;
											case 3:
												strcpy(tmpStr, "Southwest");
												break;
											case 4:
												strcpy(tmpStr, "West");
												break;
											case 5:
												strcpy(tmpStr, "Northwest");
												break;
											case 6:
												strcpy(tmpStr, "North");
												break;
											case 7:
												strcpy(tmpStr, "Northeast");
												break;
											default:
												break;
										}
										printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, tmpStr);
									}
								}
								else if ( i == 2 || i == 3 || i == 4 )
								{
									if ( propertyInt > 999 || propertyInt < -999 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
								}
								else
								{
									// enter other row entries here
								}
							}

							if ( errorMessage )
							{
								if ( errorArr[i] == 1 )
								{
									printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, colorError, "Invalid ID!");
								}
							}
						}

						spacing = 36;

						// Cycle properties with TAB.
						if ( keystatus[SDLK_TAB] )
						{
							keystatus[SDLK_TAB] = 0;
							cursorflash = ticks;
							editproperty++;
							if ( editproperty == numProperties )
							{
								editproperty = 0;
							}

							inputstr = spriteProperties[editproperty];
						}

						// select a textbox
						if ( mousestatus[SDL_BUTTON_LEFT] )
						{
							for ( int i = 0; i < numProperties; i++ )
							{
								inputField_x = subx1 + 8;
								if ( i > 5 && i <= 8 )
								{
									spacing = 18;
									int offsetBoxes = 2.5 * 36;
									inputFieldWidth = subx2 - inputField_x; // width of the text field
									if ( mouseInBounds(inputField_x - 4, inputField_x - 4 + inputFieldWidth,
										suby1 + 40 + i * spacing + offsetBoxes, suby1 + 56 + i * spacing + offsetBoxes) )
									{
										inputstr = spriteProperties[i];
										editproperty = i;
										cursorflash = ticks;
									}
								}
								else
								{
									if ( i == 5 )
									{
										inputFieldWidth = subx2 - inputField_x; // width of the text field
									}
									else
									{
										inputFieldWidth = 64; // width of the text field
									}
									spacing = 36;

									if ( mouseInBounds(inputField_x - 4, inputField_x - 4 + inputFieldWidth,
										suby1 + 40 + i * spacing, suby1 + 56 + i * spacing) )
									{
										inputstr = spriteProperties[i];
										editproperty = i;
										cursorflash = ticks;
									}
								}
							}
						}

						if ( editproperty < numProperties )   // edit
						{
							if ( !SDL_IsTextInputActive() )
							{
								SDL_StartTextInput();
								inputstr = spriteProperties[0];
							}

							// set the maximum length allowed for user input
							if ( editproperty >= 5 && editproperty <= 8 )
							{
								inputlen = 48;
							}
							else
							{
								inputlen = 4;
							}
							if ( (ticks - cursorflash) % TICKS_PER_SECOND < TICKS_PER_SECOND / 2 )
							{
								if ( editproperty > 5 && editproperty <= 8 )
								{
									spacing = 18;
									int offsetBoxes = 2.5 * 36;
									if ( (ticks - cursorflash) % TICKS_PER_SECOND < TICKS_PER_SECOND / 2 )
									{
										printText(font8x8_bmp, subx1 + 8 + strlen(spriteProperties[editproperty]) * 8, suby1 + 44 + editproperty * spacing + offsetBoxes, "\26");
									}
								}
								else
								{
									spacing = 36;
									propertyPageCursorFlash(spacing);
								}
							}
						}
					}
				}
				else if ( newwindow == 18 )
				{
					if ( selectedEntity[0] != nullptr )
					{
						int numProperties = sizeof(soundSourcePropertyNames) / sizeof(soundSourcePropertyNames[0]); //find number of entries in property list
						const int lenProperties = sizeof(soundSourcePropertyNames[0]) / sizeof(char); //find length of entry in property list
						int spacing = 36; // 36 px between each item in the list.
						int inputFieldHeader_y = suby1 + 28; // 28 px spacing from subwindow start.
						int inputField_x = subx1 + 8; // 8px spacing from subwindow start.
						int inputField_y = inputFieldHeader_y + 16;
						int inputFieldWidth = 64; // width of the text field
						int inputFieldFeedback_x = inputField_x + inputFieldWidth + 8;
						char tmpPropertyName[lenProperties] = "";
						Uint32 color = makeColorRGB(0, 255, 0);
						Uint32 colorRandom = makeColorRGB(0, 168, 255);
						Uint32 colorError = makeColorRGB(255, 0, 0);

						for ( int i = 0; i < numProperties; i++ )
						{
							int propertyInt = atoi(spriteProperties[i]);

							strcpy(tmpPropertyName, soundSourcePropertyNames[i]);
							inputFieldHeader_y = suby1 + 28 + i * spacing;
							inputField_y = inputFieldHeader_y + 16;
							// box outlines then text
							drawDepressed(inputField_x - 4, inputField_y - 4, inputField_x - 4 + inputFieldWidth, inputField_y + 16 - 4);
							// print values on top of boxes
							printText(font8x8_bmp, inputField_x, suby1 + 44 + i * spacing, spriteProperties[i]);
							printText(font8x8_bmp, inputField_x, inputFieldHeader_y, tmpPropertyName);

							if ( errorArr[i] != 1 )
							{
								if ( i == 4 )
								{
									if ( propertyInt > 1 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
									else if ( propertyInt == 0 )
									{
										printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, "Play sound on this entity");
									}
									else
									{
										printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, "Play sound global");
									}
								}
								else if ( i == 3 )
								{
									if ( propertyInt > 9999 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
								}
								if ( i == 2 )
								{
									if ( propertyInt > 1 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
								}
								else if ( i == 1 )
								{
									if ( propertyInt > 255 || propertyInt < -1 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
								}
								else if ( i == 0 )
								{
									if ( propertyInt > 999 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
								}
								else
								{
									// enter other row entries here
								}
							}

							if ( errorMessage )
							{
								if ( errorArr[i] == 1 )
								{
									printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, colorError, "Invalid ID!");
								}
							}
						}

						propertyPageTextAndInput(numProperties, inputFieldWidth);

						if ( editproperty < numProperties )   // edit
						{
							if ( !SDL_IsTextInputActive() )
							{
								SDL_StartTextInput();
								inputstr = spriteProperties[0];
							}

							// set the maximum length allowed for user input
							if ( editproperty <= 1 )
							{
								inputlen = 3;
							}
							else if ( editproperty == 2 )
							{
								inputlen = 1;
							}
							else
							{
								inputlen = 4;
							}
							propertyPageCursorFlash(spacing);
						}
					}
				}
				else if ( newwindow == 19 )
				{
					if ( selectedEntity[0] != nullptr )
					{
						int numProperties = sizeof(lightSourcePropertyNames) / sizeof(lightSourcePropertyNames[0]); //find number of entries in property list
						const int lenProperties = sizeof(lightSourcePropertyNames[0]) / sizeof(char); //find length of entry in property list
						int spacing = 36; // 36 px between each item in the list.
						int inputFieldHeader_y = suby1 + 28; // 28 px spacing from subwindow start.
						int inputField_x = subx1 + 8; // 8px spacing from subwindow start.
						int inputField_y = inputFieldHeader_y + 16;
						int inputFieldWidth = 64; // width of the text field
						int inputFieldFeedback_x = inputField_x + inputFieldWidth + 8;
						char tmpPropertyName[lenProperties] = "";
						Uint32 color = makeColorRGB(0, 255, 0);
						Uint32 colorRandom = makeColorRGB(0, 168, 255);
						Uint32 colorError = makeColorRGB(255, 0, 0);

						for ( int i = 0; i < numProperties; i++ )
						{
							int propertyInt = atoi(spriteProperties[i]);

							strcpy(tmpPropertyName, lightSourcePropertyNames[i]);
							inputFieldHeader_y = suby1 + 28 + i * spacing;
							inputField_y = inputFieldHeader_y + 16;
							// box outlines then text
							drawDepressed(inputField_x - 4, inputField_y - 4, inputField_x - 4 + inputFieldWidth, inputField_y + 16 - 4);
							// print values on top of boxes
							printText(font8x8_bmp, inputField_x, suby1 + 44 + i * spacing, spriteProperties[i]);
							printText(font8x8_bmp, inputField_x, inputFieldHeader_y, tmpPropertyName);

							if ( errorArr[i] != 1 )
							{
								if ( i == 0 || i == 2 || i == 3 || i == 5 )
								{
									if ( propertyInt > 2 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
								}
								else if ( i == 4 )
								{
									if ( propertyInt > 64 || propertyInt < -1 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
								}
								else if ( i == 6 )
								{
									if ( propertyInt > 9999 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
								}
								else if ( i == 1 )
								{
									if ( propertyInt > 255 || propertyInt < -1 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
								}
								else
								{
									// enter other row entries here
								}
							}

							if ( errorMessage )
							{
								if ( errorArr[i] == 1 )
								{
									printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, colorError, "Invalid ID!");
								}
							}
						}

						propertyPageTextAndInput(numProperties, inputFieldWidth);

						if ( editproperty < numProperties )   // edit
						{
							if ( !SDL_IsTextInputActive() )
							{
								SDL_StartTextInput();
								inputstr = spriteProperties[0];
							}

							// set the maximum length allowed for user input
							if ( editproperty == 1 )
							{
								inputlen = 3;
							}
							else if ( editproperty == 0 || editproperty == 2 || editproperty == 3 || editproperty == 5 )
							{
								inputlen = 1;
							}
							else if ( editproperty == 4 )
							{
								inputlen = 2;
							}
							else
							{
								inputlen = 4;
							}
							propertyPageCursorFlash(spacing);
						}
					}
				}
				else if ( newwindow == 20 )
				{
					if ( selectedEntity[0] != nullptr )
					{
						int numProperties = sizeof(textSourcePropertyNames) / sizeof(textSourcePropertyNames[0]); //find number of entries in property list
						const int lenProperties = sizeof(textSourcePropertyNames[0]) / sizeof(char); //find length of entry in property list
						int spacing = 36; // 36 px between each item in the list.
						int inputFieldHeader_y = suby1 + 28; // 28 px spacing from subwindow start.
						int inputField_x = subx1 + 8; // 8px spacing from subwindow start.
						int inputField_y = inputFieldHeader_y + 16;
						int inputFieldWidth = 64; // width of the text field
						int inputFieldFeedback_x = inputField_x + inputFieldWidth + 8;
						char tmpPropertyName[lenProperties] = "";
						Uint32 color = makeColorRGB(0, 255, 0);
						Uint32 colorRandom = makeColorRGB(0, 168, 255);
						Uint32 colorError = makeColorRGB(255, 0, 0);
						bool showTextSourceTooltip = false;

						for ( int i = 0; i < numProperties; i++ )
						{
							inputField_x = subx1 + 8;
							int propertyInt = atoi(spriteProperties[i]);
							if ( i >= 3 && i < 8 )
							{
								inputFieldWidth = subx2 - inputField_x; // width of the text field
								inputFieldFeedback_x = inputField_x + inputFieldWidth + 8;
								if ( i > 3 )
								{
									spacing = 18;
								}
								else
								{
									spacing = 36;
								}
							}
							else
							{
								inputFieldWidth = 64; // width of the text field
								inputFieldFeedback_x = inputField_x + inputFieldWidth + 8;
								spacing = 36;
							}

							if ( i <= 2 )
							{
								inputFieldFeedback_x = inputField_x + (inputFieldWidth + 8) * 4 - 4;
							}
							if ( i == 1 || i == 2 )
							{
								inputFieldHeader_y = suby1 + 28;
								inputField_y = inputFieldHeader_y + 16;
								inputField_x = subx1 + 8 + (inputFieldWidth + 8) * i;
								if ( i == 2 )
								{
									Uint32 colorPreview = makeColorRGB((Uint32)atoi(spriteProperties[0]),
										(Uint32)atoi(spriteProperties[1]), (Uint32)atoi(spriteProperties[2]));
									SDL_Rect src;
									src.x = subx1 + 8 + (inputFieldWidth + 8) * 3;
									src.h = 16;
									src.w = 32;
									src.y = inputField_y - 4;
									drawRect(&src, colorPreview, 255);

									printText(font8x8_bmp, inputField_x + (inputFieldWidth + 8), inputFieldHeader_y, "Hover for help");
									if ( mouseInBounds(inputField_x + (inputFieldWidth + 8), inputField_x + (inputFieldWidth + 8) + strlen("Hover for help") * 8,
										inputFieldHeader_y, inputFieldHeader_y + 16) )
									{
										showTextSourceTooltip = true;
									}
								}
							}
							else if ( i > 3 && i < 8 )
							{
								inputFieldHeader_y = suby1 + 28 + (i - 1) * spacing;
								inputField_y = inputFieldHeader_y + 16;
							}
							else
							{
								// header print
								if ( i == 3 )
								{
									inputFieldHeader_y = suby1 + 28 + (i - 2) * spacing;
								}
								else if ( i >= 8 )
								{
									inputFieldHeader_y = suby1 + 28 + (i - 4) * spacing;
								}
								else
								{
									inputFieldHeader_y = suby1 + 28;
								}
								inputField_y = inputFieldHeader_y + 16;
								strcpy(tmpPropertyName, textSourcePropertyNames[i]);
								printText(font8x8_bmp, inputField_x, inputFieldHeader_y, tmpPropertyName);
							}
							// box outlines then text
							// print values on top of boxes
							if ( i == 1 || i == 2 )
							{
								drawDepressed(inputField_x - 4, inputField_y - 4, inputField_x - 4 + inputFieldWidth, inputField_y + 16 - 4);
								printText(font8x8_bmp, inputField_x, inputField_y, spriteProperties[i]);
							}
							else
							{
								drawDepressed(inputField_x - 4, inputField_y - 4, inputField_x - 4 + inputFieldWidth, inputField_y + 16 - 4);
								printText(font8x8_bmp, inputField_x, inputField_y, spriteProperties[i]);
							}

							if ( errorArr[i] != 1 )
							{
								if ( i >= 0 && i <= 2 )
								{
									if ( propertyInt > 255 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
								}
								else if ( i == 8 )
								{
									if ( propertyInt > 9999 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
								}
								else if ( i == 9 )
								{
									if ( propertyInt > 1 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
								}
								else
								{
									// enter other row entries here
								}
							}

							if ( errorMessage )
							{
								if ( errorArr[i] == 1 )
								{
									printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, colorError, "Invalid ID!");
								}
							}
						}

						spacing = 36;

						// Cycle properties with TAB.
						if ( keystatus[SDLK_TAB] )
						{
							keystatus[SDLK_TAB] = 0;
							cursorflash = ticks;
							editproperty++;
							if ( editproperty == numProperties )
							{
								editproperty = 0;
							}

							inputstr = spriteProperties[editproperty];
						}

						// select a textbox
						if ( mousestatus[SDL_BUTTON_LEFT] )
						{
							for ( int i = 0; i < numProperties; i++ )
							{
								inputField_x = subx1 + 8;
								if ( i > 3 && i < 8 )
								{
									spacing = 18;
									inputFieldWidth = subx2 - inputField_x; // width of the text field
									if ( mouseInBounds(inputField_x - 4, inputField_x - 4 + inputFieldWidth,
										suby1 + 40 + (i - 1) * spacing, suby1 + 56 + (i - 1) * spacing) )
									{
										inputstr = spriteProperties[i];
										editproperty = i;
										cursorflash = ticks;
									}
								}
								else
								{
									if ( i == 3 )
									{
										inputFieldWidth = subx2 - inputField_x; // width of the text field
									}
									else
									{
										inputFieldWidth = 64; // width of the text field
									}
									spacing = 36;
									if ( i == 1 || i == 2 )
									{
										inputField_x = subx1 + 8 + (inputFieldWidth + 8) * i;
										spacing = 0;
									}
									else if ( i == 3 )
									{
										spacing = 12;
									}
									
									if ( i >= 8 )
									{
										if ( mouseInBounds(inputField_x - 4, inputField_x - 4 + inputFieldWidth,
											suby1 + 40 + (i - 4) * spacing, suby1 + 56 + (i - 4) * spacing) )
										{
											inputstr = spriteProperties[i];
											editproperty = i;
											cursorflash = ticks;
										}
									}
									else
									{
										if ( mouseInBounds(inputField_x - 4, inputField_x - 4 + inputFieldWidth, 
											suby1 + 40 + i * spacing, suby1 + 56 + i * spacing) )
										{
											inputstr = spriteProperties[i];
											editproperty = i;
											cursorflash = ticks;
										}
									}
								}
							}
						}

						if ( editproperty < numProperties )   // edit
						{
							if ( !SDL_IsTextInputActive() )
							{
								SDL_StartTextInput();
								inputstr = spriteProperties[0];
							}
							if ( SDL_IsTextInputActive() )
							{
								if ( textInsertCaratPosition >= 0 )
								{
									textInsertCaratPosition = std::min(textInsertCaratPosition, (int)strlen(inputstr));
									if ( editproperty < 3 || editproperty > 8 )
									{
										textInsertCaratPosition = -1;
									}
								}
								if ( keystatus[SDLK_LEFT] )
								{
									keystatus[SDLK_LEFT] = 0;
									if ( textInsertCaratPosition > 0 )
									{
										--textInsertCaratPosition;
									}
									else if ( textInsertCaratPosition == 0 )
									{
										// do nothing
									}
									else
									{
										textInsertCaratPosition = strlen(inputstr);
									}
									cursorflash = ticks;
								}
								else if ( keystatus[SDLK_RIGHT] )
								{
									keystatus[SDLK_RIGHT] = 0;
									if ( textInsertCaratPosition == -1 )
									{
										textInsertCaratPosition = strlen(inputstr);
									}
									else
									{
										++textInsertCaratPosition;
										textInsertCaratPosition = std::min((int)strlen(inputstr), textInsertCaratPosition);
									}
									cursorflash = ticks;
								}
								if ( keystatus[SDLK_RETURN] )
								{
									if ( textInsertCaratPosition >= 0 )
									{
										textInsertCaratPosition = -1;
									}
									else
									{
										textInsertCaratPosition = strlen(inputstr);
									}
									keystatus[SDLK_RETURN] = 0;
								}
							}

							// set the maximum length allowed for user input
							if ( editproperty >= 3 && editproperty < 8 )
							{
								if ( editproperty == 7 )
								{
									inputlen = 32;
								}
								else
								{
									inputlen = 48;
								}
							}
							else
							{
								if ( editproperty == 9 )
								{
									inputlen = 1;
								}
								else
								{
									inputlen = 4;
								}
							}
							if ( (ticks - cursorflash) % TICKS_PER_SECOND < TICKS_PER_SECOND / 2 )
							{
								if ( editproperty > 3 && editproperty < 8 )
								{
									spacing = 18;
									if ( textInsertCaratPosition >= 0 )
									{
										printTextFormattedColor(font8x8_bmp, subx1 + 8 + textInsertCaratPosition * 8, suby1 + 44 + (editproperty - 1) * spacing, color, "\26");
									}
									else
									{
										printText(font8x8_bmp, subx1 + 8 + strlen(spriteProperties[editproperty]) * 8, suby1 + 44 + (editproperty - 1) * spacing, "\26");
									}
								}
								else
								{
									spacing = 36;
									if ( editproperty == 1 || editproperty == 2 )
									{
										spacing = 0;
										inputFieldWidth = 64;
										printText(font8x8_bmp, subx1 + 8 + (inputFieldWidth + 8) * editproperty + strlen(spriteProperties[editproperty]) * 8, suby1 + 44, "\26");
									}
									else if ( editproperty == 3 )
									{
										if ( textInsertCaratPosition >= 0 )
										{
											printTextFormattedColor(font8x8_bmp, subx1 + 8 + textInsertCaratPosition * 8, suby1 + 44 + spacing, color, "\26");
										}
										else
										{
											printText(font8x8_bmp, subx1 + 8 + strlen(spriteProperties[editproperty]) * 8, suby1 + 44 + spacing, "\26");
										}
									}
									else
									{
										if ( editproperty >= 8 )
										{
											printText(font8x8_bmp, subx1 + 8 + strlen(spriteProperties[editproperty]) * 8, suby1 + 44 + (editproperty - 4) * spacing, "\26");
										}
										else
										{
											printText(font8x8_bmp, subx1 + 8 + strlen(spriteProperties[editproperty]) * 8, suby1 + 44 + editproperty * spacing, "\26");
										}
									}
								}
							}
						}

						if ( showTextSourceTooltip )
						{
							SDL_Rect src;
							src.w = 74 * 8 + 4;
							src.h = 240;
							src.x = omousex - src.w / 2;
							src.y = omousey + 16 - src.h / 2;
							drawTooltip(&src);
							printText(font8x8_bmp, src.x + 4, src.y + 4, "This sprite sends a message to all players in the specified color.");
							printText(font8x8_bmp, src.x + 4, src.y + 16, "Text will appear as one line unless a new line symbol is entered.");
							printText(font8x8_bmp, src.x + 4, src.y + 28, "To insert text onto a new line, enter \\n in the text field.");
							printText(font8x8_bmp, src.x + 4, src.y + 40, "To address the player's name in the text, enter @p in the text field.");
							printText(font8x8_bmp, src.x + 4, src.y + 52, "E.g \"Hello, \\n@p\"");
							printText(font8x8_bmp, src.x + 4, src.y + 64, "Adding @script to the text allows some basic scripting instead of text.");
							printText(font8x8_bmp, src.x + 4, src.y + 76, "Separate all tags with a space. Some tags allow a range of values.");
							printText(font8x8_bmp, src.x + 4, src.y + 88, "@clrplayer clears player data. @class=1 sets class to warrior");
							printText(font8x8_bmp, src.x + 4, src.y + 100, "@clrstats resets stats and HP/MP. @hunger=250 sets player hungry.");
							printText(font8x8_bmp, src.x + 4, src.y + 112, "@nextlevel=2 triggers a 2 level change.");
							printText(font8x8_bmp, src.x + 4, src.y + 124, "@copyNPC=2 player stats set to monster named \"scriptNPC\" that is NPC: 2");
							printText(font8x8_bmp, src.x + 4, src.y + 136, "");
							printText(font8x8_bmp, src.x + 4, src.y + 148, "Tags that take a map reference or range: (i.e tag=1-3,4-6 tag=8,10)");
							printText(font8x8_bmp, src.x + 4, src.y + 160, "@power=11,15 powers tile x=11,y=15");
							printText(font8x8_bmp, src.x + 4, src.y + 172, "@power=11-13,15-17 powers 3x3 grid from x=11,y=15 to x=13,y=17.");
							printText(font8x8_bmp, src.x + 4, src.y + 184, "@unpower= unpower tile(s). @freezemonsters= freeze monster on tile(s)");
							printText(font8x8_bmp, src.x + 4, src.y + 196, "@unfreezemonsters= unfreeze on tile(s). @killall= kill npcs on tile(s)");
							printText(font8x8_bmp, src.x + 4, src.y + 208, "@killenemies= kill non- ally npcs on tile(s).");
							printText(font8x8_bmp, src.x + 4, src.y + 220, "@nodropitems= npc will be marked to not drop items on death within tile(s)");
						}
					}
				}
				else if ( newwindow == 21 )
				{
					if ( selectedEntity[0] != nullptr )
					{
						int numProperties = sizeof(signalTimerPropertyNames) / sizeof(signalTimerPropertyNames[0]); //find number of entries in property list
						const int lenProperties = sizeof(signalTimerPropertyNames[0]) / sizeof(char); //find length of entry in property list
						int spacing = 36; // 36 px between each item in the list.
						int inputFieldHeader_y = suby1 + 28; // 28 px spacing from subwindow start.
						int inputField_x = subx1 + 8; // 8px spacing from subwindow start.
						int inputField_y = inputFieldHeader_y + 16;
						int inputFieldWidth = 64; // width of the text field
						int inputFieldFeedback_x = inputField_x + inputFieldWidth + 8;
						char tmpPropertyName[lenProperties] = "";
						Uint32 color = makeColorRGB(0, 255, 0);
						Uint32 colorRandom = makeColorRGB(0, 168, 255);
						Uint32 colorError = makeColorRGB(255, 0, 0);

						for ( int i = 0; i < numProperties; i++ )
						{
							int propertyInt = atoi(spriteProperties[i]);

							strcpy(tmpPropertyName, signalTimerPropertyNames[i]);
							inputFieldHeader_y = suby1 + 28 + i * spacing;
							inputField_y = inputFieldHeader_y + 16;
							// box outlines then text
							drawDepressed(inputField_x - 4, inputField_y - 4, inputField_x - 4 + inputFieldWidth, inputField_y + 16 - 4);
							// print values on top of boxes
							printText(font8x8_bmp, inputField_x, suby1 + 44 + i * spacing, spriteProperties[i]);
							printText(font8x8_bmp, inputField_x, inputFieldHeader_y, tmpPropertyName);

							if ( errorArr[i] != 1 )
							{
								if ( i == 0 )
								{
									if ( propertyInt > 3 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
									else
									{
										char tmpStr[8] = "";
										switch ( propertyInt )
										{
											case 0:
												strcpy(tmpStr, "West");
												break;
											case 1:
												strcpy(tmpStr, "South");
												break;
											case 2:
												strcpy(tmpStr, "East");
												break;
											case 3:
												strcpy(tmpStr, "North");
												break;
											default:
												break;
										}
										printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, tmpStr);
									}
								}
								else if ( i == 1 )
								{
									if ( propertyInt > 9999 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
								}
								else if ( i == 2 )
								{
									if ( propertyInt > 9999 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
									else
									{
										if ( propertyInt == 0 )
										{
											printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, "output without on/off toggling");
										}
									}
								}
								else if ( i == 3 )
								{
									if ( propertyInt > 9999 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
									else
									{
										if ( propertyInt == 0 )
										{
											printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, "repeat infinite");
										}
									}
								}
								else if ( i == 4 )
								{
									if ( propertyInt > 1 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
									else
									{
										if ( propertyInt == 0 )
										{
											printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, "turn off without input signal");
										}
										else if ( propertyInt == 1 )
										{
											printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, "stay on without input signal");
										}
									}
								}
								else if ( i == 5 )
								{
									if ( propertyInt > 1 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
									else
									{
										if ( propertyInt == 0 )
										{
											printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, "non-inverted");
										}
										else if ( propertyInt == 1 )
										{
											printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, "output inverted");
										}
									}
								}
								else
								{
									// enter other row entries here
								}
							}

							if ( errorMessage )
							{
								if ( errorArr[i] == 1 )
								{
									printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, colorError, "Invalid ID!");
								}
							}
						}

						propertyPageTextAndInput(numProperties, inputFieldWidth);

						if ( editproperty < numProperties )   // edit
						{
							if ( !SDL_IsTextInputActive() )
							{
								SDL_StartTextInput();
								inputstr = spriteProperties[0];
							}

							// set the maximum length allowed for user input
							inputlen = 4;
							propertyPageCursorFlash(spacing);
						}
					}
				}
				else if ( newwindow == 22 )
				{
					if ( selectedEntity[0] != nullptr )
					{
						int numProperties = sizeof(customPortalPropertyNames) / sizeof(customPortalPropertyNames[0]); //find number of entries in property list
						const int lenProperties = sizeof(customPortalPropertyNames[0]) / sizeof(char); //find length of entry in property list
						int spacing = 36; // 36 px between each item in the list.
						int inputFieldHeader_y = suby1 + 28; // 28 px spacing from subwindow start.
						int inputField_x = subx1 + 8; // 8px spacing from subwindow start.
						int inputField_y = inputFieldHeader_y + 16;
						int inputFieldWidth = 64; // width of the text field
						int inputFieldFeedback_x = inputField_x + inputFieldWidth + 8;
						char tmpPropertyName[lenProperties] = "";
						Uint32 color = makeColorRGB(0, 255, 0);
						Uint32 colorRandom = makeColorRGB(0, 168, 255);
						Uint32 colorError = makeColorRGB(255, 0, 0);

						for ( int i = 0; i < numProperties; i++ )
						{
							int propertyInt = atoi(spriteProperties[i]);

							strcpy(tmpPropertyName, customPortalPropertyNames[i]);
							inputFieldHeader_y = suby1 + 28 + i * spacing;
							inputField_y = inputFieldHeader_y + 16;
							// box outlines then text
							if ( i == 4 )
							{
								inputFieldWidth = 280; // width of the text field
							}
							else
							{
								inputFieldWidth = 64; // width of the text field
							}
							drawDepressed(inputField_x - 4, inputField_y - 4, inputField_x - 4 + inputFieldWidth, inputField_y + 16 - 4);
							// print values on top of boxes
							printText(font8x8_bmp, inputField_x, suby1 + 44 + i * spacing, spriteProperties[i]);
							printText(font8x8_bmp, inputField_x, inputFieldHeader_y, tmpPropertyName);

							if ( errorArr[i] != 1 )
							{
								if ( i == 0 )
								{
									if ( propertyInt > 9999 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
									else
									{
										if ( modelFileNames.find(propertyInt) != modelFileNames.end() )
										{
											std::string tmpStr = modelFileNames[propertyInt];
											printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, tmpStr.c_str());
										}
										else
										{
											printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, makeColorRGB(255, 0, 0), "Unknown Model!");
										}
									}
								}
								else if ( i == 1 )
								{
									if ( propertyInt > 9 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
									else
									{
										if ( propertyInt != 0 )
										{
											printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, 
												"using model textures %d-%d for animation", 
												atoi(spriteProperties[0]), atoi(spriteProperties[0]) + propertyInt - 1);
										}
									}
								}
								else if ( i == 2 )
								{
									if ( propertyInt > 999 || propertyInt < -999 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
								}
								else if ( i == 3 )
								{
									if ( propertyInt > 99 || propertyInt < -99 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
									else
									{
										if ( spriteProperties[4][0] != 0 && propertyInt != 0 )
										{
											char shortName[16] = "";
											strncpy(shortName, spriteProperties[4], 11);
											if ( strlen(spriteProperties[4]) > 9 )
											{
												strcat(shortName, "..");
											}
											printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, "move to instance %d of map name %s", propertyInt, shortName);
										}
										else if ( spriteProperties[4][0] != 0 && propertyInt == 0 )
										{
											char shortName[16] = "";
											strncpy(shortName, spriteProperties[4], 11);
											if ( strlen(spriteProperties[4]) > 9 )
											{
												strcat(shortName, "..");
											}
											printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, "move to first instance of map name %s", propertyInt, shortName);
										}
										else
										{
											printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, "jump %d levels", propertyInt);
										}
									}
								}
								else if ( i == 4 )
								{

								}
								else if ( i == 5 )
								{
									if ( propertyInt > 1 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
									else
									{
										if ( propertyInt == 0 )
										{
											printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, "always visible");
										}
										else if ( propertyInt == 1 )
										{
											printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, "requires power to be visible");
										}
									}
								}
								else if ( i == 6 )
								{
									if ( propertyInt > 1 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
									else
									{
										if ( propertyInt == 0 )
										{
											if ( spriteProperties[4][0] != 0 )
											{
												printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, "searching for map in normal levels");
											}
											else
											{
												printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, "no toggle");
											}
										}
										else if ( propertyInt == 1 )
										{
											if ( spriteProperties[4][0] != 0 )
											{
												printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, "searching for map in secret levels");
											}
											else
											{
												printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, "will toggle secret levels file");
											}
										}
									}
								}
								else
								{
									// enter other row entries here
								}
							}

							if ( errorMessage )
							{
								if ( errorArr[i] == 1 )
								{
									printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, colorError, "Invalid ID!");
								}
							}
						}

						propertyPageTextAndInput(numProperties, inputFieldWidth);

						if ( editproperty < numProperties )   // edit
						{
							if ( !SDL_IsTextInputActive() )
							{
								SDL_StartTextInput();
								inputstr = spriteProperties[0];
							}

							// set the maximum length allowed for user input

							if ( editproperty == 4 )
							{
								inputlen = 32;
							}
							else
							{
								inputlen = 4;
							}
							propertyPageCursorFlash(spacing);
						}
					}
				}
				else if ( newwindow == 23 )
				{
					if ( selectedEntity[0] != nullptr )
					{
						int numProperties = sizeof(tablePropertyNames) / sizeof(tablePropertyNames[0]); //find number of entries in property list
						const int lenProperties = sizeof(tablePropertyNames[0]) / sizeof(char); //find length of entry in property list
						int spacing = 36; // 36 px between each item in the list.
						int inputFieldHeader_y = suby1 + 28; // 28 px spacing from subwindow start.
						int inputField_x = subx1 + 8; // 8px spacing from subwindow start.
						int inputField_y = inputFieldHeader_y + 16;
						int inputFieldWidth = 64; // width of the text field
						int inputFieldFeedback_x = inputField_x + inputFieldWidth + 8;
						char tmpPropertyName[lenProperties] = "";
						Uint32 color = makeColorRGB(0, 255, 0);
						Uint32 colorRandom = makeColorRGB(0, 168, 255);
						Uint32 colorError = makeColorRGB(255, 0, 0);

						for ( int i = 0; i < numProperties; i++ )
						{
							int propertyInt = atoi(spriteProperties[i]);

							strcpy(tmpPropertyName, tablePropertyNames[i]);
							inputFieldHeader_y = suby1 + 28 + i * spacing;
							inputField_y = inputFieldHeader_y + 16;
							// box outlines then text
							drawDepressed(inputField_x - 4, inputField_y - 4, inputField_x - 4 + inputFieldWidth, inputField_y + 16 - 4);
							// print values on top of boxes
							printText(font8x8_bmp, inputField_x, suby1 + 44 + i * spacing, spriteProperties[i]);
							printText(font8x8_bmp, inputField_x, inputFieldHeader_y, tmpPropertyName);

							if ( errorArr[i] != 1 )
							{
								if ( i == 0 )
								{
									if ( propertyInt > 7 || propertyInt < -1 )
									{
										propertyPageError(i, -1); // reset to default -1.
									}
									else
									{
										char tmpStr[32] = "";
										switch ( propertyInt )
										{
											case -1:
												strcpy(tmpStr, "default");
												break;
											case 0:
												strcpy(tmpStr, "East");
												break;
											case 1:
												strcpy(tmpStr, "Southeast");
												break;
											case 2:
												strcpy(tmpStr, "South");
												break;
											case 3:
												strcpy(tmpStr, "Southwest");
												break;
											case 4:
												strcpy(tmpStr, "West");
												break;
											case 5:
												strcpy(tmpStr, "Northwest");
												break;
											case 6:
												strcpy(tmpStr, "North");
												break;
											case 7:
												strcpy(tmpStr, "Northeast");
												break;
											default:
												break;
										}
										printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, tmpStr);
									}
								}
								else if ( i == 1 )
								{
									if ( propertyInt > 4 || propertyInt < -1 )
									{
										propertyPageError(i, -1); // reset to default -1.
									}
									else
									{
										if ( propertyInt == -1 )
										{
											printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, "random chairs", propertyInt);
										}
										else
										{
											printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, "%d chairs", propertyInt);
										}
									}
								}
								else if ( i == 2 )
								{
									if ( propertyInt > 100 || propertyInt < -1 )
									{
										propertyPageError(i, -1); // reset to default -1.
									}
									else
									{
										if ( propertyInt == -1 )
										{
											printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, "default random item chance");
										}
										else
										{
											printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, "%d%% chance", propertyInt);
										}
									}
								}
								else
								{
									// enter other row entries here
								}
							}

							if ( errorMessage )
							{
								if ( errorArr[i] == 1 )
								{
									printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, colorError, "Invalid ID!");
								}
							}
						}

						propertyPageTextAndInput(numProperties, inputFieldWidth);

						if ( editproperty < numProperties )   // edit
						{
							if ( !SDL_IsTextInputActive() )
							{
								SDL_StartTextInput();
								inputstr = spriteProperties[0];
							}

							// set the maximum length allowed for user input
							if ( editproperty == 2 )
							{
								inputlen = 4;
							}
							else
							{
								inputlen = 3;
							}
							propertyPageCursorFlash(spacing);
						}
					}
				}
				else if ( newwindow == 24 )
				{
						if ( selectedEntity[0] != nullptr )
					{
						int numProperties = sizeof(readableBookPropertyNames) / sizeof(readableBookPropertyNames[0]); //find number of entries in property list
						const int lenProperties = sizeof(readableBookPropertyNames[0]) / sizeof(char); //find length of entry in property list
						int spacing = 36; // 36 px between each item in the list.
						int inputFieldHeader_y = suby1 + 28; // 28 px spacing from subwindow start.
						int inputField_x = subx1 + 8; // 8px spacing from subwindow start.
						int inputField_y = inputFieldHeader_y + 16;
						int inputFieldWidth = 64; // width of the text field
						int inputFieldFeedback_x = inputField_x + inputFieldWidth + 8;
						char tmpPropertyName[lenProperties] = "";
						Uint32 color = makeColorRGB(0, 255, 0);
						Uint32 colorRandom = makeColorRGB(0, 168, 255);
						Uint32 colorError = makeColorRGB(255, 0, 0);

						for ( int i = 0; i < numProperties; i++ )
						{
							int propertyInt = atoi(spriteProperties[i]);

							strcpy(tmpPropertyName, readableBookPropertyNames[i]);
							inputFieldHeader_y = suby1 + 28 + i * spacing;
							inputField_y = inputFieldHeader_y + 16;
							// box outlines then text
							if ( i == 3 )
							{
								inputFieldWidth = subx2 - inputField_x; // width of the text field
							}
							else
							{
								inputFieldWidth = 64; // width of the text field
							}
							drawDepressed(inputField_x - 4, inputField_y - 4, inputField_x - 4 + inputFieldWidth, inputField_y + 16 - 4);
							// print values on top of boxes
							printText(font8x8_bmp, inputField_x, suby1 + 44 + i * spacing, spriteProperties[i]);
							printText(font8x8_bmp, inputField_x, inputFieldHeader_y, tmpPropertyName);

							if ( errorArr[i] != 1 )
							{
								if ( i == 0 )
								{
									if ( propertyInt == 2 )
									{
										printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, makeColorRGB(200, 128, 0), "Decrepit");
									}
									else if ( propertyInt == 3 )
									{
										printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, makeColorRGB(255, 255, 0), "Worn");
									}
									else if ( propertyInt == 4 )
									{
										printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, makeColorRGB(128, 200, 0), "Servicable");
									}
									else if ( propertyInt == 5 )
									{
										printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, makeColorRGB(0, 255, 0), "Excellent");
									}
									else if ( propertyInt == 1 )
									{
										printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, makeColorRGB(255, 0, 0), "Broken");
									}
									else if ( propertyInt == 0 )
									{
										printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, colorRandom, "Random");
									}
									else if ( propertyInt < 0 || propertyInt > 5 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
								}
								else if ( i == 1 )
								{
									if ( strcmp(spriteProperties[i], "00") == 0 )
									{
										printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, colorRandom, "Random");
									}
									else if ( propertyInt > 9 || propertyInt < -9 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
								}
								else if ( i == 2 )
								{
									if ( propertyInt == 1 )
									{
										color = makeColorRGB(0, 255, 0);
										printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, "Identified");
									}
									else if ( propertyInt == 0 )
									{
										color = makeColorRGB(255, 255, 0);
										printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, "Unidentified");
									}
									else if ( propertyInt == 2 )
									{
										printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, colorRandom, "Random");
									}
									else
									{
										propertyPageError(i, 0); // reset to default 0.
									}
								}
								else
								{
									// enter other row entries here
								}
							}

							if ( errorMessage )
							{
								if ( errorArr[i] == 1 )
								{
									printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, colorError, "Invalid ID!");
								}
							}
						}

						propertyPageTextAndInput(numProperties, inputFieldWidth);

						if ( editproperty < numProperties )   // edit
						{
							if ( !SDL_IsTextInputActive() )
							{
								SDL_StartTextInput();
								inputstr = spriteProperties[0];
							}

							// set the maximum length allowed for user input

							if ( editproperty == 3 )
							{
								inputlen = 48;
							}
							else
							{
								inputlen = 4;
							}
							propertyPageCursorFlash(spacing);
						}
					}
				}
				else if ( newwindow == 26 )
				{
					if ( selectedEntity[0] != NULL )
					{
						int numProperties = sizeof(doorPropertyNames) / sizeof(doorPropertyNames[0]); //find number of entries in property list
						const int lenProperties = sizeof(doorPropertyNames[0]) / sizeof(char); //find length of entry in property list
						int spacing = 36; // 36 px between each item in the list.
						int inputFieldHeader_y = suby1 + 28; // 28 px spacing from subwindow start.
						int inputField_x = subx1 + 8; // 8px spacing from subwindow start.
						int inputField_y = inputFieldHeader_y + 16;
						int inputFieldWidth = 64; // width of the text field
						int inputFieldFeedback_x = inputField_x + inputFieldWidth + 8;
						char tmpPropertyName[lenProperties] = "";
						Uint32 color = makeColorRGB(0, 255, 0);
						Uint32 colorRandom = makeColorRGB(0, 168, 255);
						Uint32 colorError = makeColorRGB(255, 0, 0);

						for ( int i = 0; i < numProperties; i++ )
						{
							int propertyInt = atoi(spriteProperties[i]);

							strcpy(tmpPropertyName, doorPropertyNames[i]);
							inputFieldHeader_y = suby1 + 28 + i * spacing;
							inputField_y = inputFieldHeader_y + 16;
							// box outlines then text
							drawDepressed(inputField_x - 4, inputField_y - 4, inputField_x - 4 + inputFieldWidth, inputField_y + 16 - 4);
							// print values on top of boxes
							printText(font8x8_bmp, inputField_x, suby1 + 44 + i * spacing, spriteProperties[i]);
							printText(font8x8_bmp, inputField_x, inputFieldHeader_y, tmpPropertyName);

							if ( errorArr[i] != 1 )
							{
								if ( i == 0 )
								{
									if ( propertyInt > 2 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0 random.
									}
									else
									{
										char tmpStr[32] = "";
										if ( propertyInt == 1 )
										{
											strcpy(tmpStr, "force locked");
										}
										else if ( propertyInt == 2 )
										{
											strcpy(tmpStr, "force unlocked");
										}
										else
										{
											strcpy(tmpStr, "default random");
										}
										printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, tmpStr);
									}
								}
								else if ( i == 1 || i == 2 )
								{
									if ( propertyInt > 1 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default no disable
									}
									else
									{
										char tmpStr[32] = "";
										if ( propertyInt == 1 )
										{
											strcpy(tmpStr, "disabled");
										}
										printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, tmpStr);
									}
								}
								else
								{
									// enter other row entries here
								}
							}

							if ( errorMessage )
							{
								if ( errorArr[i] == 1 )
								{
									printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, colorError, "Invalid ID!");
								}
							}
						}

						propertyPageTextAndInput(numProperties, inputFieldWidth);

						if ( editproperty < numProperties )   // edit
						{
							if ( !SDL_IsTextInputActive() )
							{
								SDL_StartTextInput();
								inputstr = spriteProperties[0];
							}

							// set the maximum length allowed for user input
							inputlen = 2;
							propertyPageCursorFlash(spacing);
						}
					}
				}
				else if ( newwindow == 27 )
				{
					if ( selectedEntity[0] != NULL )
					{
						int numProperties = sizeof(gatePropertyNames) / sizeof(gatePropertyNames[0]); //find number of entries in property list
						const int lenProperties = sizeof(gatePropertyNames[0]) / sizeof(char); //find length of entry in property list
						int spacing = 36; // 36 px between each item in the list.
						int inputFieldHeader_y = suby1 + 28; // 28 px spacing from subwindow start.
						int inputField_x = subx1 + 8; // 8px spacing from subwindow start.
						int inputField_y = inputFieldHeader_y + 16;
						int inputFieldWidth = 64; // width of the text field
						int inputFieldFeedback_x = inputField_x + inputFieldWidth + 8;
						char tmpPropertyName[lenProperties] = "";
						Uint32 color = makeColorRGB(0, 255, 0);
						Uint32 colorRandom = makeColorRGB(0, 168, 255);
						Uint32 colorError = makeColorRGB(255, 0, 0);

						for ( int i = 0; i < numProperties; i++ )
						{
							int propertyInt = atoi(spriteProperties[i]);

							strcpy(tmpPropertyName, gatePropertyNames[i]);
							inputFieldHeader_y = suby1 + 28 + i * spacing;
							inputField_y = inputFieldHeader_y + 16;
							// box outlines then text
							drawDepressed(inputField_x - 4, inputField_y - 4, inputField_x - 4 + inputFieldWidth, inputField_y + 16 - 4);
							// print values on top of boxes
							printText(font8x8_bmp, inputField_x, suby1 + 44 + i * spacing, spriteProperties[i]);
							printText(font8x8_bmp, inputField_x, inputFieldHeader_y, tmpPropertyName);

							if ( errorArr[i] != 1 )
							{
								if ( i == 0 )
								{
									if ( propertyInt > 1 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default no disable
									}
									else
									{
										char tmpStr[32] = "";
										if ( propertyInt == 1 )
										{
											strcpy(tmpStr, "disabled");
										}
										printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, tmpStr);
									}
								}
								else
								{
									// enter other row entries here
								}
							}

							if ( errorMessage )
							{
								if ( errorArr[i] == 1 )
								{
									printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, colorError, "Invalid ID!");
								}
							}
						}

						propertyPageTextAndInput(numProperties, inputFieldWidth);

						if ( editproperty < numProperties )   // edit
						{
							if ( !SDL_IsTextInputActive() )
							{
								SDL_StartTextInput();
								inputstr = spriteProperties[0];
							}

							// set the maximum length allowed for user input
							inputlen = 2;
							propertyPageCursorFlash(spacing);
						}
					}
				}
				else if ( newwindow == 28 )
				{
					if ( selectedEntity[0] != nullptr )
					{
						int numProperties = sizeof(playerSpawnPropertyNames) / sizeof(playerSpawnPropertyNames[0]); //find number of entries in property list
						const int lenProperties = sizeof(playerSpawnPropertyNames[0]) / sizeof(char); //find length of entry in property list
						int spacing = 36; // 36 px between each item in the list.
						int inputFieldHeader_y = suby1 + 28; // 28 px spacing from subwindow start.
						int inputField_x = subx1 + 8; // 8px spacing from subwindow start.
						int inputField_y = inputFieldHeader_y + 16;
						int inputFieldWidth = 64; // width of the text field
						int inputFieldFeedback_x = inputField_x + inputFieldWidth + 8;
						char tmpPropertyName[lenProperties] = "";
						Uint32 color = makeColorRGB(0, 255, 0);
						Uint32 colorRandom = makeColorRGB(0, 168, 255);
						Uint32 colorError = makeColorRGB(255, 0, 0);

						for ( int i = 0; i < numProperties; i++ )
						{
							int propertyInt = atoi(spriteProperties[i]);

							strcpy(tmpPropertyName, playerSpawnPropertyNames[i]);
							inputFieldHeader_y = suby1 + 28 + i * spacing;
							inputField_y = inputFieldHeader_y + 16;
							// box outlines then text
							drawDepressed(inputField_x - 4, inputField_y - 4, inputField_x - 4 + inputFieldWidth, inputField_y + 16 - 4);
							// print values on top of boxes
							printText(font8x8_bmp, inputField_x, suby1 + 44 + i * spacing, spriteProperties[i]);
							printText(font8x8_bmp, inputField_x, inputFieldHeader_y, tmpPropertyName);

							if ( errorArr[i] != 1 )
							{
								if ( i == 0 )
								{
									if ( propertyInt > 7 || propertyInt < -1 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
									else
									{
										char tmpStr[32] = "";
										switch ( propertyInt )
										{
											case -1:
												strcpy(tmpStr, "random");
												break;
											case 0:
												strcpy(tmpStr, "East");
												break;
											case 1:
												strcpy(tmpStr, "Southeast");
												break;
											case 2:
												strcpy(tmpStr, "South");
												break;
											case 3:
												strcpy(tmpStr, "Southwest");
												break;
											case 4:
												strcpy(tmpStr, "West");
												break;
											case 5:
												strcpy(tmpStr, "Northwest");
												break;
											case 6:
												strcpy(tmpStr, "North");
												break;
											case 7:
												strcpy(tmpStr, "Northeast");
												break;
											default:
												break;
										}
										printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, tmpStr);
									}
								}
								else
								{
									// enter other row entries here
								}
							}

							if ( errorMessage )
							{
								if ( errorArr[i] == 1 )
								{
									printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, colorError, "Invalid ID!");
								}
							}
						}

						propertyPageTextAndInput(numProperties, inputFieldWidth);

						if ( editproperty < numProperties )   // edit
						{
							if ( !SDL_IsTextInputActive() )
							{
								SDL_StartTextInput();
								inputstr = spriteProperties[0];
							}

							// set the maximum length allowed for user input
							inputlen = 2;
							propertyPageCursorFlash(spacing);
						}
					}
				}
				else if ( newwindow == 29 )
				{
					if ( selectedEntity[0] != nullptr )
					{
						int numProperties = sizeof(statuePropertyNames) / sizeof(statuePropertyNames[0]); //find number of entries in property list
						const int lenProperties = sizeof(statuePropertyNames[0]) / sizeof(char); //find length of entry in property list
						int spacing = 36; // 36 px between each item in the list.
						int inputFieldHeader_y = suby1 + 28; // 28 px spacing from subwindow start.
						int inputField_x = subx1 + 8; // 8px spacing from subwindow start.
						int inputField_y = inputFieldHeader_y + 16;
						int inputFieldWidth = 64; // width of the text field
						int inputFieldFeedback_x = inputField_x + inputFieldWidth + 8;
						char tmpPropertyName[lenProperties] = "";
						Uint32 color = makeColorRGB(0, 255, 0);
						Uint32 colorRandom = makeColorRGB(0, 168, 255);
						Uint32 colorError = makeColorRGB(255, 0, 0);

						for ( int i = 0; i < numProperties; i++ )
						{
							int propertyInt = atoi(spriteProperties[i]);

							strcpy(tmpPropertyName, statuePropertyNames[i]);
							inputFieldHeader_y = suby1 + 28 + i * spacing;
							inputField_y = inputFieldHeader_y + 16;
							// box outlines then text
							if ( i == 1 )
							{
								inputFieldWidth = 80; // width of the text field
							}
							else
							{
								inputFieldWidth = 64; // width of the text field
							}
							drawDepressed(inputField_x - 4, inputField_y - 4, inputField_x - 4 + inputFieldWidth, inputField_y + 16 - 4);
							// print values on top of boxes
							printText(font8x8_bmp, inputField_x, suby1 + 44 + i * spacing, spriteProperties[i]);
							printText(font8x8_bmp, inputField_x, inputFieldHeader_y, tmpPropertyName);

							if ( errorArr[i] != 1 )
							{
								if ( i == 0 )
								{
									if ( propertyInt > 3 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
									else
									{
										char tmpStr[32] = "";
										switch ( propertyInt )
										{
											case 0:
												strcpy(tmpStr, "East");
												break;
											case 1:
												strcpy(tmpStr, "South");
												break;
											case 2:
												strcpy(tmpStr, "West");
												break;
											case 3:
												strcpy(tmpStr, "North");
												break;
											default:
												break;
										}
										printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, tmpStr);
									}
								}
								else if ( i == 1 )
								{
									if ( propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
								}
								else
								{
									// enter other row entries here
								}
							}

							if ( errorMessage )
							{
								if ( errorArr[i] == 1 )
								{
									printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, colorError, "Invalid ID!");
								}
							}
						}

						propertyPageTextAndInput(numProperties, inputFieldWidth);

						if ( editproperty < numProperties )   // edit
						{
							if ( !SDL_IsTextInputActive() )
							{
								SDL_StartTextInput();
								inputstr = spriteProperties[0];
							}

							// set the maximum length allowed for user input
							inputlen = 10;
							propertyPageCursorFlash(spacing);
						}
					}
				}
				else if ( newwindow == 30 )
				{
					if ( selectedEntity[0] != nullptr )
					{
						int numProperties = sizeof(shrineTeleportPropertyNames) / sizeof(shrineTeleportPropertyNames[0]); //find number of entries in property list
						const int lenProperties = sizeof(shrineTeleportPropertyNames[0]) / sizeof(char); //find length of entry in property list
						int spacing = 36; // 36 px between each item in the list.
						int inputFieldHeader_y = suby1 + 28; // 28 px spacing from subwindow start.
						int inputField_x = subx1 + 8; // 8px spacing from subwindow start.
						int inputField_y = inputFieldHeader_y + 16;
						int inputFieldWidth = 64; // width of the text field
						int inputFieldFeedback_x = inputField_x + inputFieldWidth + 8;
						char tmpPropertyName[lenProperties] = "";
						Uint32 color = makeColorRGB(0, 255, 0);
						Uint32 colorRandom = makeColorRGB(0, 168, 255);
						Uint32 colorError = makeColorRGB(255, 0, 0);

						for ( int i = 0; i < numProperties; i++ )
						{
							int propertyInt = atoi(spriteProperties[i]);

							strcpy(tmpPropertyName, shrineTeleportPropertyNames[i]);
							inputFieldHeader_y = suby1 + 28 + i * spacing;
							inputField_y = inputFieldHeader_y + 16;
							// box outlines then text
							drawDepressed(inputField_x - 4, inputField_y - 4, inputField_x - 4 + inputFieldWidth, inputField_y + 16 - 4);
							// print values on top of boxes
							printText(font8x8_bmp, inputField_x, suby1 + 44 + i * spacing, spriteProperties[i]);
							printText(font8x8_bmp, inputField_x, inputFieldHeader_y, tmpPropertyName);

							if ( errorArr[i] != 1 )
							{
								if ( i == 0 )
								{
									if ( propertyInt > 3 || propertyInt < -1 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
									else
									{
										char tmpStr[32] = "";
										switch ( propertyInt )
										{
											case -1:
												strcpy(tmpStr, "random");
												break;
											case 0:
												strcpy(tmpStr, "East");
												break;
											case 1:
												strcpy(tmpStr, "South");
												break;
											case 2:
												strcpy(tmpStr, "West");
												break;
											case 3:
												strcpy(tmpStr, "North");
												break;
											default:
												break;
										}
										printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, tmpStr);
									}
								}
								else if ( i == 1 || i == 2 || i == 3 )
								{
									if ( propertyInt > 999 || propertyInt < -999 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
								}
								else
								{
									// enter other row entries here
								}
							}

							if ( errorMessage )
							{
								if ( errorArr[i] == 1 )
								{
									printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, colorError, "Invalid ID!");
								}
							}
						}

						propertyPageTextAndInput(numProperties, inputFieldWidth);

						if ( editproperty < numProperties )   // edit
						{
							if ( !SDL_IsTextInputActive() )
							{
								SDL_StartTextInput();
								inputstr = spriteProperties[0];
							}

							// set the maximum length allowed for user input
							inputlen = 4;
							propertyPageCursorFlash(spacing);
						}
					}
				}
				else if ( newwindow == 25 )
				{
					//if ( selectedEntity[0] != nullptr )
					//{
					//	int numProperties = sizeof(playerClassSetterPropertyNames) / sizeof(playerClassSetterPropertyNames[0]); //find number of entries in property list
					//	const int lenProperties = sizeof(playerClassSetterPropertyNames[0]) / sizeof(char); //find length of entry in property list
					//	int spacing = 36; // 36 px between each item in the list.
					//	int inputFieldHeader_y = suby1 + 28; // 28 px spacing from subwindow start.
					//	int inputField_x = subx1 + 8; // 8px spacing from subwindow start.
					//	int inputField_y = inputFieldHeader_y + 16;
					//	int inputFieldWidth = 64; // width of the text field
					//	int inputFieldFeedback_x = inputField_x + inputFieldWidth + 8;
					//	char tmpPropertyName[lenProperties] = "";
					//	Uint32 color = makeColorRGB(0, 255, 0);
					//	Uint32 colorRandom = makeColorRGB(0, 168, 255);
					//	Uint32 colorError = makeColorRGB(255, 0, 0);

					//	for ( int i = 0; i < numProperties; i++ )
					//	{
					//		int propertyInt = atoi(spriteProperties[i]);

					//		strcpy(tmpPropertyName, playerClassSetterPropertyNames[i]);
					//		inputFieldHeader_y = suby1 + 28 + i * spacing;
					//		inputField_y = inputFieldHeader_y + 16;
					//		// box outlines then text
					//		inputFieldWidth = 64; // width of the text field
					//		drawDepressed(inputField_x - 4, inputField_y - 4, inputField_x - 4 + inputFieldWidth, inputField_y + 16 - 4);
					//		// print values on top of boxes
					//		printText(font8x8_bmp, inputField_x, suby1 + 44 + i * spacing, spriteProperties[i]);
					//		printText(font8x8_bmp, inputField_x, inputFieldHeader_y, tmpPropertyName);

					//		if ( errorArr[i] != 1 )
					//		{
					//			if ( i == 0 )
					//			{
					//				if ( propertyInt < 0 || propertyInt > NUMCLASSES - 1 )
					//				{
					//					propertyPageError(i, 0); // reset to default 0.
					//				}
					//				else
					//				{
					//					printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, playerClassLangEntry(propertyInt, 0));
					//				}
					//			}
					//			else if ( i == 1 )
					//			{
					//				if ( propertyInt > 1 || propertyInt < 0 )
					//				{
					//					propertyPageError(i, 0); // reset to default 0.
					//				}
					//				else if ( propertyInt == 1 )
					//				{
					//					printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, "Trigger once only");
					//				}
					//				else if ( propertyInt == 0 )
					//				{
					//					printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, "Can re-trigger");
					//				}
					//			}
					//			else
					//			{
					//				// enter other row entries here
					//			}
					//		}

					//		if ( errorMessage )
					//		{
					//			if ( errorArr[i] == 1 )
					//			{
					//				printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, colorError, "Invalid ID!");
					//			}
					//		}
					//	}

					//	propertyPageTextAndInput(numProperties, inputFieldWidth);

					//	if ( editproperty < numProperties )   // edit
					//	{
					//		if ( !SDL_IsTextInputActive() )
					//		{
					//			SDL_StartTextInput();
					//			inputstr = spriteProperties[0];
					//		}

					//		// set the maximum length allowed for user input
					//		inputlen = 3;
					//		propertyPageCursorFlash(spacing);
					//	}
					//}
				}
				else if ( newwindow == 31 )
				{
					if ( selectedEntity[0] != nullptr )
					{
						int numProperties = sizeof(colliderDecorationPropertyNames) / sizeof(colliderDecorationPropertyNames[0]); //find number of entries in property list
						const int lenProperties = sizeof(colliderDecorationPropertyNames[0]) / sizeof(char); //find length of entry in property list
						int spacing = 36; // 36 px between each item in the list.
						int inputFieldHeader_y = suby1 + 28; // 28 px spacing from subwindow start.
						int inputField_x = subx1 + 8; // 8px spacing from subwindow start.
						int inputField_y = inputFieldHeader_y + 16;
						int inputFieldWidth = 64; // width of the text field
						int inputFieldFeedback_x = inputField_x + inputFieldWidth + 8;
						char tmpPropertyName[lenProperties] = "";
						Uint32 color = makeColorRGB(0, 255, 0);
						Uint32 colorRandom = makeColorRGB(0, 168, 255);
						Uint32 colorError = makeColorRGB(255, 0, 0);

						for ( int i = 0; i < numProperties; i++ )
						{
							int propertyInt = atoi(spriteProperties[i]);
							inputFieldWidth = 64; // width of the text field
							spacing = 36;

							inputFieldHeader_y = suby1 + 28 + i * spacing;
							inputField_y = inputFieldHeader_y + 16;

							// box outlines then text
							drawDepressed(inputField_x - 4, inputField_y - 4, inputField_x - 4 + inputFieldWidth, inputField_y + 16 - 4);
							// print values on top of boxes
							printText(font8x8_bmp, inputField_x, suby1 + 44 + i * spacing, spriteProperties[i]);
							strcpy(tmpPropertyName, colliderDecorationPropertyNames[i]);
							printText(font8x8_bmp, inputField_x, inputFieldHeader_y, tmpPropertyName);

							if ( errorArr[i] != 1 )
							{
								if ( i == 0 || i == 8 )
								{
									if ( propertyInt > 9999 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
									else if ( i == 0 )
									{
										if ( modelFileNames.find(propertyInt) != modelFileNames.end() )
										{
											std::string tmpStr = modelFileNames[propertyInt];
											printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, tmpStr.c_str());
										}
										else
										{
											printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, makeColorRGB(255, 0, 0), "Unknown Model!");
										}
									}
								}
								else if ( i == 1 )
								{
									if ( propertyInt > 7 || propertyInt < -1 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
									else
									{
										char tmpStr[32] = "";
										switch ( propertyInt )
										{
											case -1:
												strcpy(tmpStr, "random");
												break;
											case 0:
												strcpy(tmpStr, "East");
												break;
											case 1:
												strcpy(tmpStr, "Southeast");
												break;
											case 2:
												strcpy(tmpStr, "South");
												break;
											case 3:
												strcpy(tmpStr, "Southwest");
												break;
											case 4:
												strcpy(tmpStr, "West");
												break;
											case 5:
												strcpy(tmpStr, "Northwest");
												break;
											case 6:
												strcpy(tmpStr, "North");
												break;
											case 7:
												strcpy(tmpStr, "Northeast");
												break;
											default:
												break;
										}
										printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, tmpStr);
									}
								}
								else if ( i == 2 || i == 3 || i == 4 )
								{
									if ( propertyInt > 999 || propertyInt < -999 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
								}
								else if ( i == 5 )
								{
									if ( propertyInt > 1 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
									else
									{
										char tmpStr[32] = "";
										switch ( propertyInt )
										{
											case 0:
												strcpy(tmpStr, "No collision");
												break;
											case 1:
												strcpy(tmpStr, "Has collision");
												break;
											default:
												break;
										}
										printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, tmpStr);
									}
								}
								else if ( i == 6 || i == 7 )
								{
									if ( propertyInt > 8 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
								}
								else if ( i == 8 )
								{
									if ( propertyInt > 999 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
									else
									{
										char tmpStr[32] = "";
										switch ( propertyInt )
										{
											case 0:
												strcpy(tmpStr, "Unable to damage");
												break;
											case 1:
												strcpy(tmpStr, "Breakable");
												break;
											default:
												break;
										}
										printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, tmpStr);
									}
								}
								else if ( i == 9 )
								{
									if ( propertyInt > 1 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
									else
									{
										char tmpStr[32] = "";
										switch ( propertyInt )
										{
											case 0:
												strcpy(tmpStr, "Not diggable");
												break;
											case 1:
												strcpy(tmpStr, "Diggable");
												break;
											default:
												break;
										}
										printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, tmpStr);
									}
								}
								else if ( i == 10 )
								{
									if ( EditorEntityData_t::colliderData.find(propertyInt) ==
										EditorEntityData_t::colliderData.end() )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
									else
									{
										char tmpStr[64] = "";
										auto& colliderData = EditorEntityData_t::colliderData[propertyInt];
										snprintf(tmpStr, sizeof(tmpStr), "%s", colliderData.name.c_str());
										printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, tmpStr);
									}
								}
								else
								{
									// enter other row entries here
								}
							}

							if ( errorMessage )
							{
								if ( errorArr[i] == 1 )
								{
									printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, colorError, "Invalid ID!");
								}
							}
						}

						propertyPageTextAndInput(numProperties, inputFieldWidth);

						if ( editproperty < numProperties )   // edit
						{
							if ( !SDL_IsTextInputActive() )
							{
								SDL_StartTextInput();
								inputstr = spriteProperties[0];
							}

							// set the maximum length allowed for user input
							inputlen = 5;
							propertyPageCursorFlash(spacing);
						}
					}
				}
				else if ( newwindow == 32 )
				{
					if ( selectedEntity[0] != nullptr )
					{
						int numProperties = sizeof(ANDGatePropertyNames) / sizeof(ANDGatePropertyNames[0]); //find number of entries in property list
						const int lenProperties = sizeof(ANDGatePropertyNames[0]) / sizeof(char); //find length of entry in property list
						int spacing = 36; // 36 px between each item in the list.
						int inputFieldHeader_y = suby1 + 28; // 28 px spacing from subwindow start.
						int inputField_x = subx1 + 8; // 8px spacing from subwindow start.
						int inputField_y = inputFieldHeader_y + 16;
						int inputFieldWidth = 64; // width of the text field
						int inputFieldFeedback_x = inputField_x + inputFieldWidth + 8;
						char tmpPropertyName[lenProperties] = "";
						Uint32 color = makeColorRGB(0, 255, 0);
						Uint32 colorRandom = makeColorRGB(0, 168, 255);
						Uint32 colorError = makeColorRGB(255, 0, 0);

						for ( int i = 0; i < numProperties; i++ )
						{
							int propertyInt = atoi(spriteProperties[i]);

							strcpy(tmpPropertyName, ANDGatePropertyNames[i]);
							inputFieldHeader_y = suby1 + 28 + i * spacing;
							inputField_y = inputFieldHeader_y + 16;
							// box outlines then text
							drawDepressed(inputField_x - 4, inputField_y - 4, inputField_x - 4 + inputFieldWidth, inputField_y + 16 - 4);
							// print values on top of boxes
							printText(font8x8_bmp, inputField_x, suby1 + 44 + i * spacing, spriteProperties[i]);
							printText(font8x8_bmp, inputField_x, inputFieldHeader_y, tmpPropertyName);

							if ( errorArr[i] != 1 )
							{
								if ( i == 0 )
								{
									if ( propertyInt > 3 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
									else
									{
										char tmpStr[8] = "";
										switch ( propertyInt )
										{
										case 0:
											strcpy(tmpStr, "East");
											break;
										case 1:
											strcpy(tmpStr, "South");
											break;
										case 2:
											strcpy(tmpStr, "West");
											break;
										case 3:
											strcpy(tmpStr, "North");
											break;
										default:
											break;
										}
										printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, tmpStr);
									}
								}
								else if ( i == 1 )
								{
									if ( propertyInt > 9999 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
								}
								else if ( i == 2 )
								{
									if ( propertyInt > 9999 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
									else
									{
										if ( propertyInt == 0 )
										{
											printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, "output without on/off toggling");
										}
									}
								}
								else if ( i == 3 )
								{
									if ( propertyInt > 9999 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
									else
									{
										if ( propertyInt == 0 )
										{
											printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, "repeat infinite");
										}
									}
								}
								else if ( i == 4 )
								{
									if ( propertyInt > 1 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
									else
									{
										if ( propertyInt == 0 )
										{
											printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, "turn off without input signal");
										}
										else if ( propertyInt == 1 )
										{
											printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, "stay on without input signal");
										}
									}
								}
								else if ( i == 5 )
								{
									if ( propertyInt > 1 || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
									else
									{
										if ( propertyInt == 0 )
										{
											printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, "non-inverted");
										}
										else if ( propertyInt == 1 )
										{
											printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, "output inverted");
										}
									}
								}
								else
								{
									// enter other row entries here
								}
							}

							if ( errorMessage )
							{
								if ( errorArr[i] == 1 )
								{
									printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, colorError, "Invalid ID!");
								}
							}
						}

						propertyPageTextAndInput(numProperties, inputFieldWidth);

						if ( editproperty < numProperties )   // edit
						{
							if ( !SDL_IsTextInputActive() )
							{
								SDL_StartTextInput();
								inputstr = spriteProperties[0];
							}

							// set the maximum length allowed for user input
							inputlen = 4;
							propertyPageCursorFlash(spacing);
						}
					}
				}
				else if ( newwindow == 33 )
				{
					if ( selectedEntity[0] != nullptr )
					{
						int numProperties = sizeof(pressurePlatePropertyNames) / sizeof(pressurePlatePropertyNames[0]); //find number of entries in property list
						const int lenProperties = sizeof(pressurePlatePropertyNames[0]) / sizeof(char); //find length of entry in property list
						int spacing = 36; // 36 px between each item in the list.
						int inputFieldHeader_y = suby1 + 28; // 28 px spacing from subwindow start.
						int inputField_x = subx1 + 8; // 8px spacing from subwindow start.
						int inputField_y = inputFieldHeader_y + 16;
						int inputFieldWidth = 64; // width of the text field
						int inputFieldFeedback_x = inputField_x + inputFieldWidth + 8;
						char tmpPropertyName[lenProperties] = "";
						Uint32 color = makeColorRGB(0, 255, 0);
						Uint32 colorRandom = makeColorRGB(0, 168, 255);
						Uint32 colorError = makeColorRGB(255, 0, 0);

						for ( int i = 0; i < numProperties; i++ )
						{
							int propertyInt = atoi(spriteProperties[i]);

							strcpy(tmpPropertyName, pressurePlatePropertyNames[i]);
							inputFieldHeader_y = suby1 + 28 + i * spacing;
							inputField_y = inputFieldHeader_y + 16;
							// box outlines then text
							drawDepressed(inputField_x - 4, inputField_y - 4, inputField_x - 4 + inputFieldWidth, inputField_y + 16 - 4);
							// print values on top of boxes
							printText(font8x8_bmp, inputField_x, suby1 + 44 + i * spacing, spriteProperties[i]);
							printText(font8x8_bmp, inputField_x, inputFieldHeader_y, tmpPropertyName);

							if ( errorArr[i] != 1 )
							{
								if ( i == 0 )
								{
									if ( propertyInt >= Entity::PRESSURE_PLATE_ENUM_END || propertyInt < 0 )
									{
										propertyPageError(i, 0); // reset to default 0.
									}
									else
									{
										char tmpStr[32] = "";
										switch ( propertyInt )
										{
										case Entity::PRESSURE_PLATE_DEFAULT_ALL:
											strcpy(tmpStr, "Default All");
											break;
										case Entity::PRESSURE_PLATE_PLAYERS:
											strcpy(tmpStr, "Player");
											break;
										case Entity::PRESSURE_PLATE_MONSTERS:
											strcpy(tmpStr, "Any Monster");
											break;
										case Entity::PRESSURE_PLATE_ITEMS:
											strcpy(tmpStr, "Items");
											break;
										case Entity::PRESSURE_PLATE_BOULDERS:
											strcpy(tmpStr, "Boulders");
											break;
										case Entity::PRESSURE_PLATE_PLAYERS_OR_MONSTERS:
											strcpy(tmpStr, "Players/Any Monster");
											break;
										case Entity::PRESSURE_PLATE_PLAYERS_OR_ALLIES:
											strcpy(tmpStr, "Players/Ally Monster");
											break;
										case Entity::PRESSURE_PLATE_MONSTERS_NON_ALLY:
											strcpy(tmpStr, "Non-Ally Monster");
											break;
										default:
											break;
										}
										printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, color, tmpStr);
									}
								}
								else
								{
									// enter other row entries here
								}
							}

							if ( errorMessage )
							{
								if ( errorArr[i] == 1 )
								{
									printTextFormattedColor(font8x8_bmp, inputFieldFeedback_x, inputField_y, colorError, "Invalid ID!");
								}
							}
						}

						propertyPageTextAndInput(numProperties, inputFieldWidth);

						if ( editproperty < numProperties )   // edit
						{
							if ( !SDL_IsTextInputActive() )
							{
								SDL_StartTextInput();
								inputstr = spriteProperties[0];
							}

							// set the maximum length allowed for user input
							inputlen = 2;
							propertyPageCursorFlash(spacing);
						}
					}
				}
				else if ( newwindow == 16 || newwindow == 17 )
				{
					int textColumnLeft = subx1 + 16;
					int textColumnRight = (subx2 - subx1) / 2 + 300;
					int pady = suby1 + 16;
					int spacing = 0;
					Uint32 colorHeader = makeColorRGB(0, 255, 0);
					char helptext[128];

					if ( newwindow == 16 )
					{
						printTextFormattedColor(font8x8_bmp, textColumnLeft, pady + spacing, colorHeader, "Editor File Shortcuts:");
						spacing += 12;
						strcpy(helptext, "New Map:                            CTRL + N");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);
						spacing += 12;
						strcpy(helptext, "Open:                               CTRL + O");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);
						spacing += 12;
						strcpy(helptext, "Save:                               CTRL + S");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);
						spacing += 12;
						strcpy(helptext, "Change Load/Save Directory:         CTRL + D");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);
						spacing += 12;
						strcpy(helptext, "Close Window/Dialogue:              CTRL + M");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);
						spacing += 12;
						strcpy(helptext, "Delete Text:                        Backspace or Grave (`)");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);

						spacing += 16;

						printTextFormattedColor(font8x8_bmp, textColumnLeft, pady + spacing, colorHeader, "Editor Functions:");
						spacing += 12;
						strcpy(helptext, "Open Sprite Window:                 S");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);
						spacing += 12;
						strcpy(helptext, "Open Tile Window:                   T");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);
						spacing += 12;
						strcpy(helptext, "Sprite Properties:                  F2");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);
						spacing += 12;
						strcpy(helptext, "Map Properties:                     CTRL + M");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);
						spacing += 12;
						strcpy(helptext, "Delete Selected Sprite:             DEL");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);
						spacing += 12;
						strcpy(helptext, "Cycle Stacked Sprites:              C");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);

						spacing += 16;

						printTextFormattedColor(font8x8_bmp, textColumnLeft, pady + spacing, colorHeader, "Navigation:");
						spacing += 12;
						strcpy(helptext, "Move Camera/View:                   Arrow Keys");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);
						spacing += 12;
						strcpy(helptext, "Change Current Wall Layer:          SHIFT + Scrollwheel");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);
						spacing += 12;
						strcpy(helptext, "Change Current Wall Layer:          CTRL + U, CTRL + P");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);
						spacing += 12;
						strcpy(helptext, "Toggle First Person Camera:         F");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);

						spacing += 16;

						printTextFormattedColor(font8x8_bmp, textColumnLeft, pady + spacing, colorHeader, "Tile Palette (Last Used Tiles):");
						spacing += 12;
						strcpy(helptext, "Cycle Through Current Tile Palette: Scrollwheel");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);
						spacing += 12;
						strcpy(helptext, "Cycle Through All Palettes:         CTRL + Scrollwheel");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);
						spacing += 12;
						strcpy(helptext, "Choose Specific Tile In Palette:    Numpad 0-9");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);
						spacing += 12;
						strcpy(helptext, "Choose Specific Tile In Palette:    Left Click Tile");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);
						spacing += 12;
						strcpy(helptext, "Lock Changes to Current Palette:    Numpad *");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);
						spacing += 12;
						strcpy(helptext, "Go To Next Palette:                 Numpad +");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);
						spacing += 12;
						strcpy(helptext, "Go To Previous Palette:             Numpad -");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);
						spacing += 12;
						strcpy(helptext, "Clear Tile in Palette:              Right Click Tile");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);
					}
					else if ( newwindow == 17 )
					{
						printTextFormattedColor(font8x8_bmp, textColumnLeft, pady + spacing, colorHeader, "Editing Tools:");
						spacing += 20;

						printTextFormattedColor(font8x8_bmp, textColumnLeft, pady + spacing, colorHeader, "Pencil:");
						strcpy(helptext, "        Draws currently selected tile on current wall layer.");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);
						spacing += 12;
						strcpy(helptext, "   Does not select sprites. Right click sets the selected tile");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);
						spacing += 12;
						strcpy(helptext, "   under the cursor to selected.");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);
						spacing += 20;
						printTextFormattedColor(font8x8_bmp, textColumnLeft, pady + spacing, colorHeader, "Point:");
						strcpy(helptext, "       Selects sprites only. Sprites can be moved or deleted once");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);
						spacing += 12;
						strcpy(helptext, "   placed and selected with this tool. Left click selects, right");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);
						spacing += 12;
						strcpy(helptext, "   clicking duplicates a sprite and places it the cursor.");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);
						spacing += 20;
						strcpy(helptext, "   When sprites are stacked, only the lowest listed sprite is");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);
						spacing += 12;
						strcpy(helptext, "   selected. Hovering over multiple sprites and cycling with C");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);
						spacing += 12;
						strcpy(helptext, "   allows you to change the order that sprites are drawn in");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);
						spacing += 12;
						strcpy(helptext, "   the editor.");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);
						spacing += 20;
						strcpy(helptext, "   Certain sprites like monsters, chests, boulder traps, and most");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);
						spacing += 12;
						strcpy(helptext, "   Blessed Addition sprites (sprite 75 and onwards) have extra");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);
						spacing += 12;
						strcpy(helptext, "   customisable properties when F2 is pressed while the sprite");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);
						spacing += 12;
						strcpy(helptext, "   is selected using this tool. If no sprite is selected, F2");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);
						spacing += 12;
						strcpy(helptext, "   will show properties of the last sprite selected.");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);

						spacing += 20;
						printTextFormattedColor(font8x8_bmp, textColumnLeft, pady + spacing, colorHeader, "Brush:");
						strcpy(helptext, "       Same as pencil, but draws a larger area at once.");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);

						spacing += 20;
						printTextFormattedColor(font8x8_bmp, textColumnLeft, pady + spacing, colorHeader, "Select:");
						strcpy(helptext, "        Selects area of tiles or sprites. Tiles can be copied/");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);
						spacing += 12;
						strcpy(helptext, "   pasted/deleted in groups. Sprites can be moved in groups");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);
						spacing += 12;
						strcpy(helptext, "   with ALT + Arrow Keys. Selection can be moved with CTRL + ");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);
						spacing += 12;
						strcpy(helptext, "   Arrow Keys, and resized with SHIFT + Arrow Keys.\n");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);

						spacing += 20;
						printTextFormattedColor(font8x8_bmp, textColumnLeft, pady + spacing, colorHeader, "Fill:");
						strcpy(helptext, "      Fills in left-clicked area with currently selected tile.");
						printTextFormatted(font8x8_bmp, textColumnLeft, pady + spacing, helptext);
						spacing += 12;
					}
				}

				if ( keystatus[SDLK_ESCAPE] )
				{
					keystatus[SDLK_ESCAPE] = 0;
					if ( newwindow > 1 )
					{
						//buttonCloseSpriteSubwindow(NULL);
					}
					else if ( openwindow > 0 || savewindow == 1 )
					{
						buttonCloseSubwindow(NULL);
					}
					if ( newwindow == 16 || newwindow == 17 )
					{
						buttonCloseSubwindow(NULL);
					}
				}
				if ( keystatus[SDLK_RETURN] )
				{
					keystatus[SDLK_RETURN] = 0;
					if ( newwindow > 1 )
					{
						//buttonSpritePropertiesConfirm(NULL);
					}
					else if ( openwindow == 1 )
					{
						buttonOpenConfirm(NULL);
					}
					else if ( savewindow == 1 )
					{
						//buttonSaveConfirm(NULL);
					}
					if ( newwindow == 16 )
					{
						buttonEditorToolsHelp(nullptr);
					}
					else if ( newwindow == 17 )
					{
						buttonCloseSubwindow(nullptr);
					}
				}
			}
			else
			{
				if ( SDL_IsTextInputActive() )
				{
					SDL_StopTextInput();
				}

				// handle hotkeys
				if ( keystatus[SDLK_LCTRL] || keystatus[SDLK_RCTRL] )
				{
					if ( keystatus[SDLK_n] && !keystatus[SDLK_LSHIFT] && !keystatus[SDLK_RSHIFT] )
					{
						keystatus[SDLK_n] = 0;
						buttonNew(NULL);
						groupedEntities.clear();
					}
					if ( keystatus[SDLK_s] )
					{
						keystatus[SDLK_s] = 0;
						buttonSave(NULL);
					}
					if ( keystatus[SDLK_o] )
					{
						keystatus[SDLK_o] = 0;
						buttonOpen(NULL);
						groupedEntities.clear();
					}
					if ( keystatus[SDLK_x] )
					{
						keystatus[SDLK_x] = 0;
						buttonCut(NULL);
					}
					if ( keystatus[SDLK_c] )
					{
						keystatus[SDLK_c] = 0;
						buttonCopy(NULL);
						groupedEntities.clear();
					}
					if ( keystatus[SDLK_v] )
					{
						keystatus[SDLK_v] = 0;
						buttonPaste(NULL);
						groupedEntities.clear();
					}
					if ( keystatus[SDLK_a] )
					{
						keystatus[SDLK_a] = 0;
						buttonSelectAll(NULL);
						reselectEntityGroup();
					}
					if ( keystatus[SDLK_z] )
					{
						keystatus[SDLK_z] = 0;
						buttonUndo(NULL);
						groupedEntities.clear();
					}
					if ( keystatus[SDLK_y] )
					{
						keystatus[SDLK_y] = 0;
						buttonRedo(NULL);
						groupedEntities.clear();
					}
					if ( keystatus[SDLK_g] )
					{
						keystatus[SDLK_g] = 0;
						buttonGrid(NULL);
					}
					if ( keystatus[SDLK_d] )
					{
						keystatus[SDLK_d] = 0;
						buttonOpenDirectory(NULL);
					}
					if ( keystatus[SDLK_t] )
					{
						keystatus[SDLK_t] = 0;
						buttonToolbox(NULL);
					}
					if ( keystatus[SDLK_e] )
					{
						keystatus[SDLK_e] = 0;
						buttonViewSprites(NULL);
					}
					if ( keystatus[SDLK_l] )
					{
						keystatus[SDLK_l] = 0;
						buttonAllLayers(NULL);
					}
					if ( keystatus[SDLK_h] )
					{
						keystatus[SDLK_h] = 0;
						buttonHoverText(NULL);
					}
					if ( keystatus[SDLK_i] )
					{
						keystatus[SDLK_i] = 0;
						buttonStatusBar(NULL);
					}
					if ( keystatus[SDLK_m] )
					{
						keystatus[SDLK_m] = 0;
						buttonAttributes(NULL);
					}
					//Cycle layer up.
					if ( keystatus[SDLK_u] )
					{
						keystatus[SDLK_u] = 0;
						drawlayer = std::min(drawlayer + 1, MAPLAYERS - 1);
					}
					//Cycle layer down.
					if ( keystatus[SDLK_p] )
					{
						keystatus[SDLK_p] = 0;
						drawlayer = std::max(drawlayer - 1, 0);
					}
					if ( keystatus[SDLK_LSHIFT] || keystatus[SDLK_RSHIFT] )
					{
						if ( keystatus[SDLK_n] )
						{
							keystatus[SDLK_n] = 0;
							buttonClearMap(NULL);
							groupedEntities.clear();
						}
					}
					if ( keystatus[SDLK_DOWN] )
					{
						keystatus[SDLK_DOWN] = 0;
						// move selection
						if ( selectedarea_y2 < map.height - 1 )
						{
							selectedarea_y2 += 1;
							if ( selectedarea_y1 < map.height - 1 )
							{
								selectedarea_y1 += 1;
							}
							reselectEntityGroup();
						}
					}
					else if ( keystatus[SDLK_UP] )
					{
						keystatus[SDLK_UP] = 0;
						// move selection
						if ( selectedarea_y1 > 0 )
						{
							selectedarea_y1 -= 1;
							if ( selectedarea_y2 > 0 )
							{
								selectedarea_y2 -= 1;
							}
							reselectEntityGroup();
						}
					}
					else if ( keystatus[SDLK_LEFT] )
					{
						keystatus[SDLK_LEFT] = 0;
						// move selection
						if ( selectedarea_x1 > 0 )
						{
							selectedarea_x1 -= 1;
							if ( selectedarea_x2 > 0 )
							{
								selectedarea_x2 -= 1;
							}
							reselectEntityGroup();
						}
					}
					else if ( keystatus[SDLK_RIGHT] )
					{
						keystatus[SDLK_RIGHT] = 0;
						// move selection
						if ( selectedarea_x2 < map.width - 1 )
						{
							selectedarea_x2 += 1;
							if ( selectedarea_x1 < map.width - 1 )
							{
								selectedarea_x1 += 1;
							}
							reselectEntityGroup();
						}
					}
				}
				else
				{
					if ( keystatus[SDLK_LSHIFT] || keystatus[SDLK_RSHIFT] )
					{
						if ( keystatus[SDLK_DOWN] )
						{
							keystatus[SDLK_DOWN] = 0;
							// resize selection
							if ( selectedarea_y2 < map.height - 1 && !moveSelectionNegativeY )
							{
								selectedarea_y2 += 1;
								reselectEntityGroup();
							}
							else if ( selectedarea_y1 < selectedarea_y2
								&& selectedarea_y1 < map.height - 1 && moveSelectionNegativeY )
							{
								selectedarea_y1 += 1;
								reselectEntityGroup();
							}
							else if ( selectedarea_y1 == selectedarea_y2 )
							{
								moveSelectionNegativeY = false;
								if ( selectedarea_y2 < map.height - 1 )
								{
									selectedarea_y2 += 1;
									reselectEntityGroup();
								}
							}
						}
						else if ( keystatus[SDLK_UP] )
						{
							keystatus[SDLK_UP] = 0;
							// resize selection
							if ( selectedarea_y2 > selectedarea_y1 && !moveSelectionNegativeY )
							{
								selectedarea_y2 -= 1;
								reselectEntityGroup();
							}
							else if ( selectedarea_y1 < selectedarea_y2 
								&& selectedarea_y1 > 0 && moveSelectionNegativeY )
							{
								selectedarea_y1 -= 1;
								reselectEntityGroup();
							}
							else if ( selectedarea_y1 == selectedarea_y2 )
							{
								moveSelectionNegativeY = true;
								if ( selectedarea_y1 > 0 )
								{
									selectedarea_y1 -= 1;
									reselectEntityGroup();
								}
							}
						}
						else if ( keystatus[SDLK_LEFT] )
						{
							keystatus[SDLK_LEFT] = 0;
							// resize selection
							if ( selectedarea_x2 > selectedarea_x1 && !moveSelectionNegativeX )
							{
								selectedarea_x2 -= 1;
								reselectEntityGroup();
							}
							else if ( selectedarea_x1 < selectedarea_x2
								&& selectedarea_x1 > 0 && moveSelectionNegativeX )
							{
								selectedarea_x1 -= 1;
								reselectEntityGroup();
							}
							else if ( selectedarea_x1 == selectedarea_x2 )
							{
								moveSelectionNegativeX = true;
								if ( selectedarea_x1 > 0 )
								{
									selectedarea_x1 -= 1;
									reselectEntityGroup();
								}
							}
						}
						else if ( keystatus[SDLK_RIGHT] )
						{
							keystatus[SDLK_RIGHT] = 0;
							// resize selection
							if ( selectedarea_x2 < map.width - 1 && !moveSelectionNegativeX)
							{
								selectedarea_x2 += 1;
								reselectEntityGroup();
							}
							else if ( selectedarea_x1 < selectedarea_x2
								&& selectedarea_x1 < map.width - 1 && moveSelectionNegativeX )
							{
								selectedarea_x1 += 1;
								reselectEntityGroup();
							}
							else if ( selectedarea_x1 == selectedarea_x2 )
							{
								moveSelectionNegativeX = false;
								if ( selectedarea_x2 < map.width - 1 )
								{
									selectedarea_x2 += 1;
									reselectEntityGroup();
								}
							}
						}
					}
					if ( keystatus[SDLK_s] )
					{
						keystatus[SDLK_s] = 0;
						spritepalette = 1;
					}
					if ( keystatus[SDLK_t] )
					{
						keystatus[SDLK_t] = 0;
						tilepalette = 1;
					}
					if ( keystatus[SDLK_f] )
					{
						keystatus[SDLK_f] = 0;
						button3DMode(NULL);
					}
				}
				if ( keystatus[SDLK_LALT] || keystatus[SDLK_RALT] )
				{
					if ( keystatus[SDLK_f] )
					{
						keystatus[SDLK_f] = 0;
						menuVisible = 1;
					}
					if ( keystatus[SDLK_e] )
					{
						keystatus[SDLK_e] = 0;
						menuVisible = 2;
					}
					if ( keystatus[SDLK_v] )
					{
						keystatus[SDLK_v] = 0;
						menuVisible = 3;
					}
					if ( keystatus[SDLK_m] )
					{
						keystatus[SDLK_m] = 0;
						menuVisible = 4;
					}
					if ( keystatus[SDLK_h] )
					{
						keystatus[SDLK_h] = 0;
						menuVisible = 5;
					}
					if ( keystatus[SDLK_F4] )
					{
						keystatus[SDLK_F4] = 0;
						buttonExit(NULL);
					}
					if ( keystatus[SDLK_DOWN] )
					{
						keystatus[SDLK_DOWN] = 0;
						// move entities
						makeUndo();
						if ( selectedarea_y2 < map.height - 1 )
						{
							for ( std::vector<Entity*>::iterator it = groupedEntities.begin(); it != groupedEntities.end(); ++it )
							{
								Entity* tmpEntity = *it;
								tmpEntity->y += 16;
							}
							selectedarea_y2 += 1;
							if ( selectedarea_y1 < map.height - 1 )
							{
								selectedarea_y1 += 1;
							}
						}
					}
					else if ( keystatus[SDLK_UP] )
					{
						keystatus[SDLK_UP] = 0;
						// move entities
						makeUndo();
						if ( selectedarea_y1 > 0 )
						{
							for ( std::vector<Entity*>::iterator it = groupedEntities.begin(); it != groupedEntities.end(); ++it )
							{
								Entity* tmpEntity = *it;
								tmpEntity->y -= 16;
							}
							selectedarea_y1 -= 1;
							if ( selectedarea_y2 > 0 )
							{
								selectedarea_y2 -= 1;
							}
						}
					}
					else if ( keystatus[SDLK_LEFT] )
					{
						keystatus[SDLK_LEFT] = 0;
						// move entities
						makeUndo();
						if ( selectedarea_x1 > 0 )
						{
							for ( std::vector<Entity*>::iterator it = groupedEntities.begin(); it != groupedEntities.end(); ++it )
							{
								Entity* tmpEntity = *it;
								tmpEntity->x -= 16;
							}
							selectedarea_x1 -= 1;
							if ( selectedarea_x2 > 0 )
							{
								selectedarea_x2 -= 1;
							}
						}
					}
					else if ( keystatus[SDLK_RIGHT] )
					{
						keystatus[SDLK_RIGHT] = 0;
						// move entities
						makeUndo();
						if ( selectedarea_x2 < map.width - 1 )
						{
							for ( std::vector<Entity*>::iterator it = groupedEntities.begin(); it != groupedEntities.end(); ++it )
							{
								Entity* tmpEntity = *it;
								tmpEntity->x += 16;
							}
							selectedarea_x2 += 1;
							if ( selectedarea_x1 < map.width - 1 )
							{
								selectedarea_x1 += 1;
							}
						}
					}
				}
				if ( keystatus[SDLK_DELETE] )
				{
					keystatus[SDLK_DELETE] = 0;
					buttonDelete(NULL);
					groupedEntities.clear();
				}
				if ( keystatus[SDLK_c] )
				{
					keystatus[SDLK_c] = 0;
					buttonCycleSprites(NULL);
				}
				if ( keystatus[SDLK_F1] )
				{
					keystatus[SDLK_F1] = 0;
					buttonAbout(NULL);
				}
				if ( keystatus[SDLK_h] )
				{
					keystatus[SDLK_h] = 0;
					buttonEditorControls(NULL);
				}
				if ( keystatus[SDLK_1] ) // Switch to Pencil Tool
				{
					keystatus[SDLK_1] = 0;
					selectedTool = 0;
					selectedarea = false;
				}
				if ( keystatus[SDLK_2] ) // Switch to Point Tool
				{
					keystatus[SDLK_2] = 0;
					selectedTool = 1;
					selectedarea = false;
				}
				if ( keystatus[SDLK_3] ) // Switch to Brush Tool
				{
					keystatus[SDLK_3] = 0;
					selectedTool = 2;
					selectedarea = false;
				}
				if ( keystatus[SDLK_4] ) // Switch to Select Tool
				{
					keystatus[SDLK_4] = 0;
					selectedTool = 3;
					selectedarea = false;
				}
				if ( keystatus[SDLK_5] ) // Switch to Fill Tool
				{
					keystatus[SDLK_5] = 0;
					selectedTool = 4;
					selectedarea = false;
				}
				if ( keystatus[SDLK_F2] )
				{
					keystatus[SDLK_F2] = 0;
					makeUndo();
					buttonSpriteProperties(NULL);
				}
				if ( keystatus[SDLK_KP_7] )
				{
					keystatus[SDLK_KP_7] = 0;
					selectedTile = recentUsedTiles[recentUsedTilePalette][0];
				}
				if ( keystatus[SDLK_KP_8] )
				{
					keystatus[SDLK_KP_8] = 0;
					selectedTile = recentUsedTiles[recentUsedTilePalette][1];
				}
				if ( keystatus[SDLK_KP_9] )
				{
					keystatus[SDLK_KP_9] = 0;
					selectedTile = recentUsedTiles[recentUsedTilePalette][2];
				}
				if ( keystatus[SDLK_KP_4] )
				{
					keystatus[SDLK_KP_4] = 0;
					selectedTile = recentUsedTiles[recentUsedTilePalette][3];
				}
				if ( keystatus[SDLK_KP_5] )
				{
					keystatus[SDLK_KP_5] = 0;
					selectedTile = recentUsedTiles[recentUsedTilePalette][4];
				}
				if ( keystatus[SDLK_KP_6] )
				{
					keystatus[SDLK_KP_6] = 0;
					selectedTile = recentUsedTiles[recentUsedTilePalette][5];
				}
				if ( keystatus[SDLK_KP_1] )
				{
					keystatus[SDLK_KP_1] = 0;
					selectedTile = recentUsedTiles[recentUsedTilePalette][6];
				}
				if ( keystatus[SDLK_KP_2] )
				{
					keystatus[SDLK_KP_2] = 0;
					selectedTile = recentUsedTiles[recentUsedTilePalette][7];
				}
				if ( keystatus[SDLK_KP_3] )
				{
					keystatus[SDLK_KP_3] = 0;
					selectedTile = recentUsedTiles[recentUsedTilePalette][8];
				}
				if ( keystatus[SDLK_KP_PLUS] )
				{
					keystatus[SDLK_KP_PLUS] = 0;
					recentUsedTilePalette++; //scroll through palettes 1-9
					if ( recentUsedTilePalette == 9 )
					{
						recentUsedTilePalette = 0;
					}
				}
				if ( keystatus[SDLK_KP_MINUS] )
				{
					keystatus[SDLK_KP_MINUS] = 0;
					recentUsedTilePalette--; //scroll through palettes 1-9
					if ( recentUsedTilePalette == -1 )
					{
						recentUsedTilePalette = 8;
					}
				}
				if ( keystatus[SDLK_KP_MULTIPLY] )
				{
					keystatus[SDLK_KP_MULTIPLY] = 0;
					lockTilePalette[recentUsedTilePalette] = !lockTilePalette[recentUsedTilePalette]; // toggle lock/unlock
				}
			}
			// process and draw buttons
			handleButtons();
		}

		if ( spritepalette )
		{
			x = 0;
			y = 0;
			z = 0;
			drawRect( NULL, makeColorRGB(0, 0, 0), 255 ); // wipe screen
			for ( c = 0; c < xres * yres; c++ )
			{
				palette[c] = -1;
			}
			for ( c = 0; c < numsprites; c++ )
			{
				if ( sprites[c] != NULL )
				{
					pos.x = x;
					pos.y = y;
					pos.w = sprites[c]->w;
					pos.h = sprites[c]->h;
					int scale = 1;
					if ( pos.w < 16 && pos.h < 16 )
					{
						scale = 4;
						pos.w *= scale;
						pos.h *= scale;
					}
					else if ( pos.w < 32 && pos.h < 32 )
					{
						scale = 2;
						pos.w *= scale;
						pos.h *= scale;
					}

					drawImageScaled(sprites[c], NULL, &pos);
					for ( x2 = x; x2 < x + sprites[c]->w * scale; x2++ )
					{
						for ( y2 = y; y2 < y + sprites[c]->h * scale; y2++ )
						{
							if ( x2 < xres && y2 < yres )
							{
								palette[y2 + x2 * yres] = c;
							}
						}
					}
					x += sprites[c]->w * scale;
					z = std::max(z, sprites[c]->h * scale);
					if ( c < numsprites - 1 )
					{
						if ( sprites[c + 1] != NULL )
						{
							if ( x + sprites[c + 1]->w * scale > xres )
							{
								x = 0;
								y += z;
							}
						}
						else
						{
							if ( x + sprites[0]->w * scale > xres )
							{
								x = 0;
								y += z;
							}
						}
					}
				}
				else
				{
					pos.x = x;
					pos.y = y;
					pos.w = TEXTURESIZE;
					pos.h = TEXTURESIZE;
					drawImageScaled(sprites[0], NULL, &pos);
					x += sprites[0]->w;
					z = std::max(z, sprites[0]->h);
					if ( c < numsprites - 1 )
					{
						if ( sprites[c + 1] != NULL )
						{
							if ( x + sprites[c + 1]->w > xres )
							{
								x = 0;
								y += z;
							}
						}
						else
						{
							if ( x + sprites[0]->w > xres )
							{
								x = 0;
								y += z;
							}
						}
					}
				}
			}
			if (mousestatus[SDL_BUTTON_LEFT])
			{
				mclick = 1;
			}
			if (!mousestatus[SDL_BUTTON_LEFT] && mclick)
			{
				// create a new object
				if (palette[mousey + mousex * yres] >= 0)
				{
					entity = newEntity(palette[mousey + mousex * yres], 0, map.entities, nullptr);
					selectedEntity[0] = entity;
					lastSelectedEntity[0] = selectedEntity[0];
					setSpriteAttributes(selectedEntity[0], nullptr, nullptr);
				}

				mclick = 0;
				spritepalette = 0;
			}
			if (keystatus[SDLK_ESCAPE])
			{
				mclick = 0;
				spritepalette = 0;
			}
			/*switch( palette[mousey+mousex*yres] ) {
				case 1:	strcpy(action,"PLAYER"); break;
				case 53:	strcpy(action,"PURPLEGEM"); break;
				case 37:	strcpy(action,"REDGEM"); break;
				case 74:
				case 75:	strcpy(action,"TROLL"); break;
				default:	strcpy(action,"STATIC"); break;
			}*/

			int numsprites = static_cast<int>(sizeof(spriteEditorNameStrings) / sizeof(spriteEditorNameStrings[0]));

			if ( (mousex <= xres && mousey <= yres) && palette[mousey + mousex * yres] >= 0 && palette[mousey + mousex * yres] <= numsprites )
			{
				printTextFormatted(font8x8_bmp, 0, yres - 8, "Sprite index:%5d", palette[mousey + mousex * yres]);
				printTextFormatted(font8x8_bmp, 0, yres - 16, "%s", spriteEditorNameStrings[palette[mousey + mousex * yres]]);

				char hoverTextString[1024] = "";
				snprintf(hoverTextString, 5, "%d: ", palette[mousey + mousex * yres]);
				strcat(hoverTextString, spriteEditorNameStrings[palette[mousey + mousex * yres]]);
				int hoverTextWidth = strlen(hoverTextString);

				if ( mousey - 20 <= 0 )
				{
					if ( mousex + 16 + 8 * hoverTextWidth >= xres )
					{
						// stop text being drawn above y = 0 and past window width (xres)
						drawWindowFancy(mousex - 16 - (8 + 8 * hoverTextWidth), 0, mousex - 16, 16);
						printTextFormatted(font8x8_bmp, mousex - 16 - (4 + 8 * hoverTextWidth), 4, "%s", hoverTextString);
					}
					else
					{
						// stop text being drawn above y = 0 
						drawWindowFancy(mousex + 16, 0, 16 + 8 + mousex + 8 * hoverTextWidth, 16);
						printTextFormatted(font8x8_bmp, mousex + 16 + 4, 4, "%s", hoverTextString);
					}
				}
				else
				{
					if ( mousex + 16 + 8 * hoverTextWidth >= xres )
					{
						// stop text being drawn past window width (xres)
						drawWindowFancy(xres - (8 + 8 * hoverTextWidth), mousey - 20, xres, mousey - 4);
						printTextFormatted(font8x8_bmp, xres - (4 + 8 * hoverTextWidth), mousey - 16, "%s", hoverTextString);
					}
					else
					{
						drawWindowFancy(mousex + 16, mousey - 20, 16 + 8 + mousex + 8 * hoverTextWidth, mousey - 4);
						printTextFormatted(font8x8_bmp, mousex + 16 + 4, mousey - 16, "%s", hoverTextString);
					}
				}
			}
			else
			{
				printText(font8x8_bmp, 0, yres - 8, "Click to cancel");
			}
		}
		if ( tilepalette )
		{
			x = 0;
			y = 0;
			drawRect( NULL, makeColorRGB(0, 0, 0), 255 ); // wipe screen
			for ( c = 0; c < xres * yres; c++ )
			{
				palette[c] = -1;
			}
			for ( c = 0; c < numtiles; c++ )
			{
				pos.x = x;
				pos.y = y;
				pos.w = TEXTURESIZE;
				pos.h = TEXTURESIZE;
				if ( tiles[c] != NULL )
				{
					drawImageScaled(tiles[c], NULL, &pos);
					for ( x2 = x; x2 < x + TEXTURESIZE; x2++ )
						for ( y2 = y; y2 < y + TEXTURESIZE; y2++ )
						{
							if ( x2 < xres && y2 < yres )
							{
								palette[y2 + x2 * yres] = c;
							}
						}
					x += TEXTURESIZE;
					if ( c < numtiles - 1 )
					{
						if ( x + TEXTURESIZE > xres )
						{
							x = 0;
							y += TEXTURESIZE;
						}
					}
				}
				else
				{
					drawImageScaled(sprites[0], NULL, &pos);
					x += TEXTURESIZE;
					if ( c < numtiles - 1 )
					{
						if ( x + TEXTURESIZE > xres )
						{
							x = 0;
							y += TEXTURESIZE;
						}
					}
				}
			}
			if (mousestatus[SDL_BUTTON_LEFT])
			{
				mclick = 1;
			}
			if (!mousestatus[SDL_BUTTON_LEFT] && mclick)
			{
				// select the tile under the mouse
				if ( (mousex <= xres && mousey <= yres) && palette[mousey + mousex * yres] >= 0)
				{
					selectedTile = palette[mousey + mousex * yres];
					updateRecentTileList(selectedTile);
				}
				mclick = 0;
				tilepalette = 0;
			}
			if (keystatus[SDLK_ESCAPE])
			{
				mclick = 0;
				tilepalette = 0;
			}

			int numtiles = static_cast<int>(sizeof(tileEditorNameStrings) / sizeof(tileEditorNameStrings[0]));

			if ( (mousex <= xres && mousey <= yres) && palette[mousey + mousex * yres] >= 0 && palette[mousey + mousex * yres] <= numtiles)
			{
				printTextFormatted(font8x8_bmp, 0, yres - 8, "Tile index:%5d", palette[mousey + mousex * yres]);
				printTextFormatted(font8x8_bmp, 0, yres - 16, "%s", tileEditorNameStrings[palette[mousey + mousex * yres]]);

				char hoverTextString[1024] = "";
				snprintf(hoverTextString, 5, "%d: ", palette[mousey + mousex * yres]);
				strcat(hoverTextString, tileEditorNameStrings[palette[mousey + mousex * yres]]);
				int hoverTextWidth = strlen(hoverTextString);

				if ( mousey - 20 <= 0 )
				{
					if ( mousex + 16 + 8 * hoverTextWidth >= xres )
					{
						// stop text being drawn above y = 0 and past window width (xres)
						drawWindowFancy(mousex - 16 - (8 + 8 * hoverTextWidth), 0, mousex - 16, 16);
						printTextFormatted(font8x8_bmp, mousex - 16 - (4 + 8 * hoverTextWidth), 4, "%s", hoverTextString);
					}
					else
					{
						// stop text being drawn above y = 0 
						drawWindowFancy(mousex + 16, 0, 16 + 8 + mousex + 8 * hoverTextWidth, 16);
						printTextFormatted(font8x8_bmp, mousex + 16 + 4, 4, "%s", hoverTextString);
					}
				}
				else
				{
					if ( mousex + 16 + 8 * hoverTextWidth >= xres )
					{
						// stop text being drawn past window width (xres)
						drawWindowFancy(xres - (8 + 8 * hoverTextWidth), mousey - 20, xres, mousey - 4);
						printTextFormatted(font8x8_bmp, xres - (4 + 8 * hoverTextWidth), mousey - 16, "%s", hoverTextString);
					}
					else
					{
						drawWindowFancy(mousex + 16, mousey - 20, 16 + 8 + mousex + 8 * hoverTextWidth, mousey - 4);
						printTextFormatted(font8x8_bmp, mousex + 16 + 4, mousey - 16, "%s", hoverTextString);
					}
				}
			}
			else
			{
				printText(font8x8_bmp, 0, yres - 8, "Click to cancel");
			}
		}

		// flip screen
		GO_SwapBuffers(screen);
		cycles++;
	}

	// deinit
	SDL_SetCursor(cursorArrow);
	SDL_FreeCursor(cursorPencil);
	SDL_FreeCursor(cursorPoint);
	SDL_FreeCursor(cursorBrush);
	SDL_FreeCursor(cursorFill);
	if ( palette != NULL )
	{
		free(palette);
	}
	if ( copymap.tiles != NULL )
	{
		free(copymap.tiles);
	}
	list_FreeAll(&undolist);
	saveTilePalettes();
    for (int c = 0; c < sizeof(view_t::fb) / sizeof(view_t::fb[0]); ++c) {
        camera.fb[c].destroy();
    }
	return deinitApp();
}

void propertyPageTextAndInput(int numProperties, int width)
{
	int pad_x1 = subx1 + 8;
	int spacing = 36;
	int pad_x2 = width;

	// Cycle properties with TAB.
	if ( keystatus[SDLK_TAB] )
	{
		keystatus[SDLK_TAB] = 0;
		cursorflash = ticks;
		editproperty++;
		if ( editproperty == numProperties )
		{
			editproperty = 0;
		}

		inputstr = spriteProperties[editproperty];
	}

	// select a textbox
	if ( mousestatus[SDL_BUTTON_LEFT] )
	{
		for ( int i = 0; i < numProperties; i++ )
		{
			if ( omousex >= pad_x1 - 4 && omousey >= suby1 + 40 + i * spacing && omousex < pad_x1 - 4 + pad_x2 && omousey < suby1 + 56 + i * spacing )
			{
				inputstr = spriteProperties[i];
				editproperty = i;
				cursorflash = ticks;
			}
		}
	}
}

void propertyPageError(int rowIndex, int resetValue)
{
	errorMessage = 60;
	errorArr[rowIndex] = 1;
	snprintf(spriteProperties[rowIndex], sizeof(spriteProperties[rowIndex]), "%d", resetValue); //reset
}

void propertyPageCursorFlash(int rowSpacing)
{
	if ( (ticks - cursorflash) % TICKS_PER_SECOND < TICKS_PER_SECOND / 2 )
	{
		printText(font8x8_bmp, subx1 + 8 + strlen(spriteProperties[editproperty]) * 8, suby1 + 44 + editproperty * rowSpacing, "\26");
	}
}

void reselectEntityGroup()
{
	groupedEntities.clear();
	node_t* nextnode = nullptr;
	Entity* entity = nullptr;
	for ( node_t* node = map.entities->first; node != nullptr; node = nextnode )
	{
		nextnode = node->next;
		entity = (Entity*)node->element;
		if ( entity->x / 16 >= selectedarea_x1 && entity->x / 16 <= selectedarea_x2
			&& entity->y / 16 >= selectedarea_y1 && entity->y / 16 <= selectedarea_y2 )
		{
			groupedEntities.push_back(entity);
		}
	}
}

int generateDungeon(char* levelset, Uint32 seed, std::tuple<int, int, int, int> mapParameters)
{
	return 0; // dummy function
}
