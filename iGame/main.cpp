// The project builds with /sdl, which promotes the CRT's "consider
// using xxx_s instead" C4996 deprecation into a hard error. The record
// file in Header Files/records.h is read and written with plain
// fopen/fgets/sscanf/sprintf - the same portable C the rest of the game
// is written in - so that deprecation is turned off here, before any
// CRT header is pulled in. Nothing else about the build changes.
#define _CRT_SECURE_NO_WARNINGS

#include "common.h"
#include "game_types.h"
#include "game_globals.h"

#include "home.h"
#include "controls.h"
#include "combat.h"
#include "sounds.h"
#include "images.h"
#include "next_level.h"

int main()
{
    setupLevel1();

    // Level timer and best-record table (see Header Files/records.h).
    // initLevelTimer() picks up the high-resolution clock;
    // loadBestRecords() reads best_records.txt from the working
    // directory, creating it if this is the first run.
    initLevelTimer();
    loadBestRecords();

    // Initialize the sound systems before the first game frame.
    initAttackSound();
    initGameBGM();
    initButtonSound();
    initRunningSound();
    // Level 3's own effects and its final victory music. Both are
    // guarded so they can never open twice, and every Level 3 effect
    // file is optional - a missing one just stays silent (see the
    // LEVEL 3 AUDIO block in sounds.h).
    initLevel3Sounds();
    initVictoryBGM();

    // iGraphics invokes iDraw(), iMouse(), iKeyboard(), and update()
    // through the callbacks defined in the functional headers.
    iInitialize(1350, 680, "Prince of Persia: Escape from the Palace");
    return 0;
}
