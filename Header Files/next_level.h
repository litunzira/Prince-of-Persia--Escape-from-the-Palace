#ifndef NEXT_LEVEL_H
#define NEXT_LEVEL_H

#include "game_globals.h"
#include "records.h"
#include "sounds.h"

void setupLevel1()
{
	// FIX (needed once Level 3 existed, but it was already broken for
	// Level 2): this function used to be a one-shot called from main()
	// only, so it silently depended on every global still holding its
	// zero/initial value. setupLevel2() (and now setupLevel3()) change
	// wallNo/mWallNo/jWallNo/buttonNo/EnemyNo, wipe height[]/blockData[]
	// across the whole Level 1 area, disable doubleS/doubleE, and set
	// alwaysOn on some elevators/bridges - so after playing a later
	// level, going back to Level 1 (MAIN MENU -> START GAME) left the
	// player standing in a level that no longer had any floor.
	//
	// Making it re-runnable is the whole fix: it now states its own
	// counts, clears the collision map it is about to build, and
	// explicitly clears every alwaysOn flag it relies on being off.
	// Nothing about how Level 1 actually plays changes at all - the
	// values below are exactly the ones the globals already had.
	static int splashTimerStarted = 0;
	if (!splashTimerStarted){
		splashStartTime = clock(); // start the 1.5s splash timer (first run only)
		splashTimerStarted = 1;
	}

	wallNo = 8;
	jWallNo = 2;
	mWallNo = 2;
	buttonNo = 4;
	EnemyNo = 8;

	// Clear the whole collision map so nothing from a previously played
	// level can survive underneath Level 1's platforms.
	for (int i = 0; i < 50000; i++){
		height[i] = 0;
		blockData[i] = -1;
	}

	// Level 1's elevators and bridges are ALL switch-driven; make sure
	// none of them inherited an alwaysOn flag from Level 2 / Level 3.
	jWall[0].alwaysOn = 0;
	jWall[1].alwaysOn = 0;
	mWall[0].alwaysOn = 0;
	mWall[1].alwaysOn = 0;

	wall[0].blockNo = 0;
	wall[0].sPos = 220;
	wall[0].h = 220;
	wall[0].ePos = 540;
	block[0].start = 220;
	block[0].end = 540;
	for (int i = 220 / 2; i <= 540 / 2; i++)
	{
		height[i] = 200;
	}
	wall[1].blockNo = 1;
	wall[1].sPos = 220;
	wall[1].h = 520;
	wall[1].ePos = 540;
	block[1].start = 220;
	block[1].end = 540;
	for (int i = 220 / 2; i <= 540 / 2; i++)
	{
		height[i] = 292;
	}

	enemy[0].x = 500;
	enemy[0].y = 520;
	enemy[0].life = 20;
	enemy[0].blockNo = 1;
	enemy[0].a = 0;
	enemy[0].p = 0;
	enemy[0].k = 13;
	enemy[0].life = 50;
	enemy[0].dir = 1;
	button[0].y = 220 + 48;
	button[0].x = 450;
	button[0].OnOff = 0;
	doubleS[0] = 220;
	doubleE[0] = 540;
	doubleUp[0] = 1;
	doubleUpH[0] = 520;
	doubleDownH[0] = 220;
	doubleDown[0] = 0;
	jWall[0].Pos = 92;
	jWall[0].h = 292;
	jWall[0].d = 1;
	jWall[0].upperLimit = 596;
	jWall[0].lowerLimit = 150;
	jWall[0].buttonNo = 0;
	jWall[0].blockNo = 2;
	block[2].start = 92;
	block[2].end = 92 + 64 * 2;
	for (int i = jWall[0].Pos / 2; i < (jWall[0].Pos + 128) / 2; i++){
		height[i] = jWall[0].h;
		blockData[i] = 2;
	}
	button[1].y = 400 + 48;
	button[1].x = 700;
	button[1].OnOff = 0;
	enemy[1].x = 750;
	enemy[1].y = 400;
	enemy[1].blockNo = 3;
	enemy[1].a = 0;
	enemy[1].p = 0;
	enemy[1].k = 0;
	enemy[1].life = 50;
	enemy[1].dir = -1;
	wall[2].blockNo = 3;
	wall[2].sPos = 540;
	wall[2].h = 400;
	wall[2].ePos = 540 + 320;
	block[3].start = 540;
	block[3].end = 860;
	for (int i = 520 / 2; i <= 860 / 2; i++)
	{
		height[i] = 400;
		blockData[i] = 3;
	}
	mWall[0].buttonNo = 1;
	mWall[0].left = 860;
	mWall[0].right = 1122;
	mWall[0].current = mWall[0].left;
	mWall[0].h = 380;
	mWall[0].d = 1;
	mWall[0].blockNo = 4;
	for (int i = mWall[0].left / 2; i < mWall[0].right / 2 + 64; i++){
		height[i] = 0;
		blockData[i] = -1;
	}
	for (int i = mWall[0].current / 2; i < mWall[0].current / 2 + 64; i++){
		height[i] = mWall[0].h;
		blockData[i] = 4;
	}
	block[4].start = mWall[0].current;
	block[4].end = mWall[0].current + 128;
	wall[3].blockNo = 5;
	wall[3].sPos = 1250;
	wall[3].h = 380;
	wall[3].ePos = 1250 + 320;
	block[5].start = 1250;
	block[5].end = 1250 + 320;;
	for (int i = 1250 / 2; i <= (1250 + 320) / 2; i++)
	{
		height[i] = 380;
		blockData[i] = 5;
	}
	enemy[2].x = 1480;
	enemy[2].y = 380;
	enemy[2].blockNo = 5;
	enemy[2].a = 0;
	enemy[2].p = 0;
	enemy[2].k = 0;
	enemy[2].life = 50;
	enemy[2].dir = -1;

	/* ============================================================ */
	/* NEW CONTENT: the level now keeps going past wall[3]/block[5]  */
	/* with more platforms, a second elevator, a second moving wall, */
	/* two more switches and five more enemies (8 total).            */
	/* ============================================================ */

	// --- wall[4] / block[6]: platform right after the original end ---
	wall[4].blockNo = 6;
	wall[4].sPos = 1570;
	wall[4].h = 430;
	wall[4].ePos = 1570 + 320; // 1890
	block[6].start = 1570;
	block[6].end = 1890;
	for (int i = 1570 / 2; i <= 1890 / 2; i++)
	{
		height[i] = 430;
		blockData[i] = 6;
	}

	// enemy on block 6
	enemy[3].x = 1750;
	enemy[3].y = 430;
	enemy[3].blockNo = 6;
	enemy[3].a = 0;
	enemy[3].p = 0;
	enemy[3].k = 0;
	enemy[3].life = 50;
	enemy[3].dir = -1;

	// switch on block 6 that operates jWall[1]
	button[2].y = 430 + 48;
	button[2].x = 1750;
	button[2].OnOff = 0;

	// --- jWall[1] / block[7]: an elevator bridging block 6 (h=430) and block 8 (h=500) ---
	jWall[1].Pos = 1890;
	jWall[1].h = 430;
	jWall[1].d = 1;
	jWall[1].upperLimit = 550;
	jWall[1].lowerLimit = 300;
	jWall[1].buttonNo = 2;
	jWall[1].blockNo = 7;
	block[7].start = 1890;
	block[7].end = 1890 + 128; // 2018
	for (int i = jWall[1].Pos / 2; i < (jWall[1].Pos + 128) / 2; i++){
		height[i] = jWall[1].h;
		blockData[i] = 7;
	}

	// --- wall[5] / block[8]: platform after the second elevator ---
	wall[5].blockNo = 8;
	wall[5].sPos = 2018;
	wall[5].h = 500;
	wall[5].ePos = 2018 + 320; // 2338
	block[8].start = 2018;
	block[8].end = 2338;
	for (int i = 2018 / 2; i <= 2338 / 2; i++)
	{
		height[i] = 500;
		blockData[i] = 8;
	}

	// enemy on block 8
	enemy[4].x = 2200;
	enemy[4].y = 500;
	enemy[4].blockNo = 8;
	enemy[4].a = 0;
	enemy[4].p = 0;
	enemy[4].k = 13;
	enemy[4].life = 60;
	enemy[4].dir = 1;

	// switch on block 8 that operates mWall[1]
	button[3].y = 500 + 48;
	button[3].x = 2100;
	button[3].OnOff = 0;

	// --- mWall[1] / block[9]: second moving wall bridging block 8 (h=500) and block 10 (h=460) ---
	mWall[1].buttonNo = 3;
	mWall[1].left = 2338;
	mWall[1].right = 2600;
	mWall[1].current = mWall[1].left;
	mWall[1].h = 480;
	mWall[1].d = 1;
	mWall[1].blockNo = 9;
	for (int i = mWall[1].left / 2; i < mWall[1].right / 2 + 64; i++){
		height[i] = 0;
		blockData[i] = -1;
	}
	for (int i = mWall[1].current / 2; i < mWall[1].current / 2 + 64; i++){
		height[i] = mWall[1].h;
		blockData[i] = 9;
	}
	block[9].start = mWall[1].current;
	block[9].end = mWall[1].current + 128;

	// --- wall[6] / block[10]: platform after the second moving wall ---
	wall[6].blockNo = 10;
	wall[6].sPos = 2600;
	wall[6].h = 460;
	wall[6].ePos = 2600 + 320; // 2920
	block[10].start = 2600;
	block[10].end = 2920;
	for (int i = 2600 / 2; i <= 2920 / 2; i++)
	{
		height[i] = 460;
		blockData[i] = 10;
	}

	// enemy on block 10
	enemy[5].x = 2800;
	enemy[5].y = 460;
	enemy[5].blockNo = 10;
	enemy[5].a = 0;
	enemy[5].p = 0;
	enemy[5].k = 0;
	enemy[5].life = 50;
	enemy[5].dir = -1;

	// --- wall[7] / block[11]: final, larger arena with two enemies guarding it ---
	wall[7].blockNo = 11;
	wall[7].sPos = 2920;
	wall[7].h = 350;
	wall[7].ePos = 2920 + 400; // 3320
	block[11].start = 2920;
	block[11].end = 3320;
	for (int i = 2920 / 2; i <= 3320 / 2; i++)
	{
		height[i] = 350;
		blockData[i] = 11;
	}

	// two enemies guarding the final area
	enemy[6].x = 3100;
	enemy[6].y = 350;
	enemy[6].blockNo = 11;
	enemy[6].a = 0;
	enemy[6].p = 0;
	// The two guards in Level 1's final arena also start facing away
	// from each other, for the same reason as the Level 2 pairs above.
	enemy[6].k = 0;
	enemy[6].life = 70;
	enemy[6].dir = -1;

	enemy[7].x = 3250;
	enemy[7].y = 350;
	enemy[7].blockNo = 11;
	enemy[7].a = 0;
	enemy[7].p = 0;
	enemy[7].k = 13;
	enemy[7].life = 70;
	enemy[7].dir = 1;

	/* ============================ end of new content ============================ */

	// Make sure the animation/cooldown counters start at 0 for every
	// enemy (none of them are part of the per-enemy setup above), then
	// snapshot the roster below. `m` and `animCounter` are cleared here
	// too now that setupLevel1() can be re-run after another level -
	// otherwise a retried Level 1 would inherit whatever attack
	// cooldown/walk-cycle phase those slots were left in.
	for (int i = 0; i < EnemyNo; i++){
		enemy[i].atkAnim = 0;
		enemy[i].m = 0;
		enemy[i].animCounter = 0;
	}

	// Save a pristine copy of the fully-set-up enemy roster so
	// resetGame() can restore it exactly (positions, health, count)
	// whenever the player hits RETRY or MAIN MENU. Must happen here,
	// after every enemy[] field above has been assigned, and before
	// the game loop (iInitialize) starts modifying enemy[]/EnemyNo.
	EnemyNoBackup = EnemyNo;
	for (int i = 0; i < EnemyNo; i++){
		enemyBackup[i] = enemy[i];
	}

	// Snapshot every switch (OFF) and every elevator/moving wall at its
	// starting position/height - see the "Switch / moving-wall /
	// elevator backup" comment above buttonBackup/jWallBackup/mWallBackup.
	for (int i = 0; i < buttonNo; i++){
		buttonBackup[i] = button[i];
	}
	for (int i = 0; i < jWallNo; i++){
		jWallBackup[i] = jWall[i];
	}
	for (int i = 0; i < mWallNo; i++){
		mWallBackup[i] = mWall[i];
	}

	// Span for the top-of-screen progress bar. Level 1 is finished when
	// the last guard falls, so the furthest guard is the point the player
	// physically has to reach - read from the roster just built rather
	// than hard-coded, so it follows the level design.
	setLevelProgressSpanFromEnemies(300);

	// Load/open the attack sound before the game starts so the first E press
	// does not have to pay the MP3 opening/decoding cost.
}

