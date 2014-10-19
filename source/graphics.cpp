#include "mines.hpp"

// InitVideo: Sets all video modes and loads all graphics into memory but displays nothing.
void InitVideo(SpriteEntry *eSprites)
{
	int i;

	vramSetBankA(VRAM_A_MAIN_BG_0x06000000);
	vramSetBankB(VRAM_B_MAIN_SPRITE);
	vramSetBankC(VRAM_C_SUB_BG_0x06200000);
	vramSetBankD(VRAM_D_SUB_SPRITE);
	videoSetMode(MODE_2_2D | DISPLAY_BG2_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D | DISPLAY_SPR_1D_SIZE_256);
	videoSetModeSub(MODE_5_2D | DISPLAY_BG2_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D | DISPLAY_SPR_1D_SIZE_256);

	// First: Main screen graphics. Load tiles for displaying mines.
	// Set up main BG 2 for displaying tiles.

	// A map is 256 bytes/1 KB/4 KB/16 KB big, accomodating 16x16/32x32/64x64/128x128 arrays of tiles
	// 256 total tiles can be addressed because 1 byte is being allocated in the map for each tile
	// Each tile is 8x8 pixels
	// Each tile "base" is a block of 16 KB of data. There are 16 total.
	// Here we make the map base 0, so 1 KB of data located at 0x0600 0000 is our map
	// We make the tile base 1, so 16 KB of data located at 0x0600 4000 is our tile data
	// A 16 KB block of tile data accomodates 256 tiles, each made up of 64 bytes (i.e. 8x8 pixels and 1 byte color data)
	BG2_CR = BG_RS_32x32 | BG_PRIORITY(3) | BG_TILE_BASE(1) | BG_MAP_BASE(0) | BG_WRAP_ON;
	BG2_XDX = 0x0100; BG2_XDY = 0; BG2_YDX = 0; BG2_YDY = 0x0100;

	dmaCopy(mines_bin, (uint16 *)BG_TILE_RAM(1), mines_bin_size);
	dmaCopy(minespal_bin, (uint16 *)BG_PALETTE, minespal_bin_size);
	// Tile #15 will be a black tile.
	for (i=0; i<256; i++)
		((uint16 *)BG_TILE_RAM(1))[(mines_bin_size + i)/2] = 9 | (9 << 8);

	for (i=0; i<32*32/2; i++)
		((uint16 *)BG_MAP_RAM(0))[i] = 14*4 | (14*4 << 8);


	// Second: Sub screen graphics. Load splash bitmap and sprites.
	// Set up sub BG 2 for displaying splash screen.
	SUB_BG2_CR = BG_BMP16_256x256 | BG_BMP_BASE(0) | BG_PRIORITY(3);
	SUB_BG2_XDX = 0x0100; SUB_BG2_XDY = 0; SUB_BG2_YDX = 0; SUB_BG2_YDY = 0x0100;

	for (i=0; i<256*256; i++)
		((uint16 *)BG_BMP_RAM_SUB(0))[i] = ((uint16 *)splash_bin)[i] | BIT(15);

	LoadSprites(eSprites);
}


// UpdateOAM: Updates sprite positions for sub screen.
// The only main screen sprites are animated arrows. They are updated elsewhere.
void UpdateOAM(SpriteEntry *eSprites)
{
	DC_FlushAll();
	dmaCopy(eSprites, OAM_SUB, 128*sizeof(SpriteEntry));
}


// ClearOAM: Disables all sprites.
void ClearOAM(SpriteEntry *eSprites)
{
	int i;

	for (i=0; i<128; i++)
	{
		eSprites[i].attribute[0] = ATTR0_DISABLED;
		eSprites[i].attribute[1] = 0;
		eSprites[i].attribute[2] = 0;
	}

	for (i=0; i<32; i++)
	{
		((SpriteRotation*)eSprites)[i].hdx = 256;
		((SpriteRotation*)eSprites)[i].hdy = 0;
		((SpriteRotation*)eSprites)[i].vdx = 0;
		((SpriteRotation*)eSprites)[i].vdy = 256;
	}

	UpdateOAM(eSprites);
}


