#include "mines.hpp"

MineField::MineField(int nRows, int nColumns, int nMines, int nSeed)
{
	srand(nSeed);

	this->nRows = nRows;
	this->nColumns = nColumns;
	this->nXOffset = 0;
	this->nYOffset = 0;
	this->nMines = nMines;
	this->nMarked = 0;
	this->bMarkMode = false;
	this->pField = new Square[nRows*nColumns];
	this->bGenerated = false;
	this->nGameOver = RUNNING;

	Draw();
}

MineField::~MineField()
{
	if (this->pField) delete [] this->pField;
}

void MineField::GenerateField(int nSafeRow, int nSafeColumn)
{
	int i, nRow, nColumn, nMineCount = 0;
	Square *pSquare;

	for (i=0; i<nRows*nColumns; i++)
		(pField[i]).SetPosition(i / nColumns, i % nColumns);

	while (nMineCount < nMines)
	{
		nRow = rand() % nRows;
		nColumn = rand() % nColumns;
		pSquare = GetSquare(nRow, nColumn);
		// Don't put a mine under the first square pressed
		if (pSquare->IsMine() || (nRow == nSafeRow && nColumn == nSafeColumn))
			continue;
		else
		{
			pSquare->MakeMine();
			nMineCount++;
		}
	}

	for (i=0; i<nRows*nColumns; i++)
	{
		pSquare = &(this->pField[i]);
		(pField[i]).SetValue(GetValue(pSquare));
	}

	bGenerated = true;
}

Square *MineField::GetSquare(int nRow, int nColumn)
{
	return &(pField[nRow*nColumns + nColumn]);
}

// Inputs are row and column as displayed on screen
void MineField::Press(int nScreenRow, int nScreenColumn)
{
	uint8 *pMap;
	int i, nRow, nColumn;

	nRow = nYOffset + nScreenRow;
	nColumn = nXOffset + nScreenColumn;

	if (!bGenerated) GenerateField(nRow, nColumn);

	// Ignore the press if the game is over
	if (nGameOver == FAILURE || nGameOver == VICTORY) return;

	// Ignore the press if it's not on a real square
	if (nRow >= nRows || nColumn >= nColumns) return;

	// Ignore the press if the square is already pressed
	if (GetSquare(nRow, nColumn)->IsPressed()) return;

	if (bMarkMode)
	{
		// If the square is marked, unmark it. If the square is unmarked, mark it.
		if (GetSquare(nRow, nColumn)->IsMarked()) nMarked--;
		else nMarked++;

		GetSquare(nRow, nColumn)->Mark(); // Toggles the mark
		return;
	}

	if (!GetSquare(nRow, nColumn)->IsPressed() && (GetSquare(nRow, nColumn))->IsMarked())
		return;

	GetSquare(nRow, nColumn)->Press();

	pMap = new uint8[nRows*nColumns];
	for (i=0; i<nRows*nColumns; i++) pMap[i] = 0;
	ExpandRegion(pMap, nRow, nColumn);

	for (i=0; i<nRows*nColumns; i++)
		if (pMap[i] == 1)
			GetSquare(i/nColumns, i%nColumns)->Press();

	delete [] pMap;

	// Evaluate game status. Did this push win/lose the game?
	IsGameOver();
}

// New iterative algorithm because recursive ExpandRegion was taxing DS hardware (it seemed)
void MineField::ExpandRegion(uint8 *pMap, int nRow, int nColumn)
{
	int nUpdated;
	if (GetSquare(nRow, nColumn)->GetValue() != 0
		|| GetSquare(nRow, nColumn)->IsMine()) return;

	pMap[nRow*nColumns + nColumn] = 1;

	nUpdated = 1;
	while (nUpdated != 0)
		nUpdated = ExpandPass(pMap);
}

