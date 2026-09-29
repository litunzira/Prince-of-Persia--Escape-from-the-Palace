#ifndef CONTROLS_H
#define CONTROLS_H

#include "game_globals.h"
#include "records.h"
#include "sounds.h"
#include "combat.h"
#include "reset.h"
#include "home.h"
#include "next_level.h"

// ------------------------------------------------------------------
// True floor edges for an enemy's own block.
//
// Enemy patrol/chase used to clamp against block[enemy.blockNo].start
// / .end - level-design metadata that is set by hand for static
// platforms and only kept in sync for moving platforms because
// controls.h's Moving Wall/Elevator update happens to overwrite it
// every tick. That is a SECOND copy of "where does this platform
// end", separate from height[]/blockData[] (the array the player's
// own collision - playerHeight()/getHeight() - actually reads). The
// two can drift apart: e.g. a moving bridge's block[].start/end is
// initialized once at level-load with the bridge's FULL travel rail
// rather than its actual 128px tile, and is only corrected retro-
// actively on the first physics tick. Any future level edit that
// updates one number and not the other reintroduces the same gap.
// That drift is exactly what let enemies visibly run past the edge
// of their rock: the CLAMP said one edge, but the drawn/collidable
// floor (blockData[]) ended somewhere else.
//
// These two helpers remove the second copy entirely: they walk the
// SAME blockData[] array outward from the enemy's current position
// until it stops matching the enemy's own blockNo, and return that
// as the true world-x edge. An enemy clamped against this can never
// run past where the floor it's standing on actually stops, on any
// platform, moving or static, now or after any future level change -
// because there is only one source of truth for "where is the floor"
// left in the game (this is the exact same array getHeight() reads).
inline int enemyLeftEdge(int enemyX, int blockNo)
{
	int idx = enemyX / 2;
	if (idx < 0) idx = 0;
	if (idx > 49999) idx = 49999;
	while (idx > 0 && blockData[idx - 1] == blockNo) idx--;
	return idx * 2;
}
inline int enemyRightEdge(int enemyX, int blockNo)
{
	int idx = enemyX / 2;
	if (idx < 0) idx = 0;
	if (idx > 49999) idx = 49999;
	while (idx < 49999 && blockData[idx + 1] == blockNo) idx++;
	return idx * 2;
}

// A handful of special platforms - currently just Level 1's
// double-height start block (block 0/1, controlled entirely through
// doubleS/doubleE/doubleUp/doubleDown in playerHeight()/getHeight())
// - never write their tiles into blockData[] at all, because their
// collision is handled by that separate special-case path instead.
// For those, blockData[] at the enemy's own position never matches
// its blockNo, so the scan above would immediately return a
// zero-width range and freeze the enemy in place. Detect that one
// case and fall back to the original block[].start/end range, which
// is exactly right for these hand-placed, never-moving platforms
// (they have no runtime updater that could let it drift, unlike a
// moving bridge/elevator).
inline void enemyPatrolRange(int enemyX, int blockNo, int *lo, int *hi)
{
	if (blockData[enemyX / 2] == blockNo){
		*lo = enemyLeftEdge(enemyX, blockNo);
		*hi = enemyRightEdge(enemyX, blockNo);
	}
	else{
		*lo = block[blockNo].start;
		*hi = block[blockNo].end;
	}
}

// How far (in the given direction) enemy `self` may travel before it
// would visually overlap another living enemy on the same block that
// is already further along in that same direction.
//
// Without this, every enemy's chase movement only looks at its own
// distance to the player (see the chase branch below) - it has no
// idea where any OTHER enemy on the same block is standing. So when
// two enemies converge on the player from the same side, the farther
// one just keeps walking (it's still further than COMBAT_RANGE from
// the player) straight through the nearer one, which has already
// stopped - stacking both enemy sprites on top of each other the
// moment they're both "attacking".
//
// direction: -1 = enemy is moving toward smaller x, +1 = toward
// bigger x. Returns the nearest blocking x-coordinate in that
// direction (a sentinel far away if nothing is in the way), so the
// caller can clamp its own movement/position against it - the same
// way `chaseLo`/`chaseHi` clamp movement against the block's edges.
// `blockerOut` (optional) receives the index of the enemy that actually
// set the limit, or -1 if nothing is in the way. The patrol code needs
// to know WHO it bumped into so it can turn that enemy around as well -
// see the mutual-turn fix at the bounce below.
inline int enemyQueueLimit(int self, int direction, int *blockerOut = 0)
{
	int limit = (direction < 0) ? -1000000 : 1000000;
	int blocker = -1;

	for (int i = 0; i < EnemyNo; i++){
		if (i == self) continue;
		if (enemy[i].life <= 0) continue;
		if (enemy[i].blockNo != enemy[self].blockNo) continue;

		if (direction < 0){
			// self is moving left - anything already at or to self's
			// left blocks how far left self may still go.
			if (enemy[i].x <= enemy[self].x){
				int blockedAt = enemy[i].x + ENEMY_IMG_SIZE;
				if (blockedAt > limit){ limit = blockedAt; blocker = i; }
			}
		}
		else {
			// self is moving right - anything already at or to
			// self's right blocks how far right self may still go.
			if (enemy[i].x >= enemy[self].x){
				int blockedAt = enemy[i].x - ENEMY_IMG_SIZE;
				if (blockedAt < limit){ limit = blockedAt; blocker = i; }
			}
		}
	}

	if (blockerOut) *blockerOut = blocker;
	return limit;
}

/* ================================================================== */
/* WATER / DROWNING                                                    */
/*                                                                     */
/* Level 2's pits are filled with real water (water.png/water_top.png  */
/* - see the tile section in images.h). Those pools sit in exactly two */
/* places, and both are derived straight from the level data rather    */
/* than hardcoded, so they stay correct if the level is ever retuned:  */
/*                                                                     */
/*   - under every moving bridge (mWall): the water surface is the     */
/*     bridge's own travel row, spanning left..right+128 (the full     */
/*     rail the bridge can ever occupy).                               */
/*   - under every elevator (jWall): the water surface is just below   */
/*     the elevator's lowest reach (lowerLimit), spanning the 128px    */
/*     shaft at Pos.                                                   */
/*                                                                     */
/* waterSurfaceAt() returns the world Y of the water surface at a      */
/* given x, or -1 if there is no water in that column at all. Only     */
/* Level 2 has water; Level 1's pits are unchanged, so a fall there    */
/* still behaves exactly as it always did.                             */
/* ================================================================== */
int waterSurfaceAt(int x)
{
	/* LEVEL 3 NO LONGER DROWNS.
	 *
	 * This used to return level3WaterLevel, which handed Level 3's flood
	 * to the same startDrowning() sequence Level 2's pits use. Level 3's
	 * water is ordinary water now and he can swim in it, so returning -1
	 * here switches that entire path off for this level in one place:
	 * the "fell in the water" branch in update() never fires, and
	 * updateLevel3() no longer calls startDrowning() either.
	 *
	 * Returning -1 rather than deleting the clause is deliberate - every
	 * caller already handles "no water here", which is exactly what
	 * Level 3 now wants, and Level 2's pits below are untouched. What
	 * replaces drowning is level3UpdateSwim() further down this file. */
	if (gameState == STATE_LEVEL3) return -1;

	if (gameState != STATE_LEVEL2) return -1;

	int best = -1;

	// NOTE: both numbers below are the TOP EDGE of the water as it is
	// actually drawn, so the splash lines up with the art. Each water
	// tile is drawn with iShowImage(x, y, 64, 64), i.e. y is the tile's
	// bottom, so a tile drawn at row `r` covers r .. r+64.

	for (int j = 0; j < mWallNo; j++){
		// The bridge slides along a single row; water fills that row
		// across the whole rail (left..right+64 as drawn in images.h,
		// widened a little here so the very edge of the gap still
		// counts) and everything below it. images.h draws the surface
		// tile at `mWall[j].h - 55`, so its top edge is at h - 55 + 64.
		if (x >= mWall[j].left && x <= mWall[j].right + 128){
			int s = mWall[j].h + 9;
			if (s > best) best = s;
		}
	}

	for (int j = 0; j < jWallNo; j++){
		// images.h draws the shaft's surface tile at
		// `(lowerLimit - 55) - 64`, so its top edge is at
		// lowerLimit - 55. Everything below that is water, down to
		// the floor.
		if (x >= jWall[j].Pos && x <= jWall[j].Pos + 128){
			int s = jWall[j].lowerLimit - 55;
			if (s > best) best = s;
		}
	}

	return best;
}

// Called the moment a falling player breaks the surface. Freezes the
// normal fall/landing machinery (playerHeight() and the playerY<=0
// branch in update() both bail out while `drowning` is set) and starts
// the timed drown-out.
void startDrowning(int surfaceY)
{
	if (drowning) return;

	drowning = 1;
	drownSurfaceY = surfaceY;
	drownY = surfaceY - 30;     // already breaking the surface, then sinks further
	drownTicks = 0;
	drownDrainAccum = 0.0;

	// Stop everything that assumes the player is still running around.
	stopRunningSound();
	playerIsMoving = 0;
	atMode = 0;
	landingState = 0;
	gravitalForce = 0;
	hopTimer = 0;
	playerBlockNo = -1;
	cEnemy = -1;
}

// Clears drown state - called by resetGame()/setupLevel2() so a RETRY
// never starts the next run already underwater.
void clearDrowning()
{
	drowning = 0;
	drownTicks = 0;
	drownY = 0;
	drownSurfaceY = 0;
	drownDrainAccum = 0.0;
}

// One logic tick of the drown sequence (~60/sec, same rate as update()).
// Health drains smoothly from whatever it was at the splash down to 0
// across DROWN_SECONDS, so the life bar visibly empties instead of the
// game cutting straight to the GAME OVER screen.
void updateDrowning()
{
	drownTicks++;

	// Sink: fast for the first moment (the splash pulls him under),
	// then slow and steady as he goes limp. Purely visual - the sprite
	// Y is the only thing this moves.
	int sinkStep = (drownTicks < 24) ? 2 : 1;
	drownY -= sinkStep;
	// Stop sinking once he is well under - and never let him drop
	// below the bottom of the screen, which the shallower pools (the
	// elevator shaft's waterline sits at only ~115) would otherwise
	// do, leaving an empty frame for the last second or so.
	int sinkLimit = drownSurfaceY - 140;
	if (sinkLimit < 10) sinkLimit = 10;
	if (drownY < sinkLimit) drownY = sinkLimit;

	// Drain health at a constant rate so it reaches exactly 0 at the
	// end of the window, carrying the fraction over between ticks so
	// slow drains don't round down to nothing every tick.
	drownDrainAccum += (double)playerMaxHealth / (double)DROWN_TOTAL_TICKS;
	int drain = (int)drownDrainAccum;
	if (drain > 0){
		drownDrainAccum -= drain;
		playerHealth -= drain;
		if (playerHealth < 0) playerHealth = 0;
	}

	// Out of air (or out of health) - game over, on this level.
	if (playerHealth <= 0 || drownTicks >= DROWN_TOTAL_TICKS){
		playerHealth = 0;
		drowning = 0;
		gameOverFromLevel = gameState; // RETRY should restart Level 2 / Level 3
		gameState = STATE_GAMEOVER;
	}
}

/* ================================================================== */
/* LEVEL 3 - THE WATER : SWIMMING, CLIMBING OUT, PIRANHAS              */
/*                                                                     */
/* Level 3's flood used to drown him on contact. It is ordinary water  */
/* now: he swims in it for as long as his health lasts, piranhas hunt  */
/* him while he is in there, and 'R' pulls him out onto a ledge.       */
/*                                                                     */
/* See the LEVEL 3 - SWIMMING block in game_globals.h for the numbers, */
/* and waterSurfaceAt() above for how the old drowning path is         */
/* switched off without touching Level 2.                              */
/* ================================================================== */

/* Wipes the water state. Called by setupLevel3() and by resetGame(),
 * so no fish and no half-finished swim can survive into another run -
 * or into another level. */
void clearLevel3Water()
{
	for (int i = 0; i < LEVEL3_FISH_MAX; i++){
		level3Fish[i].active = 0;
		level3Fish[i].x = 0;
		level3Fish[i].y = 0;
		level3Fish[i].dir = 1;
		level3Fish[i].frame = 0;
		level3Fish[i].animCounter = 0;
		level3Fish[i].wanderSeed = 0;
		level3Fish[i].biting = 0;
		level3Fish[i].roamX = 0;
		level3Fish[i].roamY = 0;
		level3Fish[i].roamTimer = 0;
		level3Fish[i].lastGap = 0;
		level3Fish[i].stall = 0;
	}
	level3FishNo = 0;
	level3Swimming = 0;
	level3SwimInput = 0;
	level3ClimbX = -1;
	level3ClimbY = 0;
	level3BiteFlash = 0;
	level3FishFrenzy = 0;
	level3BiteAccum = 0.0;
}

/* Is world point (x, y) open water - i.e. inside the flood and not
 * inside rock? height[] holds the TOP of the platform in each column
 * and everything below that is solid, so open water is simply anything
 * at or above that surface and at or below the waterline. */
int level3WaterFree(int x, int y)
{
	if (x < 40 || x > 49000) return 0;
	if (y < 8) return 0;
	if (y > level3WaterLevel) return 0;
	return (y >= height[x / 2]);
}

/* The stretch of open water available to a fish at column x.
 *
 * This is also where the "fish above the waterline" rule lives: the top
 * of the stretch is half a body below the surface, so no part of a fish
 * can ever break it. */
int level3FishBandAt(int x, int y, int *lowOut, int *highOut)
{
	(void)y;   /* there is only one stretch now - see below */

	if (x < LEVEL3_FISH_MIN_X || x > LEVEL3_FISH_MAX_X) return 0;

	/* The ENTIRE body of water, bed to surface, at every x. height[] is
	 * deliberately not consulted: platforms are the Prince's obstacles,
	 * not the shoal's, and consulting it is exactly what used to split
	 * the level into sections a fish could not swim between.
	 *
	 * Half a body below the surface so the whole sprite stays under it,
	 * and half a body off the bed so none of it hangs below the world. */
	int top = level3WaterLevel - LEVEL3_FISH_HALF;
	if (top < LEVEL3_FISH_HALF) return 0;       /* the flood is still too shallow */

	*lowOut = LEVEL3_FISH_HALF;
	*highOut = top;
	return 1;
}