// LoadSprites: Reads sprite graphics into memory but displays nothing.
// Number sprites are stored as 8 bit bitmaps. Each is 32x32.
void LoadSprites(SpriteEntry *eSprites)
{
	ClearOAM(eSprites);
	dmaCopy(palette_bin, SPRITE_PALETTE_SUB, palette_bin_size);
	dmaCopy(numbers_bin, SPRITE_GFX_SUB, numbers_bin_size);
	dmaCopy(words_bin, SPRITE_GFX_SUB + numbers_bin_size/2, words_bin_size);
	dmaCopy(gameover_bin, SPRITE_GFX_SUB + numbers_bin_size/2 + words_bin_size/2, gameover_bin_size);
	UpdateOAM(eSprites);

	dmaCopy(arrowpalette_bin, SPRITE_PALETTE, arrowpalette_bin_size);
	dmaCopy(arrow_bin, ((uint16*)0x6420000), arrow_bin_size); // Load arrow sprite into main sprite memory B.

	// Left
	((SpriteEntry*)OAM)[0].attribute[0] = ATTR0_COLOR_256 | ATTR0_DISABLED | 80;
	((SpriteEntry*)OAM)[0].attribute[1] = ATTR1_SIZE_16 | 0;
	((SpriteEntry*)OAM)[0].attribute[2] = 512;

	// Right
	((SpriteEntry*)OAM)[1].attribute[0] = ATTR0_COLOR_256 | ATTR0_DISABLED | 80;
	((SpriteEntry*)OAM)[1].attribute[1] = ATTR1_SIZE_16 | ATTR1_FLIP_X | 256-16;
	((SpriteEntry*)OAM)[1].attribute[2] = 512;

	// Up
	((SpriteEntry*)OAM)[2].attribute[0] = ATTR0_COLOR_256 | ATTR0_DISABLED | 0;
	((SpriteEntry*)OAM)[2].attribute[1] = ATTR1_SIZE_16 | ATTR1_ROTDATA(0) | 120;
	((SpriteEntry*)OAM)[2].attribute[2] = 512;

	// Down
	((SpriteEntry*)OAM)[3].attribute[0] = ATTR0_COLOR_256 | ATTR0_DISABLED | 192-16;
	((SpriteEntry*)OAM)[3].attribute[1] = ATTR1_SIZE_16 | ATTR1_ROTDATA(1) | 120;
	((SpriteEntry*)OAM)[3].attribute[2] = 512;

	((SpriteRotation*)OAM)[0].hdx = 0;
	((SpriteRotation*)OAM)[0].hdy = 256;
	((SpriteRotation*)OAM)[0].vdx = -256;
	((SpriteRotation*)OAM)[0].vdy = 0;

	((SpriteRotation*)OAM)[1].hdx = 0;
	((SpriteRotation*)OAM)[1].hdy = -256;
	((SpriteRotation*)OAM)[1].vdx = 256;
	((SpriteRotation*)OAM)[1].vdy = 0;
}


void AnimateArrows(int *nDirection)
{
	int nPosition;

	if (*nDirection != DIR_INWARD && *nDirection != DIR_OUTWARD) return;

	// Find out where the arrows are. All I need to know is the left arrow's position
	// relative to the left edge of the screen. All arrows are synchronized.
	nPosition = (((SpriteEntry*)OAM)[0].attribute[1] & 0x1FF) - *nDirection;

	// Erase all position data
	((SpriteEntry*)OAM)[0].attribute[1] &= 0xFE00;
	((SpriteEntry*)OAM)[1].attribute[1] &= 0xFE00;
	((SpriteEntry*)OAM)[2].attribute[0] &= 0xFF00;
	((SpriteEntry*)OAM)[3].attribute[0] &= 0xFF00;

	((SpriteEntry*)OAM)[0].attribute[1] |= nPosition;
	((SpriteEntry*)OAM)[1].attribute[1] |= 256 - 16 - nPosition;
	((SpriteEntry*)OAM)[2].attribute[0] |= nPosition;
	((SpriteEntry*)OAM)[3].attribute[0] |= 192 - 16 - nPosition;

	// If we have travelled 5 pixels, switch direction.
	if (nPosition == 5) *nDirection = DIR_OUTWARD;
	if (nPosition == 0) *nDirection = DIR_INWARD;
}