int MineField::ExpandPass(uint8 *pMap)
{
	int i, nRow, nColumn, nUpdated;

	for (i=0, nUpdated=0; i<nRows*nColumns; i++)
	{
		nRow = i/nColumns;
		nColumn = i%nColumns;

		if (pMap[i] == 1 || GetSquare(nRow, nColumn)->IsMarked() || GetSquare(nRow, nColumn)->IsMine()) continue;

		if (nRow < nRows - 1)
		{
			// Below
			if (pMap[(nRow+1)*nColumns + nColumn] == 1 &&
					GetSquare(nRow+1, nColumn)->GetValue() == 0)
			{
				pMap[i] = 1;
				nUpdated++;
				continue;
			}

			// Below and to the left
			if (nColumn > 0)
			{
				if (pMap[(nRow+1)*nColumns + (nColumn-1)] == 1 &&
					GetSquare(nRow+1, nColumn-1)->GetValue() == 0)
				{
					pMap[i] = 1;
					nUpdated++;
					continue;
				}
			}

			// Below and to the right
			if (nColumn < nColumns - 1)
			{
				if (pMap[(nRow+1)*nColumns + (nColumn+1)] == 1 &&
					GetSquare(nRow+1, nColumn+1)->GetValue() == 0)
				{
					pMap[i] = 1;
					nUpdated++;
					continue;
				}
			}
		}

		// Right
		if (nColumn < nColumns - 1)
		{
			if (pMap[nRow*nColumns + (nColumn+1)] == 1 &&
				GetSquare(nRow, nColumn+1)->GetValue() == 0)
			{
				pMap[i] = 1;
				nUpdated++;
				continue;
			}
		}

		// Left
		if (nColumn > 0)
		{
			if (pMap[nRow*nColumns + (nColumn-1)] == 1 &&
				GetSquare(nRow, nColumn-1)->GetValue() == 0)
			{
				pMap[i] = 1;
				nUpdated++;
				continue;
			}
		}

		if (nRow > 0)
		{
			// Above
			if (pMap[(nRow-1)*nColumns + nColumn] == 1 &&
									GetSquare(nRow-1, nColumn)->GetValue() == 0)
			{
				pMap[i] = 1;
				nUpdated++;
				continue;
			}

			// Above and to the left
			if (nColumn > 0)
			{
				if (pMap[(nRow-1)*nColumns + (nColumn-1)] == 1 &&
					GetSquare(nRow-1, nColumn-1)->GetValue() == 0)
				{
					pMap[i] = 1;
					nUpdated++;
					continue;
				}
			}

			// Above and to the right
			if (nColumn < nColumns - 1)
			{
				if (pMap[(nRow-1)*nColumns + (nColumn+1)] == 1 &&
					GetSquare(nRow-1, nColumn+1)->GetValue() == 0)
				{
					pMap[i] = 1;
					nUpdated++;
					continue;
				}
			}
		}
	}

	return nUpdated;
}

int MineField::GetValue(Square *pSquare)
{
	return GetValue(pSquare->GetRow(), pSquare->GetColumn());
}

int MineField::GetValue(int nRow, int nColumn)
{
	int nAdjacentMines = 0;

	if (nRow > 0)
	{
		//directly above
		if (pField[(nRow-1)*nColumns + nColumn].IsMine()) nAdjacentMines++;

		// above and to the left
		if (nColumn > 0)
			if (pField[(nRow-1)*nColumns + (nColumn - 1)].IsMine()) nAdjacentMines++;

		// above and to the right
		if (nColumn < nColumns - 1)
			if (pField[(nRow-1)*nColumns + (nColumn + 1)].IsMine()) nAdjacentMines++;
	}

	if (nRow < nRows - 1)
	{
		// directly below
		if (pField[(nRow+1)*nColumns + nColumn].IsMine()) nAdjacentMines++;

		// below and to the left
		if (nColumn > 0)
			if (pField[(nRow+1)*nColumns + (nColumn - 1)].IsMine()) nAdjacentMines++;

		// below and to the right
		if (nColumn < nColumns - 1)
			if (pField[(nRow+1)*nColumns + (nColumn + 1)].IsMine()) nAdjacentMines++;
	}

	// directly to the left
	if (nColumn > 0)
		if (pField[nRow*nColumns + nColumn - 1].IsMine()) nAdjacentMines++;

	// directly to the right
	if (nColumn < nColumns - 1)
		if (pField[nRow*nColumns + nColumn + 1].IsMine()) nAdjacentMines++;

	return nAdjacentMines;
}