/* Drop fish `i` into the nearest column that genuinely has water in it,
 * searching outward from aroundX. A fish with nowhere to go is switched
 * off entirely - it is not drawn and it cannot bite - which is what
 * happens early in the level while the flood is still only ankle deep. */
void level3PlaceFish(int i, int aroundX, int minD)
{
	for (int d = minD; d <= 5000 + minD; d += 40){
		for (int s = -1; s <= 1; s += 2){
			int x = aroundX + s * d;
			int low, high;
			/* Probed at mid-depth so it is happy to be put either under a
			 * platform or over one, whichever that column offers. */
			if (!level3FishBandAt(x, level3WaterLevel / 2, &low, &high)) continue;

			level3Fish[i].x = x;
			level3Fish[i].y = low + nextRandom(&level3FishSeed, high - low + 1);
			level3Fish[i].active = 1;
			return;
		}
		if (d == 0) continue;   /* both signs of 0 are the same column */
	}
	level3Fish[i].active = 0;
}

/* Put the piranhas in the water. Called once when Level 3 is built.
 * They are recycled forever rather than respawned, so the count never
 * drifts - but one that cannot find water starts switched off and gets
 * another go as the flood rises (see level3UpdateFish). */
void level3SpawnFish()
{
	level3FishNo = LEVEL3_FISH_MAX;
	for (int i = 0; i < LEVEL3_FISH_MAX; i++){
		level3Fish[i].dir = (i % 2) ? 1 : -1;
		level3Fish[i].frame = i % LEVEL3_FISH_FRAMES;
		level3Fish[i].animCounter = 0;
		level3Fish[i].wanderSeed = i * 37 + 11;
		level3Fish[i].biting = 0;
		level3Fish[i].active = 0;
		/* A big starting gap so the stall counter cannot fire on the
		 * very first tick of a run. */
		level3Fish[i].lastGap = 99999;
		level3Fish[i].stall = 0;
		level3PlaceFish(i, 500 + i * 260, 0);
		level3PickRoam(i);
	}
}

/* Look for a ledge he could pull himself out onto, nearest first.
 * Records it in level3ClimbX/Y for both the 'R' key and the on-screen
 * prompt, so what the HUD offers and what the key does can never
 * disagree. */
void level3FindClimb()
{
	level3ClimbX = -1;

	if (!level3Swimming) return;

	for (int d = 8; d <= LEVEL3_CLIMB_REACH; d += 2){
		for (int s = -1; s <= 1; s += 2){
			int x = playerX + s * d;
			if (x < 40 || x > 49000) continue;

			int h = height[x / 2];
			if (h <= playerY) continue;                       /* not above him  */
			if (h - playerY > LEVEL3_CLIMB_MAX_RISE) continue; /* too tall      */
			if (blockData[x / 2] < 0) continue;                /* not a platform*/

			/* Stand him a little way in from the edge so he does not
			 * immediately walk back off it. */
			level3ClimbX = x + s * 26;
			level3ClimbY = h;
			return;
		}
	}
}

/* 'R'. Pull himself out of the water and onto the ledge. */
void level3TryClimb()
{
	if (gameState != STATE_LEVEL3) return;
	if (!level3Swimming || level3ClimbX < 0) return;

	playerX = level3ClimbX;
	playerY = level3ClimbY;
	level3Swimming = 0;
	level3SwimInput = 0;
	level3ClimbX = -1;
	gravitalForce = 0;
	landingState = 0;
	/* He climbed out under his own power - the settle onto the ledge is
	 * not a fall, so it must not be charged fall damage. */
	skipFallDamage = 1;
	playerBlockNo = blockData[playerX / 2];
	hopTimer = 0;
	playClimbSound();
}

/* One stroke. Called straight from iKeyboard() so swimming uses the
 * same key-driven movement model the walking already does. */
void level3Swim(int dx, int dy)
{
	if (!level3Swimming) return;

	int nx = playerX + dx * LEVEL3_SWIM_SPEED;
	int ny = playerY + dy * LEVEL3_SWIM_RISE;

	if (dx != 0 && level3WaterFree(nx, playerY)) playerX = nx;
	if (dy != 0 && level3WaterFree(playerX, ny)) playerY = ny;

	if (dx > 0) k = 13;
	else if (dx < 0) k = 0;

	/* Any deliberate stroke suppresses the float-up for a moment, so
	 * swimming down does not fight his own buoyancy. */
	level3SwimInput = LEVEL3_SWIM_INPUT_HOLD;
	playerIsMoving = 1;
	lastMoveKeyTime = clock();
}

/* Where fish `i` should be heading while hunting him.
 *
 * `slot` is its place in the shoal this moment: the first few slots are
 * the ones allowed in to bite, on a small ring around him; the rest hold
 * back and orbit. Slots rotate over time so the same fish are not always
 * the ones with their teeth in him. */
void level3FishSlotTarget(int slot, int *tx, int *ty)
{
	const double TAU = 6.2831853;

	if (slot < LEVEL3_FISH_ATTACKERS){
		double a = (TAU * slot) / (double)LEVEL3_FISH_ATTACKERS;
		*tx = playerX + (int)(cos(a) * LEVEL3_FISH_RING_X);
		*ty = playerY + LEVEL3_BODY_MID + (int)(sin(a) * LEVEL3_FISH_RING_Y);
		return;
	}

	int n = slot - LEVEL3_FISH_ATTACKERS;
	int r = LEVEL3_FISH_STANDOFF + n * LEVEL3_FISH_STANDOFF_STEP;
	if (r > LEVEL3_FISH_STANDOFF_MAX) r = LEVEL3_FISH_STANDOFF_MAX;

	/* A slow orbit, so the ones waiting their turn drift around him
	 * instead of hanging in the water like markers on a map. */
	double a = (TAU * n) / 7.0 + level3Timer * 0.011;
	*tx = playerX + (int)(cos(a) * r);
	*ty = playerY + LEVEL3_BODY_MID + (int)(sin(a) * r * 0.55);
}

/* Pick somewhere new in the flood for fish `i` to cruise to. Anywhere at
 * all - the search runs across the whole level, not around where the
 * fish happens to be - which is what stops them living out their lives
 * in whichever pocket of water they started in. */
void level3PickRoam(int i)
{
	for (int t = 0; t < LEVEL3_FISH_ROAM_TRIES; t++){
		/* Anywhere across the full width of the water, and anywhere
		 * between the bed and the surface - so over time the shoal is
		 * spread through the whole of it rather than concentrated in one
		 * section. */
		int x = LEVEL3_FISH_MIN_X +
			nextRandom(&level3FishSeed, LEVEL3_FISH_MAX_X - LEVEL3_FISH_MIN_X + 1);
		int low, high;
		if (!level3FishBandAt(x, 0, &low, &high)) continue;

		level3Fish[i].roamX = x;
		level3Fish[i].roamY = low + nextRandom(&level3FishSeed, high - low + 1);
		level3Fish[i].roamTimer = LEVEL3_FISH_ROAM_MIN +
			nextRandom(&level3FishSeed, LEVEL3_FISH_ROAM_MAX - LEVEL3_FISH_ROAM_MIN + 1);
		return;
	}
	/* Nowhere worth going yet (the flood is still shallow) - try again
	 * shortly rather than sitting on a stale target forever. */
	level3Fish[i].roamX = level3Fish[i].x;
	level3Fish[i].roamY = level3Fish[i].y;
	level3Fish[i].roamTimer = 60;
}

/* Swim fish `i` toward (tx, ty) at `speed`, rising over anything solid
 * in the way instead of grinding into it.
 *
 * The old patrol just flipped direction whenever the column ahead was no
 * good, which is why fish ended up oscillating on the spot between two
 * platforms. Lifting over the obstacle lets them actually get somewhere. */
void level3FishStepTo(int i, int tx, int ty, int speed)
{
	int mx = tx - level3Fish[i].x;
	int my = ty - level3Fish[i].y;

	int sx = 0;
	if (mx > 2) sx = (speed < mx) ? speed : mx;
	else if (mx < -2) sx = -((speed < -mx) ? speed : -mx);

	if (sx > 0) level3Fish[i].dir = 1;
	else if (sx < 0) level3Fish[i].dir = -1;

	int nx = level3Fish[i].x + sx;
	/* Only the edges of the water stop it - never a platform. */
	if (nx >= LEVEL3_FISH_MIN_X && nx <= LEVEL3_FISH_MAX_X) level3Fish[i].x = nx;

	int vs = speed - 1;
	if (vs < 1) vs = 1;

	/* Keep the vertical target inside the water, bed to surface. */
	int low, high;
	if (level3FishBandAt(level3Fish[i].x, level3Fish[i].y, &low, &high)){
		int want = ty;
		if (want < low) want = low;
		if (want > high) want = high;
		my = want - level3Fish[i].y;
	}

	if (my > 1) level3Fish[i].y += (vs < my) ? vs : my;
	else if (my < -1) level3Fish[i].y -= (vs < -my) ? vs : -my;
}

/* Shove apart any two fish sharing the same water. One pass over the
 * pairs - 24 fish is 276 comparisons, which is nothing at 60Hz. */
void level3SeparateFish()
{
	for (int i = 0; i < level3FishNo; i++){
		if (!level3Fish[i].active) continue;
		for (int j = i + 1; j < level3FishNo; j++){
			if (!level3Fish[j].active) continue;

			int dx = level3Fish[j].x - level3Fish[i].x;
			int dy = level3Fish[j].y - level3Fish[i].y;
			if (abs(dx) >= LEVEL3_FISH_SEP_X || abs(dy) >= LEVEL3_FISH_SEP_Y) continue;

			int px = (dx >= 0) ? LEVEL3_FISH_SEP_PUSH : -LEVEL3_FISH_SEP_PUSH;
			int py = (dy >= 0) ? LEVEL3_FISH_SEP_PUSH : -LEVEL3_FISH_SEP_PUSH;
			if (dx == 0 && dy == 0){
				/* Exactly on top of each other - pick a side, or they
				 * would sit there forever cancelling each other out. */
				px = (i & 1) ? LEVEL3_FISH_SEP_PUSH : -LEVEL3_FISH_SEP_PUSH;
				py = 0;
			}
			level3Fish[i].x -= px;  level3Fish[j].x += px;
			level3Fish[i].y -= py;  level3Fish[j].y += py;
		}
	}
}