void setupLevel2()
{
	// Level 2 is deliberately different from Level 1: lower platforms,
	// a starting elevator/bridge, raised platforms, a second elevator,
	// two moving bridges, and a final arena.
	// Everything is rebuilt from scratch whenever Level 2 is entered/retried.
	wallNo = 9;
	jWallNo = 2;
	mWallNo = 3;
	buttonNo = 3; // two switches fewer than the original 5 - see the removed 1st and 2nd switches below.
	EnemyNo = 11;

	// Disable Level 1's special double-height area.
	doubleS[0] = -10000;
	doubleE[0] = -9000;

	// Cave bats: empty the sky, stand the Prince up, and re-arm the
	// first-bat delay, so a RETRY never inherits a bat that was
	// mid-swoop or a crouch that was half-finished. The seed is taken
	// from the clock so the bats do not arrive on identical beats every
	// single run (the game otherwise avoids rand() entirely - see
	// level2BatRandom() in controls.h).
	clearLevel2Bats();
	level2BatSeed = (int)clock();

	// Clear the playable height map so old platforms cannot remain.
	// Widened from the original 2700 entries to the whole array: Level 3
	// builds platforms well past index 2700, so a Level 3 -> Level 2
	// return (MAIN MENU, then replaying) would otherwise have left
	// Level 3's floor floating in the middle of Level 2's sky.
	for (int i = 0; i < 50000; i++){
		height[i] = 0;
		blockData[i] = -1;
	}

	// Level 2's switch-driven obstacles must not inherit an alwaysOn
	// flag from Level 3 (same class of bug the setupLevel1() comment
	// above describes). jWall[0] and mWall[2] set theirs to 1 below on
	// purpose - these three are the ones that must stay switch-gated.
	jWall[1].alwaysOn = 0;
	mWall[0].alwaysOn = 0;
	mWall[1].alwaysOn = 0;

	// Start area: a long low stone ledge.
	wall[0].blockNo = 0;
	wall[0].sPos = 120;
	wall[0].h = 170;
	wall[0].ePos = 480;
	block[0].start = 120;
	block[0].end = 480;
	for (int i = 60; i <= 240; i++){
		height[i] = 170;
		blockData[i] = 0;
	}

	// First step: there is deliberately NO enemy here.
	// The player starts on this quiet platform and reaches the next area
	// using the small elevator/bridge immediately at its right edge.
	// Platform 1: raised ledge after the first crossing.
	// Keep this within MAX_JUMP_HEIGHT of the starting platform so the
	// player can actually climb from the first bridge to this rock.
	wall[1].blockNo = 1;
	wall[1].sPos = 608;
	wall[1].h = 240;
	wall[1].ePos = 930;
	block[1].start = 608;
	block[1].end = 930;
	for (int i = 304; i <= 465; i++){
		height[i] = 240;
		blockData[i] = 1;
	}

	// Elevator/bridge 1: starts flush with the first platform and ends
	// flush with Platform 1, so there is an actual route forward from
	// the starting area instead of an unreachable gap.
	// The switch that used to raise this (the old 1st switch, button[0])
	// has been removed from the level - this elevator now just moves up
	// and down on its own the whole time (alwaysOn) instead.
	// The top is 240, only 70px above the start, so the player can
	// climb from the bridge onto the next rock (MAX_JUMP_HEIGHT=80).
	jWall[0].Pos = 480;
	jWall[0].h = 170;
	jWall[0].d = 1;
	jWall[0].upperLimit = 240;
	jWall[0].lowerLimit = 170;
	jWall[0].alwaysOn = 1;
	jWall[0].blockNo = 2;
	block[2].start = 480;
	block[2].end = 608;
	for (int i = 240; i < 304; i++){
		height[i] = 170;
		blockData[i] = 2;
	}

	// Platform 2: the second rock.
	// It is now at the SAME height as the first rock. There is no jump
	// between these two rocks; the player crosses the gap using the
	// horizontal moving bridge below.
	wall[2].blockNo = 3;
	wall[2].sPos = 1058;
	wall[2].h = 240;
	wall[2].ePos = 1410;
	block[3].start = 1058;
	block[3].end = 1410;
	for (int i = 529; i <= 705; i++){
		height[i] = 240;
		blockData[i] = 3;
	}

	// Moving bridge: this is the ONLY way from the first rock to the
	// second rock. It used to be started with a switch at x=880 (the
	// 2nd switch encountered along the route, right after button[0]).
	// That switch has been removed - this bridge now just runs back
	// and forth on its own the whole time (alwaysOn) instead.
	mWall[2].alwaysOn = 1;
	mWall[2].left = 930;
	mWall[2].right = 1058;
	mWall[2].current = 930;
	mWall[2].h = 240;
	mWall[2].d = 1;
	mWall[2].blockNo = 13;
	block[13].start = 930;
	block[13].end = 1058;

	// NEW CONNECTOR ROCK: this is the extra rock between the second rock
	// (320px high) and the third rock. It removes the long empty gap and
	// gives the player a clear stepping-stone route.
	wall[3].blockNo = 4;
	wall[3].sPos = 1410;
	wall[3].h = 300;
	wall[3].ePos = 1600;
	block[4].start = 1410;
	block[4].end = 1600;
	for (int i = 705; i <= 800; i++){
		height[i] = 300;
		blockData[i] = 4;
	}

	// Moving bridge 1 - the one the FIRST switch (button[0]) drives.
	//
	// FIX: this slab used to sit idle at x=1600, spanning 1600..1728 -
	// i.e. parked across the near half of the very gap it exists to
	// bridge. Since the rock on the far side (wall[4]) starts at 1760, a
	// single 100px step off the STATIONARY slab landed straight on it,
	// so the player could walk across without ever touching the switch
	// and the whole mechanism was decorative.
	//
	// (It used to be hidden by a bug rather than by design: the bridge's
	// travel rail wipes the collision map 128px past its right rail, so
	// wall[4] had no floor until 1888 and the step fell short. Putting
	// that floor back - the visible rock you could previously fall
	// straight through - is what exposed this.)
	//
	// The rail now starts 128px further west, at 1472, so the idle slab
	// rests entirely ON the connector rock (1410..1600) instead of over
	// the gap. The 1600..1760 gap is then genuinely open whenever the
	// switch is off: the longest step available from the idle slab's
	// edge reaches 1699, still 61px short of the far rock, and the same
	// is true stepping back the other way. The switch has to be used.
	//
	// Nothing about the platforms, the rock heights or the far side
	// moved - only where this slab waits when it is not running.
	mWall[0].buttonNo = 0;
	mWall[0].left = 1472;        // idle position: parked on the connector rock
	mWall[0].right = 1760;       // docks against the far rock
	mWall[0].current = 1472;
	mWall[0].h = 300;
	mWall[0].d = 1;
	mWall[0].blockNo = 5;
	for (int i = 736; i < 944; i++){   // clear the whole rail, 1472..1888
		height[i] = 0;
		blockData[i] = -1;
	}
	for (int i = 736; i < 800; i++){   // the slab itself, idle at 1472..1600
		height[i] = 300;
		blockData[i] = 5;
	}
	block[5].start = 1472;
	block[5].end = 1600;

	// Platform 3: third rock after the new connector + moving bridge.
	wall[4].blockNo = 12;
	wall[4].sPos = 1760;
	wall[4].h = 300;
	wall[4].ePos = 2090;
	block[12].start = 1760;
	block[12].end = 2090;
	for (int i = 880; i <= 1045; i++){
		height[i] = 300;
		blockData[i] = 12;
	}

	// Elevator 2: rises from the middle ledge to the upper route.
	jWall[1].Pos = 2090;
	jWall[1].h = 300;
	jWall[1].d = 1;
	jWall[1].upperLimit = 500;
	jWall[1].lowerLimit = 270;
	jWall[1].buttonNo = 1;
	jWall[1].blockNo = 6;
	block[6].start = 2090;
	block[6].end = 2218;
	for (int i = 1045; i < 1109; i++){
		height[i] = 300;
		blockData[i] = 6;
	}

	// Platform 4: upper balcony.
	wall[5].blockNo = 7;
	wall[5].sPos = 2218;
	wall[5].h = 500;
	wall[5].ePos = 2550;
	block[7].start = 2218;
	block[7].end = 2550;
	for (int i = 1109; i <= 1275; i++){
		height[i] = 500;
		blockData[i] = 7;
	}

	// Moving bridge 2: final long crossing.
	mWall[1].buttonNo = 2;
	mWall[1].left = 2550;
	mWall[1].right = 2920;
	mWall[1].current = 2550;
	mWall[1].h = 400;
	mWall[1].d = 1;
	mWall[1].blockNo = 8;
	for (int i = 1275; i < 1492; i++){
		height[i] = 0;
		blockData[i] = -1;
	}
	for (int i = 1275; i < 1339; i++){
		height[i] = 400;
		blockData[i] = 8;
	}
	block[8].start = 2550;
	block[8].end = 2678;

	// Platform 5: final chamber entrance.
	wall[6].blockNo = 9;
	wall[6].sPos = 2920;
	wall[6].h = 400;
	wall[6].ePos = 3280;
	block[9].start = 2920;
	block[9].end = 3280;
	for (int i = 1460; i <= 1640; i++){
		height[i] = 400;
		blockData[i] = 9;
	}

	// Extra high ledge: forces a final climb before the boss-like arena.
	wall[7].blockNo = 10;
	wall[7].sPos = 3280;
	wall[7].h = 480;
	wall[7].ePos = 3620;
	block[10].start = 3280;
	block[10].end = 3620;
	for (int i = 1640; i <= 1810; i++){
		height[i] = 480;
		blockData[i] = 10;
	}

	// Final arena.
	wall[8].blockNo = 11;
	wall[8].sPos = 3620;
	wall[8].h = 340;
	wall[8].ePos = 4200;
	block[11].start = 3620;
	block[11].end = 4200;
	for (int i = 1810; i <= 2100; i++){
		height[i] = 340;
		blockData[i] = 11;
	}

	// Switches: each controls one moving/elevator obstacle.
	// Two switches have been removed from the level entirely:
	// - the old 1st switch (x=400, right at the start, which used to
	//   raise jWall[0]) - that elevator now just moves up and down on
	//   its own the whole time (see jWall[0].alwaysOn above).
	// - the 2nd switch encountered along the route (x=880, which used
	//   to start the bridge from the first rock to the second rock) -
	//   that bridge (mWall[2]) also now just runs on its own (see
	//   mWall[2].alwaysOn above).
	// The remaining switches keep their original positions, packed down
	// to indices 0-2.
	button[0].x = 1240; button[0].y = 288; button[0].OnOff = 0; // starts mWall[0]
	button[1].x = 1940; button[1].y = 348; button[1].OnOff = 0; // raises jWall[1]
	button[2].x = 2400; button[2].y = 548; button[2].OnOff = 0; // starts mWall[1]

	// 11 enemies spread across the route. There is intentionally NO enemy
	// on the starting platform (block 0). The first enemies appear only
	// after the player crosses the first elevator/bridge.
	// The pair on the second rock (block 3) used to start at 1190/1360.
	// 1190 sits inside the 128px that the moving bridge mWall[2] covers
	// whenever it is docked against that rock, so that guard spent its
	// life being shoved back and forth by the bridge instead of
	// patrolling. Both are now east of the bridge's reach (1186), with
	// enough room between them to walk opposite halves of the rock.
	int ex[11] = { 720, 850, 1240, 1370, 1930, 2020, 2380, 2480, 3050, 3440, 3800 };
	int ey[11] = { 240, 240, 240, 240, 300, 300, 500, 500, 400, 480, 340 };
	int eb[11] = { 1, 1, 3, 3, 12, 12, 7, 7, 9, 10, 11 };
	int el[11] = { 50, 50, 55, 55, 50, 55, 60, 65, 65, 75, 90 };
	// Facing (k) and patrol direction (d). Every platform that carries a
	// PAIR of guards (blocks 1, 3, 12 and 7) now starts them walking
	// AWAY from each other - the left one heads left, the right one
	// heads right - so each takes a half of the rock and the two are
	// visibly patrolling in opposite directions from the first frame.
	// They used to start walking straight at each other, which meant
	// they collided immediately and (before the mutual-turn fix in
	// controls.h) ended up trudging off in the same direction together.
	int ek[11] = { 0, 13, 0, 13, 0, 13, 0, 13, 0, 13, 0 };
	int ed[11] = { -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1 };
	for (int i = 0; i < 11; i++){
		enemy[i].x = ex[i];
		enemy[i].y = ey[i];
		enemy[i].blockNo = eb[i];
		enemy[i].life = el[i];
		enemy[i].a = 0;
		enemy[i].p = 0;
		enemy[i].k = ek[i];
		enemy[i].m = 0;
		enemy[i].dir = ed[i];
		enemy[i].atkAnim = 0;
		enemy[i].animCounter = 0;
	}

	// Span for the top-of-screen progress bar. Level 2, like Level 1,
	// ends when the last guard falls, so the furthest guard is the point
	// the player physically has to reach.
	setLevelProgressSpanFromEnemies(300);

	// Start Level 2 at the left side of the dungeon.
	playerX = 300;
	playerY = 170;
	cX = 0;
	cEnemy = -1;
	gravitalForce = 0;
	fallCount = 0;
	landingState = 0;
	// FIX: same as resetGame() - the player is placed above the floor he
	// is about to stand on, so arm the one-landing skip and let that first
	// settle be free instead of charging fall damage for it.
	skipFallDamage = 1;
	enemyDamageAccum = 0.0;
	clearDrowning();

	// Health hearts. Rolled LAST, after every platform above has been
	// stamped into height[], because spawnHearts() picks its spots by
	// reading that collision map - so it always follows the level that
	// was actually built and can never hang a heart over thin air.
	//
	// The seed mixes the clock with whatever it was left at, so the
	// layout differs between runs AND between retries within a run.
	// Level 2 gets 2-3; Level 3 gets 5-7 (see LEVEL2_HEART_MIN/MAX).
	heartSeed = (int)((unsigned int)clock() ^ ((unsigned int)heartSeed * 1103515245u));
	spawnHearts(LEVEL2_HEART_MIN + nextRandom(&heartSeed, LEVEL2_HEART_MAX - LEVEL2_HEART_MIN + 1),
		500, 4100);
}