void MineField::Draw()
{
	int i, nAddress, nValue, nTile;
	Square *pSquare;

	nTile = 1;
	for (i=0; i<DISP_ROWS*DISP_COLS; i++)
	{
		//       ...........................
		//       ...........................
		//       ......oooooooooo...........
		//       ......oooooooooo...........
		//       ......oooooooooo...........
		// nXOffset = 6. nYOffset = 2;
		// first line "o" is i=0. Its address is 2*nColumns + 6 + i%DISP_COLS
		//                          = nYOffset*nColumns + nXOffset + i%DISP_COLS
		// "o" directly below. i=DISP_COLS. address is (nYOffset+i/DISP_COLS)*nColumns + nXOffset + i%DISP_COLS

		nAddress = (nYOffset + i/DISP_COLS)*nColumns + nXOffset + i%DISP_COLS;
		// If the square being displayed isn't a square of the minefield, display a generic tile
		if (i%DISP_COLS >= nColumns || i/DISP_COLS >= nRows)
			nTile = 14;
		else
		{
			pSquare = &(pField[nAddress]);
			if (!pSquare->IsPressed())
			{
				switch(nGameOver)
				{
				case FAILURE:
					if (pSquare->IsMine())
						nTile = 2;
					else if (pSquare->IsMarked()) // If a space was marked and it was not a mine
						nTile = 12;
					else
						nTile = 1;

					break;

				case VICTORY:
					if (pSquare->IsMine())
						nTile = 13;
					else
						nTile = 1;

					break;

				case RUNNING:
					if (pSquare->IsMarked())
						nTile = 13;
					else
						nTile = 1;

					break;
				}
			}
			else
			{
				if (pSquare->IsMine())
				{
					switch(nGameOver)
					{
					// Should never be in the case where a bomb is pressed but game is not over.
					// Maybe for a new mode where you're allowed some mistakes?
					case VICTORY:
					case RUNNING:
						nTile = 2;
						break;
					// True case must only occur at one mine (the one clicked to end the game)
					case FAILURE:
						nTile = 11;
						break;
					}
				}
				else
				{
					nValue = pSquare->GetValue();
					if (nValue == 0)
						nTile = 0;
					else
						nTile = nValue + 2;
				}
			}
		}

		// Equipped with tile number

		// high byte should be top right corner. low byte should be top left corner.
		((uint16*)BG_MAP_RAM(0))[2*DISP_COLS*(i/DISP_COLS) + (i%DISP_COLS)] = (uint16)(nTile*4) | (uint16)((nTile*4+1)<<8);
		// high byte should be bottom right corner. low byte should be bottom left corner.
		((uint16*)BG_MAP_RAM(0))[2*DISP_COLS*(i/DISP_COLS) + DISP_COLS + (i%DISP_COLS)] = (uint16)(nTile*4+2) | (uint16)((nTile*4+3)<<8);
	}

	// Tiles are 16x16 (each made out of 4 hardware tiles).
	// 16 tiles per row, 12 rows. I have 14 tiles.
}

void MineField::MoveScreen(int nHeld)
{
	if (nHeld & KEY_UP && nYOffset > 0) nYOffset--;
	if (nHeld & KEY_LEFT && nXOffset > 0) nXOffset--;
	if (nHeld & KEY_DOWN && nYOffset < nRows - DISP_ROWS) nYOffset++;
	if (nHeld & KEY_RIGHT && nXOffset < nColumns - DISP_COLS) nXOffset++;
}

int MineField::MinesLeft()
{
	return (nGameOver == VICTORY ? 0 : (nMines - nMarked));
}

void MineField::IsGameOver()
{
	int i, nSquaresRemaining;
	Square *pSquare;

	nSquaresRemaining = nRows*nColumns;
	for (i=0; i<nRows*nColumns; i++)
	{
		pSquare = GetSquare(i/nColumns, i%nColumns);
		if (pSquare->IsPressed())
		{
			if (pSquare->IsMine())
			{
				nGameOver = FAILURE;
				return;
			}
			nSquaresRemaining--;
		}
	}

	if (nSquaresRemaining == nMines)
	{
		nGameOver = VICTORY;
		return;
	}

	nGameOver = RUNNING;
}

int MineField::Color(int nRow, int nColumn)
{
	Square *pSquare;
	pSquare = GetSquare(nRow, nColumn);
	if (!pSquare) return RGB15(0,0,0) | BIT(15);

	//if (pSquare->IsPressed()) return RGB15(12,20,26) | BIT(15);
	if (pSquare->IsPressed()) return RGB15(26,26,26) | BIT(15);
	else
	{
		if (pSquare->IsMarked()) return RGB15(26,0,0) | BIT(15);
		else return RGB15(18,18,18) | BIT(15);
	}
}