// UpdateMenuSprites: Put information in OAM to show the setup menu.
// Sprites are laid out like
// row 1:    [MINES 0]         [LEFT 1] [# 2] [# 3] [RIGHT 4]
// row 2:    [ROWS 5]          [LEFT 6] [# 7] [# 8] [RIGHT 9]
// row 3:    [COLS 10]         [LEFT 11] [# 12] [# 13] [RIGHT 14]
// row 5:    [START 15]
void UpdateMenuSprites(SpriteEntry *eSprites, int nMenuSel, int nMines, int nRows, int nCols)
{
	int i, nSelected, *nValues[3];

	// nValues is an array of pointers to make the loop code for the three rows
	// a bit more uniform. i is the number of the row, and nValues[i] is a pointer
	// to the variable containing the information to be shown on that row.
	nValues[0] = &nMines;
	nValues[1] = &nRows;
	nValues[2] = &nCols;

	// Start
	eSprites[15].attribute[0] = ATTR0_COLOR_256 | ATTR0_NORMAL | ATTR0_TYPE_BLENDED | ATTR0_WIDE | 150;
	eSprites[15].attribute[1] = ATTR1_SIZE_64 | 96;
	eSprites[15].attribute[2] = (22 + (nMenuSel == MENU_START ? 2 : 0))*4;

	for (i=0; i<3; i++)
	{
		if (i == nMenuSel) nSelected = 1; else nSelected = 0;

		// Mines/Rows/Cols header
		eSprites[5*i].attribute[0] = ATTR0_COLOR_256 | ATTR0_NORMAL | ATTR0_TYPE_BLENDED | ATTR0_WIDE | (20+40*i); // Y = 20+40*i
		eSprites[5*i].attribute[1] = ATTR1_SIZE_64 | 20; // X = 20
		eSprites[5*i].attribute[2] = (26+2*nSelected+4*i)*4;

		// Scroller thing: 4 sprites < # # >
		eSprites[1+5*i].attribute[0] = ATTR0_COLOR_256 | ATTR0_NORMAL | ATTR0_TYPE_BLENDED | (20+40*i);
		eSprites[1+5*i].attribute[1] = ATTR1_SIZE_32 | 120;
		eSprites[1+5*i].attribute[2] = nSelected*4;

		eSprites[2+5*i].attribute[0] = ATTR0_COLOR_256 | ATTR0_NORMAL | ATTR0_TYPE_BLENDED | (20+40*i);
		eSprites[2+5*i].attribute[1] = ATTR1_SIZE_32 | 151;
		eSprites[2+5*i].attribute[2] = (2 + nSelected + 2*( *(nValues[i]) /10))*4;

		eSprites[3+5*i].attribute[0] = ATTR0_COLOR_256 | ATTR0_NORMAL | ATTR0_TYPE_BLENDED | (20+40*i);
		eSprites[3+5*i].attribute[1] = ATTR1_SIZE_32 | 182;
		eSprites[3+5*i].attribute[2] = (2 + nSelected + 2*( *(nValues[i]) %10))*4;

		eSprites[4+5*i].attribute[0] = ATTR0_COLOR_256 | ATTR0_NORMAL | ATTR0_TYPE_BLENDED | (20+40*i);
		eSprites[4+5*i].attribute[1] = ATTR1_SIZE_32 | ATTR1_FLIP_X | 212;
		eSprites[4+5*i].attribute[2] = nSelected*4;
	}

	UpdateOAM(eSprites);
}



// UpdateHUDTime/Mines: Put information in OAM to show the HUD.
// Sprites are laid out like
// row 1:    TIME    [# 0] [# 1] : [# 2] [# 3]
// row 3:	MINES    [# 4] [# 5] [# 6]
// nTime is in seconds. nMines is the number of mines minus the number of marked squares.
// Note: to get a negative sign (for counting marked mines), rotate a 1 sprite.

void UpdateHUDTime(SpriteEntry *eSprites, int nTime)
{
	// Only display "tens of minutes" digit if we're beyond 9min, 59sec.
	if (nTime > 599)
	{
		eSprites[0].attribute[0] = ATTR0_COLOR_256 | ATTR0_NORMAL | ATTR0_TYPE_BLENDED | 4;
		eSprites[0].attribute[1] = ATTR1_SIZE_32 | 104;
		eSprites[0].attribute[2] = (2 + 2*((nTime/60)/10) )*4;
	}
	else eSprites[0].attribute[0] = ATTR0_DISABLED;

	eSprites[1].attribute[0] = ATTR0_COLOR_256 | ATTR0_NORMAL | ATTR0_TYPE_BLENDED | 4;
	eSprites[1].attribute[1] = ATTR1_SIZE_32 | 136;
	eSprites[1].attribute[2] = (2 + 2*((nTime/60)%10) )*4;

	eSprites[2].attribute[0] = ATTR0_COLOR_256 | ATTR0_NORMAL | ATTR0_TYPE_BLENDED | 4;
	eSprites[2].attribute[1] = ATTR1_SIZE_32 | 178;
	eSprites[2].attribute[2] = (2 + 2*((nTime%60)/10) )*4;

	eSprites[3].attribute[0] = ATTR0_COLOR_256 | ATTR0_NORMAL | ATTR0_TYPE_BLENDED | 4;
	eSprites[3].attribute[1] = ATTR1_SIZE_32 | 210;
	eSprites[3].attribute[2] = (2 + 2*((nTime%60)%10) )*4;

	UpdateOAM(eSprites);
}