void level3UpdateFish()
{
	if (level3FishFrenzy > 0) level3FishFrenzy--;

	/* For a few seconds after he goes in, the shoal swims harder still. */
	int chaseSpeed = LEVEL3_FISH_CHASE_SPEED;
	if (level3FishFrenzy > 0) chaseSpeed += LEVEL3_FISH_FRENZY_BOOST;

	int biters = 0;

	for (int i = 0; i < level3FishNo; i++){
		if (level3Fish[i].biting > 0) level3Fish[i].biting--;

		/* A fish with no water to be in is switched off. Give it another
		 * go every so often - the flood is rising, so a column that is
		 * dry now may be deep water in a few seconds. Staggered by index
		 * so all twenty-four never search on the same tick. */
		if (!level3Fish[i].active){
			if (((level3Timer + i) % 30) == 0){
				/* Come back somewhere useful rather than always at the
				 * same index-based spot: near him when he is in the water
				 * (but far enough out that one never simply appears on
				 * top of him), otherwise anywhere in the flood. */
				int around;
				if (level3Swimming) around = playerX + ((i % 2) ? 760 : -760);
				else                around = 300 + nextRandom(&level3FishSeed, 6300);
				level3PlaceFish(i, around, 0);
				if (level3Fish[i].active) level3PickRoam(i);
			}
			continue;
		}

		/* ---- hunt, or cruise ---- */
		if (level3Swimming){
			/* NO DETECTION RANGE. The moment he is in the water the whole
			 * shoal comes for him, from wherever in the level they are -
			 * so it does not matter where he falls in. Each fish heads
			 * for its own place in the formation rather than the middle
			 * of him, which is what keeps them from piling up. */
			int slot = (i + level3Timer / LEVEL3_FISH_ROTATE) % level3FishNo;
			int tx, ty;
			level3FishSlotTarget(slot, &tx, &ty);

			/* One a long way off swims much harder to close the gap. */
			int speed = chaseSpeed;
			int gap = abs(playerX - level3Fish[i].x);
			if (gap > LEVEL3_FISH_FAR_DIST) speed += LEVEL3_FISH_FAR_BOOST;

			level3FishStepTo(i, tx, ty, speed);

			/* Is it actually getting anywhere? While the flood is low some
			 * stretches of the level are not joined by water at all, and a
			 * fish on the wrong side of a platform that reaches the bed
			 * would swim at it until the level ended. One that has made no
			 * ground for a while rejoins the hunt from a fresh spot near
			 * him instead - far enough off to swim into view rather than
			 * appear on top of him. */
			int newGap = abs(playerX - level3Fish[i].x);
			if (newGap < level3Fish[i].lastGap - 1) level3Fish[i].stall = 0;
			else level3Fish[i].stall++;
			level3Fish[i].lastGap = newGap;

			if (level3Fish[i].stall > LEVEL3_FISH_STALL_TICKS &&
				newGap > LEVEL3_FISH_STALL_GAP)
			{
				level3PlaceFish(i, playerX, LEVEL3_FISH_REJOIN_DIST);
				level3Fish[i].stall = 0;
				level3Fish[i].lastGap = abs(playerX - level3Fish[i].x);
			}
		}
		else{
			/* Nothing to hunt: cruise to a spot somewhere in the flood,
			 * then pick another. Re-picking across the WHOLE level is
			 * what spreads the shoal through all of the water instead of
			 * leaving each fish stuck where it started. */
			if (level3Fish[i].roamTimer > 0) level3Fish[i].roamTimer--;

			int arrived = (abs(level3Fish[i].roamX - level3Fish[i].x) < 24 &&
			               abs(level3Fish[i].roamY - level3Fish[i].y) < 18);
			if (level3Fish[i].roamTimer <= 0 || arrived)
				level3PickRoam(i);

			level3FishStepTo(i, level3Fish[i].roamX, level3Fish[i].roamY, LEVEL3_FISH_SPEED);

			/* A gentle rise and fall on top, each fish on its own phase
			 * so the shoal never moves in lockstep. */
			level3Fish[i].y += (int)(2.0 * sin((level3Timer + level3Fish[i].wanderSeed) * 0.05));
		}

		/* ---- tail ---- */
		level3Fish[i].animCounter++;
		if (level3Fish[i].animCounter >= LEVEL3_FISH_ANIM_TICKS){
			level3Fish[i].animCounter = 0;
			level3Fish[i].frame = (level3Fish[i].frame + 1) % LEVEL3_FISH_FRAMES;
		}
	}

	/* ---- PASS 2: push apart anything that ended up stacked ---- */
	level3SeparateFish();

	/* ---- PASS 3: KEEP THEM IN THE WATER ----
	 *
	 * Runs after the separation, so a fish shoved sideways into rock or
	 * up through the surface is put right in the same tick rather than
	 * being drawn out of place for a frame.
	 *
	 * This is also the fix for fish appearing above the waterline. The
	 * old version clamped to `floor + CLEAR` whenever a column had no
	 * room, which pinned the fish on TOP of any rock that reached the
	 * surface - out of the water, in mid-air, still able to bite. Now a
	 * column that holds no water simply is not somewhere a fish can be:
	 * it gets moved to the nearest column that does, and if there is
	 * none it is switched off until the flood rises.
	 *
	 * The stretch's top is half a body below the surface, so the whole
	 * sprite stays under it - not just the centre point. */
	for (int i = 0; i < level3FishNo; i++){
		if (!level3Fish[i].active) continue;

		int low, high;
		if (!level3FishBandAt(level3Fish[i].x, level3Fish[i].y, &low, &high)){
			level3PlaceFish(i, level3Fish[i].x, 0);
			if (!level3Fish[i].active) continue;
			if (!level3FishBandAt(level3Fish[i].x, level3Fish[i].y, &low, &high)){
				level3Fish[i].active = 0;
				continue;
			}
		}
		if (level3Fish[i].y < low) level3Fish[i].y = low;
		if (level3Fish[i].y > high) level3Fish[i].y = high;
	}

	/* ---- PASS 4: who has hold of him ---- */
	/* Requires BOTH that he is in the water and that the fish itself is
	 * under the surface. The clamp above already guarantees the second,
	 * but stating it here means a fish can never bite from out of the
	 * water even if something else ever moves one. */
	for (int i = 0; i < level3FishNo; i++){
		if (!level3Fish[i].active) continue;

		if (level3Swimming &&
			level3Fish[i].y + LEVEL3_FISH_HALF <= level3WaterLevel &&
			abs(level3Fish[i].x - playerX) <= LEVEL3_FISH_BITE_X &&
			abs(level3Fish[i].y - (playerY + LEVEL3_BODY_MID)) <= LEVEL3_FISH_BITE_Y)
		{
			if (!level3Fish[i].biting) playBiteSound();
			level3Fish[i].biting = LEVEL3_BITE_FLASH;
			level3BiteFlash = LEVEL3_BITE_FLASH;
			biters++;
		}
	}

	/* ---- the life bar, draining the whole time he is in there ----
	 *
	 * In the water at all: a slow, constant 4%/sec - the shoal is always
	 * worrying at him, so time in the water always costs. With teeth
	 * actually in him it is the full 20%/sec instead, as a TOTAL rather
	 * than per fish (see LEVEL3_BITE_PERCENT_DIV in game_globals.h for
	 * why it is capped). Either way it never stops while he is under, so
	 * if he cannot reach a wall and climb before it runs out, the
	 * `playerHealth <= 0` check below ends the game. */
	if (level3Swimming){
		int div = (biters > 0) ? LEVEL3_BITE_PERCENT_DIV : LEVEL3_WATER_DRAIN_DIV;
		level3BiteAccum += (double)playerMaxHealth /
			((double)div * (double)LEVEL3_BITE_TICKS);
	}

	/* Spend whole points of accumulated bite damage. Running the life
	 * bar out is picked up by the single `playerHealth <= 0` game-over
	 * check at the end of update() - no second copy of that rule. */
	int dmg = (int)level3BiteAccum;
	if (dmg > 0){
		level3BiteAccum -= dmg;
		playerHealth -= dmg;
		if (playerHealth < 0) playerHealth = 0;
	}
}

void level3UpdateSwim()
{
	if (gameState != STATE_LEVEL3){
		level3Swimming = 0;
		return;
	}

	if (level3BiteFlash > 0) level3BiteFlash--;
	if (level3SwimInput > 0) level3SwimInput--;

	int surface = level3WaterLevel;

	if (!level3Swimming){
		/* Into the water - whether he fell in or the flood rose over
		 * him. Either way it is a swim now, not a drowning. */
		if (playerY + LEVEL3_SWIM_ENTER_DEPTH < surface){
			level3Swimming = 1;
			playerBlockNo = -1;
			gravitalForce = 0;
			landingState = 0;
			atMode = 0;
			hopTimer = 0;
			stopRunningSound();
			/* The splash brings the shoal. Every time he goes in - not
			 * just the first time - they come at him hard for a few
			 * seconds before settling back to a normal hunt. */
			level3FishFrenzy = LEVEL3_FISH_FRENZY_TICKS;
		}
	}
	else{
		/* Out of the water again - only really reachable if the flood
		 * drops below him, since climbing out clears the flag itself. */
		if (playerY > surface){
			level3Swimming = 0;
			gravitalForce = 0;
			skipFallDamage = 1;
		}
	}

	if (!level3Swimming){
		level3ClimbX = -1;
		return;
	}

	/* ---- float toward the surface when he stops swimming ---- */
	if (level3SwimInput == 0){
		int rest = surface - LEVEL3_SWIM_SURFACE_GAP;
		if (playerY < rest && level3WaterFree(playerX, playerY + 1)) playerY++;
		else if (playerY > rest) playerY--;
	}

	/* ---- never let him end up inside the stonework ---- */
	int floorY = height[playerX / 2];
	if (playerY < floorY) playerY = floorY;
	if (playerY > surface) playerY = surface;

	level3FindClimb();
}

/* ================================================================== */
/* HEALTH HEARTS (Levels 2 and 3) : LOGIC                              */
/*                                                                     */
/* Hearts hang over the platforms and give back 10% of the life bar.   */
/* Both the number of them and where they hang are rolled fresh every  */
/* time the level is built, and they sit just out of standing reach so */
/* the only way to take one is to jump for it. See the HEALTH HEARTS   */
/* block in game_globals.h for the geometry.                           */
/* ================================================================== */

/* Empties the board. resetGame() calls this for every level, and
 * spawnHearts() starts from it, so no heart can survive into a level
 * that should not have any. */
void clearHearts()
{
	for (int i = 0; i < HEART_MAX; i++){
		heart[i].active = 0;
		heart[i].x = 0;
		heart[i].y = 0;
	}
	heartNo = 0;
	heartMsgTimer = 0;
	heartBobTick = 0;
}

/* Is world column x a piece of flat, open platform that a heart could
 * hang over - with enough clear floor either side for the Prince to
 * stand under it and jump? */
int heartSpotIsGood(int x, int surface)
{
	if (surface <= 0) return 0;

	/* The floor has to be flat across the whole landing area, otherwise
	 * the heart ends up over a step or right on a platform edge where
	 * a 'W' press climbs instead of hopping. */
	for (int k = x - HEART_CLEAR_SPAN; k <= x + HEART_CLEAR_SPAN; k += 2){
		if (k < 0) return 0;
		if (height[k / 2] != surface) return 0;
	}

	/* Never hang one inside water. In Level 2 that would be a pool; in
	 * Level 3 the flood starts low but rises, so this only rules out
	 * the spots that are already drowned at the moment the level is
	 * built - anything it swallows later is simply a heart the player
	 * needed to collect on the way past. */
	int surfaceY = waterSurfaceAt(x);
	if (surfaceY >= 0 && surface + HEART_FLOAT_HEIGHT <= surfaceY + 20) return 0;
	/* Level 3 is checked against its flood directly, because
	 * waterSurfaceAt() deliberately reports "no water" there now (the
	 * level swims instead of drowning) and would otherwise let a heart
	 * be hung somewhere he cannot reach from dry footing. */
	if (gameState == STATE_LEVEL3 &&
		surface + HEART_FLOAT_HEIGHT <= level3WaterLevel + 20) return 0;

	/* Keep them spread out rather than clustered on one platform. */
	for (int i = 0; i < heartNo; i++){
		if (abs(heart[i].x - x) < HEART_MIN_SPACING) return 0;
	}
	return 1;
}

/* Scatter `count` hearts across the platforms between minX and maxX.
 * Placement is by dart-throwing at the SHARED collision map rather than
 * against a hand-written table of spots, so it automatically follows
 * whatever platforms the level actually built - and it can never put a
 * heart somewhere there is no floor. */
void spawnHearts(int count, int minX, int maxX)
{
	clearHearts();

	if (count > HEART_MAX) count = HEART_MAX;
	if (maxX <= minX) return;

	for (int n = 0; n < count; n++){
		for (int tries = 0; tries < HEART_PLACE_TRIES; tries++){
			int x = minX + nextRandom(&heartSeed, maxX - minX);
			x &= ~1;                       /* the collision map is indexed by x/2 */

			int surface = height[x / 2];
			if (!heartSpotIsGood(x, surface)) continue;

			heart[heartNo].active = 1;
			heart[heartNo].x = x;
			heart[heartNo].y = surface + HEART_FLOAT_HEIGHT;
			heartNo++;
			break;
		}
		/* A level with nowhere left to put one simply gets fewer
		 * hearts - never an unreachable one, and never a crash. */
	}
}

void updateHearts()
{
	if (gameState != STATE_LEVEL2 && gameState != STATE_LEVEL3) return;

	heartBobTick++;
	if (heartMsgTimer > 0) heartMsgTimer--;

	/* Nothing to collect while he is under water or falling. */
	if (drowning || playerBlockNo == -1) return;

	/* How high he can reach right now. hopTimer/hopMaxHeight are the
	 * existing jump visual (see the 'w' handler below and the hop
	 * offset in iDraw()); using the peak of the arc rather than the
	 * exact position this instant makes the grab forgiving, which is
	 * what you want from a pickup. */
	int reachTop = playerY + charImgSize;
	if (hopTimer > 0) reachTop += hopMaxHeight;

	for (int i = 0; i < heartNo; i++){
		if (!heart[i].active) continue;

		int heartLow = heart[i].y - HEART_IMG_SIZE / 2;
		int heartHigh = heart[i].y + HEART_IMG_SIZE / 2;

		if (abs(heart[i].x - playerX) > HEART_REACH_X) continue;
		if (heartLow > reachTop || heartHigh < playerY) continue;

		heart[i].active = 0;
		playerHealth += playerMaxHealth / HEART_HEAL_DIV;
		if (playerHealth > playerMaxHealth) playerHealth = playerMaxHealth;
		heartMsgTimer = HEART_MSG_TICKS;
		playHeartSound();
	}
}

/* ================================================================== */
/* LEVEL 2 - CAVE BATS : LOGIC                                         */
/*                                                                     */
/* One bat at a time (up to LEVEL2_BAT_MAX in the air) sweeps in from  */
/* off-screen at head height and flies straight across. Duck under it  */
/* with 'Q' and it passes harmlessly overhead; stand there and it      */
/* takes 20% of the life bar, once per bat.                            */
/*                                                                     */
/* See the LEVEL 2 - CAVE BATS block in game_globals.h for the         */
/* geometry that makes the duck work.                                  */
/* ================================================================== */

/* Wipes every bat out of the air and stands the Prince back up.
 * Called by setupLevel2() (so a RETRY never inherits a bat that was
 * mid-swoop) and by resetGame() for every other level, so no bat state
 * can ever leak out of Level 2. */
void clearLevel2Bats()
{
	for (int i = 0; i < LEVEL2_BAT_MAX; i++){
		level2Bat[i].active = 0;
		level2Bat[i].x = 0;
		level2Bat[i].y = 0;
		level2Bat[i].dir = 1;
		level2Bat[i].frame = 0;
		level2Bat[i].animCounter = 0;
		level2Bat[i].struck = 0;
	}
	level2BatTimer = LEVEL2_BAT_FIRST_GAP;
	level2DuckTimer = 0;
	level2BatHitFlash = 0;
	level2BatHintTimer = 0;
	level2BatHintsLeft = LEVEL2_BAT_HINT_COUNT;
}

/* A number in [0, span), advancing the caller's OWN seed.
 *
 * The rest of the game deliberately avoids rand() (see
 * level3ShakeOffset() below): there is no global generator state to get
 * out of step and nothing else in the game is perturbed by a draw from
 * here. This is that same cheap integer hash, with the seed passed in
 * so the bats and the hearts each keep their own stream instead of
 * sharing one. Both are seeded from the clock when their level is
 * built, so no two runs come out the same. */
int nextRandom(int *seed, int span)
{
	if (span < 1) return 0;
	*seed = (int)((unsigned int)(*seed) * 1103515245u + 12345u);
	return (int)(((unsigned int)(*seed) >> 16) & 0x7fffu) % span;
}

int level2BatRandom(int span)
{
	return nextRandom(&level2BatSeed, span);
}

/* Is the Prince crouched right now? Only meaningful in Level 2 - the
 * 'Q' key does nothing anywhere else. */
