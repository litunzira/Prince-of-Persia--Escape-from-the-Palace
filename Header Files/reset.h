#ifndef RESET_H
#define RESET_H

#include "game_globals.h"
#include "records.h"
#include "sounds.h"

void resetGame(){
	stopRunningSound();
	// LEVEL TIMER: every start and every restart of a level comes
	// through here, so this is the one place the clock has to be put
	// back to zero. It is only reset, never started - update() starts
	// it on the first tick that the player is actually playing, which
	// keeps menu time out of the reading. The saved best records are
	// deliberately untouched: restarting never costs the player a
	// record, and the new attempt can only replace one by being faster.
	resetLevelTimer();
	// The progress bar goes back to empty with it. The span itself is
	// re-established by whichever setupLevelN() runs below; this only
	// clears the "already finished" flag so a new attempt starts at the
	// player's real position again.
	resetLevelProgress();
	playerHealth = playerMaxHealth;
	playerX = 300;
	playerY = 300;
	cX = 0;
	cEnemy = -1;
	fallCount = 0;
	gravitalForce = 0;
	playerMoveAccum = 0.0; // clear leftover fractional movement from the previous run
	atMode = 0;
	playerAtkLastFrame = -1; // clear leftover attack-swing state from the previous run
	landingState = 0;        // clear any in-progress landing pause from the previous run
	hopTimer = 0;            // clear any in-progress hop from the previous run
	playerIsMoving = 0;      // start the next run showing the idle sprite, not mid-walk-cycle
	lastMoveKeyTime = 0;     // clear leftover movement-timing state from the previous run
	// Switch checkpoints belong to one run of one level: starting or
	// retrying a level always begins at that level's own start, never at
	// a switch banked during the previous attempt.
	checkpointX = -1;
	checkpointY = 0;
	checkpointMsgTimer = 0;

	// FIX: the player is placed at playerY = 300, which is ABOVE the
	// floor he is about to stand on (Level 1's is at 220), so every
	// fresh start and every RETRY began with a short drop that landing
	// charged fall damage for - costing health before the player had
	// touched a key. Arm the one-landing skip so that settle is free.
	skipFallDamage = 1;
	enemyDamageAccum = 0.0;  // clear leftover fractional enemy-damage state from the previous run
	clearDrowning();         // never start the next run already under water
	// Level 2's cave bats. Every read of them is gated on STATE_LEVEL2
	// already, so they cannot reach Levels 1/3 - this is the same
	// belt-and-braces hygiene as clearDrowning() above, and it means the
	// crouch can never carry over into a level that has no 'Q' key.
	// setupLevel2() calls it again (and re-seeds) when Level 2 is the
	// level being built.
	clearLevel2Bats();
	// Health hearts. setupLevel2()/setupLevel3() roll a fresh batch
	// straight after this; Level 1 has none, so clearing here is what
	// leaves it with none.
	clearHearts();
	// Level 3's water. Every read is gated on STATE_LEVEL3 already, so
	// this is the same hygiene as clearDrowning() above - it means a
	// half-finished swim can never carry into a level with no water in
	// it. setupLevel3() calls it again and restocks the piranhas when
	// Level 3 is the level being built.
	clearLevel3Water();
	// Level 3's own systems (collapsing slabs, the flood, the Guardian,
	// the escape sequence) are all gated on this, so leaving Level 3 for
	// any reason switches every one of them off in a single place.
	// setupLevel3() turns it back on when Level 3 is (re)entered.
	level3Active = (gameState == STATE_LEVEL3);

	// LEVEL 3 retry: rebuilt completely from scratch, exactly like
	// Level 2. setupLevel3() re-stamps the whole collision map, restores
	// every collapsed slab, drops the flood back to its starting level,
	// parks every bridge/elevator at its start position, turns every
	// switch off, restores all ten guards to full health, gives the
	// Guardian all of its life back, clears the escape/boss-phase state
	// and every level3* timer, and re-places the player. Nothing from
	// the failed run - and nothing from Level 2 - survives it.
	if (gameState == STATE_LEVEL3){
		setupLevel3();
		return;
	}

	if (gameState == STATE_LEVEL2){
		setupLevel2();
		return;
	}

	// LEVEL 1: rebuild the world first, then restore the backups.
	//
	// The rebuild is new. setupLevel1() used to run exactly once, from
	// main(), which quietly meant Level 1 could only ever be played
	// before any other level: setupLevel2()/setupLevel3() wipe the
	// shared height[]/blockData[] collision map that Level 1's platforms
	// live in and change wallNo/jWallNo/mWallNo/buttonNo/EnemyNo, so
	// coming back to Level 1 afterwards (GAME OVER -> MAIN MENU ->
	// START GAME) dropped the player into a level with no floor at all.
	// setupLevel1() is now re-runnable and rebuilds all of that; the
	// backup restore below then runs on top of it exactly as before.
	setupLevel1();

	// Restore the enemy roster (position/health/count) from the
	// pristine snapshot taken at the end of main(). Without this,
	// EnemyNo and enemy[] would still reflect whatever kills happened
	// in the previous run, so a retried game could start with fewer
	// enemies than it should - or none at all.
	EnemyNo = EnemyNoBackup;
	for (int i = 0; i < EnemyNo; i++){
		enemy[i] = enemyBackup[i];
	}

	// FIX: restore every switch to OFF and every elevator/moving wall to
	// its starting position/height, from the pristine snapshot taken at
	// the end of main(). Without this, a switch the player left ON (and
	// whatever position that left the elevator/moving wall in) carried
	// straight over into the next RETRY/MAIN MENU run. Just restoring
	// the struct fields is enough - the height[]/blockData[] arrays for
	// these tiles are unconditionally rebuilt from jWall[]/mWall[] every
	// single update() tick (see the Jumping Wall / Moving Wall update
	// sections below), so they self-correct on the very next tick after
	// gameState goes back to STATE_GAME.
	for (int i = 0; i < buttonNo; i++){
		button[i] = buttonBackup[i];
	}
	for (int i = 0; i < jWallNo; i++){
		jWall[i] = jWallBackup[i];
	}
	for (int i = 0; i < mWallNo; i++){
		mWall[i] = mWallBackup[i];
	}
}


#endif