// Slower than double buffering (is this true?), but not visibly slow on hardware.
// Slow on emulators. With double buffering, I was getting unpredictable garbage on screen
// (usually small horizontal black lines at the top left or bottom right corners of this
// draw area). That bug didn't show up in any emulators, and I could not trace that bug for
// the life of me.
void MineField::UpdateHUDMap()
{
	int i, j, nColor, nRMax, nCMax, nWidth, nHeight, nOffset;
	uint16 *uBuffer;

	nWidth = 209+1;
	nHeight = 97+1;
	nOffset = 256*((nHeight - 4*nRows - 2)/2) + (nWidth - 4*nColumns)/2;

	uBuffer = ((uint16*)BG_BMP_RAM_SUB(0)) + 256*90 + 25;

	for (i=0; i<nHeight; i++)
		for (j=0; j<nWidth; j++)
			uBuffer[256*i + j] = RGB15(31,31,31) | BIT(15);

	if (nGameOver == RUNNING)
	{
		for (i=0; i<nRows; i++)
		{
			for (j=0; j<nColumns; j++)
			{
				nColor = Color(i, j);
				uBuffer[nOffset + 256 + 1 + 256*4*i + 4*j] = nColor;
				uBuffer[nOffset + 256 + 1 + 256*4*i + 4*j + 1] = nColor;
				uBuffer[nOffset + 256 + 1 + 256*4*i + 4*j + 2] = nColor;
				uBuffer[nOffset + 256 + 1 + 256*(4*i+1) + 4*j] = nColor;
				uBuffer[nOffset + 256 + 1 + 256*(4*i+1) + 4*j + 1] = nColor;
				uBuffer[nOffset + 256 + 1 + 256*(4*i+1) + 4*j + 2] = nColor;
				uBuffer[nOffset + 256 + 1 + 256*(4*i+2) + 4*j] = nColor;
				uBuffer[nOffset + 256 + 1 + 256*(4*i+2) + 4*j + 1] = nColor;
				uBuffer[nOffset + 256 + 1 + 256*(4*i+2) + 4*j + 2] = nColor;
			}
		}

		nColor = RGB15(0,0,0) | BIT(15);

		nRMax = (nRows > DISP_ROWS ? DISP_ROWS : nRows);
		nCMax = (nColumns > DISP_COLS ? DISP_COLS : nColumns);
		for (i=0; i<nRMax; i++)
		{
			// Left border
			uBuffer[nOffset + 256*4*(nYOffset + i) + 4*nXOffset] = nColor;
			uBuffer[nOffset + 256*(4*(nYOffset + i)+1) + 4*nXOffset] = nColor;
			uBuffer[nOffset + 256*(4*(nYOffset + i)+2) + 4*nXOffset] = nColor;
			uBuffer[nOffset + 256*(4*(nYOffset + i)+3) + 4*nXOffset] = nColor;

			// Right border
			uBuffer[nOffset + 256*4*(nYOffset + i) + 4*(nXOffset + nCMax)] = nColor;
			uBuffer[nOffset + 256*(4*(nYOffset + i)+1) + 4*(nXOffset + nCMax)] = nColor;
			uBuffer[nOffset + 256*(4*(nYOffset + i)+2) + 4*(nXOffset + nCMax)] = nColor;
			uBuffer[nOffset + 256*(4*(nYOffset + i)+3) + 4*(nXOffset + nCMax)] = nColor;
		}

		for (i=0; i<nCMax; i++)
		{
			// Top border
			uBuffer[nOffset + 256*4*nYOffset + 4*(nXOffset + i)] = nColor;
			uBuffer[nOffset + 256*4*nYOffset + 4*(nXOffset + i) + 1] = nColor;
			uBuffer[nOffset + 256*4*nYOffset + 4*(nXOffset + i) + 2] = nColor;
			uBuffer[nOffset + 256*4*nYOffset + 4*(nXOffset + i) + 3] = nColor;

			// Bottom border
			uBuffer[nOffset + 256*4*(nYOffset + nRMax) + 4*(nXOffset + i)] = nColor;
			uBuffer[nOffset + 256*4*(nYOffset + nRMax) + 4*(nXOffset + i) + 1] = nColor;
			uBuffer[nOffset + 256*4*(nYOffset + nRMax) + 4*(nXOffset + i) + 2] = nColor;
			uBuffer[nOffset + 256*4*(nYOffset + nRMax) + 4*(nXOffset + i) + 3] = nColor;
		}

		// Bottom-right corner
		uBuffer[nOffset + 256*4*(nYOffset + nRMax) + 4*(nXOffset + nCMax)] = nColor;
	}

	//for (i=0; i<nHeight; i++)
		//dmaCopy(uBuffer+nWidth*i, ((uint16*)BG_BMP_RAM_SUB(0)) + 256*(90+i) + 25, nWidth*2);
}
