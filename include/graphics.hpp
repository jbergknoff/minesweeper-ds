#include "mines_bin.h"
#include "minespal_bin.h"
#include "splash_bin.h"
#include "menu_bin.h"
#include "hud_bin.h"

#include "numbers_bin.h"
#include "words_bin.h"
#include "gameover_bin.h"
#include "palette_bin.h"
#include "arrow_bin.h"
#include "arrowpalette_bin.h"

#define FRAME_TIME	16740 // Microseconds. VBlank occurs with frequency 59.737 Hz

void InitVideo(SpriteEntry *eSprites);
void UpdateOAM(SpriteEntry *eSprites);
void ClearOAM(SpriteEntry *eSprites);
void LoadSprites(SpriteEntry *eSprites);
void AnimateArrows(int *nDirection);
void UpdateMenuSprites(SpriteEntry *eSprites, int nMenuSel, int nMines, int nRows, int nCols);
void UpdateHUDTime(SpriteEntry *eSprites, int nTime);
void UpdateHUDMines(SpriteEntry *eSprites, int nMines);
void UpdateHUDGameOver(SpriteEntry *eSprites, int nStatus, int nMenuSel);
void UpdateHUDPaused(SpriteEntry *eSprites, int nMenuSel);