/* ================================================================== */
/* LEVEL 3 - "THE FINAL ESCAPE"                                        */
/*                                                                     */
/* The third and final level. Built with exactly the same primitives   */
/* as Levels 1 and 2 - static wall[] platforms, jWall[] elevators,     */
/* mWall[] moving bridges, button[] switches, enemy[] guards, and the  */
/* single shared height[]/blockData[] collision map - plus two new     */
/* level-3-only systems layered on top: collapsing slabs and one       */
/* rising flood (see the LEVEL 3 block in game_globals.h).             */
/*                                                                     */
/* Route, west to east (all numbers are world x / Wall::h heights):    */
/*                                                                     */
/* STAGE 1  x     0-1450  h 170   calm entry hall; teaches collapsing  */
/*                                slabs on flat, unhurried ground      */
/* STAGE 2  x  1450-2600  h 170   the palace starts shaking: a moving  */
/*                        ->280   bridge and the first elevator        */
/* STAGE 3  x  2600-4328  h 280   the flood rises hard; the route      */
/*                        ->380   turns upward                         */
/* STAGE 4  x  4328-5828  h 380   everything at once - slabs, bridges, */
/*                                switches, gaps and guards            */
/* STAGE 5  x  5828-6550  h 470   the Guardian's arena                 */
/* ESCAPE   x  6550-7700  h 470   opens only when the Guardian falls   */
/*                                                                     */
/* Every platform hands over to the next at exactly the height the     */
/* Prince is already standing at, or one MAX_JUMP_HEIGHT step below    */
/* it, so there is never a jump the player cannot see the landing of.  */
/* ================================================================== */