int level2IsDucking()
{
	return (gameState == STATE_LEVEL2 && level2DuckTimer > 0);
}

/* 'Q' pressed. A crouch lasts LEVEL2_DUCK_TICKS and can be refreshed by
 * pressing again, but it cannot be started in mid-air or while
 * drowning: it is a dodge made from solid ground, not a free pose. */
void level2StartDuck()
{
	if (gameState != STATE_LEVEL2) return;
	if (drowning || playerBlockNo == -1) return;

	level2DuckTimer = LEVEL2_DUCK_TICKS;
	// Crouching abandons any swing in progress - see the 'a'/'d'/' '
	// handlers in iKeyboard(), which all refuse to run while ducking.
	atMode = 0;
	playerIsMoving = 0;
	stopRunningSound();
}

/* Release one bat from whichever side of the view gives the player the
 * longest look at it coming. */
void level2ReleaseBat(int slot)
{
	/* -cX is the world x at the left edge of the screen; the window is
	 * 1350 wide (see iInitialize() in main.cpp). */
	int viewLeft = -cX;
	int viewRight = viewLeft + 1350;

	/* Come from behind whichever edge is FURTHER from the Prince, so he
	 * always gets the maximum warning rather than a bat appearing on
	 * top of him. */
	int dir;
	if ((playerX - viewLeft) >= (viewRight - playerX)) dir = 1;   /* from the left, flying right */
	else                                               dir = -1;  /* from the right, flying left */

	level2Bat[slot].active = 1;
	level2Bat[slot].dir = dir;
	level2Bat[slot].x = (dir > 0) ? (viewLeft - LEVEL2_BAT_MARGIN)
	                              : (viewRight + LEVEL2_BAT_MARGIN);
	/* Starts a little above its cruising height and settles down onto
	 * it as it closes - it reads as a swoop out of the cave roof rather
	 * than a sprite sliding in on a rail. */
	level2Bat[slot].y = playerY + LEVEL2_BAT_HEIGHT + 90;
	level2Bat[slot].frame = 0;
	level2Bat[slot].animCounter = 0;
	level2Bat[slot].struck = 0;

	/* Teach the control on the first bat or two of the run. */
	if (level2BatHintsLeft > 0){
		level2BatHintsLeft--;
		level2BatHintTimer = LEVEL2_BAT_HINT_TICKS;
	}

	playBatSound();
}

void updateLevel2Bats()
{
	if (gameState != STATE_LEVEL2){
		return;
	}

	/* Both timers are ticked here rather than in the draw so they last
	 * the same length of time on every machine, whatever the frame
	 * rate - same reasoning as checkpointMsgTimer in update(). */
	if (level2DuckTimer > 0) level2DuckTimer--;
	if (level2BatHitFlash > 0) level2BatHitFlash--;
	if (level2BatHintTimer > 0) level2BatHintTimer--;

	/* Nothing swoops at a man who is already under water. */
	if (drowning) return;

	/* ---------------- Release a new bat ---------------- */
	if (level2BatTimer > 0){
		level2BatTimer--;
	}
	else{
		int slot = -1;
		for (int i = 0; i < LEVEL2_BAT_MAX; i++){
			if (!level2Bat[i].active){ slot = i; break; }
		}
		if (slot >= 0) level2ReleaseBat(slot);

		/* Re-arm whether or not a slot was free: if all three are
		 * busy this simply waits out another gap instead of spinning
		 * on a full roster every tick. */
		level2BatTimer = LEVEL2_BAT_GAP_MIN +
			level2BatRandom(LEVEL2_BAT_GAP_MAX - LEVEL2_BAT_GAP_MIN + 1);
	}

	/* ---------------- Fly, flap, strike ---------------- */
	int standTop = playerY + charImgSize;
	int duckTop = playerY + LEVEL2_DUCK_HEIGHT;
	int playerTop = level2IsDucking() ? duckTop : standTop;

	for (int i = 0; i < LEVEL2_BAT_MAX; i++){
		if (!level2Bat[i].active) continue;

		level2Bat[i].x += level2Bat[i].dir * LEVEL2_BAT_SPEED;

		/* Settle onto head height. Tracking the Prince's CURRENT feet
		 * rather than the height he was at when the bat launched keeps
		 * the dodge honest when he changes platform mid-approach. */
		int want = playerY + LEVEL2_BAT_HEIGHT;
		if (level2Bat[i].y > want){
			level2Bat[i].y -= LEVEL2_BAT_EASE;
			if (level2Bat[i].y < want) level2Bat[i].y = want;
		}
		else if (level2Bat[i].y < want){
			level2Bat[i].y += LEVEL2_BAT_EASE;
			if (level2Bat[i].y > want) level2Bat[i].y = want;
		}

		level2Bat[i].animCounter++;
		if (level2Bat[i].animCounter >= LEVEL2_BAT_ANIM_TICKS){
			level2Bat[i].animCounter = 0;
			level2Bat[i].frame = (level2Bat[i].frame + 1) % LEVEL2_BAT_FRAMES;
		}

		/* Strike. The bat's claws sweep a band LEVEL2_BAT_BAND either
		 * side of its centre; it connects when that band overlaps the
		 * Prince's silhouette, which is exactly what the crouch
		 * shortens. A bat that has already connected flies on harmlessly
		 * - one hit each, so a slow bat can never grind the life bar
		 * down tick by tick. */
		if (!level2Bat[i].struck && playerBlockNo != -1){
			int batLow = level2Bat[i].y - LEVEL2_BAT_BAND;
			int batHigh = level2Bat[i].y + LEVEL2_BAT_BAND;
			if (abs(level2Bat[i].x - playerX) <= LEVEL2_BAT_REACH &&
				batLow <= playerTop && batHigh >= playerY){
				level2Bat[i].struck = 1;
				playerHealth -= playerMaxHealth / LEVEL2_BAT_DAMAGE_DIV;
				if (playerHealth < 0) playerHealth = 0;
				level2BatHitFlash = LEVEL2_BAT_FLASH;
				playBatHitSound();
				/* Running out of health here is picked up by the single
				 * `playerHealth <= 0` game-over check at the end of
				 * update() - no second copy of that rule. */
			}
		}

		/* Gone past the far edge of the view - retire the slot. */
		int viewLeft = -cX;
		int viewRight = viewLeft + 1350;
		if (level2Bat[i].dir > 0 && level2Bat[i].x > viewRight + LEVEL2_BAT_MARGIN)
			level2Bat[i].active = 0;
		else if (level2Bat[i].dir < 0 && level2Bat[i].x < viewLeft - LEVEL2_BAT_MARGIN)
			level2Bat[i].active = 0;
	}
}

/* ================================================================== */
/* LEVEL 3 - "THE FINAL ESCAPE" : LOGIC                                */
/*                                                                     */
/* One function, updateLevel3(), called once per logic tick from       */
/* update() and ONLY while gameState == STATE_LEVEL3, plus a handful   */
/* of small helpers. Levels 1 and 2 never execute a line of it.        */
/*                                                                     */
/* It owns, in this order:                                             */
/*   1. the stage the player has reached,                              */
/*   2. how high the flood has risen,                                  */
/*   3. how hard the palace is shaking,                                */
/*   4. every collapsing slab's state AND its collision,               */
/*   5. the Guardian (patrol, chase, swing, phases, death),            */
/*   6. the escape sequence and the palace door,                       */
/*   7. handing a submerged player to the existing drowning code.      */
/* ================================================================== */

// Short on-screen line ("STAGE 3 - THE WATER IS RISING"). Copied by
// hand rather than with strncpy() so the project keeps compiling under
// /sdl, which turns the deprecated-CRT warning into a hard error.
void level3SetBanner(const char *text, int ticks)
{
	int i = 0;
	while (text[i] != '\0' && i < (int)sizeof(level3BannerText) - 1){
		level3BannerText[i] = text[i];
		i++;
	}
	level3BannerText[i] = '\0';
	level3BannerTimer = ticks;
}

/* Writes (or erases) every collapsing slab's floor in the SHARED
 * height[]/blockData[] collision map - the same array the player's own
 * playerHeight()/getHeight(), the enemy patrol-edge scan, and every
 * other platform in the game read. Re-run every single logic tick, so:
 *
 *   - a slab that is still SOLID or SHAKING always has exactly the
 *     collision its artwork promises, and
 *   - the moment a slab starts FALLING its tiles go back to height 0 /
 *     blockData -1 in the very same tick, so there is never any
 *     invisible floor left standing where a platform used to be.
 *
 * Because it is driven off the one shared array, there is no second
 * copy of "where does this platform end" that could drift out of sync. */
void level3StampCollapse()
{
	for (int i = 0; i < level3CollapseNo; i++){
		CollapsePlatform *c = &level3Collapse[i];
		int solid = (c->enabled &&
			(c->state == LEVEL3_CP_SOLID || c->state == LEVEL3_CP_SHAKING));

		for (int t = c->sPos / 2; t < c->ePos / 2; t++){
			height[t] = solid ? c->h : 0;
			blockData[t] = solid ? c->blockNo : -1;
		}
	}
}

// Start a slab's warning shake without the player having to stand on
// it - used by the Guardian fight, which brings the arena's own
// approach ledges down as its phases change.
void level3ForceCollapse(int idx)
{
	if (idx < 0 || idx >= level3CollapseNo) return;
	if (!level3Collapse[idx].enabled) return;
	if (level3Collapse[idx].state != LEVEL3_CP_SOLID) return;

	level3Collapse[idx].state = LEVEL3_CP_SHAKING;
	level3Collapse[idx].timer = level3Collapse[idx].warnTicks;
	playCollapseWarnSound();
}

// How far the whole palace is displaced this frame. Deterministic (a
// cheap integer hash of the logic tick, no rand() and no state of its
// own), so a RETRY always begins perfectly still and the shake looks
// identical on every machine.
int level3ShakeOffset()
{
	if (gameState != STATE_LEVEL3) return 0;
	if (level3ShakeLevel <= 0) return 0;

	unsigned int n = (unsigned int)level3Timer * 1103515245u + 12345u;
	int j = (int)((n >> 16) & 0x7fffu);
	return (j % (level3ShakeLevel * 2 + 1)) - level3ShakeLevel;
}

// Does the Guardian's body stop the Prince walking further in `dir`
// (-1 = left, +1 = right)? Mirrors the cEnemy block in the 'a'/'d' key
// handlers, which the Guardian is not part of because it is not in the
// enemy[] roster.
int level3GuardianBlocksPlayer(int dir)
{
	if (gameState != STATE_LEVEL3 || !level3Active) return 0;
	if (!level3BossActive) return 0;
	if (level3Boss.dead || level3Boss.dying || level3Boss.life <= 0) return 0;
	if (abs(playerY - level3Boss.y) > 48) return 0;

	if (dir < 0)
		return (playerX > level3Boss.x && playerX - level3Boss.x < LEVEL3_BOSS_RANGE);

	return (level3Boss.x > playerX && level3Boss.x - playerX < LEVEL3_BOSS_RANGE);
}

// Opens the sealed gate behind the arena. This is the ONLY thing that
// makes the last four platforms of the level real: until it runs, they
// have no entry in height[]/blockData[] at all and are not drawn, so
// the palace door genuinely cannot be reached before the Guardian
// falls - there is no invisible shortcut past the boss.
void level3OpenEscapeRoute()
{
	level3EscapeMode = 1;
	level3EscapeTimer = 0;

	wallNo = LEVEL3_WALLS_TOTAL;
	level3StampRange(wall[12].sPos, wall[12].ePos, wall[12].h, 12);
	level3StampRange(wall[13].sPos, wall[13].ePos, wall[13].h, 13);
	level3StampRange(wall[14].sPos, wall[14].ePos, wall[14].h, 14);

	level3Collapse[7].enabled = 1;
	level3Collapse[8].enabled = 1;
	level3StampCollapse();

	playEscapeSound();
	level3SetBanner("RUN!  THE PALACE IS COMING DOWN!", 300);
}

