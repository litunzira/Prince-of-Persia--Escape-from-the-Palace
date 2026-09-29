#ifndef HOME_H
#define HOME_H

#include "game_globals.h"
#include "records.h"
#include "sounds.h"
#include "reset.h"
#include "next_level.h"

inline void handleHomeMouse(int mouseButton, int state, int mx, int my)
{
    if (mouseButton != GLUT_LEFT_BUTTON || state != GLUT_DOWN)
        return;

    if (gameState == STATE_HOME)
    {
        if (mx >= startBtnX1 && mx <= startBtnX2 &&
            my >= startBtnY1 && my <= startBtnY2)
        {
            playButtonSound();
            stopVictoryBGM();
            gameState = STATE_GAME;
            // Rebuild Level 1 from scratch. This used to just flip the
            // state, which silently relied on nothing having touched
            // Level 1's world since main() built it once - but Level 2
            // and Level 3 both overwrite the shared height[]/blockData[]
            // collision map and the wall/elevator/bridge/switch/enemy
            // counts, so starting a new game after playing a later level
            // dropped the player into a Level 1 with no floor. resetGame()
            // now routes to setupLevel1() for STATE_GAME (see reset.h).
            resetGame();
            startGameBGM();
        }
        else if (mx >= creditsBtnX1 && mx <= creditsBtnX2 &&
                 my >= creditsBtnY1 && my <= creditsBtnY2)
        {
            playButtonSound();
            gameState = STATE_CREDITS;
        }
        else if (mx >= storyBtnX1 && mx <= storyBtnX2 &&
                 my >= storyBtnY1 && my <= storyBtnY2)
        {
            playButtonSound();
            storyPage = 0;
            storyPageStartTime = clock();
            gameState = STATE_STORY;
        }
        else if (mx >= instBtnX1 && mx <= instBtnX2 &&
                 my >= instBtnY1 && my <= instBtnY2)
        {
            playButtonSound();
            gameState = STATE_INSTRUCTIONS;
        }
        // RECORDS: the best completion time for each level. Its box sits
        // below CREDITS and does not overlap any of the four above, so
        // none of them changes behaviour.
        else if (mx >= recordsBtnX1 && mx <= recordsBtnX2 &&
                 my >= recordsBtnY1 && my <= recordsBtnY2)
        {
            playButtonSound();
            // Re-read the file rather than trusting the copy in memory,
            // so the table is right even if a run was recorded by an
            // earlier session of the game.
            loadBestRecords();
            gameState = STATE_RECORDS;
        }
        return;
    }

    if (gameState == STATE_RECORDS)
    {
        if (mx >= recordsBackBtnX1 && mx <= recordsBackBtnX2 &&
            my >= recordsBackBtnY1 && my <= recordsBackBtnY2)
        {
            playButtonSound();
            gameState = STATE_HOME;
        }
        return;
    }

    if (gameState == STATE_CREDITS)
    {
        if (mx >= creditsHomeBtnX1 && mx <= creditsHomeBtnX2 &&
            my >= creditsHomeBtnY1 && my <= creditsHomeBtnY2)
        {
            playButtonSound();
            stopGameBGM();
            gameState = STATE_HOME;
        }
        return;
    }

    // STORY is intentionally automatic: it does not wait for a BACK click.
    if (gameState == STATE_STORY)
        return;

    if (gameState == STATE_INSTRUCTIONS)
    {
        if (mx >= instBackBtnX1 && mx <= instBackBtnX2 &&
            my >= instBackBtnY1 && my <= instBackBtnY2)
        {
            playButtonSound();
            stopGameBGM();
            gameState = STATE_HOME;
        }
        return;
    }

    if (gameState == STATE_GAMEOVER)
    {
        if (mx >= retryBtnX1 && mx <= retryBtnX2 &&
            my >= retryBtnY1 && my <= retryBtnY2)
        {
            // Retry the level the player actually died in (Level 1 or
            // Level 2) - gameState must be set BEFORE resetGame(), since
            // resetGame() checks it to decide whether to call
            // setupLevel2() or restore the Level 1 backups.
            gameState = gameOverFromLevel;
            resetGame();
            playButtonSound();
            startGameBGM();
        }
        else if (mx >= menuBtnX1 && mx <= menuBtnX2 &&
                 my >= menuBtnY1 && my <= menuBtnY2)
        {
            gameState = gameOverFromLevel;
            resetGame();
            playButtonSound();
            stopGameBGM();
            gameState = STATE_HOME;
        }
        else if (mx >= exitBtnX1 && mx <= exitBtnX2 &&
                 my >= exitBtnY1 && my <= exitBtnY2)
        {
            playButtonSound();
            PlaySoundA(NULL, NULL, 0);
            closeButtonSound();
            closeRunningSound();
            closeGameBGM();
            closeVictoryBGM();   // Level 3's victory music uses its own MCI alias
            exit(0);
        }
        return;
    }

    /* FINAL VICTORY: the end of the whole game. The only thing on this
     * screen is the HOME button painted into victory.png - there is
     * deliberately no NEXT LEVEL, since Level 3 is the last level.
     *
     * Tested against victoryHomeBtn* rather than homeBtn*: the two
     * screens are different pictures and their buttons are in different
     * places, so sharing one box would leave this one unclickable. */
    if (gameState == STATE_VICTORY)
    {
        if (mx >= victoryHomeBtnX1 && mx <= victoryHomeBtnX2 &&
            my >= victoryHomeBtnY1 && my <= victoryHomeBtnY2)
        {
            playButtonSound();
            stopVictoryBGM();
            // Rebuild Level 1 so the next START GAME begins a clean run
            // (resetGame() dispatches on gameState - see reset.h).
            gameState = STATE_GAME;
            resetGame();
            gameState = STATE_HOME;
        }
        return;
    }

    if (gameState == STATE_LEVELCOMPLETE)
    {
        // NEXT LEVEL: this click is the ONLY thing that starts the next
        // level. Until the player presses it, gameState just sits at
        // STATE_LEVELCOMPLETE - update() returns immediately for any
        // state that is not a gameplay state, so nothing sets itself up
        // or starts on its own.
        //
        // The route now depends on which level was just finished
        // (levelCompleteFromLevel, set in update()'s level-complete
        // check): Level 1 -> Level 2, Level 2 -> Level 3. Level 3 never
        // reaches this screen at all - it ends in STATE_VICTORY - so
        // there is no case here that could ask for a Level 4.
        if (mx >= nextLevelBtnX1 && mx <= nextLevelBtnX2 &&
            my >= nextLevelBtnY1 && my <= nextLevelBtnY2)
        {
            playButtonSound();
            // gameState is set FIRST in both branches, because resetGame()
            // dispatches on it to decide which level to build.
            if (levelCompleteFromLevel == STATE_LEVEL2){
                gameState = STATE_LEVEL3;
                resetGame();          // -> setupLevel3()
            }
            else{
                gameState = STATE_LEVEL2;
                resetGame();          // -> setupLevel2()
            }
            startGameBGM();
            return;
        }

        if (mx >= homeBtnX1 && mx <= homeBtnX2 &&
            my >= homeBtnY1 && my <= homeBtnY2)
        {
            playButtonSound();
            // Go back to a clean Level 1 so the next START GAME is a
            // fresh run rather than whatever level was just completed.
            gameState = STATE_GAME;
            resetGame();
            gameState = STATE_HOME;
        }
    }
}
#endif
