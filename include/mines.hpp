#include <nds.h>
#include <stdlib.h>
#include <stdio.h>
#include "graphics.hpp"

#define DISP_ROWS 12
#define DISP_COLS 16

// tile order:
// 1. blank pushed. 2. blank unpushed. 3. mine. 4. one. 5. two. 6. three. 7. four
// 8. five. 9. six. 10. seven. 11. eight. 12. mine (stepped on). 13. bomb (wrongly marked). 14. marked
// each tile is in the order: top left corner, top right corner, bottom left corner, bottom right corner

// Game states
#define SPLASH		0
#define MENULOADING	1
#define	MENU		2
#define RUNNING		3
#define PAUSED		4
#define VICTORY		5
#define FAILURE		6

// Menu selections
#define MENU_MINES	0
#define MENU_ROWS	1
#define MENU_COLS	2
#define MENU_START	3

// Paused/game over menu selections
#define MENU_AGAIN	0
#define MENU_NEW	1
#define MENU_RESUME	2

// Arrow animation directions
#define DIR_INWARD	-1
#define DIR_OUTWARD	1

// Lid status
#define LID_OPEN	0
#define LID_CLOSED	1

class Square
{
	public:
		Square() { nRow = -1; nColumn = -1; bPressed = false; bMine = false; bMarked = false; }

		void MakeMine() { bMine = true; }
		void SetPosition(int nRow, int nColumn) { this->nRow = nRow; this->nColumn = nColumn; }
		void SetValue(int nValue) { this->nValue = nValue; }
		void Press() { bPressed = true; }
		void Mark() { bMarked = !bMarked; }

		int GetRow() { return nRow; }
		int GetColumn() { return nColumn; }
		int GetValue() { return nValue; }
		bool IsMine() { return bMine; }
		bool IsMarked() { return bMarked; }
		bool IsPressed() { return bPressed; }

	private:
		bool bPressed;
		bool bMine;
		bool bMarked;
		int nRow;
		int nColumn;
		int nValue;
};

class MineField
{
	public:
		MineField(int nRows, int nColumns, int nMines, int nSeed);
		~MineField();

		void Draw();
		void MoveScreen(int nHeld);
		void Press(int nScreenRow, int nScreenColumn);
		void UpdateHUDMap();
		int MinesLeft();
		int GameStatus() { return nGameOver; }
		void SetMarkMode() { bMarkMode = true; }
		void UnsetMarkMode() { bMarkMode = false; }
		void Pause() { nGameOver = PAUSED; }
		void Unpause() { nGameOver = RUNNING; }
		bool MarkMode() { return bMarkMode; }
		bool OffsetFromLeft() { return (nXOffset > 0); }
		bool OffsetFromRight() { return (nXOffset < nColumns - DISP_COLS); }
		bool OffsetFromTop() { return (nYOffset > 0); }
		bool OffsetFromBottom() { return (nYOffset < nRows - DISP_ROWS); }

	private:
		Square *pField;
		int nRows;
		int nColumns;
		int nXOffset; // Measured in columns/rows. Not in pixels.
		int nYOffset;
		int nMines;
		int nMarked;
		int nGameOver;
		bool bMarkMode;
		bool bGenerated;

		void ExpandRegion(uint8 *pMap, int nRow, int nColumn);
		int ExpandPass(uint8 *pMap);
		void GenerateField(int nSafeRow, int nSafeColumn);
		Square *GetSquare(int nRow, int nColumn);
		int GetValue(Square *pSquare);
		int GetValue(int nRow, int nColumn);
		int Color(int nRow, int nColumn);
		void IsGameOver();
};

void StartNewGame(SpriteEntry *eSprites, MineField **mMineField, int nRows, int nCols, int nMines, int nFrame);
void RefreshArrows(MineField *mMineField, int nStatus);
void UpdateHUDMap(MineField *mMineField, int nStatus);