void updateLevel3()
{
	if (gameState != STATE_LEVEL3 || !level3Active) return;

	level3Timer++;
	if (level3BannerTimer > 0) level3BannerTimer--;

	/* -------------------------------------------------------------- */
	/* 1. STAGE PROGRESSION                                             */
	/*                                                                  */
	/* Driven by the furthest x the player has actually reached, never  */
	/* by raw time, so a careful player is not punished for being slow  */
	/* and backtracking can never wind the pressure back down.          */
	/* -------------------------------------------------------------- */
	if (playerX > level3MaxPlayerX) level3MaxPlayerX = playerX;

	int stage = 1;
	if (level3MaxPlayerX >= 5828)      stage = 5;
	else if (level3MaxPlayerX >= 4328) stage = 4;
	else if (level3MaxPlayerX >= 2600) stage = 3;
	else if (level3MaxPlayerX >= 1450) stage = 2;

	if (stage > level3Stage){
		level3Stage = stage;
		playWaterRiseSound();
		if (level3Stage == 2)      level3SetBanner("STAGE 2  -  THE PALACE BEGINS TO SHAKE", 240);
		else if (level3Stage == 3) level3SetBanner("STAGE 3  -  THE WATER IS RISING  -  CLIMB!", 240);
		else if (level3Stage == 4) level3SetBanner("STAGE 4  -  THE PALACE IS COMING APART", 240);
		else if (level3Stage == 5) level3SetBanner("STAGE 5  -  THE GUARDIAN'S HALL", 240);
	}

	/* -------------------------------------------------------------- */
	/* 2. THE RISING FLOOD                                              */
	/*                                                                  */
	/* Each stage raises the water's TARGET; the water itself then      */
	/* climbs toward it one pixel at a time so it is always a visible,  */
	/* gradual rise rather than a jump. The targets are chosen against  */
	/* the level's own platform heights: each one drowns the ledges the */
	/* player has already left behind (so old routes really do close)   */
	/* while staying a clear margin below the ground they are standing  */
	/* on now. The target only ever increases.                          */
	/* -------------------------------------------------------------- */
	if (!level3EscapeMode){
		int target = LEVEL3_WATER_START;
		if (level3Stage == 1)      target = 60;    /* far below the 170 entry hall - harmless */
		else if (level3Stage == 2) target = 120;   /* lapping at the 170 ledges               */
		else if (level3Stage == 3) target = 235;   /* every 170 ledge is now gone             */
		else if (level3Stage == 4) target = 340;   /* every 280 ledge is now gone             */
		else                       target = 400;   /* knee-deep on the 380 run to the arena   */

		/* The Guardian fight pushes it higher still - this is what makes
		 * the arena itself shrink while the battle is going on. The
		 * arena floor is at 470 and LEVEL3_SUBMERGE_MARGIN is 34, so
		 * anything above 436 would drown a player standing on it: the
		 * phase-4 level stops at 430, six pixels clear, which is close
		 * enough to look lethal and far enough not to be. */
		if (level3BossPhase >= 2 && target < 405) target = 405;
		if (level3BossPhase >= 3 && target < 418) target = 418;
		if (level3BossPhase >= 4 && target < 430) target = 430;

		/* THE FLOOD NEVER STOPS.
		 *
		 * On top of the per-stage targets above, the water creeps up on
		 * its own once the opening grace period is over, at a pace far
		 * slower than the stage targets so it never touches a player who
		 * is actually making progress (it stays well under every stage
		 * target for the first several minutes).
		 *
		 * It is here so the level can never dead-end. A collapsing slab
		 * never comes back, so a player who crosses one and then walks
		 * back can find every route off that ledge gone - and with a
		 * fixed per-stage water level they would be left standing on
		 * safe ground with nothing left to do and no way to die. The
		 * creep guarantees the palace finishes flooding no matter what,
		 * so that player is taken by the water and gets a clean RETRY
		 * instead of a stuck game. */
		int creep = LEVEL3_WATER_START;
		if (level3Timer > LEVEL3_CREEP_GRACE)
			creep += (level3Timer - LEVEL3_CREEP_GRACE) / LEVEL3_CREEP_TICKS;
		if (creep > LEVEL3_CREEP_MAX) creep = LEVEL3_CREEP_MAX;
		if (creep > target) target = creep;

		if (target > level3WaterTarget) level3WaterTarget = target;
		level3WaterRiseTicks = LEVEL3_WATER_RISE_TICKS;
	}
	else{
		/* Final escape: the flood is no longer aiming at anything - it
		 * just keeps coming, and it WILL swallow the 470-high escape
		 * corridor if the player takes too long over it. */
		level3WaterTarget = 620;
		level3WaterRiseTicks = LEVEL3_WATER_ESCAPE_TICKS;
	}

	if (level3WaterLevel < level3WaterTarget){
		level3WaterTick++;
		if (level3WaterTick >= level3WaterRiseTicks){
			level3WaterTick = 0;
			level3WaterLevel++;
		}
	}

	/* -------------------------------------------------------------- */
	/* 3. HOW HARD THE PALACE SHAKES                                    */
	/* -------------------------------------------------------------- */
	level3ShakeLevel = 0;
	if (level3Stage >= 2)      level3ShakeLevel = 1;
	if (level3Stage >= 4)      level3ShakeLevel = 2;
	if (level3BossPhase >= 2 && level3BossActive) level3ShakeLevel = 3;
	if (level3EscapeMode)      level3ShakeLevel = 4;

	/* -------------------------------------------------------------- */
	/* 4. COLLAPSING SLABS                                              */
	/*                                                                  */
	/* SOLID -> (the Prince steps on it) -> SHAKING for warnTicks ->    */
	/* FALLING (collision gone immediately, debris still dropping) ->   */
	/* GONE. A slab never comes back during a run, which is exactly     */
	/* what keeps the player moving forward.                            */
	/* -------------------------------------------------------------- */
	for (int i = 0; i < level3CollapseNo; i++){
		CollapsePlatform *c = &level3Collapse[i];
		if (!c->enabled) continue;

		if (c->state == LEVEL3_CP_SOLID){
			/* playerBlockNo comes straight out of the shared collision
			 * map, so this can only ever fire for a slab the Prince is
			 * genuinely standing on.
			 *
			 * forcedOnly slabs ignore his weight entirely - those are
			 * the Guardian arena's two approach ledges, which have to
			 * stay solid while he walks in to start the fight and are
			 * brought down later by the fight's own phases. Without
			 * this they would start counting down the moment he
			 * stepped off the elevator and drop him into the water
			 * before he ever reached the arena floor. */
			if (!drowning && !c->forcedOnly && playerBlockNo == c->blockNo){
				c->state = LEVEL3_CP_SHAKING;
				c->timer = c->warnTicks;
				playCollapseWarnSound();
			}
		}
		else if (c->state == LEVEL3_CP_SHAKING){
			c->timer--;
			if (c->timer <= 0){
				c->state = LEVEL3_CP_FALLING;
				c->timer = LEVEL3_CP_FALL_TICKS;
				c->fallY = 0;
				level3CollapseState++;
				playCollapseSound();
			}
		}
		else if (c->state == LEVEL3_CP_FALLING){
			c->fallY += 6;
			c->timer--;
			if (c->timer <= 0) c->state = LEVEL3_CP_GONE;
		}
	}
	/* Push every slab's current state back into the shared collision
	 * map - including clearing the ones that have just given way. */
	level3StampCollapse();

	/* -------------------------------------------------------------- */
	/* 5. THE FINAL GUARDIAN                                            */
	/* -------------------------------------------------------------- */
	if (!level3Boss.dead){
		/* Wake it the moment the Prince reaches the arena approach. */
		if (!level3BossActive && !level3Boss.dying &&
			playerX >= level3Collapse[5].sPos - 150)
		{
			level3BossActive = 1;
			level3Boss.active = 1;
			level3SetBanner("THE PALACE GUARDIAN BARS YOUR ESCAPE", 240);
		}

		/* Keep its feet on the floor it is actually standing on, read
		 * from the same height[] the player's own collision reads - so
		 * it can never sink through the arena. */
		if (blockData[level3Boss.x / 2] == level3Boss.blockNo)
			level3Boss.y = height[level3Boss.x / 2];

		/* THE platform boundary. enemyPatrolRange() walks the shared
		 * blockData[] array outward from the Guardian's own position
		 * until it stops matching its block - i.e. it returns where the
		 * arena floor genuinely stops, not a hand-maintained second
		 * copy of that number. Clamping against it (one half-sprite in
		 * from each edge) is what guarantees the Guardian can never
		 * walk off its platform or through the arena walls, no matter
		 * how the fight goes or how the level is retuned later. */
		int edgeLo, edgeHi;
		enemyPatrolRange(level3Boss.x, level3Boss.blockNo, &edgeLo, &edgeHi);
		int lo = edgeLo + LEVEL3_BOSS_IMG_SIZE / 2;
		int hi = edgeHi - LEVEL3_BOSS_IMG_SIZE / 2;
		if (hi < lo) hi = lo;

		if (level3Boss.dying){
			/* Death animation - no more damage in either direction. */
			level3Boss.deathTimer--;
			if (level3Boss.deathTimer <= 0){
				level3Boss.dying = 0;
				level3Boss.dead = 1;
				level3BossActive = 0;
				level3OpenEscapeRoute();
			}
		}
		else if (level3Boss.life <= 0){
			level3Boss.dying = 1;
			level3Boss.deathTimer = LEVEL3_BOSS_DEATH_TICKS;
			level3Boss.a = 0;
			level3Boss.atkAnim = 0;
			level3Boss.hitFlash = 0;
			playGuardianDeathSound();
			level3SetBanner("THE GUARDIAN FALLS", 200);
		}
		else if (level3BossActive && !drowning){
			/* ---- Phase transitions (each fires exactly once) ---- */
			int lifePct = (level3Boss.life * 100) / level3Boss.maxLife;
			if (level3BossPhase < 2 && lifePct <= 75){
				level3BossPhase = 2;
				level3ForceCollapse(5);   /* the arena's outer ledge gives way */
				level3SetBanner("PHASE 2  -  THE ARENA IS BREAKING APART", 210);
			}
			else if (level3BossPhase < 3 && lifePct <= 50){
				level3BossPhase = 3;
				level3ForceCollapse(6);   /* and the inner one - the floor shrinks */
				level3SetBanner("PHASE 3  -  THE WATER IS POURING IN", 210);
			}
			else if (level3BossPhase < 4 && lifePct <= 25){
				level3BossPhase = 4;
				level3SetBanner("PHASE 4  -  FINISH IT!", 210);
			}

			if (level3Boss.hitFlash > 0) level3Boss.hitFlash--;

			/* Faster and more relentless every phase. */
			int speed = (level3BossPhase >= 4) ? 4 : ((level3BossPhase >= 3) ? 3 : 2);
			int cooldown = (level3BossPhase >= 4) ? 40 :
				((level3BossPhase >= 3) ? 54 : ((level3BossPhase >= 2) ? 66 : 74));

			if (level3Boss.a){
				/* A swing is playing. Damage lands once, on the trigger
				 * tick, exactly like the ordinary enemy swings. */
				if (level3Boss.atkAnim == LEVEL3_BOSS_ATTACK_TICKS)
					applyGuardianStrikeDamage();

				level3Boss.atkAnim--;
				if (level3Boss.atkAnim <= 0){
					level3Boss.a = 0;
					level3Boss.m = 0;
				}
			}
			else{
				if (level3Boss.m < cooldown) level3Boss.m++;

				int dist = playerX - level3Boss.x;
				int sameFloor = (abs(playerY - level3Boss.y) <= 48);
				level3Boss.k = (dist < 0) ? 0 : 13;   /* always face the Prince */

				if (sameFloor && abs(dist) <= LEVEL3_BOSS_RANGE){
					if (level3Boss.m >= cooldown){
						level3Boss.a = 1;
						level3Boss.atkAnim = LEVEL3_BOSS_ATTACK_TICKS;
						level3Boss.m = 0;
						playAttackSound();   /* reuses the existing sword sound */
					}
				}
				else{
					level3Boss.x += (dist < 0) ? -speed : speed;
					level3Boss.animCounter++;
					if (level3Boss.animCounter >= LEVEL3_BOSS_ANIM_DIVISOR){
						level3Boss.animCounter = 0;
						level3Boss.p++;
					}
				}
			}
		}
		else{
			/* Not woken yet: patrol its own arena, edge to edge. */
			if (level3Boss.dir == 0) level3Boss.dir = -1;
			level3Boss.x += level3Boss.dir * 2;
			if (level3Boss.x <= lo){ level3Boss.x = lo; level3Boss.dir = 1; }
			else if (level3Boss.x >= hi){ level3Boss.x = hi; level3Boss.dir = -1; }
			level3Boss.k = (level3Boss.dir == 1) ? 13 : 0;

			level3Boss.animCounter++;
			if (level3Boss.animCounter >= LEVEL3_BOSS_ANIM_DIVISOR){
				level3Boss.animCounter = 0;
				level3Boss.p++;
			}
		}

		/* The clamp itself - applied after EVERY branch above (chase,
		 * patrol, and the stagger combat.h applies when the Prince
		 * lands a blow), so there is no path that can leave the
		 * Guardian standing past the edge of its floor. */
		if (level3Boss.x < lo) level3Boss.x = lo;
		if (level3Boss.x > hi) level3Boss.x = hi;
	}

	/* -------------------------------------------------------------- */
	/* 6. THE FINAL ESCAPE AND THE PALACE DOOR                          */
	/* -------------------------------------------------------------- */
	if (level3EscapeMode){
		level3EscapeTimer++;

		if (!level3ExitReached && !drowning &&
			playerBlockNo != -1 && playerX >= level3ExitX)
		{
			level3ExitReached = 1;
			// Level 3 is won here rather than on the LEVEL COMPLETE
			// screen, so its time is banked here - same single call the
			// other two levels use. The victory picture deliberately
			// shows no text, so the time is not drawn over it; it is
			// still saved and appears on the RECORDS screen.
			finishLevelRun(levelNumberForState(gameState));
			markLevelProgressComplete();     // exactly full at the door
			stopRunningSound();
			stopGameBGM();
			startVictoryBGM();
			gameState = STATE_VICTORY;
			return;
		}
	}

	/* -------------------------------------------------------------- */
	/* 7. THE WATER                                                     */
	/*                                                                  */
	/* This used to call startDrowning() the moment the flood rose      */
	/* LEVEL3_SUBMERGE_MARGIN over his feet. Level 3's water is         */
	/* ordinary water now: going under it is a swim, not a death, so    */
	/* the whole swim/piranha/climb system runs here instead. It is     */
	/* driven from update() rather than from here (see the call beside  */
	/* the Level 3 hook) because it has to happen BEFORE the falling    */
	/* and landing code, not after it.                                  */
	/* -------------------------------------------------------------- */
}

