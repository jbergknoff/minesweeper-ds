#include "mines.hpp"

int main()
{
	int nMines, nRows, nCols;
	int nMenuCounter, nAnimTimer, nTimer, nSecs, nDirection, nLid;
	int nStatus, nFrame, nMenuSel, nX, nY;
	MineField *mMineField = 0;
	touchPosition tPosition;
	SpriteEntry *eSprites;
	uint32 nHeld, nDown, nUp;

	powerON(POWER_ALL_2D);
	irqInit();
	irqSet(IRQ_VBLANK, 0);

	// Allocate internal copy of OAM for holding sprite information.
	eSprites = (SpriteEntry *)calloc(128, sizeof(SpriteEntry));
	InitVideo(eSprites);

	nMines = 30;
	nRows = 16;
	nCols = 20;
	nStatus = SPLASH;
	nMenuCounter = 0;
	nFrame = 0;
	nAnimTimer = 0;
	nTimer = 0;
	nSecs = 0;
	nMenuSel = MENU_MINES;
	nDown = nHeld = nUp = 0;
	nDirection = DIR_INWARD;
	nLid = LID_OPEN;

	// Main loop. Distinguish between states of execution and react accordingly.
	while (1)
	{
		tPosition.px = 0;
		tPosition.py = 0;
		scanKeys();
		nDown = keysDown();
		nHeld = keysHeld();
		nUp = keysUp();
		tPosition = touchReadXY();

		// It seems to me, from testing, that:
		// KEY_LID isn't set unless touchReadXY() is called first.
		// This has to do with the way lid information is stored
		// in the ARM7 register EXTKEYIN 0x4000136 with XY data.
		if (nDown & KEY_LID && nLid == LID_OPEN)
		{
			powerOFF(POWER_ALL_2D);
			nLid = LID_CLOSED;
			continue;
		}

		if (nUp & KEY_LID && nLid == LID_CLOSED)
		{
			powerON(POWER_ALL_2D);
			nLid = LID_OPEN;
			continue;
		}

		// Do nothing while lid is closed
		if (nLid == LID_CLOSED)
			continue;

		nAnimTimer += FRAME_TIME;
		if (nAnimTimer >= 60000)
		{
			nAnimTimer -= 60000;
			if (nStatus == RUNNING || nStatus == VICTORY || nStatus == FAILURE)
				AnimateArrows(&nDirection);
		}


		// MENULOADING: Fade splash screen to white and display setup menu.
		if (nStatus == MENULOADING)
		{
			if (nMenuCounter <= 31)
				SUB_BLEND_Y = nMenuCounter++;
			else
			{
				// Enter menu mode after the menu finishes loading.
				nStatus = MENU;

				// Load the HUD into sub BG memory instead of the splash screen.
				for (nMenuCounter = 0; nMenuCounter<256*256; nMenuCounter++)
					((uint16 *)BG_BMP_RAM_SUB(0))[nMenuCounter] = ((uint16 *)hud_bin)[nMenuCounter] | BIT(15);

				nMenuSel = MENU_MINES;
				UpdateMenuSprites(eSprites, nMenuSel, nMines, nRows, nCols);
				nMenuCounter = 0;
			}

			swiWaitForVBlank();
			continue;
		}


		// SPLASH: On any user input, load the menu.
		if (nStatus == SPLASH && nDown)
		{
			SUB_BLEND_CR = BLEND_FADE_WHITE | BLEND_SRC_BG2;
			SUB_BLEND_Y = 0;
			nStatus = MENULOADING;
		}


		// MENU: Handle user input.
		else if (nStatus == MENU)
		{
			// Start the game if player presses start or touches start button, or presses A when on the start option.
			if (nDown & KEY_START || (nDown & KEY_A && nMenuSel == MENU_START)
				|| (nDown & KEY_TOUCH && (tPosition.px > 80 && tPosition.py > 145 && tPosition.px < 170 && tPosition.py < 185)))
			{
				// Begin a new game.
				lcdSwap();
				StartNewGame(eSprites, &mMineField, nRows, nCols, nMines, nFrame);

				nStatus = RUNNING;
				nTimer = 0;
				nSecs = 0;

				swiWaitForVBlank();
				continue;
			}

			if (nDown & KEY_LEFT)
			{
				if (nMenuSel == MENU_MINES && nMines > 1) nMines--;
				else if (nMenuSel == MENU_ROWS)
				{
					if (nRows > 5) nRows--;
					if (nMines >= nRows*nCols) nMines = nRows*nCols - 1;
				}
				else if (nMenuSel == MENU_COLS)
				{
					if (nCols > 5) nCols--;
					if (nMines >= nRows*nCols) nMines = nRows*nCols - 1;
				}
			}

			else if (nDown & KEY_RIGHT)
			{
				if (nMenuSel == MENU_MINES && nMines < nRows*nCols-1 && nMines < 99) nMines++;
				else if (nMenuSel == MENU_ROWS && nRows < 24) nRows++;
				else if (nMenuSel == MENU_COLS && nCols < 52) nCols++;
			}

			else if ((nDown & KEY_UP) && nMenuSel > MENU_MINES) nMenuSel--;

			else if ((nDown & KEY_DOWN) && nMenuSel < MENU_START) nMenuSel++;

			else if (nDown & KEY_TOUCH)
			{
				// Clicked in the vertical band corresponding to mines
				if (tPosition.py > 25 && tPosition.py < 45)
				{
					nMenuSel = MENU_MINES;
					if (tPosition.px > 125 && tPosition.px < 150 && nMines > 1) nMines--;
					else if (tPosition.px > 215 && tPosition.px < 240 && nMines < nRows*nCols-1 && nMines < 99) nMines++;
				}

				// Ditto for rows
				else if (tPosition.py > 65 && tPosition.py < 85)
				{
					nMenuSel = MENU_ROWS;
					if (tPosition.px > 125 && tPosition.px < 150)
					{
						if (nRows > 5) nRows--;
						if (nMines >= nRows*nCols) nMines = nRows*nCols - 1;
					}
					else if (tPosition.px > 215 && tPosition.px < 240 && nRows < 24) nRows++;
				}

				// Ditto for columns
				else if (tPosition.py > 105 && tPosition.py < 125)
				{
					nMenuSel = MENU_COLS;
					if (tPosition.px > 125 && tPosition.px < 150)
					{
						if (nCols > 5) nCols--;
						if (nMines >= nRows*nCols) nMines = nRows*nCols - 1;
					}
					else if (tPosition.px > 215 && tPosition.px < 240 && nCols < 52) nCols++;
				}
			}

			UpdateMenuSprites(eSprites, nMenuSel, nMines, nRows, nCols);
		}


		else if (nStatus == RUNNING)
		{
			// Every 1 second, update the timer and show the new time.
			nTimer += FRAME_TIME;
			if (nTimer >= 1000000)
			{
				nTimer -= 1000000;
				nSecs++;
				UpdateHUDTime(eSprites, nSecs);
			}

			if (nDown & KEY_START)
			{
				nStatus = PAUSED;
				mMineField->Pause();
				mMineField->UpdateHUDMap();

				nMenuSel = MENU_RESUME;
				UpdateHUDPaused(eSprites, nMenuSel);
				videoSetMode(0); // Blank the main screen
				swiWaitForVBlank();
				continue;
			}

			if (nDown & KEY_TOUCH)
				tPosition = touchReadXY();
			else
			{
				tPosition.px = 0;
				tPosition.py = 0;
			}

			if (nHeld & KEY_L)
				if (mMineField) mMineField->SetMarkMode();

			if (nUp & KEY_L)
				if (mMineField) mMineField->UnsetMarkMode();

			if (mMineField)
			{
				if (tPosition.px > 0 && tPosition.py > 0)
				{
					if (nCols < DISP_COLS) nX = (256 - nCols*16)/2; else nX = 0;
					if (nRows < DISP_ROWS) nY = (192 - nRows*16)/2; else nY = 0;
					mMineField->Press((tPosition.py - nY)/16, (tPosition.px - nX)/16);
					nStatus = mMineField->GameStatus();
					UpdateHUDMines(eSprites, mMineField->MinesLeft());
					if (nStatus == VICTORY || nStatus == FAILURE)
					{
						nMenuSel = MENU_AGAIN;
						UpdateHUDGameOver(eSprites, nStatus, nMenuSel);
					}
					mMineField->UpdateHUDMap();
				}

				//if (nHeld & KEY_R && nHeld & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT))
				if (nHeld & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT))
				{
					mMineField->MoveScreen(nHeld);
					mMineField->UpdateHUDMap();
				}

				RefreshArrows(mMineField, nStatus);
				mMineField->Draw();
			}
		}

		else if (nStatus == VICTORY || nStatus == FAILURE)
		{
			if (mMineField && nHeld & KEY_R && nHeld & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT))
			{
				mMineField->MoveScreen(nHeld);
				RefreshArrows(mMineField, nStatus);
				mMineField->Draw();
				mMineField->UpdateHUDMap();
			}

			// Pressing any direction toggles the game over menu option.
			else if (nDown & (KEY_LEFT | KEY_RIGHT | KEY_UP | KEY_DOWN))
			{
				nMenuSel = 1 - nMenuSel;
				UpdateHUDGameOver(eSprites, nStatus, nMenuSel);
			}

			else if (nDown & (KEY_A | KEY_START))
			{
				switch (nMenuSel)
				{
				case MENU_NEW:
					lcdSwap();
					ClearOAM(eSprites);
					SUB_BLEND_CR = BLEND_FADE_WHITE | BLEND_SRC_BG2;
					SUB_BLEND_Y = 0;
					nStatus = MENULOADING;
					RefreshArrows(mMineField, MENU); // Erase scroll arrows
					break;

				case MENU_AGAIN:
					StartNewGame(eSprites, &mMineField, nRows, nCols, nMines, nFrame);
					nStatus = RUNNING;
					nTimer = 0;
					nSecs = 0;

					tPosition.px = 0;
					tPosition.py = 0;

					swiWaitForVBlank();
					continue;
				}
			}
		}


		else if (nStatus == PAUSED)
		{
			if (nDown & (KEY_LEFT | KEY_RIGHT | KEY_UP | KEY_DOWN))
			{
				if (nMenuSel == MENU_RESUME) nMenuSel = MENU_NEW;
				else nMenuSel = MENU_RESUME;
				UpdateHUDPaused(eSprites, nMenuSel);
			}

			else if (nDown & (KEY_A | KEY_START))
			{
				// Restore the main screen
				videoSetMode(MODE_2_2D | DISPLAY_BG2_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D | DISPLAY_SPR_1D_SIZE_256);

				switch (nMenuSel)
				{
				case MENU_NEW:
					lcdSwap();
					ClearOAM(eSprites);
					SUB_BLEND_CR = BLEND_FADE_WHITE | BLEND_SRC_BG2;
					SUB_BLEND_Y = 0;
					nStatus = MENULOADING;
					RefreshArrows(mMineField, MENU); // Erase scroll arrows
					break;

				case MENU_RESUME:
					nStatus = RUNNING;
					mMineField->Unpause();
					ClearOAM(eSprites);
					UpdateHUDTime(eSprites, nSecs);
					UpdateHUDMines(eSprites, nMines);
					mMineField->UpdateHUDMap();
					continue;
				}
			}
		}

		swiWaitForVBlank();
		nFrame++;
	}

	free(eSprites);
	return 0;
}