/* Writes one static platform's floor into the SHARED collision map and
 * records its extent in block[] - the two things every other system in
 * the game (player collision, enemy patrol edges, the edge-of-block
 * movement hints) reads. Deliberately half-open [sPos, ePos) so two
 * platforms that touch never fight over the tile on the seam. */
inline void level3StampRange(int sPos, int ePos, int h, int blockNo)
{
	for (int i = sPos / 2; i < ePos / 2; i++){
		height[i] = h;
		blockData[i] = blockNo;
	}
	block[blockNo].start = sPos;
	block[blockNo].end = ePos;
}

/* One static palace platform. `stamp` is 0 for the escape-route
 * platforms, which exist in wall[] from the start (so the level data is
 * all in one place) but have NO collision and are not drawn until the
 * Guardian dies and the gate behind the arena opens. */
inline void level3MakeWall(int idx, int sPos, int ePos, int h, int blockNo, int stamp)
{
	wall[idx].sPos = sPos;
	wall[idx].ePos = ePos;
	wall[idx].h = h;
	wall[idx].blockNo = blockNo;
	block[blockNo].start = sPos;
	block[blockNo].end = ePos;
	if (stamp) level3StampRange(sPos, ePos, h, blockNo);
}

/* One collapsing slab. Always exactly LEVEL3_CP_WIDTH (128px) wide, so
 * it is drawn out of the same two corner tiles an elevator or a moving
 * bridge uses and reads as part of the same palace. Its collision is
 * (re)written every logic tick by level3StampCollapse() in controls.h,
 * which is also what removes it the moment the slab stops being solid. */