void update()
{
	if (!isGameplayState()) return; // no physics/AI before gameplay starts

	// LEVEL TIMER. update() has just proved the player is actually
	// playing - this line is never reached from the splash, the menu,
	// the story, the instructions, the credits, the records table or
	// either end screen - so the first tick of a fresh attempt is
	// exactly when the clock should start. resetGame() puts the timer
	// back to IDLE, which is what makes a restart begin from zero
	// again (and it never touches the saved records).
	if (levelTimerState == LEVEL_TIMER_IDLE)
		startLevelTimer(levelNumberForState(gameState));

	// Countdown for the "CHECKPOINT" confirmation. Ticked here rather
	// than in the draw so it lasts the same time on every machine.
	if (checkpointMsgTimer > 0) checkpointMsgTimer--;

	// Drowning owns the player completely while it runs - no falling,
	// no fall damage, no respawn, no enemy AI acting on a player who
	// is already under water.
	if (drowning){
		updateDrowning();
		return;
	}

	/* LEVEL 3: the water.
	 *
	 * Runs BEFORE the falling/landing block below, because while he is
	 * swimming none of that applies - there is no gravity, no landing,
	 * no fall damage and no respawn, and the swim code owns playerY.
	 * A no-op on Levels 1 and 2, which still fall and drown exactly as
	 * they always did. */
	level3UpdateSwim();

	if (playerBlockNo == -1 && !level3Swimming){
		// Falling into one of Level 2's pools starts the drown
		// sequence instead of the normal "hit the floor" handling
		// below. Checked before the playerY<=0 test because the
		// water surface always sits well above the floor.
		int surfaceY = waterSurfaceAt(playerX);
		if (surfaceY >= 0 && playerY <= surfaceY - 20){
			// -20 so the splash fires as his body actually breaks the
			// surface rather than the moment his feet graze the top
			// edge of the water.
			startDrowning(surfaceY);
			return;
		}

		if (playerY <= 0){
			// FIX: don't apply the respawn/game-over consequence the
			// instant the player touches the ground. Freeze them here,
			// fully visible (drawFree() keeps rendering the falling
			// sprite - see iDraw()'s Player section), for
			// landingPauseSeconds (0.5s) first, so the player actually
			// gets to see themself land before anything else happens.
			if (!landingState){
				landingState = 1;
				landingStartTime = clock();
			}
			else{
				double elapsed = (double)(clock() - landingStartTime) / CLOCKS_PER_SEC;
				if (elapsed >= landingPauseSeconds){
					landingState = 0; // pause is over, apply the consequence now

					fallCount++;
					if (fallCount == 1){
						// 1st time the player hits the ground from the top:
						// lose half of max health and respawn.
						playerHealth -= playerMaxHealth / 2;
						if (playerHealth < 0) playerHealth = 0;

						// CHECKPOINT: come back at the last switch the
						// player operated rather than at the very start
						// of the level. Every switch records itself when
						// pressed (see recordCheckpoint() below), and the
						// recorded spot is the surface of the static
						// platform that switch stands on, so this is
						// always solid ground. With no switch pressed yet
						// it falls back to the original start position.
						if (checkpointX >= 0){
							playerX = checkpointX;
							playerY = checkpointY;
						}
						else{
							playerX = 300;
							playerY = 300;
						}
						cX = 0;   // the camera clamp just below re-centres on the player
						cEnemy = -1;
						gravitalForce = 0;
						// FIX: the short drop from the respawn height
						// (300) down to the actual block surface right
						// after this was still being charged normal
						// fall damage on landing (gravitalForce/5 in
						// the playerBlockNo != -1 branch below), on
						// top of the half-health penalty just applied
						// above - so respawning quietly cost extra
						// health a second time. Skip exactly one
						// landing's worth of fall damage after a
						// respawn.
						skipFallDamage = 1;
					}
					else{
						// 2nd time: game over.
						playerHealth = 0;
						gameOverFromLevel = gameState; // remember Level 1 vs Level 2 for RETRY
						gameState = STATE_GAMEOVER;
					}
				}
			}
		}
		else{
			gravitalForce++;
			landingState = 0; // still airborne above ground - make sure no stale pause carries over
		}
	}
	else if (!level3Swimming){
		// While swimming neither branch runs: he is weightless, there is
		// no landing to charge for, and letting this branch run anyway
		// would quietly burn the one-shot skipFallDamage flag that the
		// climb out of the water arms.
		//
		// FIX: skip fall damage entirely for the one landing that
		// follows a respawn OR the start of a level (see skipFallDamage
		// above, which resetGame()/setupLevel2()/setupLevel3() now all
		// set). Without this, the short settle from the spawn height
		// down onto the actual floor was quietly charged - at the very
		// start of the game, and again on top of the half-health
		// penalty a respawn had just applied.
		if (skipFallDamage){
			skipFallDamage = 0;
		}
		else{
			// FIX: only the part of the fall BEYOND the free grace
			// hurts, so stepping down from one brick onto a lower one
			// no longer nibbles the life bar. See the
			// FALL_DAMAGE_FREE_TICKS comment in game_globals.h for the
			// numbers and for how to disable fall damage completely.
			int fallTicks = gravitalForce - FALL_DAMAGE_FREE_TICKS;
			if (fallTicks > 0){
				playerHealth -= fallTicks / 5;
				if (playerHealth < 0) playerHealth = 0; // BUGFIX: clamp - this could previously go negative and corrupt the health bar rendering
			}
		}
		gravitalForce = 0;
		landingState = 0; // standing on a block - not in a landing pause
	}
	/*Camera*//*Update*//*Start*/
	// Unchanged for every level. The window is 1350px wide and the
	// player is held between screen x 200 and x 800, so there are
	// always ~550px of level visible ahead of him - which is why none
	// of Level 3's gaps, slabs or bridges is ever a blind jump, however
	// much longer than the earlier levels it is.
	if (playerX + cX>800)cX = 800 - playerX;
	else if (playerX + cX < 200) cX = 200 - playerX;
	/*Camera*//*Update*//*End*/

	/* LEVEL 3: stages, flood, collapsing slabs, Guardian, escape. Runs
	 * before the elevator/bridge/enemy sections below so the collision
	 * map those sections read is already up to date for this tick.
	 * Returns immediately for Levels 1 and 2. */
	if (gameState == STATE_LEVEL3){
		updateLevel3();
		if (gameState != STATE_LEVEL3) return; // reached the palace door - the level is over
		if (drowning) return;                  // the flood just took him
	}

	/* LEVEL 2: the cave bats, and the crouch that dodges them. Placed
	 * beside the Level 3 hook above for the same reason - it runs after
	 * the camera has been updated for this tick, since a bat launches
	 * from the edge of the current view. Returns immediately for Levels
	 * 1 and 3. */
	updateLevel2Bats();

	/* Health hearts. Runs for Levels 2 and 3 and returns immediately for
	 * Level 1, and is placed here - after the camera, before the
	 * platform sections - for the same reason as the two hooks above. */
	updateHearts();

	/*Jumping Wall*//*Update*//*Start*/
	for (int j = 0; j < jWallNo; j++){
		// alwaysOn elevators (their switch was removed from the level)
		// run on their own every frame; every other elevator still needs
		// its button pressed on, exactly as before.
		if (jWall[j].alwaysOn || button[jWall[j].buttonNo].OnOff){
			jWall[j].h += jWall[j].d;
			if (jWall[j].h >= jWall[j].upperLimit || jWall[j].h <= jWall[j].lowerLimit) jWall[j].d = -jWall[j].d;
		}
		for (int i = jWall[j].Pos / 2; i < (jWall[j].Pos + 128) / 2; i++){
			height[i] = jWall[j].h;
			if (playerBlockNo == jWall[j].blockNo) playerY = jWall[j].h;
		}
	}
	/*Jumping Wall*//*Update*//*End*/


	/*Moving Wall*//*Update*//*Start*/
	/* FIX (both requested behaviors):

	1) "second moving wall stops moving when the player is standing on
	it" - the old code re-called playerHeight() *inside* this loop to
	figure out whether the player was currently riding the wall being
	processed. That recomputation used height[]/blockData[], which by
	that point could already have been partially rewritten by an
	*earlier* iteration of this same loop (i.e. by mWall[0] before
	mWall[1] is even reached), and it also stomped on the player's
	global playerBlockNo/playerY as a side effect. That made the
	"is the player riding this wall" check unreliable for every wall
	after the first one processed each frame, so the 2nd wall would
	randomly fail to detect the player and effectively drop them -
	which reads as the wall "stopping" under them.
	Fix: figure out which block the player is standing on exactly
	ONCE, at the very start of this section (riderBlock), before any
	wall has moved this frame, and reuse that same snapshot for every
	wall. This makes rider-detection correct and identical no matter
	how many moving walls there are.

	2) "once a moving wall starts moving it shouldn't stop" - the old
	code turned the wall's own switch off (button[...].OnOff = 0) the
	moment it reached either end, "parking" it there until manually
	switched on again. Now the wall never turns its switch off by
	itself: as long as OnOff is on, it keeps moving every frame,
	bouncing back and forth between left and right forever (it just
	flips direction when it hits a limit, and current is clamped so
	it can never overshoot past its rails). The switch only stops it
	if the player presses it again. */
	int riderBlock = playerBlockNo; // snapshot: block the player is on before any wall moves this frame
	for (int j = 0; j < mWallNo; j++){
		// alwaysOn walls (their switch was removed from the level) run on
		// their own every frame; every other wall still needs its button
		// pressed on, exactly as before.
		if (mWall[j].alwaysOn || button[mWall[j].buttonNo].OnOff){
			mWall[j].current += mWall[j].d;

			if (mWall[j].current <= mWall[j].left){
				mWall[j].current = mWall[j].left;
				mWall[j].d = abs(mWall[j].d);   // hit left rail -> now head right
			}
			else if (mWall[j].current >= mWall[j].right){
				mWall[j].current = mWall[j].right;
				mWall[j].d = -abs(mWall[j].d);  // hit right rail -> now head left
			}

			if (riderBlock == mWall[j].blockNo) playerX += mWall[j].d;
		}
		/* Clear the whole rail the bridge can ever occupy - left..right
		 * plus the 128px the tile itself is wide - so last tick's tile
		 * is erased wherever it was. */
		int railLo = mWall[j].left / 2;
		int railHi = mWall[j].right / 2 + 64;
		for (int i = railLo; i < railHi; i++){
			height[i] = 0;
			blockData[i] = -1;
		}

		/* BUGFIX: put back any STATIC rock that rail happens to cross.
		 *
		 * The clear above erases 128px past `right`, because that is how
		 * far the bridge tile's right edge can reach. In Level 3 every
		 * platform after a bridge starts exactly at right+128, so the
		 * rail only ever crosses empty space. In Levels 1 and 2 several
		 * platforms start at `right` instead, so the clear was wiping
		 * the first 128px of the NEXT rock out of the collision map on
		 * every single tick, while that rock carried on being drawn.
		 * That strip was solid-looking stone with nothing under it:
		 *   - the player fell straight through it, and
		 *   - enemies on that platform were penned into a patrol range
		 *     128px narrower than the rock they were standing on, which
		 *     is why they never walked the full width of their block.
		 *
		 * There used to be a hand-written version of this fix for one
		 * bridge only (`if (mWall[j].blockNo == 13)`, rebuilding Level
		 * 2's second rock by hardcoded index). This replaces it with the
		 * general rule for every bridge and every rock: whatever static
		 * platform the rail overlaps is restamped from wall[] itself, so
		 * the collision map always matches the rock that is drawn. */
		for (int w = 0; w < wallNo; w++){
			int wLo = wall[w].sPos / 2;
			int wHi = wall[w].ePos / 2;
			int s = (wLo > railLo) ? wLo : railLo;
			int e = (wHi < railHi) ? wHi : railHi;
			for (int i = s; i < e; i++){
				height[i] = wall[w].h;
				blockData[i] = wall[w].blockNo;
			}
		}

		/* The bridge's own tile goes on last, so wherever it currently
		 * is, it is what the player and the enemies stand on. */
		for (int i = mWall[j].current / 2; i < mWall[j].current / 2 + 64; i++){
			height[i] = mWall[j].h;
			blockData[i] = mWall[j].blockNo;
		}

		block[mWall[j].blockNo].start = mWall[j].current;
		block[mWall[j].blockNo].end = mWall[j].current + 128;
	}
	/*Moving Wall*//*Update*//*End*/

	/*Enemy*//*Update*//*Start*/

	// Choose the nearest enemy on the player's current block.
	// This is important when a platform has more than one enemy: the
	// old code overwrote cEnemy for every enemy on the same block, so
	// the last enemy in the array became the target even if another
	// enemy was standing directly beside the player. That made enemies
	// on the second rock effectively impossible to hit.
	cEnemy = -1;
	if (playerBlockNo != -1){
		int bestDistance = 1000000;
		for (int t = 0; t < EnemyNo; t++){
			if (enemy[t].life > 0 && enemy[t].blockNo == playerBlockNo){
				int dist = abs(playerX - enemy[t].x);
				if (dist < bestDistance){
					bestDistance = dist;
					cEnemy = t;
				}
			}
		}
	}

	for (int j = 0; j < EnemyNo; j++){
		if (enemy[j].life <= 0){
			// BUGFIX: this whole block used to swap the last enemy
			// (index t) into slot j and just do EnemyNo--, without
			// checking whether `cEnemy` (the enemy currently being
			// fought) was pointing at t. If it was, cEnemy was left
			// dangling: after the shrink, index t is no longer a
			// valid "active" slot, so the player could end up
			// fighting a stale/removed enemy, or attacks would
			// silently stop landing on the enemy that actually moved
			// into slot j. We now redirect cEnemy to follow the swap,
			// and also re-examine slot j (j--) since a different
			// enemy now lives there and would otherwise be skipped
			// for a full frame.
			int t = EnemyNo - 1;
			if (j == t){
				// last enemy in the array - it's just removed, no swap
				if (cEnemy == j) cEnemy = -1;
			}
			else{
				if (cEnemy == t) cEnemy = j;        // the enemy being moved into slot j
				else if (cEnemy == j) cEnemy = -1;  // the enemy that just died was targeted
				enemy[j] = enemy[t];
				j--; // reprocess slot j next iteration - it now holds a different enemy
			}
			EnemyNo--;
			continue;
		}
		if (playerBlockNo == enemy[j].blockNo){
			// cEnemy was selected above as the nearest enemy on this
			// block. Do not overwrite it here.
			// Keep the enemy's sprite fully over its platform even while
			// chasing the player. enemy.x is the sprite center, so the
			// legal center range is exactly one half-sprite in from each
			// side of the ACTUAL floor (see enemyPatrolRange above) rather
			// than the separate block[].start/end bookkeeping, so the enemy
			// can never visibly run past where its rock stops.
			int edgeLo, edgeHi;
			enemyPatrolRange(enemy[j].x, enemy[j].blockNo, &edgeLo, &edgeHi);
			int chaseLo = edgeLo + ENEMY_IMG_SIZE / 2;
			int chaseHi = edgeHi - ENEMY_IMG_SIZE / 2;
			if (chaseHi < chaseLo) chaseHi = chaseLo;

			if (playerX < enemy[j].x){
				enemy[j].k = 0;
				if (enemy[j].x - playerX <= COMBAT_RANGE){
					if (enemy[j].m == 5){ enemy[j].a = 1; enemy[j].atkAnim = ENEMY_ATTACK_TICKS; }
				}
				else {
					enemy[j].x -= 2;
					if (enemy[j].x < chaseLo) enemy[j].x = chaseLo;
					// Don't chase straight through an enemy that's
					// already queued up closer to the player.
					int queueLimit = enemyQueueLimit(j, -1);
					if (enemy[j].x < queueLimit) enemy[j].x = queueLimit;
					advanceEnemyAnim(&enemy[j]); // slowed-down walk-cycle advance (see ENEMY_ANIM_TICK_DIVISOR above)
				}
			}
			else {
				enemy[j].k = 13;
				if (playerX - enemy[j].x <= COMBAT_RANGE){
					if (enemy[j].m == 5){ enemy[j].a = 1; enemy[j].atkAnim = ENEMY_ATTACK_TICKS; }
				}
				else {
					enemy[j].x += 2;
					if (enemy[j].x > chaseHi) enemy[j].x = chaseHi;
					// Don't chase straight through an enemy that's
					// already queued up closer to the player.
					int queueLimit = enemyQueueLimit(j, 1);
					if (enemy[j].x > queueLimit) enemy[j].x = queueLimit;
					advanceEnemyAnim(&enemy[j]); // slowed-down walk-cycle advance (see ENEMY_ANIM_TICK_DIVISOR above)
				}
			}
		}
		else{
			// NEW: patrol behavior. The player isn't on this enemy's
			// block, so instead of standing still, walk back and
			// forth between the two ends of the block the enemy
			// belongs to. block[enemy[j].blockNo] is looked up live
			// (not cached) so this stays correct even for enemies
			// that live on a moving wall or elevator block, whose
			// start/end shift every frame in the Moving/Jumping Wall
			// update sections above.
			// enemy.x is the sprite center. These limits put the sprite's
			// left/right edge exactly at the ACTUAL floor's left/right edge
			// (see enemyPatrolRange above - which reads the same array the
			// player's own collision reads), not the separate block[].start/
			// end bookkeeping, so patrols cover the full usable width of the
			// rock without ever running past where it actually ends.
			int edgeLo, edgeHi;
			enemyPatrolRange(enemy[j].x, enemy[j].blockNo, &edgeLo, &edgeHi);
			int lo = edgeLo + ENEMY_IMG_SIZE / 2;
			int hi = edgeHi - ENEMY_IMG_SIZE / 2;

			// BUGFIX: when several guards share one platform, split it
			// between them and give each its own stretch to walk.
			//
			// They used to share the whole rock and rely on bumping into
			// each other to turn around. That does not actually keep
			// them apart: once a pair has settled into travelling the
			// same way with more than a sprite's width between them they
			// never touch again, so neither one ever turns, and they
			// just drift along together on one side of the platform -
			// which is the "two enemies running at the same side"
			// problem. Their sub-ranges also ended up wildly uneven,
			// leaving one of them pacing a few pixels while the other
			// had the whole rock.
			//
			// Slicing the rock is deterministic: each guard always has
			// its own half (or third) to patrol, so the pair is visibly
			// covering different parts of the platform and turning at
			// different times. The slices are recomputed every tick from
			// the LIVING guards on the block, so when one dies the
			// survivor immediately inherits the whole rock. Chasing is
			// deliberately left unsliced above - an enemy hunting the
			// player must still be able to cross its entire platform.
			int shareCount = 0, shareIndex = 0;
			for (int i = 0; i < EnemyNo; i++){
				if (enemy[i].life <= 0) continue;
				if (enemy[i].blockNo != enemy[j].blockNo) continue;
				if (i < j) shareIndex++;
				shareCount++;
			}
			if (shareCount > 1 && hi > lo){
				int span = (hi - lo) / shareCount;
				if (span >= 24){            // only if each slice is worth walking
					lo = lo + shareIndex * span;
					hi = lo + span;
				}
			}

			if (hi < lo){
				// block is too narrow to patrol (e.g. a 128px
				// elevator/moving-wall tile) - just stay put, facing
				// whichever way it was last facing.
			}
			else{
				if (enemy[j].dir == 0) enemy[j].dir = 1; // safety default, in case a slot was never explicitly initialized

				enemy[j].x += enemy[j].dir * 2; // same walk speed as the chase movement above

				// Bounce off another patrolling enemy on the same
				// block the same way it bounces off the block's own
				// edges below, instead of walking straight through it.
				//
				// BUGFIX: turn the OTHER enemy around too.
				//
				// This bounce used to flip only the enemy currently
				// being processed. The enemies are stepped in array
				// order in one pass, so when two of them met, the first
				// one processed turned away - and by the time the second
				// was processed the first had already moved out of its
				// path, so it never bounced at all and just carried on
				// in the same direction. The result was that the pair
				// stopped patrolling in opposition and set off walking
				// the same way together, then bunched up against one end
				// of their platform. That is exactly the "both enemies
				// run to the same side" behaviour.
				//
				// A collision is mutual, so both now turn away from each
				// other on the same tick and the pair stays in
				// opposition for good.
				int blocker = -1;
				int queueLimit = enemyQueueLimit(j, enemy[j].dir, &blocker);
				if (enemy[j].dir < 0 && enemy[j].x < queueLimit){
					enemy[j].x = queueLimit;
					enemy[j].dir = 1;
					if (blocker >= 0){
						enemy[blocker].dir = -1;          // it heads the other way
						enemy[blocker].k = 0;             // ...and faces that way
					}
				}
				else if (enemy[j].dir > 0 && enemy[j].x > queueLimit){
					enemy[j].x = queueLimit;
					enemy[j].dir = -1;
					if (blocker >= 0){
						enemy[blocker].dir = 1;
						enemy[blocker].k = 13;
					}
				}

				// BUGFIX: only turn around if the enemy actually WALKED
				// into this edge - not when the edge moved onto it.
				//
				// An enemy's usable floor is not fixed. A moving bridge
				// docks against the end of a rock (mWall[2] against the
				// second rock in Level 2, for instance), and while it is
				// parked there those 128px belong to the bridge, so the
				// rock's patrol range shrinks by that much and springs
				// back when the bridge leaves. The clamp used to force
				// `dir` every time it fired, so an enemy that the
				// shrinking edge was shoving eastward had its direction
				// pinned to "east" on every single tick - it could never
				// turn around, and the second enemy on that rock got
				// pushed along in front of it. Both then ran the same way
				// for as long as the bridge kept nudging them, which is
				// exactly the "two enemies patrolling the same side"
				// behaviour.
				//
				// Being pushed is not the same as arriving, so the
				// direction now only flips when the enemy was genuinely
				// heading at the edge it hit.
				if (enemy[j].x <= lo){
					enemy[j].x = lo;
					if (enemy[j].dir < 0) enemy[j].dir = 1;
				}
				else if (enemy[j].x >= hi){
					enemy[j].x = hi;
					if (enemy[j].dir > 0) enemy[j].dir = -1;
				}

				enemy[j].k = (enemy[j].dir == 1) ? 13 : 0; // face the direction of travel
				// BUGFIX: this used to be gated behind `if (enemy[j].m
				// != 5)`, same as the chase branch's p++ used to look.
				// But enemy[j].m is a general cooldown counter that
				// climbs to 5 and then STAYS at 5 (see the `if
				// (enemy[j].m != 5) enemy[j].m++;` in iDraw()'s Enemy
				// section) - it reaches 5 within the first second of
				// gameplay and never resets while patrolling (only
				// resets to 0 after an attack finishes). So gating the
				// walk-cycle frame counter on it meant the patrol
				// animation would advance for a moment and then freeze
				// solid on a single frame for the rest of the level -
				// exactly the "only one enemy image" symptom. The walk
				// animation should simply keep looping the whole time
				// the enemy is walking, so this now increments
				// unconditionally, same as the chase-movement branches
				// above already do. It now goes through
				// advanceEnemyAnim() (see ENEMY_ANIM_TICK_DIVISOR
				// above) so the patrol walk-cycle plays at the same
				// slowed-down pace as the chase walk-cycle, instead of
				// advancing every single tick.
				advanceEnemyAnim(&enemy[j]);
			}
		}
	}
	/*Enemy*//*Update*//*End*/

	// BUGFIX: previously the only way to reach STATE_GAMEOVER was
	// falling off the level twice - health draining to 0 purely from
	// combat/fall damage was never checked, so the player could sit
	// at 0 HP (or, before the clamp above, negative HP with a broken
	// health bar) and keep playing indefinitely.
	// Also now checks STATE_LEVEL2, not just STATE_GAME - previously,
	// running out of health this way in Level 2 was never detected at
	// all, so the bg50.png Game Over screen never showed there.
	/* LEVEL 3: the piranhas.
	 *
	 * Run HERE, at the end of the tick, rather than beside the swim code
	 * at the top: the elevator and moving-bridge sections above re-stamp
	 * height[] every tick, so a fish clamped into open water before they
	 * ran could have a bridge slide into the same spot immediately
	 * afterwards and spend a frame inside the stonework. Clamping them
	 * after the platforms have settled means the collision map they are
	 * measured against is this tick's final one.
	 *
	 * Bite damage therefore lands just before the `playerHealth <= 0`
	 * check below, so running the life bar out to a piranha is caught on
	 * the same tick as any other cause. */
	if (gameState == STATE_LEVEL3) level3UpdateFish();

	if (playerHealth <= 0 && isGameplayState()){
		gameOverFromLevel = gameState; // remember Level 1 / 2 / 3 for RETRY
		gameState = STATE_GAMEOVER;
	}

	/* ---------------- Level complete check ---------------- */
	// The level is finished the moment the very last enemy is killed
	// (EnemyNo is decremented above every time an enemy's life drops
	// to 0). Switch to the Level Completed screen (bg30.png); further
	// gameplay updates stop automatically since update() returns
	// immediately for any gameState other than STATE_GAME/STATE_LEVEL2.
	// For both Level 1 and Level 2, the bg30.png completion screen is
	// shown immediately after the final enemy is killed. Level 2 starts
	// from the NEXT LEVEL button when completing Level 1; after completing
	// Level 2, the same completion screen remains active. See
	// handleHomeMouse()'s STATE_LEVELCOMPLETE case in home.h, which is
	// where resetGame()/setupLevel2()/startGameBGM() now happen instead.
	//
	// LEVEL 3 IS DELIBERATELY EXCLUDED from this rule. Killing its ten
	// ordinary guards is not what finishes it: the level ends when the
	// Prince reaches the palace door at the end of the escape run (see
	// section 6 of updateLevel3(), which switches straight to
	// STATE_VICTORY). So Level 3 can never land on the LEVEL COMPLETE
	// screen, and the NEXT LEVEL button can never be asked for a Level
	// 4 that does not exist.
	if (EnemyNo <= 0 && (gameState == STATE_GAME || gameState == STATE_LEVEL2)){
		// Stop the clock on the same tick the level is won, before the
		// state changes - finishLevelRun() also compares the time with
		// the saved record and stores it if it is faster. It is the same
		// one call Level 3 makes, so neither level has timing logic of
		// its own.
		finishLevelRun(levelNumberForState(gameState));
		// The existing completion condition stays the source of truth for
		// the progress bar too, so it reads exactly 100% here rather than
		// whatever the last guard's x happened to work out to.
		markLevelProgressComplete();
		levelCompleteFromLevel = gameState; // Level 1 -> Level 2, Level 2 -> Level 3
		gameState = STATE_LEVELCOMPLETE;
	}

}
void playerHeight(){
	int h;
	// While the ground-landing pause is active, keep the player pinned
	// right at ground level and skip the normal fall-speed step below -
	// otherwise this function (called every iDraw(), i.e. potentially
	// much faster than the throttled update() logic tick) would keep
	// dragging playerY further negative underneath the 0.5s pause that
	// update() is timing, which would desync the two.
	if (landingState){
		playerBlockNo = -1;
		playerY = 0;
		return;
	}

	// Same idea while drowning: the drown sequence owns the player's
	// position (updateDrowning() sinks drownY itself). Without this,
	// playerHeight() - which runs every iDraw(), not on the throttled
	// logic tick - would keep dragging playerY down to the floor
	// underneath the drown animation and re-trigger the normal
	// landing/respawn path the moment it got there.
	if (drowning){
		playerBlockNo = -1;
		playerY = drownSurfaceY;
		return;
	}

	// LEVEL 3 swimming: same reasoning as drowning above. The swim code
	// in level3UpdateSwim() owns playerY while he is in the water, and
	// this function runs every iDraw() rather than on the throttled logic
	// tick - so without this it would drag him down 5px a frame straight
	// through the water and re-trigger the landing path at the bottom.
	if (level3Swimming){
		playerBlockNo = -1;
		return;
	}

	playerBlockNo = -1;
	if (playerX >= doubleS[0] && playerX <= doubleE[0]){
		if (playerY < doubleUpH[0]) {
			h = doubleDownH[0];
			if ((playerY - h) >= 0 && (playerY - h)<5) playerBlockNo = doubleDown[0];
		}
		else {
			h = doubleUpH[0];
			if ((playerY - h) >= 0 && (playerY - h)<5) playerBlockNo = doubleUp[0];
		}
	}
	else{
		h = height[playerX / 2];
		if ((playerY - h) >= 0 && (playerY - h)<5) playerBlockNo = blockData[playerX / 2];
	}

	if (playerBlockNo == -1){
		// FIX: this used to decrement playerY by 5 on every single
		// iDraw() call, which is NOT rate-limited (see the "Logic-rate
		// cap" comment above iDraw()) - so on a fast machine the
		// player could fall dozens/hundreds of pixels between two
		// consecutive rendered frames, making the fall (and therefore
		// the falling sprite drawFree() draws) effectively invisible.
		// Now the fall step is throttled to the same ~60-ticks/sec
		// rate as update(), using its own small static clock so the
		// player visibly, smoothly falls at a consistent real-world
		// speed no matter how fast the graphics loop calls iDraw().
		static clock_t lastFallStep = 0;
		clock_t nowFall = clock();
		if (lastFallStep == 0 || (double)(nowFall - lastFallStep) / CLOCKS_PER_SEC >= targetUpdateInterval){
			playerY -= 5;
			if (playerY < 0) playerY = 0;
			lastFallStep = nowFall;
		}
	}
	else playerY = h;
}