void UpdateHUDMines(SpriteEntry *eSprites, int nMines)
{
	// Most significant digit of mine count can be a digit or a minus sign (rotated 1)
	if (nMines < -99) nMines = -99;
	if (nMines > 999) nMines = 999;
	if (nMines < 0)
	{
		eSprites[4].attribute[0] = ATTR0_COLOR_256 | ATTR0_ROTSCALE | ATTR0_TYPE_BLENDED | 40;
		eSprites[4].attribute[1] = ATTR1_SIZE_32 | ATTR1_ROTDATA(0) | 146;
		eSprites[4].attribute[2] = 4*4;

		((SpriteRotation *)eSprites)[0].hdx = 0;
		((SpriteRotation *)eSprites)[0].hdy = 256;
		((SpriteRotation *)eSprites)[0].vdx = -256;
		((SpriteRotation *)eSprites)[0].vdy = 0;

		nMines = -nMines;
	}
	else
	{
		eSprites[4].attribute[0] = ATTR0_COLOR_256 | ATTR0_NORMAL | ATTR0_TYPE_BLENDED | 40;
		eSprites[4].attribute[1] = ATTR1_SIZE_32 | 146;
		eSprites[4].attribute[2] = (2 + 2*(nMines/100) )*4;
	}

	eSprites[5].attribute[0] = ATTR0_COLOR_256 | ATTR0_NORMAL | ATTR0_TYPE_BLENDED | 40;
	eSprites[5].attribute[1] = ATTR1_SIZE_32 | 178;
	eSprites[5].attribute[2] = (2 + 2*((nMines%100)/10) )*4;

	eSprites[6].attribute[0] = ATTR0_COLOR_256 | ATTR0_NORMAL | ATTR0_TYPE_BLENDED | 40;
	eSprites[6].attribute[1] = ATTR1_SIZE_32 | 210;
	eSprites[6].attribute[2] = (2 + 2*(nMines%10) )*4;

	UpdateOAM(eSprites);
}


void UpdateHUDGameOver(SpriteEntry *eSprites, int nStatus, int nMenuSel)
{
	int i, nFailure;
	if (nStatus == FAILURE) nFailure = 1; else nFailure = 0;

	for (i=0; i<3; i++)
	{
		eSprites[7+i].attribute[0] = ATTR0_COLOR_256 | ATTR0_NORMAL | ATTR0_TYPE_BLENDED | 90;
		eSprites[7+i].attribute[1] = ATTR1_SIZE_64 | 32 + 64*i;
		eSprites[7+i].attribute[2] = (22 + 28 + 4*i + 12*nFailure)*4;
	}

	eSprites[10].attribute[0] = ATTR0_COLOR_256 | ATTR0_NORMAL | ATTR0_TYPE_BLENDED | ATTR0_WIDE | 150;
	eSprites[10].attribute[1] = ATTR1_SIZE_64 | 64;
	eSprites[10].attribute[2] = (26+2*(nMenuSel == MENU_AGAIN ? 1 : 0)+12)*4;

	eSprites[11].attribute[0] = ATTR0_COLOR_256 | ATTR0_NORMAL | ATTR0_TYPE_BLENDED | ATTR0_WIDE | 150;
	eSprites[11].attribute[1] = ATTR1_SIZE_64 | 128;
	eSprites[11].attribute[2] = (26+2*(nMenuSel == MENU_NEW ? 1 : 0)+16)*4;

	UpdateOAM(eSprites);
}


void UpdateHUDPaused(SpriteEntry *eSprites, int nMenuSel)
{
	int i;

	for (i=0; i<3; i++)
	{
		eSprites[7+i].attribute[0] = ATTR0_COLOR_256 | ATTR0_NORMAL | ATTR0_TYPE_BLENDED | 90;
		eSprites[7+i].attribute[1] = ATTR1_SIZE_64 | 32 + 64*i;
		eSprites[7+i].attribute[2] = (22 + 28 + 24 + 4*i)*4;
	}

	eSprites[10].attribute[0] = ATTR0_COLOR_256 | ATTR0_NORMAL | ATTR0_TYPE_BLENDED | ATTR0_WIDE | 150;
	eSprites[10].attribute[1] = ATTR1_SIZE_64 | 64;
	eSprites[10].attribute[2] = (26+2*(nMenuSel == MENU_RESUME ? 1 : 0)+20)*4;

	eSprites[11].attribute[0] = ATTR0_COLOR_256 | ATTR0_NORMAL | ATTR0_TYPE_BLENDED | ATTR0_WIDE | 150;
	eSprites[11].attribute[1] = ATTR1_SIZE_64 | 128;
	eSprites[11].attribute[2] = (26+2*(nMenuSel == MENU_NEW ? 1 : 0)+16)*4;

	UpdateOAM(eSprites);
}