inline void level3MakeCollapse(int idx, int sPos, int h, int blockNo, int warnTicks, int enabled, int forcedOnly)
{
	level3Collapse[idx].sPos = sPos;
	level3Collapse[idx].ePos = sPos + LEVEL3_CP_WIDTH;
	level3Collapse[idx].h = h;
	level3Collapse[idx].blockNo = blockNo;
	level3Collapse[idx].warnTicks = warnTicks;
	level3Collapse[idx].enabled = enabled;
	level3Collapse[idx].forcedOnly = forcedOnly;
	level3Collapse[idx].state = LEVEL3_CP_SOLID;
	level3Collapse[idx].timer = 0;
	level3Collapse[idx].fallY = 0;
	level3Collapse[idx].shakeSeed = idx * 7 + 3;

	block[blockNo].start = sPos;
	block[blockNo].end = sPos + LEVEL3_CP_WIDTH;
}

inline void level3MakeElevator(int idx, int pos, int low, int high, int blockNo, int btn, int always)
{
	jWall[idx].Pos = pos;
	jWall[idx].h = low;
	jWall[idx].d = 1;
	jWall[idx].lowerLimit = low;
	jWall[idx].upperLimit = high;
	jWall[idx].buttonNo = btn;
	jWall[idx].blockNo = blockNo;
	jWall[idx].alwaysOn = always;

	for (int i = pos / 2; i < (pos + 128) / 2; i++){
		height[i] = low;
		blockData[i] = blockNo;
	}
	block[blockNo].start = pos;
	block[blockNo].end = pos + 128;
}