int getHeight(int tx, int ty){
	int h;
	if (tx >= doubleS[0] && tx <= doubleE[0]){
		if (ty > doubleDownH[0]) {
			h = doubleUpH[0];
		}
		else {
			h = doubleDownH[0];
		}
	}
	else{
		h = height[tx / 2];
	}
	return h;
}

/* ------------------------------------------------------------------ */
/* SWITCH CHECKPOINTS                                                   */
/*                                                                      */
/* Called every time the player operates a switch, by key or by mouse.  */
/* The switch becomes the place a fall sends them back to.              */
/*                                                                      */
/* A checkpoint only ever moves FORWARD. All three levels run west to   */
/* east, so refusing to move it back means walking back and pressing an */
/* earlier switch a second time cannot cost the player the progress     */
/* they have already banked - which is what anyone expects a checkpoint */
/* to do.                                                               */
/*                                                                      */
/* Reaching a new one also clears fallCount, so the "second fall is     */
/* fatal" rule restarts from each checkpoint rather than counting       */
/* across the whole level. Without that the checkpoint would rarely be  */
/* worth anything: the fall that sent the player back there would       */
/* already have used up their one allowed fall.                         */
/* ------------------------------------------------------------------ */
inline void recordCheckpoint(int buttonIndex)
{
	int bx = button[buttonIndex].x;
	int by = button[buttonIndex].y - 48;   // the platform surface under the switch

	if (bx <= checkpointX) return;         // never move a checkpoint backwards

	checkpointX = bx;
	checkpointY = by;
	fallCount = 0;
	checkpointMsgTimer = 150;              // ~2.5s of on-screen confirmation
}