void StartNewGame(SpriteEntry *eSprites, MineField **mMineField, int nRows, int nCols, int nMines, int nFrame)
{
	ClearOAM(eSprites);
	UpdateHUDTime(eSprites, 0);
	UpdateHUDMines(eSprites, nMines);
	SUB_BLEND_CR = 0;
	SUB_BLEND_Y = 0;

	// Start a new game
	if (*mMineField) (*mMineField)->~MineField();
	*mMineField = new MineField(nRows, nCols, nMines, nFrame);
	if (nCols < DISP_COLS) BG2_CX = -((256 - nCols*16)/2) << 8; else BG2_CX = 0;
	if (nRows < DISP_ROWS) BG2_CY = -((192 - nRows*16)/2) << 8; else BG2_CY = 0;
	(*mMineField)->UpdateHUDMap();
}

void RefreshArrows(MineField *mMineField, int nStatus)
{
	// Enable or disable animated arrows in each direction, depending
	// on whether we're offset from that edge of the board.
	((SpriteEntry*)OAM)[0].attribute[0] &= 0xFCFF;
	((SpriteEntry*)OAM)[1].attribute[0] &= 0xFCFF;
	((SpriteEntry*)OAM)[2].attribute[0] &= 0xFCFF;
	((SpriteEntry*)OAM)[3].attribute[0] &= 0xFCFF;

	if (mMineField->OffsetFromLeft() && nStatus != MENU)
		((SpriteEntry*)OAM)[0].attribute[0] |= ATTR0_NORMAL;
	else
		((SpriteEntry*)OAM)[0].attribute[0] |= ATTR0_DISABLED;

	if (mMineField->OffsetFromRight() && nStatus != MENU)
		((SpriteEntry*)OAM)[1].attribute[0] |= ATTR0_NORMAL;
	else
		((SpriteEntry*)OAM)[1].attribute[0] |= ATTR0_DISABLED;

	if (mMineField->OffsetFromTop() && nStatus != MENU)
		((SpriteEntry*)OAM)[2].attribute[0] |= ATTR0_ROTSCALE;
	else
		((SpriteEntry*)OAM)[2].attribute[0] |= ATTR0_DISABLED;

	if (mMineField->OffsetFromBottom() && nStatus != MENU)
		((SpriteEntry*)OAM)[3].attribute[0] |= ATTR0_ROTSCALE;
	else
		((SpriteEntry*)OAM)[3].attribute[0] |= ATTR0_DISABLED;
}