inline void level3MakeBridge(int idx, int left, int right, int h, int blockNo, int btn, int always)
{
	mWall[idx].left = left;
	mWall[idx].right = right;
	mWall[idx].current = left;
	mWall[idx].h = h;
	mWall[idx].d = 1;
	mWall[idx].buttonNo = btn;
	mWall[idx].blockNo = blockNo;
	mWall[idx].alwaysOn = always;

	// Clear the whole rail the bridge can ever occupy, then stamp only
	// the 128px the bridge tile is actually on right now - the same two
	// loops update() runs for every moving wall on every logic tick.
	for (int i = left / 2; i < right / 2 + 64; i++){
		height[i] = 0;
		blockData[i] = -1;
	}
	for (int i = left / 2; i < left / 2 + 64; i++){
		height[i] = h;
		blockData[i] = blockNo;
	}
	block[blockNo].start = left;
	block[blockNo].end = left + 128;
}

void setupLevel3()
{
	// Level 3 is rebuilt completely from scratch every single time it is
	// entered OR retried (resetGame() routes straight here for
	// STATE_LEVEL3), so nothing - no collapsed slab, no raised water, no
	// flipped switch, no wounded Guardian, no Level 2 leftovers - can
	// ever survive into the next run.
	wallNo = LEVEL3_WALLS_MAIN;   // grows to LEVEL3_WALLS_TOTAL when the escape gate opens
	jWallNo = 3;
	mWallNo = 3;
	buttonNo = 4;
	EnemyNo = 8;
	level3CollapseNo = LEVEL3_CP_COUNT;

	// Disable Level 1's special double-height area (same as Level 2).
	doubleS[0] = -10000;
	doubleE[0] = -9000;

	// Wipe the entire shared collision map first - Level 3 reaches far
	// further east than either earlier level, and nothing from a
	// previous run may remain underneath it.
	for (int i = 0; i < 50000; i++){
		height[i] = 0;
		blockData[i] = -1;
	}

	/* -------------------------------------------------------------- */
	/* STAGE 1 - the entry hall. Flat, quiet, and the only place the    */
	/* collapsing-slab mechanic is taught on its own: two slabs, both   */
	/* at the same height as the ledges either side of them, with the   */
	/* longest warning time in the level (2s and ~1.7s).                */
	/* -------------------------------------------------------------- */
	level3MakeWall(0, 100, 520, 170, 0, 1);              // start ledge
	level3MakeCollapse(0, 520, 170, 50, 120, 1, 0);         // FIRST collapsing slab - 2s warning
	level3MakeWall(1, 648, 1000, 170, 1, 1);             // safe ground + first guard
	level3MakeCollapse(1, 1000, 170, 51, 100, 1, 0);        // second slab, slightly quicker
	level3MakeWall(2, 1128, 1450, 170, 2, 1);            // switch platform

	/* -------------------------------------------------------------- */
	/* STAGE 2 - the palace begins to shake. A switch-driven moving     */
	/* bridge over open water, then the first elevator lifts the route  */
	/* from 170 up to 280.                                             */
	/* -------------------------------------------------------------- */
	level3MakeBridge(0, 1450, 1700, 170, 30, 0, 0);      // switch 0
	level3MakeWall(3, 1828, 2150, 170, 3, 1);
	level3MakeElevator(0, 2150, 170, 280, 40, 1, 0);     // switch 1
	level3MakeWall(4, 2278, 2600, 280, 4, 1);

	/* -------------------------------------------------------------- */
	/* STAGE 3 - the flood rises hard enough to swallow every 170-high  */
	/* ledge behind the player, and the route climbs again to 380.      */
	/* -------------------------------------------------------------- */
	level3MakeCollapse(2, 2600, 280, 52, 90, 1, 0);
	level3MakeWall(5, 2728, 3050, 280, 5, 1);
	level3MakeElevator(1, 3050, 280, 380, 41, 2, 0);     // switch 2
	level3MakeWall(6, 3178, 3500, 380, 6, 1);
	level3MakeCollapse(3, 3500, 380, 53, 80, 1, 0);
	level3MakeWall(7, 3628, 3950, 380, 7, 1);

	/* -------------------------------------------------------------- */
	/* STAGE 4 - everything the level has taught, combined: a second    */
	/* switch-driven bridge, a slab between two guarded platforms, and  */
	/* a bridge that runs on its own so it has to be timed rather than  */
	/* started.                                                        */
	/* -------------------------------------------------------------- */
	level3MakeBridge(1, 3950, 4200, 380, 31, 3, 0);      // switch 3
	level3MakeWall(8, 4328, 4650, 380, 8, 1);
	level3MakeCollapse(4, 4650, 380, 54, 75, 1, 0);
	level3MakeWall(9, 4778, 5050, 380, 9, 1);
	level3MakeBridge(2, 5050, 5250, 380, 32, 0, 1);      // no switch - always running
	level3MakeWall(10, 5378, 5700, 380, 10, 1);

	/* -------------------------------------------------------------- */
	/* STAGE 5 - the Guardian's arena, reached by an elevator that runs */
	/* on its own. The two slabs at 5828 and 5956 are the arena's own   */
	/* approach ledges: they are NOT triggered by standing on them,     */
	/* they are brought down by the Guardian fight itself (phase 2 and  */
	/* phase 3), which is what physically shrinks the safe ground while */
	/* the battle is still going on.                                    */
	/* -------------------------------------------------------------- */
	level3MakeElevator(2, 5700, 380, 470, 42, 0, 1);     // no switch - always running
	level3MakeCollapse(5, 5828, 470, 55, 60, 1, 1);         // falls in boss phase 2
	level3MakeCollapse(6, 5956, 470, 56, 60, 1, 1);         // falls in boss phase 3
	level3MakeWall(11, 6084, LEVEL3_GATE_X, 470, LEVEL3_ARENA_BLOCK, 1);

	/* -------------------------------------------------------------- */
	/* THE FINAL ESCAPE - sealed behind the arena gate. These four      */
	/* platforms and two slabs have NO collision and are not drawn      */
	/* until the Guardian falls (see the escape branch of               */
	/* updateLevel3() in controls.h, which raises wallNo to             */
	/* LEVEL3_WALLS_TOTAL and stamps their floors in).                  */
	/* -------------------------------------------------------------- */
	level3MakeWall(12, LEVEL3_GATE_X, 6800, 470, 12, 0);
	level3MakeCollapse(7, 6800, 470, 57, 45, 0, 0);         // enabled on escape
	level3MakeWall(13, 6928, 7180, 470, 13, 0);
	level3MakeCollapse(8, 7180, 470, 58, 45, 0, 0);         // enabled on escape
	level3MakeWall(14, 7308, 7700, 470, 14, 0);
	level3ExitX = 7520;                                  // the palace door

	/* ---------------- Switches ---------------- */
	// button.y sits one half-tile above the platform surface, exactly
	// like every switch in Levels 1 and 2.
	button[0].x = 1330; button[0].y = 170 + 48; button[0].OnOff = 0; // starts bridge mWall[0]
	button[1].x = 2110; button[1].y = 170 + 48; button[1].OnOff = 0; // raises elevator jWall[0]
	button[2].x = 2960; button[2].y = 280 + 48; button[2].OnOff = 0; // raises elevator jWall[1]
	button[3].x = 3860; button[3].y = 380 + 48; button[3].OnOff = 0; // starts bridge mWall[1]

	/* ---------------- Ordinary palace guards ---------------- */
	// Deliberately NONE on the starting ledge and none on any collapsing
	// slab or bridge: a slab the player has to fight on top of would be
	// unreadable, and the slabs are the one thing in the level that is
	// always about moving, not standing.
	//
	// Eight guards, one per platform, and DELIBERATELY WEAKER than
	// Level 2's (40-55 life here against Level 2's 50-90). Level 3's
	// difficulty is meant to come from the palace itself - the slabs
	// giving way, the water climbing, the clock - not from the same
	// fights with bigger numbers on them. The player also has to arrive
	// at the Guardian with enough health left to actually fight it.
	int ex[8] = {  850, 1990, 2450, 2900, 3350, 4500, 4930, 5550 };
	int ey[8] = {  170,  170,  280,  280,  380,  380,  380,  380 };
	int eb[8] = {    1,    3,    4,    5,    6,    8,    9,   10 };
	int el[8] = {   40,   45,   45,   45,   50,   50,   55,   55 };
	int ek[8] = {    0,   13,    0,   13,    0,   13,    0,   13 };
	int ed[8] = {   -1,    1,   -1,    1,   -1,    1,   -1,    1 };
	for (int i = 0; i < EnemyNo; i++){
		enemy[i].x = ex[i];
		enemy[i].y = ey[i];
		enemy[i].blockNo = eb[i];
		enemy[i].life = el[i];
		enemy[i].a = 0;
		enemy[i].p = 0;
		enemy[i].k = ek[i];
		enemy[i].m = 0;
		enemy[i].dir = ed[i];
		enemy[i].atkAnim = 0;
		enemy[i].animCounter = 0;
	}

	/* ---------------- The final Guardian ---------------- */
	level3Boss.x = 6400;
	level3Boss.y = 470;
	level3Boss.life = LEVEL3_BOSS_MAX_LIFE;
	level3Boss.maxLife = LEVEL3_BOSS_MAX_LIFE;
	level3Boss.blockNo = LEVEL3_ARENA_BLOCK;
	level3Boss.k = 0;              // facing left, toward the approaching Prince
	level3Boss.p = 0;
	level3Boss.animCounter = 0;
	level3Boss.a = 0;
	level3Boss.atkAnim = 0;
	level3Boss.m = 0;
	level3Boss.dir = -1;
	level3Boss.active = 0;
	level3Boss.hitFlash = 0;
	level3Boss.dying = 0;
	level3Boss.deathTimer = 0;
	level3Boss.dead = 0;

	/* ---------------- Level 3 runtime state ---------------- */
	level3Active = 1;
	level3Timer = 0;
	level3Stage = 1;
	level3WaterLevel = LEVEL3_WATER_START;
	level3WaterTarget = LEVEL3_WATER_START;
	level3WaterTick = 0;
	level3WaterRiseTicks = LEVEL3_WATER_RISE_TICKS;
	level3MaxPlayerX = 0;
	level3CollapseState = 0;
	level3ShakeLevel = 0;
	level3BossActive = 0;
	level3BossPhase = 1;
	level3EscapeMode = 0;
	level3EscapeTimer = 0;
	level3ExitReached = 0;
	level3BannerTimer = 0;
	level3BannerText[0] = '\0';

	// Write the starting collision for every slab that is part of the
	// world right now, BEFORE the first frame - otherwise the Prince
	// would drop straight through the first slab he stepped onto.
	level3StampCollapse();

	// Span for the top-of-screen progress bar. Level 3 really does end at
	// a world x - the palace door - and that is the exact same
	// level3ExitX the completion test in updateLevel3() compares against,
	// so the bar reaches full precisely as the level is won.
	setLevelProgressSpan(300, level3ExitX);

	/* ---------------- Player ---------------- */
	playerX = 300;
	playerY = 170;
	cX = 0;
	k = 13;                 // facing right, into the palace
	p = 0;
	cEnemy = -1;
	gravitalForce = 0;
	fallCount = 0;
	landingState = 0;
	// FIX: same as resetGame() - the player is placed above the floor he
	// is about to stand on, so arm the one-landing skip and let that first
	// settle be free instead of charging fall damage for it.
	skipFallDamage = 1;
	atMode = 0;
	playerAtkLastFrame = -1;
	hopTimer = 0;
	playerIsMoving = 0;
	lastMoveKeyTime = 0;
	enemyDamageAccum = 0.0;
	playerMoveAccum = 0.0;
	clearDrowning();

	// The water: empty the swim state and put the piranhas back in.
	// Level 3's flood no longer drowns him, so this is what replaces
	// that - see the LEVEL 3 - THE WATER block in controls.h.
	clearLevel3Water();
	level3SpawnFish();

	// Health hearts - same as setupLevel2(), rolled last so the spots
	// come from the collision map this level just built. Level 3 gets
	// noticeably more of them than Level 2: it is longer, the flood
	// eventually swallows the low ones, and the Guardian is waiting at
	// the end of it.
	heartSeed = (int)((unsigned int)clock() ^ ((unsigned int)heartSeed * 1103515245u));
	spawnHearts(LEVEL3_HEART_MIN + nextRandom(&heartSeed, LEVEL3_HEART_MAX - LEVEL3_HEART_MIN + 1),
		500, 6400);

	level3SetBanner("STAGE 1  -  THE FINAL ESCAPE", 260);
}

// P key: skip Level 1 and go directly to the redesigned Level 2.
inline void handlePButton()
{
    if (gameState == STATE_GAME)
    {
        resetGame();
        gameState = STATE_LEVEL2;
        setupLevel2();
        startGameBGM();
    }
}

// O key: jump straight to Level 3 for testing, from ANY screen (home,
// either earlier level, the level-complete screen, game over, or the
// victory screen). Unlike the P shortcut this is not gated to one
// level, because the whole point of it is to reach the final level
// without having to play through the first two.
//
// It goes through the normal entry path - resetGame() dispatches to
// setupLevel3() because gameState is already STATE_LEVEL3 - so the
// level is built exactly the same way the NEXT LEVEL button builds it,
// with a full player/health/state reset. It never touches Level 1 or
// Level 2's own progression.
inline void handleOButton()
{
    stopVictoryBGM();
    gameState = STATE_LEVEL3;
    resetGame();      // -> setupLevel3()
    startGameBGM();
}

#endif