void iMouseMove(int mx, int my)
{
}

void iMouse(int mouseButton, int state, int mx, int my)
{
    if (isGameplayState())
    {
        if (mouseButton == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
        {
            for (int j = 0; j < buttonNo; j++)
            {
                if (mx >= button[j].x - 32 + cX &&
                    mx <= button[j].x + 32 + cX &&
                    my >= button[j].y &&
                    my <= button[j].y + 64)
                {
                    button[j].OnOff = !button[j].OnOff;
                    recordCheckpoint(j);   // switches double as checkpoints
                    playButtonSound();
                    return;
                }
            }

            if (mx >= gameHomeBtnX1 && mx <= gameHomeBtnX2 &&
                my >= gameHomeBtnY1 && my <= gameHomeBtnY2)
            {
                playButtonSound();
                resetGame();
                stopGameBGM();
                gameState = STATE_HOME;
                return;
            }
        }
        return;
    }

    handleHomeMouse(mouseButton, state, mx, my);
}
void iKeyboard(unsigned char key)
{
	// 'O' - TESTING SHORTCUT: jump straight to Level 3 from wherever
	// you are. Handled BEFORE the gameplay gate below on purpose, so it
	// also works from the home screen, the level-complete screen, the
	// game-over screen and the victory screen - not just mid-level.
	// It does not touch, skip or alter the normal Level 1 -> Level 2 ->
	// Level 3 progression in any way; it is purely an extra entry point.
	if (key == 'o' || key == 'O'){
		handleOButton();
		return;
	}

	// 'P' - the original testing shortcut into Level 2, unchanged.
	// Moved up here beside 'O' for symmetry; handlePButton() has always
	// gated itself to STATE_GAME internally, so it behaves exactly as
	// it always did.
	if (key == 'p'){
		handlePButton();
		return;
	}

	if (!isGameplayState()) return; // ignore gameplay keys before gameplay starts

	// Once the prince is in the water, moving/jumping/attacking keys do
	// nothing - he is drowning, and the only thing left is watching the
	// life bar run out. The in-game HOME button (handled in iMouse())
	// still works, so the player is never actually stuck.
	if (drowning) return;

	// 'Q' - DUCK. Level 2's bats sweep across at head height; crouching
	// under one is the only way to avoid the 20% of the life bar it
	// takes off. See the LEVEL 2 - CAVE BATS block in game_globals.h.
	// Does nothing at all in Levels 1 and 3 (level2StartDuck() gates
	// itself on STATE_LEVEL2), so no other level's controls change.
	if (key == 'q' || key == 'Q'){
		level2StartDuck();
		return;
	}

	// While crouched the Prince stays put - running, climbing, flipping
	// switches and swinging are all refused until the crouch runs out
	// (LEVEL2_DUCK_TICKS, about two thirds of a second). That is what
	// makes ducking a real decision with a cost, rather than a stance
	// worth holding permanently. Arrow keys route through here too (see
	// iSpecialKeyboard() below), so they are covered by the same gate.
	if (level2IsDucking()) return;

	// LEVEL 3 - IN THE WATER.
	//
	// Swimming takes over A/D/W/S (and, through iSpecialKeyboard(), the
	// arrow keys) for as long as he is in the water, then hands them
	// straight back. Every walking branch below is guarded on
	// `playerBlockNo != -1` and playerBlockNo is -1 while swimming, so
	// they could not have fired anyway - this just gives those keys
	// something to do instead of nothing. 'R' hauls him out.
	//
	// None of this is reachable outside Level 3: level3Swimming can only
	// be set by level3UpdateSwim(), which returns immediately for every
	// other level. Levels 1 and 2 keep exactly the controls they had.
	if (key == 'r' || key == 'R'){
		level3TryClimb();
		return;
	}
	if (level3Swimming){
		if (key == 'a' || key == 'A') level3Swim(-1, 0);
		else if (key == 'd' || key == 'D') level3Swim(1, 0);
		else if (key == 'w' || key == 'W') level3Swim(0, 1);
		// 'e' as well as 's': iSpecialKeyboard() maps the DOWN arrow to
		// 'e', and that handler is shared with Levels 1 and 2, so taking
		// 'e' as a down-stroke HERE - inside a branch only Level 3's
		// water can reach - makes the arrow keys swim without touching
		// the shared mapping or the switch key anywhere else.
		else if (key == 's' || key == 'S' || key == 'e' || key == 'E') level3Swim(0, -1);
		return;
	}

	if (key == 'a' && playerBlockNo != -1){
		if (playerX - block[playerBlockNo].start < 52) {
			int nh = getHeight(playerX - 100, playerY);
			// FIX: only step off this edge if the ground ahead is NOT
			// higher than the player at all. The old check here was
			// `!(nh>playerY && nh - playerY <= MAX_JUMP_HEIGHT)`, which
			// only blocked movement when the wall ahead was climbable
			// (<= MAX_JUMP_HEIGHT) - a wall taller than that (nh>playerY
			// but the height difference is too big to climb) slipped
			// through the negated condition and let the player walk
			// straight into it. Since there was no actual platform at
			// that spot at the player's current height, playerBlockNo
			// then came back -1 next tick and the player just dropped
			// straight down in front of the wall instead of stopping at
			// its base. Climbing a reachable step is still handled
			// separately by the 'w' key below.
			if (!(nh > playerY)) {
				playerX -= 102;
				cEnemy = -1;
			}
		}
		else {
			// Speed bumped to 1.6x (see nextMoveStep()/PLAYER_SPEED_MULTIPLIER above).
			// The Guardian check is the Level 3 equivalent of the cEnemy
			// check beside it: the boss is not in the enemy[] roster, so
			// it needs its own "you cannot walk through me" test to stop
			// the two sprites overlapping. It is a no-op on Levels 1/2.
			if (!(cEnemy != -1 && playerX - enemy[cEnemy].x < COMBAT_RANGE) &&
				!level3GuardianBlocksPlayer(-1)) playerX -= nextMoveStep();
		}
		if (k == 2) p = 0;
		else p++;
		k = 0;
		// NEW: mark the player as actively moving so iDraw() shows the
		// walk-cycle (drawCharacter()) instead of the idle sprite (see
		// playerIsMoving/moveIdleTimeout above). Recorded regardless of
		// whether an actual step happened this call (e.g. blocked next
		// to an enemy) since the player is still actively trying to
		// move, not standing idle.
		playerIsMoving = 1;
		lastMoveKeyTime = clock();
		startRunningSound();

	}
	if (key == 'd' && playerBlockNo != -1){
		if (block[playerBlockNo].end - playerX < 50){
			int nh = getHeight(playerX + 100, playerY);
			// FIX: same "walks into an unclimbable wall and falls" bug as
			// the 'a' branch above - see the comment there. Block moving
			// forward whenever the ground ahead is higher at all; only
			// climbable steps get handled (via 'w'), and unclimbable
			// walls now correctly stop the player instead of dropping
			// them.
			if (!(nh > playerY)) {
				playerX += 100;
				cEnemy = -1;
			}
		}

		else {
			// Speed bumped to 1.6x (see nextMoveStep()/PLAYER_SPEED_MULTIPLIER above).
			// Same Level 3 Guardian body-block as the 'a' branch above.
			if (!(cEnemy != -1 && enemy[cEnemy].x - playerX < COMBAT_RANGE) &&
				!level3GuardianBlocksPlayer(1)) playerX += nextMoveStep();
		}
		if (k == 0) p = 0;
		else p++;
		k = 13;
		// NEW: same idle/moving bookkeeping as the 'a' branch above.
		playerIsMoving = 1;
		lastMoveKeyTime = clock();
		startRunningSound();
	}
	if (key == 'w' && playerBlockNo != -1){
		// Climb/jump up onto a taller neighbouring block. This is only
		// possible from right at the edge of the current block (the
		// same spots where 'a'/'d' show the "Jump left/right" hint),
		// and only onto a block that's at most MAX_JUMP_HEIGHT taller
		// than where the player is currently standing - i.e. the
		// player can hop up one reasonable step, never scale a wall
		// that's much higher than that. (MAX_JUMP_HEIGHT itself is
		// unchanged - only the cosmetic effect drawn at the takeoff
		// spot got taller (now 8x its original height), see the
		// "Jump Effect" block in iDraw().)
		int dir = 0;
		if (playerX - block[playerBlockNo].start < 52) dir = -1;      // at left edge -> climb left
		else if (block[playerBlockNo].end - playerX < 50) dir = 1;    // at right edge -> climb right

		int climbed = 0; // did this 'w' press actually climb onto something?
		if (dir != 0){
			int tx = playerX + dir * 100;
			int nh = getHeight(tx, playerY);
			if (nh > playerY && nh - playerY <= MAX_JUMP_HEIGHT){
				// trigger the jump visual at the takeoff point
				jumpEffectTimer = 12;
				jumpEffectX = playerX;
				jumpEffectY = playerY;

				playerX = tx;
				playerY = nh;
				cEnemy = -1;
				k = (dir == 1) ? 13 : 0;
				climbed = 1;
			}
		}

		// Give a visual hop on EVERY 'W' press while standing on a block.
		// FIX: a press that actually climbs (dir!=0 and target reachable)
		// still gets the normal small in-place hop, same as before - but
		// a press where there's nothing to climb (dir == 0, at a spot
		// with no edge to jump from, or dir != 0 but the target wasn't
		// climbable/wasn't there) now plays a taller, longer hop instead
		// of the same tiny 14px one, so pressing 'w'/Up next to a wall or
		// in open space reads as an actual jump attempt rather than a
		// barely-visible twitch. This is purely cosmetic (see the
		// hopTimer/drawFree/drawCharacter draw-offset logic in iDraw())
		// - it never touches playerY/playerBlockNo, so it can't affect
		// collision or falling.
		if (climbed){
			hopDuration = normalHopDuration;
			hopMaxHeight = normalHopMaxHeight;
		}
		else{
			hopDuration = noClimbHopDuration;
			hopMaxHeight = noClimbHopMaxHeight;
		}
		hopTimer = hopDuration;
	}
	if (key == 'e'){
		// E is now used only for clicking/toggling nearby switches.
		for (int j = 0; j < buttonNo; j++){
			if (abs(playerX - button[j].x) <= 32 && button[j].y > playerY && button[j].y - playerY <= 48){
				if (button[j].OnOff) button[j].OnOff = 0;
				else button[j].OnOff = 1;
				recordCheckpoint(j);   // switches double as checkpoints
				playButtonSound();
			}
		}
	}
	if (key == ' '){
		// SPACE is now used for shooting/attacking.
		if (atMode == 0){
			// Do not allow another attack to start while the current swing
			// is still running. This also prevents repeatedly restarting
			// the sound effect and eliminates key spam stutter.
			atMode = PLAYER_ATTACK_TICKS;
			playerAtkLastFrame = -1;
			playAttackSound();
		}
	}

}

// Arrow-key controls for iGraphics / GLUT special-key input.
// GLUT reports special keys (arrows, F-keys, etc.) as plain ints via
// glutSpecialFunc - NOT as the unsigned char used for normal iKeyboard()
// presses - so this takes an int and just forwards to the same
// iKeyboard() logic already used for 'a'/'d'/'w'/space, keeping movement
// identical whether it comes from WASD or the arrow keys.
void iSpecialKeyboard(int key)
{
    if (!isGameplayState()) return;
    if (drowning) return; // arrow keys are dead while under water too

    switch (key)
    {
    case GLUT_KEY_LEFT:
        iKeyboard('a');
        break;
    case GLUT_KEY_UP:
        iKeyboard('w');
        break;
    case GLUT_KEY_RIGHT:
        iKeyboard('d');
        break;
    case GLUT_KEY_DOWN:
        iKeyboard('e');
        break;
    }
}

#endif
