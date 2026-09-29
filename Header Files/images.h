#ifndef IMAGES_H
#define IMAGES_H

#include "game_globals.h"
#include "records.h"
#include "sounds.h"
#include "combat.h"

char *n[] = { "prince/l1.bmp", "prince/l2.bmp", "prince/l3.bmp", "prince/l4.bmp", "prince/l5.bmp", "prince/l6.bmp", "prince/l7.bmp", "prince/l8.bmp",
"prince/l9.bmp", "prince/l10.bmp", "prince/l11.bmp", "prince/l12.bmp", "prince/l13.bmp",
"prince/r1.bmp", "prince/r2.bmp", "prince/r3.bmp", "prince/r4.bmp", "prince/r5.bmp", "prince/r6.bmp", "prince/r7.bmp", "prince/r8.bmp",
"prince/r9.bmp", "prince/r10.bmp", "prince/r11.bmp", "prince/r12.bmp", "prince/r13.bmp" };

/* ---------------------------------------------------------------- */
/* New character art (left-facing walk/idle cycle): 13 frames,      */
/* l1.png..l13.png, each ~80x80 PNG. These replace the old           */
/* prince/l1.bmp..l13.bmp frames wherever k==0 (facing left) is      */
/* used for the walk-cycle sprite - shared by both the player and    */
/* enemies, same as the old n[] array was.                           */
/* Right-facing (k!=0) uses the matching r1.png..r13.png set below.  */
/* Loaded lazily via iLoadImage() (like bg2Image etc.) since          */
/* iShowBMP() only understands .bmp, not .png.                       */
/* ---------------------------------------------------------------- */
#define LEFT_FRAME_COUNT 13
char *leftFrameFiles[LEFT_FRAME_COUNT] = {
	"Assets/Player/l1.png", "Assets/Player/l2.png", "Assets/Player/l3.png", "Assets/Player/l4.png", "Assets/Player/l5.png",
	"Assets/Player/l6.png", "Assets/Player/l7.png", "Assets/Player/l8.png", "Assets/Player/l9.png", "Assets/Player/l10.png",
	"Assets/Player/l11.png", "Assets/Player/l12.png", "Assets/Player/l13.png"
};
int leftFrameImg[LEFT_FRAME_COUNT];
int leftFrameLoaded[LEFT_FRAME_COUNT] = { 0 };
int charImgSize = 80; // native pixel size of the new PNGs (square, confirmed 80x80)

/* ---------------------------------------------------------------- */
/* Right-facing character art: 13 frames, prince/r1.png..r13.png,    */
/* assumed 80x80 (same convention as the left-facing set above).     */
/* Replaces the old prince/r1.bmp..r13.bmp frames wherever k!=0       */
/* (facing right) is used for the walk-cycle sprite.                 */
/* ---------------------------------------------------------------- */
#define RIGHT_FRAME_COUNT 13
char *rightFrameFiles[RIGHT_FRAME_COUNT] = {
	"Assets/Player/r1.png", "Assets/Player/r2.png", "Assets/Player/r3.png", "Assets/Player/r4.png", "Assets/Player/r5.png",
	"Assets/Player/r6.png", "Assets/Player/r7.png", "Assets/Player/r8.png", "Assets/Player/r9.png", "Assets/Player/r10.png",
	"Assets/Player/r11.png", "Assets/Player/r12.png", "Assets/Player/r13.png"
};
int rightFrameImg[RIGHT_FRAME_COUNT];
int rightFrameLoaded[RIGHT_FRAME_COUNT] = { 0 };

/* ---------------------------------------------------------------- */
/* Attack animation art: 3-frame sword swing for each facing         */
/* direction - al1.png/al2.png/al3.png (facing left, k==0) and       */
/* ar1.png/ar2.png/ar3.png (facing right, k!=0). These replace the   */
/* old single-frame al.bmp/ar.bmp "flash" that used to play whenever */
/* the player or an enemy landed a hit. Same lazy iLoadImage() +      */
/* alpha-blend approach as the walk-cycle frames above, since these  */
/* are PNGs with transparency. Drop the 6 files in wherever the old  */
/* al.bmp/ar.bmp lived (same folder, no path prefix).                */
/* ---------------------------------------------------------------- */
#define ATTACK_FRAME_COUNT 3
char *attackLeftFiles[ATTACK_FRAME_COUNT] = { "Assets/Player/al1.png", "Assets/Player/al2.png", "Assets/Player/al3.png" };
char *attackRightFiles[ATTACK_FRAME_COUNT] = { "Assets/Player/ar1.png", "Assets/Player/ar2.png", "Assets/Player/ar3.png" };
int attackLeftImg[ATTACK_FRAME_COUNT];
int attackRightImg[ATTACK_FRAME_COUNT];
int attackLeftLoaded[ATTACK_FRAME_COUNT] = { 0 };
int attackRightLoaded[ATTACK_FRAME_COUNT] = { 0 };
// Native size of the attack PNGs isn't known here - the old al.bmp/
// ar.bmp were drawn at their native BMP size via iShowBMP, which
// takes no width/height, but iShowImage() needs one explicitly. This
// is a starting guess; bump it up/down if the swing looks too small/
// cropped or too big/stretched once you run the game.
int atkImgSize = 100;
// Draws attack-animation frame `frame` (0,1,2) facing left (k==0) or
// right (k!=0) with its corner at (x, y) - same anchor convention the
// old iShowBMP(x, y, "al.bmp"/"ar.bmp") calls used, so the existing
// offsets at the call sites (playerX-45 / playerX-20 / enemy.x-45 /
// enemy.x-20) still line the sprite up roughly the same way as before.
void preloadAttackImages(){
	// Load all six attack sprites before the player can press E.
	// Previously drawAttack() loaded each PNG the first time that
	// particular frame was needed, which caused the visible pause on
	// the first attack.
	for (int i = 0; i < ATTACK_FRAME_COUNT; i++){
		if (!attackLeftLoaded[i]){
			attackLeftImg[i] = iLoadImage(attackLeftFiles[i]);
			attackLeftLoaded[i] = 1;
		}
		if (!attackRightLoaded[i]){
			attackRightImg[i] = iLoadImage(attackRightFiles[i]);
			attackRightLoaded[i] = 1;
		}
	}
}

void drawAttack(int x, int y, int k, int frame){
	if (frame < 0) frame = 0;
	if (frame >= ATTACK_FRAME_COUNT) frame = ATTACK_FRAME_COUNT - 1;
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	if (k == 0)
		iShowImage(x, y, atkImgSize, atkImgSize, attackLeftImg[frame]);
	else
		iShowImage(x, y, atkImgSize, atkImgSize, attackRightImg[frame]);
	glDisable(GL_BLEND);
}

/* ---------------------------------------------------------------- */
/* Falling sprite art: freeL.png / freeR.png, shown while the player  */
/* is airborne (playerBlockNo == -1), replacing the old freeL.bmp /   */
/* freeR.bmp. Same lazy iLoadImage() + alpha-blend approach as the    */
/* other PNG assets above, since these have transparency.             */
/* ---------------------------------------------------------------- */
int freeLImage, freeRImage;
int freeLLoaded = 0, freeRLoaded = 0;
// Native size of the new PNGs isn't known here - starting guess is
// the same 80x80 the walk-cycle frames use. Adjust if the falling
// sprite looks too small/cropped or too big/stretched on screen.
int freeImgSize = 80;

// Draws the falling/airborne sprite at (x, y) - x/y are the same
// top-left corner the old iShowBMP(playerX - 32 + cX, playerY, ...)
// call used, so the existing -32 offset at the call site still lines
// it up the same way as before.
void drawFree(int x, int y, int k){
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	if (k == 0){
		if (!freeLLoaded){
			freeLImage = iLoadImage("Assets/Player/freeL.png");
			freeLLoaded = 1;
		}
		iShowImage(x, y, freeImgSize, freeImgSize, freeLImage);
	}
	else{
		if (!freeRLoaded){
			freeRImage = iLoadImage("Assets/Player/freeR.png");
			freeRLoaded = 1;
		}
		iShowImage(x, y, freeImgSize, freeImgSize, freeRImage);
	}
	glDisable(GL_BLEND);
}

/* ---------------------------------------------------------------- */
/* Drowning art: drown1.png .. drown6.png.                            */
/*                                                                    */
/* Six frames of the prince going under. RE-POSED: these used to be   */
/* the running sprite simply recoloured, which read as a man jogging  */
/* through green soup rather than drowning.                            */
/*                                                                    */
/* They are still HIS artwork, pixel for pixel - same hair, face,      */
/* vest, sash, trousers and boots. The frames are built from           */
/* prince/l0.png by splitting it into body / left arm / right arm and  */
/* rotating each arm about its own shoulder so both reach upward for   */
/* the surface; nothing is drawn by hand. Across the six frames the    */
/* body then tips further and further over as the fight goes out of    */
/* him, his last breath streams up, and the poison drains the colour   */
/* out of him. The frame                                               */
/* shown is picked purely from how far through the drown timer the    */
/* player is (see drawDrowningScene() below), so the art tracks the   */
/* life bar draining: as his health runs out, he sinks deeper and     */
/* fades further into the water.                                      */
/* ---------------------------------------------------------------- */
char *drownFiles[DROWN_FRAME_COUNT] = {
	"Assets/Player/drown1.png", "Assets/Player/drown2.png", "Assets/Player/drown3.png",
	"Assets/Player/drown4.png", "Assets/Player/drown5.png", "Assets/Player/drown6.png"
};
int drownImage[DROWN_FRAME_COUNT];
int drownImgLoaded[DROWN_FRAME_COUNT] = { 0 };
// The drown frames are 96x96 (bigger than the 80px walk frames) so
// there is room around the body for the bubbles baked into the art.
int drownImgSize = 96;

void drawDrownSprite(int x, int y, int frame)
{
	if (frame < 0) frame = 0;
	if (frame >= DROWN_FRAME_COUNT) frame = DROWN_FRAME_COUNT - 1;
	if (!drownImgLoaded[frame]){
		drownImage[frame] = iLoadImage(drownFiles[frame]);
		drownImgLoaded[frame] = 1;
	}
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	iShowImage(x, y, drownImgSize, drownImgSize, drownImage[frame]);
	glDisable(GL_BLEND);
}

/* ---------------------------------------------------------------- */
/* The whole drowning scene, drawn in place of the player sprite for  */
/* as long as `drowning` is set (see the Player section of iDraw()).  */
/* Everything here is cosmetic - the actual state (how far along the  */
/* drown is, how deep he has sunk, how much health is left) is owned  */
/* by updateDrowning() in controls.h.                                 */
/* ---------------------------------------------------------------- */
void drawDrowningScene()
{
	int surface = drownSurfaceY;
	int cx = playerX + cX;               // the prince's centre on screen
	double progress = (double)drownTicks / (double)DROWN_TOTAL_TICKS;
	if (progress > 1.0) progress = 1.0;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/* --- the water closing over the screen --- */
	// A blue wash that deepens as he sinks, so the whole frame reads
	// as "under water" by the end rather than just the sprite.
	glColor4f(0.15f, 0.42f, 0.05f, (float)(0.12 + 0.38 * progress));
	iFilledRectangle(0, 0, 1350, 680);

	/* --- splash rings at the point of entry --- */
	// Three rings spreading out and fading over the first half second.
	if (drownTicks < 30){
		double t = drownTicks / 30.0;
		for (int i = 0; i < 3; i++){
			glColor4f(1.0f, 1.0f, 1.0f, (float)(0.70 * (1.0 - t)));
			iFilledEllipse(cx, surface + 16, 20 + i * 15 + t * 45, 5 + i * 2);
		}
	}

	/* --- the prince himself --- */
	// A little side-to-side thrash while he still has air left; it
	// stops once he goes limp in the last quarter of the timer.
	int sway = 0;
	if (progress < 0.75){
		int phase = (drownTicks / 6) % 4;
		if (phase == 1) sway = 4;
		else if (phase == 3) sway = -4;
	}
	int frame = (int)(progress * DROWN_FRAME_COUNT);
	drawDrownSprite(cx - drownImgSize / 2 + sway, drownY - 24, frame);

	/* --- bubbles streaming up from him and popping at the surface --- */
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	for (int i = 0; i < 10; i++){
		int span = 110 + (i * 23) % 70;            // how far this bubble rises
		int rise = (drownTicks * 2 + i * 19) % span;
		double bx = cx + (i - 5) * 8 + ((i % 2) ? 4 : -4);
		double by = drownY + 12 + rise;
		if (by > surface + 24) continue;           // gone once it reaches the top
		double r = 2.0 + (i % 3);
		glColor4f(0.75f, 0.95f, 0.35f, (float)(0.60 * (1.0 - (double)rise / span)));
		iFilledCircle(bx, by, r);
	}
	glDisable(GL_BLEND);

	/* --- "air running out" readout --- */
	// A bar that empties in step with the life bar at the bottom of
	// the screen, plus a warning line, so it is obvious what is
	// happening and that it is about to be fatal.
	int airW = (int)(300 * (1.0 - progress));
	if (airW < 0) airW = 0;
	iSetcolor(255, 255, 255);
	iRectangle(525, 620, 304, 16);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(0.55f, 0.90f, 0.15f, 0.9f);
	iFilledRectangle(527, 622, airW, 12);
	glDisable(GL_BLEND);

	iSetcolor(255, 255, 255);
	// Same sequence, same timer, same health drain in both levels - only
	// the caption changes, because in Level 3 the poison is rising up
	// through the palace around the player, rather than being a pool he
	// fell into.
	if (gameState == STATE_LEVEL3){
		iText(538, 596, "DROWNING - THE POISON HAS RISEN OVER YOU!", GLUT_BITMAP_9_BY_15);
		iText(505, 575, "The poison has closed over you. There is no way back up.", GLUT_BITMAP_8_BY_13);
	}
	else{
		iText(560, 596, "DROWNING - poisoned water!", GLUT_BITMAP_9_BY_15);
		iText(495, 575, "You fell into the poison water. There is no way back up.", GLUT_BITMAP_8_BY_13);
	}

	// Leave the GL colour the way the rest of iDraw() expects to find it.
	iSetcolor(255, 255, 255);
}

/* ---------------------------------------------------------------- */
/* Switch icon art: onSwitch.png / offSwitch.png, replacing the old   */
/* onSwitch.bmp / offSwitch.bmp (plain BMPs can't have a transparent  */
/* background, which is why they used to show up with a black box     */
/* around them). Same lazy iLoadImage() + alpha-blend approach as the */
/* other PNG assets above.                                            */
/* ---------------------------------------------------------------- */
int onSwitchImage, offSwitchImage;
int onSwitchLoaded = 0, offSwitchLoaded = 0;
// Native size of onSwitch.bmp/offSwitch.bmp - kept the same as the
// old BMPs so the switch doesn't change size on screen.
int switchImgSize = 64;

// Draws the switch icon at (x, y) - same top-left corner the old
// iShowBMP(button[j].x - 32 + cX, button[j].y, ...) call used.
void drawSwitch(int x, int y, int onOff){
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	if (onOff){
		if (!onSwitchLoaded){
			onSwitchImage = iLoadImage("Assets/Objects/onSwitch.png");
			onSwitchLoaded = 1;
		}
		iShowImage(x, y, switchImgSize, switchImgSize, onSwitchImage);
	}
	else{
		if (!offSwitchLoaded){
			offSwitchImage = iLoadImage("Assets/Objects/offSwitch.png");
			offSwitchLoaded = 1;
		}
		iShowImage(x, y, switchImgSize, switchImgSize, offSwitchImage);
	}
	glDisable(GL_BLEND);
}

/* ---------------------------------------------------------------- */
/* Wall tile art: lc.png, rc.png, rcb.png, rw.png, rwb.png, uw.png,   */
/* wall.png - replacing the old lc.bmp/rc.bmp/rcb.bmp/rw.bmp/         */
/* rwb.bmp/uw.bmp/wall.bmp used to build the static walls, jumping    */
/* walls, and moving walls. These are fully opaque 64x64 tiles (no    */
/* transparent background), so no alpha blending is needed here -     */
/* just a lazy iLoadImage() + iShowImage() the same as the other      */
/* background images (bg1Image, bg2Image, etc.) already do.           */
/* ---------------------------------------------------------------- */
int lcImage, rcImage, rcbImage, rwImage, rwbImage, uwImage, wallTileImage;
int lcLoaded = 0, rcLoaded = 0, rcbLoaded = 0, rwLoaded = 0, rwbLoaded = 0, uwLoaded = 0, wallTileLoaded = 0;
int wallTileSize = 64; // all seven pieces are 64x64, matching the old BMPs

/* ---------------------------------------------------------------- */
/* Cave wall tile art (Level 2 ONLY): cave_lc.png, cave_rc.png,       */
/* cave_rcb.png, cave_rw.png, cave_rwb.png, cave_uw.png, cave_wall.png*/
/* These are the exact same 7-piece tile set as the brick walls above */
/* (same 64x64 size, same role in the wall/jWall/mWall drawing loops  */
/* below), just re-textured as rough cave rock so the whole running/  */
/* platforming area of Level 2 reads as the inside of a cave. Nothing */
/* else changes: Level 1 keeps using lc.png/rc.png/.../wall.png       */
/* exactly as before, and every position/size/gameplay value in       */
/* next_level.h is untouched - only which image gets stamped onto     */
/* each wall tile differs, and only while gameState == STATE_LEVEL2.  */
/* ---------------------------------------------------------------- */
int caveLcImage, caveRcImage, caveRcbImage, caveRwImage, caveRwbImage, caveUwImage, caveWallTileImage;
int caveLcLoaded = 0, caveRcLoaded = 0, caveRcbLoaded = 0, caveRwLoaded = 0, caveRwbLoaded = 0, caveUwLoaded = 0, caveWallTileLoaded = 0;

/* ---------------------------------------------------------------- */
/* SAND tiles - LEVEL 3 ONLY.                                         */
/*                                                                    */
/* Level 3 is outside the palace, crossing the open kingdom, so the    */
/* cool blue-grey brickwork of the palace interior has no business     */
/* being there. It uses weathered desert sandstone instead.            */
/*                                                                    */
/* The sand_*.png tiles are derived from the original brick tiles, so  */
/* every bevel, the lit top face, the shadowed right side face and all */
/* the tile-to-tile alignment are identical - platforms assemble       */
/* exactly as they always did. Only the surface differs.               */
/*                                                                    */
/* Exactly the same mechanism the cave tiles above already use: the    */
/* swap happens purely inside these draw helpers, gated on gameState,  */
/* so no position, size or gameplay value anywhere changes - and       */
/* Levels 1 and 2 keep their own tiles untouched.                      */
/* ---------------------------------------------------------------- */
int sandLcImage, sandRcImage, sandRcbImage, sandRwImage, sandRwbImage, sandUwImage, sandWallTileImage;
int sandLcLoaded = 0, sandRcLoaded = 0, sandRcbLoaded = 0, sandRwLoaded = 0, sandRwbLoaded = 0, sandUwLoaded = 0, sandWallTileLoaded = 0;

// cave_ceiling.png: cave_uw.png flipped vertically, so the jagged rocky
// rim hangs down like stalactites instead of sitting up like a floor
// edge. Used only for the bottom-most row of the "pack everything above
// the running band with rock" fill added around setupLevel2()'s wall[]
// platforms below - see the Cave fill block after /*Wall*//*End*/.
int caveCeilingImage;
int caveCeilingLoaded = 0;
#define CAVE_RUN_CLEARANCE 150
// Hard ceiling for where a ceiling row is allowed to start: the screen
// is only 680px tall, so if platform height + CAVE_RUN_CLEARANCE would
// push the ceiling row above 680 (e.g. an elevator whose upperLimit is
// already close to the top of the screen), clamp it back on-screen -
// otherwise the ceiling fill entirely misses the visible area and a
// gap of open background shows above the platform/shaft.
#define CAVE_MAX_CEIL_START 615

void drawCaveCeiling(int x, int y){
	if (!caveCeilingLoaded){ caveCeilingImage = iLoadImage("Assets/Platforms/Level2/cave_ceiling.png"); caveCeilingLoaded = 1; }
	iShowImage(x, y, wallTileSize, wallTileSize, caveCeilingImage);
}

void drawLC(int x, int y){
	if (gameState == STATE_LEVEL3){
		if (!sandLcLoaded){ sandLcImage = iLoadImage("Assets/Platforms/Level3/sand_lc.png"); sandLcLoaded = 1; }
		iShowImage(x, y, wallTileSize, wallTileSize, sandLcImage);
		return;
	}
	if (gameState == STATE_LEVEL2){
		if (!caveLcLoaded){ caveLcImage = iLoadImage("Assets/Platforms/Level2/cave_lc.png"); caveLcLoaded = 1; }
		iShowImage(x, y, wallTileSize, wallTileSize, caveLcImage);
		return;
	}
	if (!lcLoaded){ lcImage = iLoadImage("Assets/Platforms/Level1/lc.png"); lcLoaded = 1; }
	iShowImage(x, y, wallTileSize, wallTileSize, lcImage);
}
void drawRC(int x, int y){
	if (gameState == STATE_LEVEL3){
		if (!sandRcLoaded){ sandRcImage = iLoadImage("Assets/Platforms/Level3/sand_rc.png"); sandRcLoaded = 1; }
		iShowImage(x, y, wallTileSize, wallTileSize, sandRcImage);
		return;
	}
	if (gameState == STATE_LEVEL2){
		if (!caveRcLoaded){ caveRcImage = iLoadImage("Assets/Platforms/Level2/cave_rc.png"); caveRcLoaded = 1; }
		iShowImage(x, y, wallTileSize, wallTileSize, caveRcImage);
		return;
	}
	if (!rcLoaded){ rcImage = iLoadImage("Assets/Platforms/Level1/rc.png"); rcLoaded = 1; }
	iShowImage(x, y, wallTileSize, wallTileSize, rcImage);
}
void drawRCB(int x, int y){
	if (gameState == STATE_LEVEL3){
		if (!sandRcbLoaded){ sandRcbImage = iLoadImage("Assets/Platforms/Level3/sand_rcb.png"); sandRcbLoaded = 1; }
		iShowImage(x, y, wallTileSize, wallTileSize, sandRcbImage);
		return;
	}
	if (gameState == STATE_LEVEL2){
		if (!caveRcbLoaded){ caveRcbImage = iLoadImage("Assets/Platforms/Level2/cave_rcb.png"); caveRcbLoaded = 1; }
		iShowImage(x, y, wallTileSize, wallTileSize, caveRcbImage);
		return;
	}
	if (!rcbLoaded){ rcbImage = iLoadImage("Assets/Platforms/Level1/rcb.png"); rcbLoaded = 1; }
	iShowImage(x, y, wallTileSize, wallTileSize, rcbImage);
}
void drawRW(int x, int y){
	if (gameState == STATE_LEVEL3){
		if (!sandRwLoaded){ sandRwImage = iLoadImage("Assets/Platforms/Level3/sand_rw.png"); sandRwLoaded = 1; }
		iShowImage(x, y, wallTileSize, wallTileSize, sandRwImage);
		return;
	}
	if (gameState == STATE_LEVEL2){
		if (!caveRwLoaded){ caveRwImage = iLoadImage("Assets/Platforms/Level2/cave_rw.png"); caveRwLoaded = 1; }
		iShowImage(x, y, wallTileSize, wallTileSize, caveRwImage);
		return;
	}
	if (!rwLoaded){ rwImage = iLoadImage("Assets/Platforms/Level1/rw.png"); rwLoaded = 1; }
	iShowImage(x, y, wallTileSize, wallTileSize, rwImage);
}
void drawRWB(int x, int y){
	if (gameState == STATE_LEVEL3){
		if (!sandRwbLoaded){ sandRwbImage = iLoadImage("Assets/Platforms/Level3/sand_rwb.png"); sandRwbLoaded = 1; }
		iShowImage(x, y, wallTileSize, wallTileSize, sandRwbImage);
		return;
	}
	if (gameState == STATE_LEVEL2){
		if (!caveRwbLoaded){ caveRwbImage = iLoadImage("Assets/Platforms/Level2/cave_rwb.png"); caveRwbLoaded = 1; }
		iShowImage(x, y, wallTileSize, wallTileSize, caveRwbImage);
		return;
	}
	if (!rwbLoaded){ rwbImage = iLoadImage("Assets/Platforms/Level1/rwb.png"); rwbLoaded = 1; }
	iShowImage(x, y, wallTileSize, wallTileSize, rwbImage);
}
void drawUW(int x, int y){
	if (gameState == STATE_LEVEL3){
		if (!sandUwLoaded){ sandUwImage = iLoadImage("Assets/Platforms/Level3/sand_uw.png"); sandUwLoaded = 1; }
		iShowImage(x, y, wallTileSize, wallTileSize, sandUwImage);
		return;
	}
	if (gameState == STATE_LEVEL2){
		if (!caveUwLoaded){ caveUwImage = iLoadImage("Assets/Platforms/Level2/cave_uw.png"); caveUwLoaded = 1; }
		iShowImage(x, y, wallTileSize, wallTileSize, caveUwImage);
		return;
	}
	if (!uwLoaded){ uwImage = iLoadImage("Assets/Platforms/Level1/uw.png"); uwLoaded = 1; }
	iShowImage(x, y, wallTileSize, wallTileSize, uwImage);
}
void drawWallTile(int x, int y){
	if (gameState == STATE_LEVEL3){
		if (!sandWallTileLoaded){ sandWallTileImage = iLoadImage("Assets/Platforms/Level3/sand_wall.png"); sandWallTileLoaded = 1; }
		iShowImage(x, y, wallTileSize, wallTileSize, sandWallTileImage);
		return;
	}
	if (gameState == STATE_LEVEL2){
		if (!caveWallTileLoaded){ caveWallTileImage = iLoadImage("Assets/Platforms/Level2/cave_wall.png"); caveWallTileLoaded = 1; }
		iShowImage(x, y, wallTileSize, wallTileSize, caveWallTileImage);
		return;
	}
	if (!wallTileLoaded){ wallTileImage = iLoadImage("Assets/Platforms/Level1/wall.png"); wallTileLoaded = 1; }
	iShowImage(x, y, wallTileSize, wallTileSize, wallTileImage);
}

/* ---------------------------------------------------------------- */
/* Water tiles: water.png (body) / water_top.png (surface, with a     */
/* foamy highlight line) - drawn in the pit underneath every moving   */
/* wall (mWall) and in every elevator shaft (jWall).                  */
/*                                                                    */
/* CHANGED: Level 2's pits used to be filled with the green           */
/* poison_water.png / poison_water_top.png tiles, which were purely   */
/* decorative - falling into them was treated exactly like falling    */
/* onto bare ground. They are now the blue real-water tiles, and      */
/* falling into one starts the drowning sequence instead (see         */
/* waterSurfaceAt() / startDrowning() / updateDrowning() in           */
/* controls.h and drawDrowningScene() further down this file).        */
/* water_top.png is used for the surface row the player would see     */
/* first looking down, and plain water.png tiles the rest of the way  */
/* down to the floor.                                                 */
/* ---------------------------------------------------------------- */
int waterImage, waterTopImage;
int waterLoaded = 0, waterTopLoaded = 0;

void drawWater(int x, int y){
	if (!waterLoaded){ waterImage = iLoadImage("Assets/Water/water.png"); waterLoaded = 1; }
	iShowImage(x, y, wallTileSize, wallTileSize, waterImage);
}
void drawWaterTop(int x, int y){
	if (!waterTopLoaded){ waterTopImage = iLoadImage("Assets/Water/water_top.png"); waterTopLoaded = 1; }
	iShowImage(x, y, wallTileSize, wallTileSize, waterTopImage);
}

/* ---------------------------------------------------------------- */
/* Poison tiles - LEVEL 2 ONLY.                                       */
/*                                                                    */
/* poison_water.png / poison_water_top.png are dark, murky, toxic     */
/* green: a thing you would not put a hand in, as opposed to the blue */
/* water.png / water_top.png pair. Level 2's pits use these; LEVEL 3  */
/* KEEPS USING water.png - its flood is rising sea water, not poison, */
/* and nothing about Level 3 changes here.                            */
/* ---------------------------------------------------------------- */
int poisonImage, poisonTopImage;
int poisonLoaded = 0, poisonTopLoaded = 0;

void drawPoison(int x, int y){
	if (!poisonLoaded){ poisonImage = iLoadImage("Assets/Water/poison_water.png"); poisonLoaded = 1; }
	iShowImage(x, y, wallTileSize, wallTileSize, poisonImage);
}
void drawPoisonTop(int x, int y){
	if (!poisonTopLoaded){ poisonTopImage = iLoadImage("Assets/Water/poison_water_top.png"); poisonTopLoaded = 1; }
	iShowImage(x, y, wallTileSize, wallTileSize, poisonTopImage);
}

/* Is this exact world x covered by a static rock? */
int level2PointIsRock(int worldX)
{
	for (int w = 0; w < wallNo; w++){
		if (worldX >= wall[w].sPos && worldX < wall[w].ePos) return 1;
	}
	return 0;
}

/* Splits [lo, hi) into the stretches that NO static rock covers, and
 * writes them into runs[]. Returns how many it found.
 *
 * A moving bridge's rail reaches 128px past its right rail, and in
 * Levels 1 and 2 that overlaps the start of the next platform - so the
 * pool under the bridge has to stop where the rock starts. The edges of
 * the rock almost never land on the 64px water-tile grid, so this works
 * to the pixel and the caller clips the tiles with a scissor rectangle:
 * testing whole tile columns instead (the first attempt at this) either
 * painted water over the rock or, when the column was skipped, left a
 * bare gap of up to 63px with nothing drawn in it at all. */
#define LEVEL2_MAX_RUNS 8
int level2OpenRuns(int lo, int hi, int runs[][2], int maxRuns)
{
	int n = 0;
	int start = -1;

	for (int x = lo; x <= hi; x += 2){
		int rock = (x >= hi) ? 1 : level2PointIsRock(x);
		if (!rock){
			if (start < 0) start = x;
		}
		else if (start >= 0){
			if (n < maxRuns){ runs[n][0] = start; runs[n][1] = x; n++; }
			start = -1;
		}
	}
	return n;
}

/* Turns on a scissor rectangle covering world span [a, b) so tiles drawn
 * inside it are clipped exactly at the rock's edge. Returns 0 (and turns
 * nothing on) if the span is entirely off screen. */
int level2BeginClip(int a, int b)
{
	int sx = a + cX;
	int ex = b + cX;
	if (sx < 0) sx = 0;
	if (ex > 1350) ex = 1350;
	if (ex <= sx) return 0;

	glEnable(GL_SCISSOR_TEST);
	glScissor(sx, 0, ex - sx, 680);
	return 1;
}

/* ---------------------------------------------------------------- */
/* POISON - making a body of it actually read as liquid                */
/*                                                                    */
/* Used by BOTH Level 2's pits and Level 3's rising flood, so the two  */
/* levels share one look.                                             */
/*                                                                    */
/* Each pool is a single 64x64 tile repeated, which on its own reads   */
/* as a flat green wall with a pattern on it rather than as water.     */
/* This pass draws over the tiles and adds the four things that sell   */
/* a body of water:                                                   */
/*                                                                    */
/*   DEPTH    - it darkens and goes colder the deeper it gets, so the  */
/*              pool has a bottom instead of being one flat colour     */
/*   MOTION   - the waterline rolls, with two sine waves at different  */
/*              speeds so it never visibly repeats                     */
/*   LIGHT    - bright streaks drift along just under the surface      */
/*   BUBBLES  - they rise out of the dark and pop at the top           */
/*                                                                    */
/* Every pool is derived from exactly the same level data that         */
/* waterSurfaceAt() in controls.h uses to decide where the player       */
/* drowns, so what is drawn and what is lethal cannot disagree.        */
/*                                                                    */
/* The animation is driven off clock(), not off a frame counter, so it */
/* runs at one speed regardless of how fast the machine draws.         */
/* ---------------------------------------------------------------- */
void drawWaterPool(int left, int right, int surfaceY)
{
	if (surfaceY <= 0) return;

	int l = left + cX;
	int r = right + cX;
	if (r < -80 || l > 1350 + 80) return;   // off screen entirely
	if (l < -80) l = -80;
	if (r > 1350 + 80) r = 1350 + 80;

	int width = r - l;
	if (width <= 0) return;

	double t = (double)clock() / (double)CLOCKS_PER_SEC;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/* --- DEPTH: a vertical gradient from the surface down to the floor.
	 *
	 * FIX: this used to fade to (0.01, 0.06, 0.03) at 86% opacity, which
	 * is as good as black - so the bottom of a deep pit stopped looking
	 * like poison and started looking like a hole punched through the
	 * level. It now settles on a dark GREEN at 58%, so the depth is
	 * still obvious but every part of the pool still reads as the same
	 * liquid, and the cave stays visible through it. --- */
	glBegin(GL_QUADS);
	glColor4f(0.14f, 0.46f, 0.20f, 0.12f);
	glVertex2f((float)l, (float)surfaceY);
	glVertex2f((float)r, (float)surfaceY);
	glColor4f(0.03f, 0.19f, 0.08f, 0.58f);
	glVertex2f((float)r, 0.0f);
	glVertex2f((float)l, 0.0f);
	glEnd();

	/* --- MOTION: the surface rolls. The crest is a sickly yellow-green
	 *     scum, not clean white foam. --- */
	for (int x = l; x < r; x += 8){
		double w = sin(x * 0.045 + t * 2.1) * 3.0
			+ sin(x * 0.016 - t * 1.3) * 2.0;
		int y = surfaceY + (int)w;
		glColor4f(0.30f, 0.66f, 0.24f, 0.32f);   // lit band just under the crest
		iFilledRectangle(x, y - 9, 9, 10);
		glColor4f(0.78f, 0.94f, 0.42f, 0.55f);   // the scum line itself
		iFilledRectangle(x, y - 1, 9, 3);
	}

	/* --- GLOW: toxic light drifting along under the surface --- */
	for (int i = 0; i < 5; i++){
		int span = width + 120;
		int px = l - 60 + (int)(t * 22.0 + i * 73.0) % span;
		float a = (float)(0.07 + 0.07 * sin(t * 1.9 + i));
		glColor4f(0.62f, 0.95f, 0.36f, a);
		iFilledEllipse(px, surfaceY - 16 - i * 3, 24 + i * 6, 3, 16);
	}

	/* --- BUBBLES of gas rising out of the depths --- */
	for (int i = 0; i < 10; i++){
		int bx = l + ((i * 97 + 29) % width);
		int cycle = 150 + (i * 31) % 110;
		int rise = (int)(t * 30.0 + i * 47) % cycle;
		int by = surfaceY - cycle + rise;
		if (by > surfaceY - 6) continue;       // already popped
		double frac = (double)rise / (double)cycle;
		glColor4f(0.60f, 0.96f, 0.34f, (float)(0.14 + 0.32 * frac));
		iFilledCircle(bx + (int)(sin(t * 2.6 + i) * 3.0), by, 1.5 + (i % 3), 10);
	}

	glDisable(GL_BLEND);
	iSetcolor(255, 255, 255);
}

/* Draws the effects above over every Level 2 pool. The pools come
 * straight out of mWall[]/jWall[], the same two places waterSurfaceAt()
 * reads them from. A bridge's rail is split into contiguous runs that
 * skip any static rock it overlaps, so the rolling surface only ever
 * appears over real water. */
void drawLevel2WaterEffects()
{
	if (gameState != STATE_LEVEL2) return;

	for (int j = 0; j < mWallNo; j++){
		int runs[LEVEL2_MAX_RUNS][2];
		int nRuns = level2OpenRuns(mWall[j].left, mWall[j].right + 128,
			runs, LEVEL2_MAX_RUNS);
		// Exactly the same runs the tiles were drawn into above, so the
		// rolling surface starts and stops on the water, not on the rock.
		for (int rr = 0; rr < nRuns; rr++)
			drawWaterPool(runs[rr][0], runs[rr][1], mWall[j].h + 9);
	}

	for (int j = 0; j < jWallNo; j++)
		drawWaterPool(jWall[j].Pos, jWall[j].Pos + 128, jWall[j].lowerLimit - 55);
}

// Draws the walk-cycle sprite for either the player or an enemy at
// world position (centerX, y) - centerX is the character's true
// center X (NOT pre-offset like the old `playerX - 32` call sites
// were), since the two frame sets need different centering math for
// their different native sizes (100px PNG vs whatever size the old
// bmp frames were, hence the old -32 magic number).
// k: 0 = facing left (new PNGs), non-zero = facing right (old bmp).
// frame: the animation counter (p / enemy[j].p) - wrapped to
// whichever frame set's actual length internally.
// Animation playback speed as a fraction of the raw update-tick
// counter (p / enemy[j].p, which itself increments once per ~60Hz
// logic tick - see the "Logic-rate cap" comment above). The counter
// keeps counting every tick as before (nothing else that reads p/
// enemy[j].p is affected); only how fast drawCharacter() advances
// through the frame list changes. The walk-cycle now plays 1.3x
// slower than the original (unscaled) speed: ANIM_SPEED_NUM/DEN =
// 10/13, i.e. 1 / 1.3, so the frame only steps forward on 10 out of
// every 13 ticks on average. To tune further: NUM/DEN = 1/S gives an
// S-times slowdown.
#define ANIM_SPEED_NUM 10
#define ANIM_SPEED_DEN 13

/* ---------------------------------------------------------------- */
/* Enemy walk-cycle speed (NEW - fixes enemy animating too fast)     */
/* ---------------------------------------------------------------- */
// enemy[j].p (the walk-cycle frame counter for enemies, run through
// the same ANIM_SPEED_NUM/DEN scaling above via drawCharacter()) used
// to be incremented once every single update() tick (~60/sec) the
// whole time an enemy was walking (chasing or patrolling). The
// player's equivalent counter (p) only advances once per 'a'/'d'
// key-repeat event, which happens at a much lower real-world rate -
// so even though both go through the exact same drawCharacter()
// scaling, the enemy walk-cycle visibly cycled through its 13 frames
// far faster than the player's, making enemies look like they were
// animating in fast-forward.
// Fix: only let enemy[j].p actually advance once every
// ENEMY_ANIM_TICK_DIVISOR update() ticks, via advanceEnemyAnim()
// below (each enemy tracks its own animCounter so multiple enemies
// throttle independently and correctly regardless of how many are
// walking at once). This does NOT touch enemy[j].x/movement speed at
// all - enemies still walk at the same pixels-per-tick pace as
// before; only how fast their walk-cycle SPRITE advances is slowed
// down, to roughly match the player's pace. Raise/lower this divisor
// to make the enemy walk-cycle slower/faster still.
#define ENEMY_ANIM_TICK_DIVISOR 4

void advanceEnemyAnim(Enemy *e){
	e->animCounter++;
	if (e->animCounter >= ENEMY_ANIM_TICK_DIVISOR){
		e->animCounter = 0;
		e->p++;
	}
}

void drawCharacter(int centerX, int y, int k, int frame){
	int animFrame = (frame * ANIM_SPEED_NUM) / ANIM_SPEED_DEN;
	if (k == 0){
		int idx = animFrame % LEFT_FRAME_COUNT;
		if (!leftFrameLoaded[idx]){
			leftFrameImg[idx] = iLoadImage(leftFrameFiles[idx]);
			leftFrameLoaded[idx] = 1;
		}
		// The PNGs have a proper alpha channel (transparent outside the
		// character), but iShowImage() doesn't enable alpha blending on
		// its own - without this, the transparent pixels' underlying
		// RGB (black) gets painted as a solid black box instead of
		// letting the background show through. Turning blending on
		// just for this draw call fixes that; turning it back off
		// afterward keeps every other (fully opaque) iShowImage/iShowBMP
		// call in the file working exactly as before.
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		iShowImage(centerX - charImgSize / 2, y, charImgSize, charImgSize, leftFrameImg[idx]);
		glDisable(GL_BLEND);
	}
	else{
		int idx = animFrame % RIGHT_FRAME_COUNT;
		if (!rightFrameLoaded[idx]){
			rightFrameImg[idx] = iLoadImage(rightFrameFiles[idx]);
			rightFrameLoaded[idx] = 1;
		}
		// Same alpha-blend fix as the left side above.
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		iShowImage(centerX - charImgSize / 2, y, charImgSize, charImgSize, rightFrameImg[idx]);
		glDisable(GL_BLEND);
	}
}


/* ---------------------------------------------------------------- */
/* Enemy character art - completely separate from main player       */
/* Walk left : le1.png ... le10.png                                 */
/* Walk right: re1.png ... re10.png                                 */
/* Attack left : lea1.png ... lea3.png                              */
/* Attack right: rea1.png ... rea3.png                              */
/* ---------------------------------------------------------------- */
#define ENEMY_WALK_FRAME_COUNT 10
#define ENEMY_ATTACK_FRAME_COUNT 3

char *enemyLeftFiles[ENEMY_WALK_FRAME_COUNT] = {
	"Assets/Enemies/le1.png", "Assets/Enemies/le2.png", "Assets/Enemies/le3.png", "Assets/Enemies/le4.png", "Assets/Enemies/le5.png",
	"Assets/Enemies/le6.png", "Assets/Enemies/le7.png", "Assets/Enemies/le8.png", "Assets/Enemies/le9.png", "Assets/Enemies/le10.png"
};
char *enemyRightFiles[ENEMY_WALK_FRAME_COUNT] = {
	"Assets/Enemies/re1.png", "Assets/Enemies/re2.png", "Assets/Enemies/re3.png", "Assets/Enemies/re4.png", "Assets/Enemies/re5.png",
	"Assets/Enemies/re6.png", "Assets/Enemies/re7.png", "Assets/Enemies/re8.png", "Assets/Enemies/re9.png", "Assets/Enemies/re10.png"
};
char *enemyAttackLeftFiles[ENEMY_ATTACK_FRAME_COUNT] = {
	"Assets/Enemies/lea1.png", "Assets/Enemies/lea2.png", "Assets/Enemies/lea3.png"
};
char *enemyAttackRightFiles[ENEMY_ATTACK_FRAME_COUNT] = {
	"Assets/Enemies/rea1.png", "Assets/Enemies/rea2.png", "Assets/Enemies/rea3.png"
};

int enemyLeftImg[ENEMY_WALK_FRAME_COUNT];
int enemyRightImg[ENEMY_WALK_FRAME_COUNT];
int enemyAttackLeftImg[ENEMY_ATTACK_FRAME_COUNT];
int enemyAttackRightImg[ENEMY_ATTACK_FRAME_COUNT];

int enemyLeftLoaded[ENEMY_WALK_FRAME_COUNT] = { 0 };
int enemyRightLoaded[ENEMY_WALK_FRAME_COUNT] = { 0 };
int enemyAttackLeftLoaded[ENEMY_ATTACK_FRAME_COUNT] = { 0 };
int enemyAttackRightLoaded[ENEMY_ATTACK_FRAME_COUNT] = { 0 };

int enemyImgSize = ENEMY_IMG_SIZE;
int enemyAtkImgSize = 80;

void drawEnemy(int centerX, int y, int dir, int frame){
	int animFrame = (frame * ANIM_SPEED_NUM) / ANIM_SPEED_DEN;
	int idx = animFrame % ENEMY_WALK_FRAME_COUNT;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (dir == 0){
		if (!enemyLeftLoaded[idx]){
			enemyLeftImg[idx] = iLoadImage(enemyLeftFiles[idx]);
			enemyLeftLoaded[idx] = 1;
		}
		iShowImage(centerX - enemyImgSize / 2, y,
			enemyImgSize, enemyImgSize, enemyLeftImg[idx]);
	}
	else{
		if (!enemyRightLoaded[idx]){
			enemyRightImg[idx] = iLoadImage(enemyRightFiles[idx]);
			enemyRightLoaded[idx] = 1;
		}
		iShowImage(centerX - enemyImgSize / 2, y,
			enemyImgSize, enemyImgSize, enemyRightImg[idx]);
	}

	glDisable(GL_BLEND);
}

void drawEnemyAttack(int x, int y, int dir, int frame){
	if (frame < 0) frame = 0;
	if (frame >= ENEMY_ATTACK_FRAME_COUNT)
		frame = ENEMY_ATTACK_FRAME_COUNT - 1;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (dir == 0){
		if (!enemyAttackLeftLoaded[frame]){
			enemyAttackLeftImg[frame] =
				iLoadImage(enemyAttackLeftFiles[frame]);
			enemyAttackLeftLoaded[frame] = 1;
		}
		iShowImage(x, y, enemyAtkImgSize, enemyAtkImgSize,
			enemyAttackLeftImg[frame]);
	}
	else{
		if (!enemyAttackRightLoaded[frame]){
			enemyAttackRightImg[frame] =
				iLoadImage(enemyAttackRightFiles[frame]);
			enemyAttackRightLoaded[frame] = 1;
		}
		iShowImage(x, y, enemyAtkImgSize, enemyAtkImgSize,
			enemyAttackRightImg[frame]);
	}

	glDisable(GL_BLEND);
}

/* ---------------------------------------------------------------- */
/* Idle sprite art: prince/l0.png (facing left) and prince/r0.png    */
/* (facing right) - single, non-animated "standing still" frames,    */
/* distinct from the 13-frame walk cycles above. Drawn with the       */
/* exact same centering/size convention (charImgSize, centered on     */
/* centerX) as drawCharacter(), so swapping between idle and walking  */
/* never causes the sprite to visibly shift or resize. Same lazy      */
/* iLoadImage() + alpha-blend approach as every other PNG asset here, */
/* since these have transparent backgrounds too.                      */
/* ---------------------------------------------------------------- */
int idleLImage, idleRImage;
int idleLLoaded = 0, idleRLoaded = 0;

void drawIdle(int centerX, int y, int k){
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	if (k == 0){
		if (!idleLLoaded){
			idleLImage = iLoadImage("Assets/Player/l0.png");
			idleLLoaded = 1;
		}
		iShowImage(centerX - charImgSize / 2, y, charImgSize, charImgSize, idleLImage);
	}
	else{
		if (!idleRLoaded){
			idleRImage = iLoadImage("Assets/Player/r0.png");
			idleRLoaded = 1;
		}
		iShowImage(centerX - charImgSize / 2, y, charImgSize, charImgSize, idleRImage);
	}
	glDisable(GL_BLEND);
}

/* ---------------------------------------------------------------- */
/* Health hearts (Levels 2 and 3).                                    */
/*                                                                    */
/* heart.png, 32x32, drawn with a slow bob and a soft warm glow behind */
/* it. The bob is purely cosmetic - the grab test in updateHearts()    */
/* uses the heart's resting y, so what you can reach never changes     */
/* with the animation.                                                 */
/* ---------------------------------------------------------------- */
int heartImage;
int heartImgLoaded = 0;

void drawHearts(){
	if (gameState != STATE_LEVEL2 && gameState != STATE_LEVEL3) return;

	if (!heartImgLoaded){
		heartImage = iLoadImage("Assets/Objects/heart.png");
		heartImgLoaded = 1;
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	for (int i = 0; i < heartNo; i++){
		if (!heart[i].active) continue;

		/* Two out-of-step sines so neighbouring hearts never bob in
		 * lockstep with each other. */
		double ph = heartBobTick * 0.055 + i * 1.7;
		int bob = (int)(4.0 * sin(ph));
		int sx = heart[i].x + cX;
		int sy = heart[i].y + bob;

		/* Warm glow, so a heart reads against a dark cave wall. */
		glColor4f(1.0f, 0.42f, 0.36f, (float)(0.16 + 0.07 * sin(ph * 1.6)));
		iFilledCircle(sx, sy, HEART_IMG_SIZE * 0.78);

		iShowImage(sx - HEART_IMG_SIZE / 2, sy - HEART_IMG_SIZE / 2,
			HEART_IMG_SIZE, HEART_IMG_SIZE, heartImage);
	}
	glDisable(GL_BLEND);
	// Put the draw colour back to plain white. This runs mid-scene, just
	// before the player, so leaving the glow's pink in place would tint
	// anything drawn after it that does not set its own colour first.
	iSetcolor(255, 255, 255);
}

/* ---------------------------------------------------------------- */
/* Swimming pose (Level 3's water).                                   */
/*                                                                    */
/* swim_l1..l3 / swim_r1..r3, 128x128. Like the drowning frames these */
/* are built FROM prince/l0.png rather than drawn by hand: the sprite  */
/* is split into body and arms, the arms are rotated about their own   */
/* shoulders to reach forward, and the whole figure is laid over to    */
/* horizontal. So it is his own artwork - hair, face, sash, trousers,  */
/* boots - swimming, rather than the upright standing pose sliding     */
/* through the water, which is what used to be drawn here.            */
/*                                                                    */
/* Three frames cycle as a stroke: reach, pull, recover.               */
/* ---------------------------------------------------------------- */
#define SWIM_FRAME_COUNT 3
int swimImage[2][SWIM_FRAME_COUNT];
int swimLoaded = 0;
int swimImgSize = 128;

void loadSwimImages(){
	if (swimLoaded) return;
	swimLoaded = 1;
	swimImage[0][0] = iLoadImage("Assets/Player/swim_l1.png");
	swimImage[0][1] = iLoadImage("Assets/Player/swim_l2.png");
	swimImage[0][2] = iLoadImage("Assets/Player/swim_l3.png");
	swimImage[1][0] = iLoadImage("Assets/Player/swim_r1.png");
	swimImage[1][1] = iLoadImage("Assets/Player/swim_r2.png");
	swimImage[1][2] = iLoadImage("Assets/Player/swim_r3.png");
}

/* centerX / feetY use the same convention as drawIdle()/drawCharacter():
 * centred horizontally on the player, with feetY where his feet would be
 * if he were standing. The horizontal body is nudged up from there so it
 * sits around his middle rather than below him. */
void drawSwim(int centerX, int feetY, int k, int frame){
	loadSwimImages();
	int set = (k == 0) ? 0 : 1;
	int f = frame % SWIM_FRAME_COUNT;
	if (f < 0) f = 0;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	iShowImage(centerX - swimImgSize / 2, feetY - 26, swimImgSize, swimImgSize, swimImage[set][f]);
	glDisable(GL_BLEND);
}

/* ---------------------------------------------------------------- */
/* Crouch pose (Level 2's 'Q' duck).                                  */
/*                                                                    */
/* CHANGED: this used to draw the ordinary standing idle sprite at    */
/* LEVEL2_DUCK_HEIGHT instead of charImgSize - which did not read as  */
/* ducking at all, just as the Prince suddenly becoming a short man.  */
/* He is now down on one knee: crawl_l.png / crawl_r.png.             */
/*                                                                    */
/* Built FROM prince/l0.png and prince/r0.png, so it is his own        */
/* artwork: hair, face, vest, sash, trousers, boots.                   */
/*                                                                    */
/* HOW, and why not the obvious ways. Posing this from cut-up parts    */
/* does not work: seen from the side his two legs overlap, so the      */
/* artwork holds no separate far leg to pose, and cutting one out of   */
/* the other leaves a hole at the waist and slabs where thighs should  */
/* be. Rotating a whole frame only tips him over. What bending a knee  */
/* really does to a side-on silhouette is SHORTEN THE LEGS - so that   */
/* is what is done: head, torso and arms are copied untouched, pixel   */
/* for pixel, and only the leg band is folded up beneath them. The     */
/* back leg folds harder than the front, which puts its boot up and    */
/* behind while the front boot stays flat on the ground, and that is   */
/* what reads as one knee down rather than a two-footed squat.         */
/* Nothing above the waist is resampled, which is what keeps him       */
/* recognisably himself.                                               */
/*                                                                    */
/* The figure occupies exactly the BOTTOM LEVEL2_DUCK_HEIGHT rows of  */
/* its 128x128 tile, and iShowImage() takes y as the quad's bottom -  */
/* so drawing the tile at his feet puts him flat on the platform,     */
/* filling precisely the crouched silhouette the bat's strike test    */
/* measures against (see updateLevel2Bats() in controls.h). What the  */
/* bat has to clear is exactly what is on screen.                     */
/* ---------------------------------------------------------------- */
#define CRAWL_IMG_SIZE 128
int crawlImage[2];
int crawlLoaded = 0;

// Same shape as loadBatImages()/loadFishImages()/loadSwimImages(): the
// flag is set BEFORE the loads, and the loading is its own function.
void loadCrawlImages(){
	if (crawlLoaded) return;
	crawlLoaded = 1;
	crawlImage[0] = iLoadImage("Assets/Player/crawl_l.png");
	crawlImage[1] = iLoadImage("Assets/Player/crawl_r.png");
}

void drawDuck(int centerX, int y, int k){
	loadCrawlImages();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	iShowImage(centerX - CRAWL_IMG_SIZE / 2, y,
		CRAWL_IMG_SIZE, CRAWL_IMG_SIZE, crawlImage[(k == 0) ? 0 : 1]);
	glDisable(GL_BLEND);
}

/* ---------------------------------------------------------------- */
/* Level 2's cave bats.                                               */
/*                                                                    */
/* Six 64x64 frames - bat_l1..l3 (flying left) and bat_r1..r3 (flying  */
/* right) - loaded lazily and alpha-blended, exactly like every other  */
/* PNG in this file. bat[].y is the CENTRE of the sprite (that is what */
/* the strike test in controls.h measures from), so the draw offsets   */
/* by half the sprite in both axes.                                    */
/* ---------------------------------------------------------------- */
int batImage[2][LEVEL2_BAT_FRAMES];
int batLoaded = 0;

void loadBatImages(){
	if (batLoaded) return;
	batLoaded = 1;
	batImage[0][0] = iLoadImage("Assets/Enemies/Bats/bat_l1.png");
	batImage[0][1] = iLoadImage("Assets/Enemies/Bats/bat_l2.png");
	batImage[0][2] = iLoadImage("Assets/Enemies/Bats/bat_l3.png");
	batImage[1][0] = iLoadImage("Assets/Enemies/Bats/bat_r1.png");
	batImage[1][1] = iLoadImage("Assets/Enemies/Bats/bat_r2.png");
	batImage[1][2] = iLoadImage("Assets/Enemies/Bats/bat_r3.png");
}

void drawLevel2Bats(){
	if (gameState != STATE_LEVEL2) return;

	loadBatImages();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	for (int i = 0; i < LEVEL2_BAT_MAX; i++){
		if (!level2Bat[i].active) continue;

		int set = (level2Bat[i].dir > 0) ? 1 : 0;
		int frame = level2Bat[i].frame % LEVEL2_BAT_FRAMES;
		iShowImage(level2Bat[i].x - LEVEL2_BAT_IMG_SIZE / 2 + cX,
			level2Bat[i].y - LEVEL2_BAT_IMG_SIZE / 2,
			LEVEL2_BAT_IMG_SIZE, LEVEL2_BAT_IMG_SIZE,
			batImage[set][frame]);
	}
	glDisable(GL_BLEND);
}

/* ================================================================== */
/* LEVEL 3 - "THE FINAL ESCAPE" : DRAWING                              */
/*                                                                     */
/* Everything here is cosmetic. All of the actual state - which slab   */
/* is falling, how high the flood is, what the Guardian is doing - is  */
/* owned by updateLevel3() in controls.h; these functions only read    */
/* it. Every one of them returns immediately unless Level 3 is the     */
/* level being played, so Levels 1 and 2 draw exactly as they always   */
/* did.                                                                */
/*                                                                     */
/* The art is deliberately built out of what the game already has -    */
/* the same 64x64 palace brick tiles Level 1 uses (lc/rc/rcb/uw/wall), */
/* the same water.png/water_top.png the Level 2 pools are made of, and */
/* the same enemy sprite sheet for the Guardian - so the final level   */
/* still reads as the same 2D side-view Prince of Persia game. What    */
/* makes it feel like the end is the palace moving: the shake, the     */
/* dust, the debris, the rising water and the collapsing floor.        */
/* ================================================================== */

/* Deterministic pseudo-noise in [-range, range]. Used for shake, dust
 * and debris. No rand(), no seeding, and no state of its own, so a
 * RETRY always looks the same and there is nothing extra to reset. */
inline int level3Noise(int n, int range)
{
	if (range <= 0) return 0;
	unsigned int h = (unsigned int)n * 2654435761u;
	h ^= (h >> 13);
	h *= 2246822519u;
	h ^= (h >> 16);
	return (int)(h % (unsigned int)(range * 2 + 1)) - range;
}

/* ---------------------------------------------------------------- */
/* The flood, drawn BEHIND the level so platforms still standing above */
/* the waterline sit on top of it. Whatever ends up submerged is then  */
/* tinted by drawLevel3Effects(), which is what makes a drowned ledge   */
/* read as being under the flood rather than in front of it.           */
/*                                                                    */
/* CHANGED: this used to draw Level 2's poison_water tiles, because the */
/* flood used to be the same poison and it drowned him on contact.     */
/* Level 3's water is ordinary water now and he swims in it, so it has */
/* its own tiles - sea_water.png / sea_water_top.png, blue and lit,    */
/* NEW files so that Level 2's poison is completely untouched.         */
/* ---------------------------------------------------------------- */
int seaImage, seaTopImage;
int seaLoaded = 0, seaTopLoaded = 0;

void drawSea(int x, int y){
	if (!seaLoaded){ seaImage = iLoadImage("Assets/Water/sea_water.png"); seaLoaded = 1; }
	iShowImage(x, y, wallTileSize, wallTileSize, seaImage);
}
void drawSeaTop(int x, int y){
	if (!seaTopLoaded){ seaTopImage = iLoadImage("Assets/Water/sea_water_top.png"); seaTopLoaded = 1; }
	iShowImage(x, y, wallTileSize, wallTileSize, seaTopImage);
}

void drawLevel3WaterBody()
{
	if (gameState != STATE_LEVEL3) return;

	int top = level3WaterLevel - 64;   // the tile whose TOP edge is the waterline
	if (top < -64) return;

	// Keep the tiles locked to the world grid (not the screen) so the
	// water does not visibly slide sideways as the camera scrolls.
	int off = ((cX % 64) + 64) % 64;

	for (int sx = off - 64; sx < 1350 + 64; sx += 64){
		drawSeaTop(sx, top);
		for (int gy = top - 64; gy > -64; gy -= 64)
			drawSea(sx, gy);
	}
}

/* ---------------------------------------------------------------- */
/* The piranhas. Drawn after the water body and before the player, so */
/* they swim behind him rather than blotting him out.                 */
/* ---------------------------------------------------------------- */
int fishImage[2][LEVEL3_FISH_FRAMES];
int fishLoaded = 0;

void loadFishImages(){
	if (fishLoaded) return;
	fishLoaded = 1;
	fishImage[0][0] = iLoadImage("Assets/Enemies/Fish/fish_l1.png");
	fishImage[0][1] = iLoadImage("Assets/Enemies/Fish/fish_l2.png");
	fishImage[0][2] = iLoadImage("Assets/Enemies/Fish/fish_l3.png");
	fishImage[1][0] = iLoadImage("Assets/Enemies/Fish/fish_r1.png");
	fishImage[1][1] = iLoadImage("Assets/Enemies/Fish/fish_r2.png");
	fishImage[1][2] = iLoadImage("Assets/Enemies/Fish/fish_r3.png");
}

void drawLevel3Fish()
{
	if (gameState != STATE_LEVEL3) return;

	loadFishImages();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	for (int i = 0; i < level3FishNo; i++){
		if (!level3Fish[i].active) continue;

		int sx = level3Fish[i].x + cX;
		if (sx < -80 || sx > 1430) continue;        // off screen - skip the work
		int sy = level3Fish[i].y;

		/* A fish with its teeth in him thrashes, and gets a red flash
		 * behind it, so a bite is obvious rather than a silent drain. */
		int jitter = 0;
		if (level3Fish[i].biting > 0){
			jitter = ((level3Fish[i].biting % 4) < 2) ? 3 : -3;
			// Deliberately faint: up to five of these overlap when the
			// shoal has him, and at any real strength they stack into one
			// dark red blob that swallows him and the fish both.
			glColor4f(0.88f, 0.14f, 0.11f, 0.15f);
			iFilledCircle(sx, sy, LEVEL3_FISH_DRAW * 0.62);
		}

		int set = (level3Fish[i].dir > 0) ? 1 : 0;
		int frame = level3Fish[i].frame % LEVEL3_FISH_FRAMES;
		iShowImage(sx - LEVEL3_FISH_DRAW / 2 + jitter, sy - LEVEL3_FISH_DRAW / 2,
			LEVEL3_FISH_DRAW, LEVEL3_FISH_DRAW, fishImage[set][frame]);
	}
	glDisable(GL_BLEND);
	iSetcolor(255, 255, 255);
}

/* ---------------------------------------------------------------- */
/* Level 3's underwater cast.                                         */
/*                                                                    */
/* Deliberately a SEPARATE function from Level 2's drawWaterPool():   */
/* that one is tuned for murky green poison and Level 2 still uses it */
/* exactly as it is. This is clear blue water - it darkens with depth */
/* rather than going opaque, because the player has to be able to see */
/* himself and the piranhas while he is down there.                    */
/* ---------------------------------------------------------------- */
void drawLevel3SeaTint(int left, int right, int surfaceY)
{
	if (surfaceY <= 0) return;

	int x0 = left + cX, x1 = right + cX;
	if (x0 < 0) x0 = 0;
	if (x1 > 1350) x1 = 1350;
	if (x1 <= x0) return;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/* Depth: a few bands, each a little deeper blue than the last. Kept
	 * light at the top so the swim stays readable. */
	int bands = 7;
	for (int b = 0; b < bands; b++){
		int yTop = surfaceY - (surfaceY * b) / bands;
		int yBot = surfaceY - (surfaceY * (b + 1)) / bands;
		float k = (float)b / (float)(bands - 1);
		glColor4f(0.04f, 0.20f, 0.38f, 0.10f + 0.24f * k);
		iFilledRectangle(x0, yBot, x1 - x0, yTop - yBot);
	}

	/* Shafts of light coming down through the surface. */
	for (int i = 0; i < 9; i++){
		int lx = ((i * 271 + level3Timer / 3) % 1500) - 75;
		if (lx < x0 - 60 || lx > x1 + 60) continue;
		glColor4f(0.72f, 0.90f, 1.0f, 0.045f);
		iFilledRectangle(lx, surfaceY - 260, 26 + (i % 3) * 10, 260);
	}

	/* The surface itself, bright and moving. */
	glColor4f(0.80f, 0.94f, 1.0f, 0.34f);
	iFilledRectangle(x0, surfaceY - 3, x1 - x0, 5);
	for (int i = 0; i < 26; i++){
		int wx = x0 + ((i * 137 + level3Timer * 2) % (x1 - x0));
		glColor4f(1.0f, 1.0f, 1.0f, 0.22f);
		iFilledRectangle(wx, surfaceY - 1, 12 + (i % 4) * 7, 3);
	}

	/* Bubbles drifting up. */
	for (int i = 0; i < 16; i++){
		int bx = x0 + ((i * 313) % (x1 - x0));
		int span = 180 + (i * 37) % 200;
		int rise = (level3Timer * 2 + i * 53) % span;
		int by = rise;
		if (by > surfaceY) continue;
		glColor4f(0.86f, 0.96f, 1.0f, (float)(0.34 * (1.0 - (double)rise / span)));
		iFilledCircle(bx, by, 2.0 + (i % 3));
	}

	glDisable(GL_BLEND);
	iSetcolor(255, 255, 255);
}

/* ---------------------------------------------------------------- */
/* One collapsing slab.                                               */
/*                                                                    */
/* SOLID    - drawn exactly like an elevator/bridge tile, so it is     */
/*            indistinguishable from safe ground until it is stepped   */
/*            on (that is the whole point of the mechanic).            */
/* SHAKING  - jitters harder and harder as its timer runs down, with   */
/*            widening cracks and dust pouring off the underside: an   */
/*            unmistakable "get off me" warning.                       */
/* FALLING  - the collision is ALREADY gone at this point; what is     */
/*            drawn is just debris tumbling away.                      */
/* GONE     - nothing at all.                                          */
/* ---------------------------------------------------------------- */
void drawLevel3CollapseSlab(CollapsePlatform *c)
{
	if (!c->enabled) return;
	if (c->state == LEVEL3_CP_GONE) return;

	int dx = 0, dy = 0;
	int warn = 0;   // 0..100, how far through the warning it is

	if (c->state == LEVEL3_CP_SHAKING){
		int span = (c->warnTicks > 0) ? c->warnTicks : 1;
		warn = ((span - c->timer) * 100) / span;
		int amp = 1 + (warn * 4) / 100;                   // 1px -> 5px
		dx = level3Noise(level3Timer * 3 + c->shakeSeed, amp);
		dy = level3Noise(level3Timer * 3 + c->shakeSeed + 77, amp / 2);
	}
	else if (c->state == LEVEL3_CP_FALLING){
		dy = -c->fallY;
		dx = level3Noise(c->shakeSeed + c->fallY, 3);
	}

	int sx = c->sPos + dx + cX;
	int sy = c->h - 55 + dy;

	// Cull anything well off screen - the level is long.
	if (sx < -260 || sx > 1350 + 130) return;

	// The slab itself: the same two corner tiles every 128px platform in
	// the game is built from, so it matches the palace exactly.
	drawLC(sx, sy);
	drawRCB(sx + 64, sy);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (c->state == LEVEL3_CP_SHAKING){
		// Cracks running across the slab's surface, opening up as the
		// warning runs out.
		float a = (float)(0.25 + 0.65 * (warn / 100.0));
		glColor4f(0.10f, 0.05f, 0.03f, a);
		int surf = c->h + dy;
		for (int i = 0; i < 4; i++){
			int x0 = sx + 12 + i * 30;
			int jag = 4 + (warn * 10) / 100;
			iLine(x0, surf, x0 + 8, surf - jag);
			iLine(x0 + 8, surf - jag, x0 + 16, surf - 2);
		}

		// A hot warning rim that pulses faster the closer it is to going.
		int pulse = (level3Timer / (2 + (100 - warn) / 20)) % 2;
		if (pulse){
			glColor4f(1.0f, 0.55f, 0.15f, (float)(0.20 + 0.45 * (warn / 100.0)));
			iFilledRectangle(sx, surf - 3, 128, 5);
		}

		// Dust shaking loose from the underside.
		for (int i = 0; i < 6; i++){
			int n = level3Timer + c->shakeSeed + i * 31;
			int px = sx + 10 + ((n * 17) % 110);
			int py = surf - 14 - ((n * 7) % 30);
			glColor4f(0.68f, 0.60f, 0.48f, (float)(0.10 + 0.30 * (warn / 100.0)));
			iFilledCircle(px, py, 2.0 + (i % 3));
		}
	}
	else if (c->state == LEVEL3_CP_FALLING){
		// Chunks of the slab breaking away and tumbling into the water.
		double fade = 1.0 - (double)c->fallY / (double)(LEVEL3_CP_FALL_TICKS * 6 + 1);
		if (fade < 0.0) fade = 0.0;
		for (int i = 0; i < 7; i++){
			int n = c->shakeSeed + i * 53;
			int px = sx + 8 + ((n * 23) % 112);
			int py = c->h - 62 - c->fallY - ((n * 11) % 40);
			glColor4f(0.42f, 0.36f, 0.29f, (float)(0.85 * fade));
			iFilledRectangle(px, py, 8 + (i % 3) * 4, 7 + (i % 2) * 4);
		}
		// The dust cloud left hanging where it used to be.
		for (int i = 0; i < 8; i++){
			int n = c->shakeSeed + i * 19;
			int px = sx + 6 + ((n * 29) % 116);
			int py = c->h - 48 + ((n * 13) % 26);
			glColor4f(0.70f, 0.63f, 0.52f, (float)(0.35 * fade));
			iFilledCircle(px, py, 6.0 + (i % 4) * 3.0);
		}
	}

	glDisable(GL_BLEND);
	iSetcolor(255, 255, 255);
}

void drawLevel3CollapsePlatforms()
{
	if (gameState != STATE_LEVEL3) return;
	for (int i = 0; i < level3CollapseNo; i++)
		drawLevel3CollapseSlab(&level3Collapse[i]);
}

/* ---------------------------------------------------------------- */
/* The two fixed set-pieces of the final level: the gate that seals    */
/* the escape route behind the Guardian's arena, and the palace door   */
/* it opens onto.                                                     */
/*                                                                    */
/* The gate is not just decoration: until the Guardian falls, the four */
/* platforms behind it have no entry in the collision map at all (see  */
/* level3OpenEscapeRoute() in controls.h), so the door genuinely       */
/* cannot be reached early.                                            */
/* ---------------------------------------------------------------- */
void drawLevel3Structures()
{
	if (gameState != STATE_LEVEL3) return;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/* ---- the sealed arena gate ---- */
	int gx = LEVEL3_GATE_X + cX;
	if (gx > -140 && gx < 1350 + 140){
		int base = 470;
		if (!level3EscapeMode){
			// Closed portcullis: a heavy stone frame with iron bars.
			glColor4f(0.16f, 0.13f, 0.10f, 0.96f);
			iFilledRectangle(gx - 10, base, 30, 168);
			glColor4f(0.30f, 0.26f, 0.20f, 0.92f);
			iFilledRectangle(gx - 6, base + 150, 22, 18);
			for (int i = 0; i < 5; i++){
				glColor4f(0.34f, 0.31f, 0.27f, 0.95f);
				iFilledRectangle(gx - 6 + i * 6, base, 3, 150);
			}
			// A faint red "this way is shut" glow.
			int pulse = (level3Timer / 24) % 2;
			glColor4f(0.85f, 0.15f, 0.10f, pulse ? 0.30f : 0.16f);
			iFilledRectangle(gx - 12, base, 34, 168);
		}
		else{
			// Open: the bars have been drawn up into the arch.
			glColor4f(0.16f, 0.13f, 0.10f, 0.90f);
			iFilledRectangle(gx - 10, base + 120, 30, 48);
			for (int i = 0; i < 5; i++){
				glColor4f(0.34f, 0.31f, 0.27f, 0.90f);
				iFilledRectangle(gx - 6 + i * 6, base + 122, 3, 44);
			}
		}
	}

	/* ---- the palace door at the end of the escape run ---- */
	if (level3EscapeMode){
		int dx = level3ExitX + cX;
		if (dx > -200 && dx < 1350 + 200){
			int base = 470;

			// Stone frame.
			glColor4f(0.24f, 0.20f, 0.15f, 1.0f);
			iFilledRectangle(dx - 46, base, 104, 150);
			glColor4f(0.34f, 0.29f, 0.22f, 1.0f);
			iFilledRectangle(dx - 52, base + 140, 116, 16);

			// The open doorway, with daylight behind it - the first
			// light the Prince has seen since Level 1.
			int glow = (level3Timer / 8) % 10;
			glColor4f(1.0f, 0.93f, 0.62f, 0.92f);
			iFilledRectangle(dx - 34, base, 80, 126);
			glColor4f(1.0f, 0.98f, 0.80f, (float)(0.35 + glow * 0.03));
			iFilledRectangle(dx - 24, base, 60, 112);

			// A soft halo spilling out of it.
			glColor4f(1.0f, 0.92f, 0.60f, 0.16f);
			iFilledCircle(dx + 6, base + 66, 92.0);

			iSetcolor(255, 255, 255);
			iText(dx - 40, base + 162, "ESCAPE", GLUT_BITMAP_9_BY_15);
		}
	}

	glDisable(GL_BLEND);
	iSetcolor(255, 255, 255);
}

/* ---------------------------------------------------------------- */
/* The final Guardian.                                                */
/*                                                                    */
/* Uses the EXISTING enemy sprite sheet (the le / re walk frames and   */
/* the lea / rea attack frames) rather than new art, drawn at          */
/* LEVEL3_BOSS_IMG_SIZE instead of the 64px ordinary guards use, with  */
/* a dark aura at its feet and a battle-damage flash so it still       */
/* clearly reads as something much bigger and more dangerous.          */
/* ---------------------------------------------------------------- */
void drawGuardianWalk(int centerX, int y, int dir, int frame)
{
	int animFrame = (frame * ANIM_SPEED_NUM) / ANIM_SPEED_DEN;
	int idx = animFrame % ENEMY_WALK_FRAME_COUNT;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (dir == 0){
		if (!enemyLeftLoaded[idx]){
			enemyLeftImg[idx] = iLoadImage(enemyLeftFiles[idx]);
			enemyLeftLoaded[idx] = 1;
		}
		iShowImage(centerX - LEVEL3_BOSS_IMG_SIZE / 2, y,
			LEVEL3_BOSS_IMG_SIZE, LEVEL3_BOSS_IMG_SIZE, enemyLeftImg[idx]);
	}
	else{
		if (!enemyRightLoaded[idx]){
			enemyRightImg[idx] = iLoadImage(enemyRightFiles[idx]);
			enemyRightLoaded[idx] = 1;
		}
		iShowImage(centerX - LEVEL3_BOSS_IMG_SIZE / 2, y,
			LEVEL3_BOSS_IMG_SIZE, LEVEL3_BOSS_IMG_SIZE, enemyRightImg[idx]);
	}

	glDisable(GL_BLEND);
}

void drawGuardianSwing(int x, int y, int dir, int frame)
{
	if (frame < 0) frame = 0;
	if (frame >= ENEMY_ATTACK_FRAME_COUNT) frame = ENEMY_ATTACK_FRAME_COUNT - 1;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (dir == 0){
		if (!enemyAttackLeftLoaded[frame]){
			enemyAttackLeftImg[frame] = iLoadImage(enemyAttackLeftFiles[frame]);
			enemyAttackLeftLoaded[frame] = 1;
		}
		iShowImage(x, y, LEVEL3_BOSS_ATK_IMG_SIZE, LEVEL3_BOSS_ATK_IMG_SIZE,
			enemyAttackLeftImg[frame]);
	}
	else{
		if (!enemyAttackRightLoaded[frame]){
			enemyAttackRightImg[frame] = iLoadImage(enemyAttackRightFiles[frame]);
			enemyAttackRightLoaded[frame] = 1;
		}
		iShowImage(x, y, LEVEL3_BOSS_ATK_IMG_SIZE, LEVEL3_BOSS_ATK_IMG_SIZE,
			enemyAttackRightImg[frame]);
	}

	glDisable(GL_BLEND);
}

void drawLevel3Guardian()
{
	if (gameState != STATE_LEVEL3) return;
	if (level3Boss.dead) return;

	int bx = level3Boss.x + cX;
	int by = level3Boss.y;
	if (bx < -200 || bx > 1350 + 200) return;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/* --- the shadow/aura it stands in --- */
	glColor4f(0.35f, 0.04f, 0.06f, 0.30f);
	iFilledEllipse(bx, by + 6, 58, 12);

	if (level3Boss.dying){
		/* --- death: it slumps, and dust swallows it --- */
		double t = 1.0 - (double)level3Boss.deathTimer / (double)LEVEL3_BOSS_DEATH_TICKS;
		int drop = (int)(t * 42);
		int sway = level3Noise(level3Boss.deathTimer, 4);

		drawGuardianWalk(bx + sway, by - drop, level3Boss.k, level3Boss.p);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		// The body fading out behind a growing cloud of dust.
		glColor4f(0.12f, 0.10f, 0.09f, (float)(0.75 * t));
		iFilledRectangle(bx - LEVEL3_BOSS_IMG_SIZE / 2, by - drop,
			LEVEL3_BOSS_IMG_SIZE, LEVEL3_BOSS_IMG_SIZE);
		for (int i = 0; i < 12; i++){
			int n = i * 37 + level3Boss.deathTimer / 4;
			int px = bx + level3Noise(n, 60);
			int py = by + 8 + (int)(t * ((n * 13) % 70));
			glColor4f(0.66f, 0.58f, 0.46f, (float)(0.45 * (1.0 - t * 0.6)));
			iFilledCircle(px, py, 8.0 + (i % 5) * 4.0 * t);
		}
		glDisable(GL_BLEND);
		iSetcolor(255, 255, 255);
		return;
	}

	glDisable(GL_BLEND);

	/* --- the Guardian itself --- */
	if (level3Boss.a == 1){
		int frame = ((LEVEL3_BOSS_ATTACK_TICKS - level3Boss.atkAnim) * ENEMY_ATTACK_FRAME_COUNT)
			/ LEVEL3_BOSS_ATTACK_TICKS;
		if (level3Boss.k == 0) drawGuardianSwing(bx - 68, by, 0, frame);
		else                   drawGuardianSwing(bx - 30, by, 1, frame);
	}
	else{
		drawGuardianWalk(bx, by, level3Boss.k, level3Boss.p);
	}

	/* --- hit reaction: a red flash over the body --- */
	// Kept inside the silhouette (an ellipse over its chest rather than
	// a rectangle over the whole 96px sprite box) so it reads as the
	// Guardian being hurt rather than as a red square blinking on.
	if (level3Boss.hitFlash > 0){
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		double f = (double)level3Boss.hitFlash / (double)LEVEL3_BOSS_HIT_FLASH;
		glColor4f(1.0f, 0.15f, 0.10f, (float)(0.38 * f));
		iFilledEllipse(bx, by + 46, 30, 34);
		glColor4f(1.0f, 0.65f, 0.25f, (float)(0.45 * f));
		iFilledEllipse(bx, by + 52, 14, 16);
		glDisable(GL_BLEND);
	}

	iSetcolor(255, 255, 255);
}

/* ---------------------------------------------------------------- */
/* Everything drawn ON TOP of the world: the underwater tint, the      */
/* waterline, airborne dust and falling masonry, and the Level 3 HUD   */
/* (Guardian health, stage banner, escape prompt).                     */
/*                                                                    */
/* The palace visibly degrades as the level goes on - NORMAL PALACE -> */
/* DAMAGED -> COLLAPSING -> FLOODED -> FINAL ESCAPE - purely by        */
/* layering more dust, more debris and a stronger warm/red cast over   */
/* the same art, so nothing about the game's look changes: it is the   */
/* same side-view palace, just falling down.                           */
/* ---------------------------------------------------------------- */
void drawLevel3Effects()
{
	if (gameState != STATE_LEVEL3) return;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/* ---- everything below the waterline reads as submerged ---- */
	// CHANGED: this used to call Level 2's drawWaterPool(), which is
	// tuned for opaque green poison. Level 3's water is clear blue and
	// the player now swims about in it, so it gets its own, lighter
	// treatment - see drawLevel3SeaTint() above. drawWaterPool() itself
	// is untouched and Level 2 still uses it exactly as before.
	if (level3WaterLevel > 0){
		drawLevel3SeaTint(-cX - 64, -cX + 1350 + 64, level3WaterLevel);
		// that leaves blending off - the haze, debris and colour cast
		// below all need it back on.
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	/* ---- dust haze: thicker with every stage ---- */
	float haze = 0.0f;
	if (level3Stage >= 2) haze = 0.05f;
	if (level3Stage >= 3) haze = 0.09f;
	if (level3Stage >= 4) haze = 0.13f;
	if (level3EscapeMode) haze = 0.20f;
	if (haze > 0.0f){
		glColor4f(0.62f, 0.44f, 0.26f, haze);
		iFilledRectangle(0, 0, 1350, 680);
	}

	/* ---- masonry falling through the frame ---- */
	int debris = level3ShakeLevel * 6;
	if (level3EscapeMode) debris = 34;
	for (int i = 0; i < debris; i++){
		int n = i * 71 + 13;
		int span = 700 + ((n * 7) % 380);
		int fall = (level3Timer * (3 + (n % 4)) + (n * 37)) % span;
		int px = ((n * 131) % 1350);
		int py = 700 - fall;
		if (py < -20) continue;
		glColor4f(0.46f, 0.39f, 0.31f, 0.75f);
		iFilledRectangle(px, py, 5 + (n % 9), 5 + (n % 7));
	}

	/* ---- a red cast over the whole frame during the escape ---- */
	if (level3EscapeMode){
		int pulse = (level3Timer / 10) % 12;
		glColor4f(0.70f, 0.08f, 0.05f, (float)(0.07 + pulse * 0.006));
		iFilledRectangle(0, 0, 1350, 680);
	}

	glDisable(GL_BLEND);

	/* ================= HUD ================= */

	/* ---- water warning / swim readout, beside the life bar ---- */
	// CHANGED: the water no longer drowns him, so the old "!! THE POISON
	// IS AT YOUR FEET - CLIMB !!" panic line would be a lie. Rising water
	// is now a warning, not a death sentence, and the important prompt is
	// the one that tells him how to get back out again.
	if (level3Swimming){
		if (level3ClimbX >= 0){
			int pulse = (level3Timer / 12) % 2;
			if (pulse) iSetcolor(255, 230, 120);
			else       iSetcolor(255, 255, 255);
			iText(430, 663, "PRESS  R  TO CLIMB OUT ONTO THE LEDGE", GLUT_BITMAP_9_BY_15);
			iSetcolor(255, 255, 255);
		}
		else{
			/* The life bar is draining the whole time he is under, so say
			 * so - otherwise a player watching it fall has no idea the
			 * water itself is the cause and no reason to hurry. */
			int pulse = (level3Timer / 14) % 2;
			if (pulse) iSetcolor(255, 170, 170);
			else       iSetcolor(190, 225, 255);
			iText(378, 663, "IN THE WATER  -  THE PIRANHAS ARE ON YOU  -  FIND A WALL AND PRESS R",
				GLUT_BITMAP_9_BY_15);
			iSetcolor(255, 255, 255);
		}
	}
	else if (playerBlockNo != -1){
		int clearance = playerY - level3WaterLevel;
		if (clearance < 90){
			int pulse = (level3Timer / 12) % 2;
			if (pulse) iSetcolor(255, 120, 60);
			else       iSetcolor(255, 255, 255);
			iText(455, 663, "THE WATER IS AT YOUR FEET  -  KEEP CLIMBING", GLUT_BITMAP_9_BY_15);
			iSetcolor(255, 255, 255);
		}
	}

	/* ---- bitten: a red pulse round the edge of the frame ---- */
	if (level3BiteFlash > 0){
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		float ba = 0.30f * level3BiteFlash / (float)LEVEL3_BITE_FLASH;
		glColor4f(0.80f, 0.06f, 0.05f, ba);
		iFilledRectangle(0, 0, 1350, 26);
		iFilledRectangle(0, 654, 1350, 26);
		iFilledRectangle(0, 0, 26, 680);
		iFilledRectangle(1324, 0, 26, 680);
		glDisable(GL_BLEND);
		iSetcolor(255, 255, 255);
	}

	/* ---- Guardian health bar, across the top of the screen ---- */
	if (level3BossActive && !level3Boss.dead && level3Boss.life > 0){
		int barX = 375, barY = 620, barW = 600, barH = 20;

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(0.0f, 0.0f, 0.0f, 0.55f);
		iFilledRectangle(barX - 6, barY - 22, barW + 12, barH + 30);

		int fill = (level3Boss.life * barW) / level3Boss.maxLife;
		if (fill < 0) fill = 0;
		// The bar deepens from gold to blood red as the phases advance.
		if (level3BossPhase >= 4)      glColor4f(0.95f, 0.10f, 0.08f, 0.95f);
		else if (level3BossPhase >= 3) glColor4f(0.95f, 0.35f, 0.08f, 0.95f);
		else if (level3BossPhase >= 2) glColor4f(0.95f, 0.60f, 0.10f, 0.95f);
		else                           glColor4f(0.90f, 0.80f, 0.25f, 0.95f);
		iFilledRectangle(barX, barY, fill, barH);
		glDisable(GL_BLEND);

		iSetcolor(255, 255, 255);
		iRectangle(barX - 1, barY - 1, barW + 2, barH + 2);
		iText(barX + 200, barY + 26, "THE PALACE GUARDIAN", GLUT_BITMAP_9_BY_15);

		char phaseText[32];
		phaseText[0] = 'P'; phaseText[1] = 'H'; phaseText[2] = 'A'; phaseText[3] = 'S';
		phaseText[4] = 'E'; phaseText[5] = ' ';
		phaseText[6] = (char)('0' + level3BossPhase);
		phaseText[7] = ' '; phaseText[8] = '/'; phaseText[9] = ' '; phaseText[10] = '4';
		phaseText[11] = '\0';
		iText(barX + barW - 66, barY + 26, phaseText, GLUT_BITMAP_8_BY_13);
	}

	/* ---- stage / event banner: REMOVED ----
	 *
	 * This used to put a black plate and a line of text across the upper
	 * middle of the screen whenever something happened - "STAGE 1 - THE
	 * FINAL ESCAPE", "STAGE 2 - THE PALACE BEGINS TO SHAKE", the boss
	 * PHASE lines, "RUN! THE PALACE IS COMING DOWN!" and so on. Nothing
	 * is drawn for any of them now.
	 *
	 * Only the drawing is gone. level3SetBanner() and the stage/phase
	 * logic behind it are untouched, so stages still advance, the flood
	 * still rises on schedule, the Guardian still changes phase and the
	 * escape still triggers - exactly as before. The text is simply
	 * never shown. That also means the banners can be put back by
	 * restoring this one block, with nothing else to undo.
	 */

	/* ---- escape prompt ---- */
	if (level3EscapeMode && !level3ExitReached){
		// Dark plate behind it - this sits over open palace background,
		// which is far too light for plain white/yellow text to read on.
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(0.0f, 0.0f, 0.0f, 0.58f);
		iFilledRectangle(440, 462, 470, 48);
		glDisable(GL_BLEND);

		int pulse = (level3Timer / 15) % 2;
		if (pulse) iSetcolor(255, 255, 0);
		else       iSetcolor(255, 255, 255);
		iText(470, 492, "RUN EAST - REACH THE ESCAPING DOOR!", GLUT_BITMAP_9_BY_15);

		int away = level3ExitX - playerX;
		if (away < 0) away = 0;
		char dist[32];
		int m = away / 10;                 // rough "metres" so it stays short
		int i = 0;
		if (m >= 1000){ dist[i++] = (char)('0' + (m / 1000) % 10); }
		if (m >= 100) { dist[i++] = (char)('0' + (m / 100) % 10); }
		if (m >= 10)  { dist[i++] = (char)('0' + (m / 10) % 10); }
		dist[i++] = (char)('0' + m % 10);
		dist[i++] = 'm'; dist[i] = '\0';
		iSetcolor(255, 255, 255);
		iText(650, 470, dist, GLUT_BITMAP_9_BY_15);
	}

	iSetcolor(255, 255, 255);
}

/* ---------------------------------------------------------------- */
/* Logic-rate cap                                                     */
/* ---------------------------------------------------------------- */
// Every bit of movement in this file (enemy[j].x += 2, playerX -= 2,
// walk-cycle frame counters, etc.) happens once per update() CALL, not
// once per real second - so how fast things move depends on how many
// times per second update() runs, which in turn used to depend on how
// expensive each draw was. Swapping several iShowBMP() calls (which
// re-read the file from disk every frame) for cached iLoadImage()
// textures made drawing cheaper, so the loop started calling update()
// MORE often than before - which is what made everything look faster,
// even though no per-call movement amount changed.
// NOTE: this only throttles update() (the game logic), never iDraw()
// itself/iClear() - skipping the actual drawing on some callbacks
// caused a stale backbuffer to get presented on those frames, which is
// what made the screen look blurry/flickery. Drawing can safely run as
// often as the system wants; only the movement math needs pinning.

/* ==================================================================== */
/* SHARED TEXT/PLATE HELPERS FOR THE TIMER AND RECORDS UI                */
/*                                                                       */
/* glutBitmapLength() gives the exact rendered pixel width of a string   */
/* in a given GLUT bitmap font, which is what lets the plates below be   */
/* sized from their text instead of from a guessed constant. It is the   */
/* same measurement the checkpoint banner now uses.                      */
/* ==================================================================== */
inline int textWidthPx(void *font, const char *s)
{
	return glutBitmapLength(font, (const unsigned char *)s);
}

inline void iTextCentered(double centreX, double y, const char *s, void *font)
{
	iText(centreX - textWidthPx(font, s) / 2.0, y, (char *)s, font);
}

inline void iTextRight(double rightX, double y, const char *s, void *font)
{
	iText(rightX - textWidthPx(font, s), y, (char *)s, font);
}

/* ==================================================================== */
/* THE LEVEL PROGRESS BAR                                                */
/*                                                                       */
/* A framed bar across the top-centre of the gameplay screen. The FILL   */
/* shows how far through the level the Prince physically is; the digits  */
/* inside it show the elapsed time. The two are independent:             */
/*                                                                       */
/*   fill   <- getLevelProgress()      (world x, from records.h)         */
/*   digits <- getElapsedLevelTime()   (the high-resolution clock)       */
/*                                                                       */
/* So standing still leaves the bar exactly where it is while the digits */
/* keep climbing, and running east fills the bar regardless of how long  */
/* it took. Neither feeds the other, and neither affects the recorded    */
/* completion time.                                                      */
/*                                                                       */
/* Position: centred on x=675 (half of the 1350px window), sitting in    */
/* the empty strip along the top. It clears the life bar (top-LEFT,      */
/* x 20..145) and stops above the checkpoint/bat/heart banners, which    */
/* all end at y=634.                                                     */
/* ==================================================================== */
#define TIMER_BAR_W      420
#define TIMER_BAR_H      30
#define TIMER_BAR_X1     ((1350 - TIMER_BAR_W) / 2)
#define TIMER_BAR_Y1     638

inline void drawLevelTimerBar()
{
	long elapsed = getElapsedLevelTime();
	double frac = getLevelProgress();       // physical progress, NOT time
	int x1 = TIMER_BAR_X1, y1 = TIMER_BAR_Y1;
	int x2 = x1 + TIMER_BAR_W, y2 = y1 + TIMER_BAR_H;
	int innerW = TIMER_BAR_W - 6;
	int fillW;
	char buf[16];

	fillW = (int)(innerW * frac + 0.5);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/* drop shadow, so the frame reads against any backdrop */
	glColor4f(0.0f, 0.0f, 0.0f, 0.45f);
	iFilledRectangle(x1 - 3, y1 - 3, TIMER_BAR_W + 6, TIMER_BAR_H + 6);

	/* unfilled track */
	glColor4f(0.05f, 0.07f, 0.14f, 0.88f);
	iFilledRectangle(x1 + 3, y1 + 3, innerW, TIMER_BAR_H - 6);

	/* the fill itself, in two bands so it has a little depth */
	if (fillW > 0){
		int half = (TIMER_BAR_H - 6) / 2;
		glColor4f(0.74f, 0.55f, 0.20f, 0.95f);
		iFilledRectangle(x1 + 3, y1 + 3, fillW, half);
		glColor4f(0.95f, 0.78f, 0.36f, 0.95f);
		iFilledRectangle(x1 + 3, y1 + 3 + half, fillW, (TIMER_BAR_H - 6) - half);
	}

	/* gold frame, two pixels thick */
	glColor4f(0.80f, 0.66f, 0.34f, 0.95f);
	iRectangle(x1, y1, TIMER_BAR_W, TIMER_BAR_H);
	iRectangle(x1 + 1, y1 + 1, TIMER_BAR_W - 2, TIMER_BAR_H - 2);

	glDisable(GL_BLEND);

	/* The real time, centred in the bar. It is outlined on all four
	 * sides rather than given a single drop shadow, because the fill
	 * creeps underneath it part-way through the level - so the digits
	 * have to stay legible against the dark track AND against the bright
	 * amber fill. */
	formatLevelTime(elapsed, buf);
	{
		double tx = (x1 + x2) / 2.0, ty = y1 + 11;
		int d;
		static const int ox[4] = { -1, 1, 0, 0 };
		static const int oy[4] = { 0, 0, -1, 1 };

		glColor3f(0.0f, 0.0f, 0.0f);
		for (d = 0; d < 4; d++)
			iTextCentered(tx + ox[d], ty + oy[d], buf, GLUT_BITMAP_9_BY_15);

		glColor3f(1.0f, 0.99f, 0.93f);
		iTextCentered(tx, ty, buf, GLUT_BITMAP_9_BY_15);
	}

	glColor3f(1.0f, 1.0f, 1.0f);            // leave the colour as found
}

/* ==================================================================== */
/* THE RECORDS SCREEN                                                    */
/*                                                                       */
/* Drawn over the Instructions artwork. Two plates in that page's own    */
/* style sit on it: one over the page title, carrying "LEVEL RECORDS",   */
/* and one over the grid of key cards, carrying the table. The rest of   */
/* the picture - the ornamental frame, the Prince, the palace, the       */
/* brazier and the BACK plate - is left showing, which is what makes     */
/* this read as another page of the same book.                           */
/*                                                                       */
/* The times come from the table records.h loaded out of                 */
/* best_records.txt, so the screen always shows what is really saved.    */
/* A level that has never been finished shows "No Record".               */
/* ==================================================================== */

/* A panel in the Instructions page's own style: near-opaque navy with a
 * thin gold edge. */
inline void drawRecordsPanel(int x1, int y1, int x2, int y2)
{
	// Fully opaque, with blending off for the fill. The page title and
	// the key cards underneath are bright gold on near-black, and even a
	// few per cent of them coming through reads as a legible ghost - so
	// the panel covers rather than tints.
	glDisable(GL_BLEND);
	glColor4f(0.055f, 0.085f, 0.180f, 1.0f);
	iFilledRectangle(x1, y1, x2 - x1, y2 - y1);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(0.72f, 0.60f, 0.32f, 0.95f);
	iRectangle(x1, y1, x2 - x1, y2 - y1);
	iRectangle(x1 + 3, y1 + 3, (x2 - x1) - 6, (y2 - y1) - 6);
	glDisable(GL_BLEND);
}

inline void drawRecordsRows()
{
	/* the page title band, and the block of key cards below it */
	/* Sized to cover the page's own title band and its whole grid of key
	 * cards, including the rounded corners that stick out furthest. */
	const int titleX1 = 358, titleY1 = 584, titleX2 = 992, titleY2 = 666;
	const int tableX1 = 286, tableY1 = 102, tableX2 = 1066, tableY2 = 580;
	const int labelX = 400, valueX = 950;
	int i, rowY;
	char label[32], value[24];

	drawRecordsPanel(titleX1, titleY1, titleX2, titleY2);
	glColor3f(0.0f, 0.0f, 0.0f);
	iTextCentered((titleX1 + titleX2) / 2.0 + 1, titleY1 + 29,
	              "LEVEL RECORDS", GLUT_BITMAP_TIMES_ROMAN_24);
	glColor3f(0.95f, 0.82f, 0.46f);
	iTextCentered((titleX1 + titleX2) / 2.0, titleY1 + 30,
	              "LEVEL RECORDS", GLUT_BITMAP_TIMES_ROMAN_24);

	drawRecordsPanel(tableX1, tableY1, tableX2, tableY2);

	/* The rows are centred in the panel, so the table stays balanced
	 * however many levels the game grows to. */
	{
		const int rowStep = 80;
		int block = (RECORD_LEVELS_SHOWN - 1) * rowStep;
		rowY = (tableY1 + tableY2) / 2 + block / 2;
	}

	for (i = 1; i <= RECORD_LEVELS_SHOWN; i++){
		long best = getBestRecord(i);

		sprintf(label, "Level %d", i);
		if (best < 0) strcpy(value, "No Record");
		else          formatLevelTime(best, value);

		/* a faint rule under each row, like the cards' dividers */
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(0.72f, 0.60f, 0.32f, 0.28f);
		iLine(labelX - 20, rowY - 14, valueX + 20, rowY - 14);
		glDisable(GL_BLEND);

		glColor3f(0.0f, 0.0f, 0.0f);
		iText(labelX + 1, rowY - 1, label, GLUT_BITMAP_HELVETICA_18);
		glColor3f(0.93f, 0.90f, 0.82f);
		iText(labelX, rowY, label, GLUT_BITMAP_HELVETICA_18);

		glColor3f(0.0f, 0.0f, 0.0f);
		iTextRight(valueX + 1, rowY - 1, value, GLUT_BITMAP_HELVETICA_18);
		if (best < 0) glColor3f(0.58f, 0.58f, 0.62f);   // never completed
		else          glColor3f(0.96f, 0.82f, 0.46f);
		iTextRight(valueX, rowY, value, GLUT_BITMAP_HELVETICA_18);

		rowY -= 80;
	}
	glColor3f(1.0f, 1.0f, 1.0f);
}

/* ==================================================================== */
/* THE TIME PANEL ON THE LEVEL COMPLETE SCREEN                           */
/*                                                                       */
/* Drawn over bg30.png between the "LEVEL COMPLETED" title and the NEXT  */
/* LEVEL button, which is the one clear band on that artwork. Values     */
/* come from lastRun* (set by finishLevelRun() on the tick the level was */
/* won), so nothing is recalculated here and the clock is already stopped*/
/* ==================================================================== */
inline void drawLevelCompleteTimes()
{
	int px1 = 430, py1 = 286, pw = 490, ph = 88;
	double cx = px1 + pw / 2.0;
	char buf[16];

	// Opaque, for the same reason as the records panels: the artwork's
	// laurel and stars sit directly behind this plate and ghost through
	// at anything less.
	glDisable(GL_BLEND);
	glColor4f(0.03f, 0.05f, 0.11f, 1.0f);
	iFilledRectangle(px1, py1, pw, ph);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(0.78f, 0.64f, 0.32f, 0.95f);
	iRectangle(px1, py1, pw, ph);
	iRectangle(px1 + 2, py1 + 2, pw - 4, ph - 4);
	glDisable(GL_BLEND);

	glColor3f(0.86f, 0.74f, 0.42f);
	iTextCentered(cx, py1 + 62, "YOUR TIME", GLUT_BITMAP_9_BY_15);

	formatLevelTime(lastRunTime, buf);
	glColor3f(1.0f, 0.98f, 0.90f);
	iTextCentered(cx, py1 + 34, buf, GLUT_BITMAP_TIMES_ROMAN_24);

	if (lastRunNewRecord){
		glColor3f(0.99f, 0.84f, 0.36f);
		iTextCentered(cx, py1 + 12, "NEW RECORD!", GLUT_BITMAP_9_BY_15);
	}
	else{
		char line[48];
		char bestBuf[16];
		formatLevelTime(lastRunBest, bestBuf);
		sprintf(line, "BEST RECORD   %s", bestBuf);
		glColor3f(0.72f, 0.72f, 0.76f);
		iTextCentered(cx, py1 + 12, line, GLUT_BITMAP_9_BY_15);
	}
	glColor3f(1.0f, 1.0f, 1.0f);
}

void iDraw()
{
	iClear();

	// Preload the attack sprites on the first draw, while the splash
	// screen is being shown. This removes the first-E loading hitch.
	static int attackImagesPreloaded = 0;
	if (!attackImagesPreloaded){
		preloadAttackImages();
		attackImagesPreloaded = 1;
	}

	/* ---------------- SPLASH SCREEN : bg1.png for 1.5s ---------------- */
	if (gameState == STATE_SPLASH)
	{
		if (!bg1Loaded){
			bg1Image = iLoadImage("Assets/Backgrounds/bg1.png");
			bg1Loaded = 1;
		}
		iShowImage(0, 0, 1350, 680, bg1Image);

		double elapsed = (double)(clock() - splashStartTime) / CLOCKS_PER_SEC;
		if (elapsed >= 1.5){
			gameState = STATE_HOME;
		}
		return;
	}

	/* ---------------- HOME SCREEN : bg2.png + START/CREDITS click ---------------- */
	if (gameState == STATE_HOME)
	{
		if (!homeMenuLoaded){
			homeMenuImage = iLoadImage("Assets/Backgrounds/home_menu.png");
			homeMenuLoaded = 1;
		}
		iShowImage(0, 0, 1350, 680, homeMenuImage);
		// Nothing is drawn on top. All FIVE buttons - including RECORDS -
		// are part of the artwork, exactly as the original four always
		// were; clicking them is handled in iMouse(). home_menu.png is
		// bg2.png with the RECORDS plate copied out of its own CREDITS
		// plate and set one menu step lower, so the new entry is the
		// same object as the others rather than something drawn over
		// the top of them.
		return;
	}

	/* ---------------- RECORDS SCREEN : the Instructions artwork ---------------- */
	/* Deliberately the same background as the Instructions page, so the
	 * records read as another information page of the same game rather
	 * than a screen of their own. The table is drawn over the panel grid
	 * that page carries; the BACK plate at the bottom of the picture is
	 * reused as the way home (recordsBackBtn* in game_globals.h). */
	if (gameState == STATE_RECORDS)
	{
		if (!instLoaded){
			instImage = iLoadImage("Assets/Backgrounds/inst.png");
			instLoaded = 1;
		}
		iShowImage(0, 0, 1350, 680, instImage);
		drawRecordsRows();
		return;
	}

	/* ---------------- CREDITS SCREEN : credits.png + HOME click ---------------- */
	if (gameState == STATE_CREDITS)
	{
		if (!creditsLoaded){
			creditsImage = iLoadImage("Assets/Backgrounds/credits.png");
			creditsLoaded = 1;
		}
		iShowImage(0, 0, 1350, 680, creditsImage);
		// No button is drawn on top - the "Home" button (bottom-right)
		// is already part of credits.png (see creditsHomeBtnX1/Y1/X2/Y2
		// above). Clicking it is handled in iMouse().
		return;
	}

/* ---------------- STORY SCREEN : s1..s4, 3 seconds each ---------------- */
    if (gameState == STATE_STORY)
    {
        if (storyPage < 0) storyPage = 0;
        if (storyPage >= STORY_PAGE_COUNT) storyPage = STORY_PAGE_COUNT - 1;

        if (!storyPageImgLoaded[storyPage]){
            storyPageImage[storyPage] = iLoadImage(storyPageFiles[storyPage]);
            storyPageImgLoaded[storyPage] = 1;
        }

        iShowImage(0, 0, 1350, 680, storyPageImage[storyPage]);

        double elapsed =
            (double)(clock() - storyPageStartTime) / CLOCKS_PER_SEC;

        if (elapsed >= storyPageDuration){
            if (storyPage < STORY_PAGE_COUNT - 1){
                storyPage++;
                storyPageStartTime = clock();
            }
            else{
                gameState = STATE_HOME;
                storyPage = 0;
            }
        }
        return;
    }


	/* ---------------- INSTRUCTIONS SCREEN : inst.png + BACK click ---------------- */
	if (gameState == STATE_INSTRUCTIONS)
	{
		if (!instLoaded){
			instImage = iLoadImage("Assets/Backgrounds/inst.png");
			instLoaded = 1;
		}
		iShowImage(0, 0, 1350, 680, instImage);
		// No button is drawn on top - the "BACK" button (bottom-center)
		// is already part of inst.png (see instBackBtnX1/Y1/X2/Y2
		// above). Clicking it is handled in iMouse().
		return;
	}

	/* ---------------- GAME OVER SCREEN : bg50.png + RETRY/MENU/EXIT clicks ---------------- */
	if (gameState == STATE_GAMEOVER)
	{
		if (!bg50Loaded){
			bg50Image = iLoadImage("Assets/Backgrounds/bg50.png");
			bg50Loaded = 1;
		}
		iShowImage(0, 0, 1350, 680, bg50Image);
		// No buttons/text are drawn on top - RETRY / MAIN MENU / EXIT are
		// already part of bg50.png (see retryBtn/menuBtn/exitBtn above).
		// Clicking them is handled in iMouse().
		return;
	}

	/* ---------------- FINAL VICTORY SCREEN ---------------- */
	/* Reached only by walking out of the palace door at the end of
	 * Level 3's escape run.
	 *
	 * CHANGED: this used to be the level-complete artwork (bg30.png)
	 * dimmed right down with six lines of text written over it. It is
	 * now its own finished picture, victory.png, drawn 1:1 over the
	 * window - and NOTHING is drawn on top of it: no wash, no caption,
	 * no button plate. The HOME button is part of the artwork, and
	 * clicking it is handled in handleHomeMouse() against
	 * victoryHomeBtn* (see game_globals.h), which is measured off this
	 * picture rather than reusing the level-complete screen's box. */
	if (gameState == STATE_VICTORY)
	{
		if (!victoryLoaded){
			victoryImage = iLoadImage("Assets/Backgrounds/victory.png");
			victoryLoaded = 1;
		}
		iShowImage(0, 0, 1350, 680, victoryImage);
		return;
	}

	/* ---------------- LEVEL COMPLETE SCREEN : bg30.png + HOME click ---------------- */
	if (gameState == STATE_LEVELCOMPLETE)
	{
		if (!bg30Loaded){
			bg30Image = iLoadImage("Assets/Backgrounds/bg30.png");
			bg30Loaded = 1;
		}
		iShowImage(0, 0, 1350, 680, bg30Image);
		// No button is drawn on top - HOME (and NEXT LEVEL) are already
		// part of bg30.png (see homeBtnX1/Y1/X2/Y2 above). Clicking HOME
		// is handled in iMouse().
		//
		// The completion time and the best record go in the band between
		// the "LEVEL COMPLETED" title and the NEXT LEVEL button. The
		// clock was already stopped, and the record already compared and
		// saved, on the tick the level was won - this only displays it.
		drawLevelCompleteTimes();
		return;
	}

	/* -------- GAME SCREEN backdrops: L1 = bg5, L2 = bg6, L3 = bg7 -------- */
	// CHANGED: Level 3 used to share Level 1's interior backdrop (bg5),
	// on the reasoning that Level 3 IS the palace. It now has its own -
	// bg7.png, the palace seen from OUTSIDE: the great curtain wall with
	// its towers and gate, a skyline of domes and minarets beyond it, and
	// an open courtyard in front. The Prince is out of the building and
	// running for the edge of the kingdom, and the backdrop should say so.
	//
	// Level 3 still uses Level 1's brick tile set for its platforms
	// (drawLC()/drawWallTile()/... only swap to the cave tiles while
	// gameState == STATE_LEVEL2) - only the backdrop changed, so the
	// level's own layout and look are otherwise exactly as they were.
	// Levels 1 and 2 keep bg5 and bg6 untouched.
	if (gameState == STATE_LEVEL2){
		if (!bg6Loaded){
			bg6Image = iLoadImage("Assets/Backgrounds/bg6.png");
			bg6Loaded = 1;
		}
		iShowImage(0, 0, 1350, 680, bg6Image);
	}
	else if (gameState == STATE_LEVEL3){
		if (!bg7Loaded){
			bg7Image = iLoadImage("Assets/Backgrounds/bg7.png");
			bg7Loaded = 1;
		}
		iShowImage(0, 0, 1350, 680, bg7Image);
	}
	else{
		if (!bg5Loaded){
			bg5Image = iLoadImage("Assets/Backgrounds/bg5.png");
			bg5Loaded = 1;
		}
		iShowImage(0, 0, 1350, 680, bg5Image);
	}

	/* LEVEL 3: the whole palace shakes as it comes down.
	 *
	 * The shake is folded into the world camera (cX) for the duration of
	 * the world drawing only, and taken straight back out again further
	 * down - BEFORE the Info/HOME overlays (which belong to the screen,
	 * not the world) and before update() runs - so cX is still the true,
	 * un-shaken camera everywhere any logic reads it. */
	int level3Shake = 0;
	if (gameState == STATE_LEVEL3){
		level3Shake = level3ShakeOffset();
		cX += level3Shake;
		drawLevel3WaterBody();   // drawn first, so standing platforms sit on top of it
		// The piranhas go in with the water, BEFORE the platforms.
		//
		// They are free to swim anywhere in the flood now - the platforms
		// are the Prince's obstacles, not theirs - so one can be behind a
		// platform. Drawing them here means the brickwork hides it, the
		// way scenery in front of it should, instead of a fish appearing
		// to swim across the face of the wall.
		drawLevel3Fish();
	}

	/*Wall*//*Start*/
	for (int j = 0; j < wallNo; j++){
		int gh = wall[j].h - 55;
		drawLC(wall[j].sPos + cX, gh);
		int gx = wall[j].sPos + 64;
		while (gx < wall[j].ePos - 64) {
			drawUW(gx + cX, gh);
			gx += 64;
		}
		gx = wall[j].sPos;
		for (int i = 1; i < 3; i++){
			gh = wall[j].h - 55 - i * 64;
			gx = wall[j].sPos;
			while (gx < wall[j].ePos - 64){
				drawWallTile(gx + cX, gh);
				gx += 64;
			}

		}
		drawRC(wall[j].ePos - 64 + cX, wall[j].h - 55);
		drawRW(wall[j].ePos - 64 + cX, wall[j].h - 55 - 64);
		drawRWB(wall[j].ePos - 64 + cX, wall[j].h - 55 - 64 - 64);

	}
	/*Wall*//*End*/

	/* ------------------------------------------------------------ */
	/* Cave fill (Level 2 ONLY): pack every static wall[] platform    */
	/* solid with cave rock both below and above the 3 rows drawn     */
	/* just above - EXCEPT a clear "running band" directly over each  */
	/* platform's surface, left open for the Prince, an enemy, and a  */
	/* switch to render unobstructed. CAVE_RUN_CLEARANCE is sized off */
	/* the switches: button.y sits at platform height + 48, and the   */
	/* switch art itself is 64px tall, so a switch's top edge is at   */
	/* platform height + 112 - CAVE_RUN_CLEARANCE (150) leaves a bit   */
	/* of headroom above that. Everything below the platform (down to */
	/* the floor of the screen) and everything above the clear band   */
	/* (up to the top of the screen) gets tiled with cave rock, so the*/
	/* level reads as a solid rock tunnel with only the path the      */
	/* Prince actually runs/climbs/rides on left open. Static wall[]   */
	/* platforms, elevators (jWall), and moving bridges (mWall) are    */
	/* ALL covered below - see the second loop further down for the   */
	/* jWall/mWall treatment, which leaves open only the shaft/row     */
	/* each one actually travels through.                              */
	/* ------------------------------------------------------------ */
	if (gameState == STATE_LEVEL2){
		for (int j = 0; j < wallNo; j++){
			int top = wall[j].h - 55; // y of the top row drawn in the loop above

			// Pack solid rock DOWN from below the 3 rows already drawn
			// (top, top-64, top-128) all the way to the floor of the screen.
			for (int gh = top - 64 * 3; gh > -64; gh -= 64){
				int gx = wall[j].sPos;
				while (gx < wall[j].ePos - 64){
					drawWallTile(gx + cX, gh);
					gx += 64;
				}
				drawWallTile(wall[j].ePos - 64 + cX, gh);
			}

			// Pack solid rock UP from just above the clear running band,
			// starting with one jagged/stalactite row, then plain rock
			// the rest of the way to the top of the screen (680).
			int ceilStart = wall[j].h + CAVE_RUN_CLEARANCE;
			if (ceilStart > CAVE_MAX_CEIL_START) ceilStart = CAVE_MAX_CEIL_START; // keep the ceiling row on-screen so there is never a visible gap up top
			int gx = wall[j].sPos;
			while (gx < wall[j].ePos - 64){
				drawCaveCeiling(gx + cX, ceilStart);
				gx += 64;
			}
			drawCaveCeiling(wall[j].ePos - 64 + cX, ceilStart);

			for (int gh = ceilStart + 64; gh < 680; gh += 64){
				gx = wall[j].sPos;
				while (gx < wall[j].ePos - 64){
					drawWallTile(gx + cX, gh);
					gx += 64;
				}
				drawWallTile(wall[j].ePos - 64 + cX, gh);
			}
		}

		/* ------------------------------------------------------------ */
		/* Same treatment for the elevators (jWall) and moving bridges    */
		/* (mWall): water fills everything except wherever the elevator/  */
		/* bridge tile itself currently sits (drawn afterwards, in the    */
		/* existing Jumping Wall / Moving Wall sections below, which      */
		/* paints over the water at wherever it currently is) - for an    */
		/* elevator that's the vertical shaft it rides between lowerLimit */
		/* and upperLimit, for a moving bridge that's the single          */
		/* horizontal row it slides along. Water fills the entire shaft/  */
		/* row (not just the pit below it) so nothing but water and the   */
		/* elevator/bridge itself is ever visible there - previously the  */
		/* shaft/row itself was left completely undrawn wherever the      */
		/* elevator/bridge wasn't currently covering it, which let the    */
		/* raw background art (e.g. a painted-in wooden bridge) show      */
		/* through looking like a stray extra wall floating in the pit.   */
		/* Rock still fills below the water (down to the floor) and above */
		/* the shaft/row (CAVE_RUN_CLEARANCE past its travel range, up to */
		/* the ceiling), same as before.                                  */
		/* ------------------------------------------------------------ */
		for (int j = 0; j < jWallNo; j++){
			int gx0 = jWall[j].Pos;
			int shaftBottom = jWall[j].lowerLimit - 55; // lowest row the elevator's own tile ever occupies
			// NOTE: nothing below reads the elevator's CURRENT row, or
			// the top of its travel, any more - the shaft above the
			// waterline is simply left open. See the comment below.

			// BUGFIX: THE WATER LEVEL IN THIS SHAFT IS FIXED.
			//
			// This used to fill the shaft with WATER from the bottom up
			// to `curRow` - wherever the elevator happened to be right
			// now - and pack rock in above it. So every time the
			// elevator climbed, another row of water was drawn
			// underneath it: the pool looked like it was rising with
			// the platform, and sinking again on the way down.
			//
			// Nothing about the actual water ever moved. The surface
			// that decides when the player drowns is
			// `jWall[j].lowerLimit - 55` (see waterSurfaceAt() in
			// controls.h) and it is a constant - so the drawing was
			// simply disagreeing with the game.
			//
			// FIX: the shaft above the waterline is left OPEN, so the
			// cave background (bg6.png) shows straight through it.
			//
			// Nothing at all is drawn between the waterline and the
			// ceiling fill further down.
			// Two earlier attempts filled it in and both were wrong: a
			// near-black wash read as a hole punched in the level, and
			// packing it with cave_wall tiles put a slab of WALL where
			// the cave itself should be visible. The elevator rides
			// through open air in a cave, so open air is what belongs
			// here - and it matches the gap above the shaft and the gaps
			// between the platforms, which have always shown the
			// background.
			//
			// (The original reason this space was filled was that the
			// old bg6.png had level-like shapes painted into it that
			// showed through and read as stray walls. The background is
			// now a plain cave wall, so there is nothing left to hide.)

			// Poison below the shaft's lowest reach, down to the floor -
			// the row right under the shaft gets the surface tile.
			drawPoisonTop(gx0 + cX, shaftBottom - 64);
			drawPoisonTop(gx0 + 64 + cX, shaftBottom - 64);
			for (int gh = shaftBottom - 128; gh > -64; gh -= 64){
				drawPoison(gx0 + cX, gh);
				drawPoison(gx0 + 64 + cX, gh);
			}

			// Rock above the shaft's highest reach (plus running clearance),
			// starting with a jagged/stalactite row, up to the ceiling.
			int ceilStart = jWall[j].upperLimit + CAVE_RUN_CLEARANCE;
			if (ceilStart > CAVE_MAX_CEIL_START) ceilStart = CAVE_MAX_CEIL_START;
			drawCaveCeiling(gx0 + cX, ceilStart);
			drawCaveCeiling(gx0 + 64 + cX, ceilStart);
			for (int gh = ceilStart + 64; gh < 680; gh += 64){
				drawWallTile(gx0 + cX, gh);
				drawWallTile(gx0 + 64 + cX, gh);
			}
		}

		for (int j = 0; j < mWallNo; j++){
			int top = mWall[j].h - 55; // the row the bridge itself travels along

			// Water fills the bridge's own row and the pit under it,
			// across the FULL rail the bridge can ever occupy
			// (left .. right+128, matching waterSurfaceAt()), so no part
			// of the gap shows raw background art. The bridge's own draw
			// (below) paints over wherever it currently sits.
			//
			// BUGFIX: skip any tile column that a static rock already
			// occupies. Several rails in Levels 1 and 2 overlap the
			// first 128px of the next rock, and the water used to be
			// painted straight over it - so a strip of solid platform
			// was drawn as open water. (That strip had no collision
			// either; see the matching fix in the Moving Wall section of
			// update() in controls.h.) Now rock stays rock and only the
			// real gap is water.
			int railEnd = mWall[j].right + 128;
			int runs[LEVEL2_MAX_RUNS][2];
			int nRuns = level2OpenRuns(mWall[j].left, railEnd, runs, LEVEL2_MAX_RUNS);
			int gx;
			for (int rr = 0; rr < nRuns; rr++){
				int a = runs[rr][0], b = runs[rr][1];
				if (!level2BeginClip(a, b)) continue;
				// Tiles start at the run's own left edge and the scissor
				// trims the last one, so the pool fills the gap exactly -
				// no bare strip at the rock's edge, no water on the rock.
				for (gx = a; gx < b; gx += 64){
					drawPoisonTop(gx + cX, top);
					for (int gh = top - 64; gh > -64; gh -= 64)
						drawPoison(gx + cX, gh);
				}
				glDisable(GL_SCISSOR_TEST);
			}

			// Rock above the bridge's row (plus running clearance), across
			// that same full span, up to the ceiling.
			int ceilStart = mWall[j].h + CAVE_RUN_CLEARANCE;
			if (ceilStart > CAVE_MAX_CEIL_START) ceilStart = CAVE_MAX_CEIL_START;
			gx = mWall[j].left;
			while (gx < mWall[j].right){
				drawCaveCeiling(gx + cX, ceilStart);
				gx += 64;
			}
			drawCaveCeiling(mWall[j].right + cX, ceilStart);

			for (int gh2 = ceilStart + 64; gh2 < 680; gh2 += 64){
				gx = mWall[j].left;
				while (gx < mWall[j].right){
					drawWallTile(gx + cX, gh2);
					gx += 64;
				}
				drawWallTile(mWall[j].right + cX, gh2);
			}
		}
	}



	/* LEVEL 2: give the pools depth, a moving surface, light and
	 * bubbles, over the flat repeated tiles drawn above. Placed here so
	 * it sits on top of the water but UNDER the elevator, the bridge,
	 * the switches and the characters - all of which are drawn below
	 * and should read as being in front of the water, not under it. */
	drawLevel2WaterEffects();

	/* LEVEL 3: the collapsing slabs and the level's two set-pieces (the
	 * sealed arena gate and the palace door) are drawn here, in among
	 * the static walls, so they sit in the same visual layer as every
	 * other piece of palace masonry. */
	if (gameState == STATE_LEVEL3){
		drawLevel3CollapsePlatforms();
		drawLevel3Structures();
	}

	/*Jumping Wall*//*Start*/
	for (int j = 0; j < jWallNo; j++){
		drawLC(jWall[j].Pos + cX, jWall[j].h - 55);
		drawRCB(jWall[j].Pos + 64 + cX, jWall[j].h - 55);
	}
	/*Jumping Wall*//*End*/



	/*Moving Wall*//*Start*/
	for (int j = 0; j < mWallNo; j++){
		drawLC(mWall[j].current + cX, mWall[j].h - 55);
		drawRCB(mWall[j].current + 64 + cX, mWall[j].h - 55);
	}
	/*Moving Wall*//*End*/

	/*Switch*//*Start*/
	// Drawn BEFORE both enemies and the player below, so the switch
	// icon never renders on top of a character standing/walking over
	// it - previously this was only placed before Player, so a switch
	// overlapping an enemy still drew on top of that enemy.
	for (int j = 0; j < buttonNo; j++){
		drawSwitch(button[j].x - 32 + cX, button[j].y, button[j].OnOff);
	}
	/*Switch*//*End*/

	/*Enemy*//*Start*/

	for (int j = 0; j < EnemyNo; j++){
		if (enemy[j].a == 1){
			// 3-frame swing animation, spread proportionally across
			// ENEMY_ATTACK_TICKS ticks (see the "Attack swing speed"
			// comment block above drawAttack() for why a proportional
			// formula is used instead of a fixed divisor). atkAnim
			// still counts down to 0 across successive draw calls;
			// frame still steps through 0, 1, 2 as the swing plays
			// out, just at whatever pace ENEMY_ATTACK_TICKS dictates.
			int frame = ((ENEMY_ATTACK_TICKS - enemy[j].atkAnim) * ENEMY_ATTACK_FRAME_COUNT) / ENEMY_ATTACK_TICKS;
			if (enemy[j].k == 0) drawEnemyAttack(enemy[j].x - 45 + cX, enemy[j].y, 0, frame);
			else drawEnemyAttack(enemy[j].x - 20 + cX, enemy[j].y, 1, frame);

			// BUGFIX: these checks used to be one-sided
			// (`enemy.x - playerX <= 64` / `playerX - enemy.x <= 64`
			// with no lower bound), so a large *negative* difference
			// (player standing on the opposite side of the map from
			// the enemy) still satisfied "<= 64" and dealt damage from
			// across the level. Now we require the player to actually
			// be on the side the enemy is facing/swinging toward, and
			// within 64px of it.
			// Damage is applied once per swing, exactly on the trigger
			// tick (atkAnim == ENEMY_ATTACK_TICKS) rather than on
			// "frame == 0", since frame == 0 can now be true for more
			// than one tick. Checking atkAnim's starting value keeps
			// it a single hit per swing.
			if (enemy[j].atkAnim == ENEMY_ATTACK_TICKS){
				// NEW: play the same swing sound effect when an enemy
				// lands their attack (kept optional/commented so it's
				// easy to remove if you only want the sound on the
				// player's own swings). Uncomment the next line to
				// enable enemy attack sound too:
				// playAttackSound();
				applyEnemyStrikeDamage(j);
			}

			enemy[j].atkAnim--;
			if (enemy[j].atkAnim <= 0){
				enemy[j].a = 0;
				enemy[j].m = 0;
			}
		}
		else{
			drawEnemy(enemy[j].x + cX, enemy[j].y, enemy[j].k, enemy[j].p);
			if (enemy[j].m != 5) enemy[j].m++;
		}
		// FIX: was drawn at enemy[j].y + 46, which lands inside the
		// character sprite (drawCharacter()/drawAttack() draw the
		// 80x80 sprite spanning enemy[j].y to enemy[j].y + charImgSize,
		// so +46 sat roughly mid-chest) instead of above their head.
		// Now drawn just above the top of the sprite (charImgSize=80)
		// with a small gap, so it always floats over the enemy's head
		// regardless of which frame/pose is currently showing.
		iSetcolor(255, 0, 0);
		iLine(enemy[j].x - 25 + cX, enemy[j].y + enemyImgSize + 10, enemy[j].x - 25 + enemy[j].life + cX, enemy[j].y + enemyImgSize + 10);
		iSetcolor(255, 255, 255);
	}
	/*Enemy*//*End*/

	/* LEVEL 3: the final Guardian. Drawn after the ordinary guards and
	 * before the player, so the Prince always renders in front of it. */
	drawLevel3Guardian();

	/* Health hearts, drawn before the player so he passes in front of
	 * one as he jumps up through it. No-op on Level 1. */
	drawHearts();

	/*Player*//*Start*/
	playerHeight();

	// Recompute whether the player currently counts as "moving" for
	// idle-sprite purposes (see the playerIsMoving/lastMoveKeyTime
	// comment above) - if enough real time has passed since the last
	// 'a'/'d' key was processed in iKeyboard(), treat the player as
	// idle regardless of how the OS/engine repeats key events while
	// held down.
	if (lastMoveKeyTime == 0 || (double)(clock() - lastMoveKeyTime) / CLOCKS_PER_SEC >= moveIdleTimeout){
		playerIsMoving = 0;
		stopRunningSound();
	}

	// Compute this frame's hop offset (parabolic: 0 at start/end,
	// peak at the midpoint) before drawing, then count the timer
	// down. This is purely a draw-position offset, applied only in
	// the standing (playerBlockNo != -1) branches below - falling
	// already has its own dedicated drawFree() animation, so the hop
	// doesn't apply there.
	int hopOffset = 0;
	if (hopTimer > 0){
		double hopProgress = (double)(hopDuration - hopTimer) / hopDuration;
		hopOffset = (int)(hopMaxHeight * 4.0 * hopProgress * (1.0 - hopProgress));
		hopTimer--;
	}

	if (drowning){
		// Level 2 water: the prince has broken the surface and is
		// going under. This owns the whole player draw for the rest
		// of the sequence - the falling/idle/walk/attack sprites are
		// all irrelevant now. See drawDrowningScene() above and
		// updateDrowning() in controls.h.
		drawDrowningScene();
	}
	else if (level3Swimming){
		// LEVEL 3: in the water.
		//
		// CHANGED: this used to reuse the upright walk cycle, so he slid
		// through the water standing bolt upright. He now has a proper
		// horizontal swimming pose (see drawSwim() above), stroking
		// through a three-frame cycle, with a slow bob on top so he is
		// visibly floating. playerBlockNo is -1 while swimming, so this
		// has to be tested BEFORE the falling branch below or he would
		// show the falling sprite the whole time.
		int swimBob = (int)(3.0 * sin(level3Timer * 0.11));
		int swimFrame = (level3Timer / 9) % SWIM_FRAME_COUNT;
		drawSwim(playerX + cX, playerY + swimBob, k, swimFrame);
	}
	else if (playerBlockNo == -1){
		// Covers both "still falling" (landingState == 0) and "landed,
		// sitting in the 0.5s pause" (landingState == 1) - in both
		// cases the player is airborne/grounded-off-a-block and should
		// show the falling sprite rather than the walk-cycle, so it
		// stays visibly on screen the whole time instead of popping
		// straight to the respawn/game-over screen.
		drawFree(playerX - 32 + cX, playerY, k);
	}
	else if (level2IsDucking()){
		// LEVEL 2: crouched under a bat. Tested before the attack and
		// idle/walk branches because the crouch owns the pose outright -
		// iKeyboard() refuses to start a swing or a step while it is
		// running, so there is never anything else to show. No hop
		// offset either: he is pressed to the floor, not springing.
		drawDuck(playerX + cX, playerY, k);
	}
	else if (atMode) {
		// 3-frame swing, spread proportionally across
		// PLAYER_ATTACK_TICKS ticks (see the "Attack swing speed"
		// comment block above drawAttack() above). atMode counts down
		// PLAYER_ATTACK_TICKS..1 while the attack is active, and the
		// proportional formula below spreads frames 0,1,2 across that
		// range as evenly as the tick count allows.
		int frame = ((PLAYER_ATTACK_TICKS - atMode) * ATTACK_FRAME_COUNT) / PLAYER_ATTACK_TICKS;
		if (k == 0) drawAttack(playerX - 45 + cX, playerY + hopOffset, 0, frame);
		else drawAttack(playerX - 20 + cX, playerY + hopOffset, 1, frame);
		// BUGFIX: same one-sided-distance bug as the enemy attack above,
		// mirrored here for the player's attack on the enemy - now
		// requires being on the correct side as well as in range.
		// Damage lands once per animation frame (see playerAtkLastFrame
		// comment above) instead of on a raw alternating-tick toggle,
		// so the swing always deals exactly ATTACK_FRAME_COUNT hits
		// total, regardless of PLAYER_ATTACK_TICKS.
		if (frame != playerAtkLastFrame){
			playerAtkLastFrame = frame;
			applyPlayerStrikeDamage();
		}
	}
	else{
		// NEW: standing still (not falling, not attacking) now shows
		// the dedicated idle sprite (prince/l0.png / prince/r0.png)
		// instead of the walk-cycle whenever the player hasn't pressed
		// 'a'/'d' recently (see playerIsMoving above). Actively
		// walking still plays the normal 13-frame walk cycle exactly
		// as before.
		if (playerIsMoving) drawCharacter(playerX + cX, playerY + hopOffset, k, p);
		else drawIdle(playerX + cX, playerY + hopOffset, k);
	}
	if (atMode) atMode--;
	/*Player*//*End*/

	/* LEVEL 2: the cave bats, drawn AFTER the player so one sweeping
	 * past passes in front of him - which is also what makes it read
	 * clearly whether a bat went over a crouched Prince or straight
	 * through a standing one. No-op on Levels 1 and 3. */
	drawLevel2Bats();

	/*Jump Effect*//*Start*/
	// Small "burst" of rays drawn at the spot the player jumped from,
	// widening/rising for a handful of frames then disappearing. Purely
	// visual feedback for pressing 'W'; doesn't affect gameplay.
	//
	// HEIGHT x8 (per request): first scaled to x4, then doubled again
	// to x8 total - only the VERTICAL reach of the burst is scaled up
	// here, every term that offsets a point away from `ey` (the
	// takeoff Y) vertically has been multiplied by 8 (relative to the
	// original) below. The horizontal spread (the ex-based X offsets)
	// is left exactly as it was, so the effect gets taller/skinnier,
	// not wider. This is purely cosmetic and is completely separate
	// from MAX_JUMP_HEIGHT (the actual climb height used in
	// iKeyboard()) - that value is untouched, so how high the player
	// can physically climb onto a block has not changed at all.
	if (jumpEffectTimer > 0){
		int prog = 12 - jumpEffectTimer; // 0 .. 11 as the effect plays out
		int ex = jumpEffectX + cX;
		int ey = jumpEffectY;
		iSetcolor(255, 255, 120);
		iLine(ex - 10, ey + 32, ex - 22 - prog * 2, ey + 144 + prog * 16);
		iLine(ex + 10, ey + 32, ex + 22 + prog * 2, ey + 144 + prog * 16);
		iLine(ex, ey + 48, ex, ey + 192 + prog * 16);
		iLine(ex - 16, ey + 16, ex - 8, ey + 112 + prog * 8);
		iLine(ex + 16, ey + 16, ex + 8, ey + 112 + prog * 8);
		iSetcolor(255, 255, 255);
		jumpEffectTimer--;
	}
	/*Jump Effect*//*End*/

	/* LEVEL 3: overlays that sit on top of the finished world - the
	 * underwater tint over everything the flood has swallowed, the
	 * waterline, dust and falling masonry, and the Level 3 HUD. The
	 * world camera's shake is removed immediately afterwards, so
	 * everything below this point (the life bar, the movement hint, the
	 * HOME button and, crucially, update()) sees the true camera. */
	if (gameState == STATE_LEVEL3){
		drawLevel3Effects();
		cX -= level3Shake;
		level3Shake = 0;
	}

	/* LEVEL 2: a bat just connected. A short red wash over the whole
	 * frame, fading out over LEVEL2_BAT_FLASH ticks - without it, losing
	 * 20% of the life bar to something that flies past in a fraction of
	 * a second is easy to miss entirely. Drawn in screen space (no cX)
	 * and only ever while the flash timer is running, so it costs
	 * nothing the rest of the time. */
	if (gameState == STATE_LEVEL2 && level2BatHitFlash > 0){
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(0.75f, 0.06f, 0.06f, 0.42f * level2BatHitFlash / (float)LEVEL2_BAT_FLASH);
		iFilledRectangle(0, 0, 1350, 680);
		glDisable(GL_BLEND);
	}

	/* "+10% HEALTH" - brief confirmation that a heart was taken, so the
	 * life bar jumping up is clearly connected to what just happened. */
	if (heartMsgTimer > 0){
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		float ea = (heartMsgTimer > 40) ? 0.62f : (0.62f * heartMsgTimer / 40.0f);
		glColor4f(0.0f, 0.0f, 0.0f, ea);
		iFilledRectangle(583, 520, 184, 34);
		glDisable(GL_BLEND);

		iSetcolor(255, 128, 128);
		iText(600, 531, "+10%  HEALTH", GLUT_BITMAP_9_BY_15);
	}

	/* LEVEL 2: "a bat is coming, press Q" - shown for the first couple
	 * of bats in a run only (see level2BatHintsLeft), since the
	 * instructions screen is a fixed image and cannot mention the new
	 * key. Same banner styling as the CHECKPOINT line below. */
	if (gameState == STATE_LEVEL2 && level2BatHintTimer > 0){
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		float ha = (level2BatHintTimer > 40) ? 0.62f : (0.62f * level2BatHintTimer / 40.0f);
		glColor4f(0.0f, 0.0f, 0.0f, ha);
		iFilledRectangle(487, 560, 376, 34);
		glDisable(GL_BLEND);

		iSetcolor(255, 226, 120);
		iText(520, 571, "BAT INCOMING  -  press Q to duck", GLUT_BITMAP_9_BY_15);
	}

	/* CHECKPOINT confirmation. Shown for a couple of seconds after the
	 * player operates a switch, so it is clear that the switch banked
	 * their progress as well as working whatever it is wired to. Drawn
	 * in screen space (no cX) because it belongs to the HUD, and in all
	 * three levels since every level's switches are checkpoints. */
	/* FIX: the plate behind this line used to be a hard-coded 376px wide
	 * starting at x=487, but the message renders 396px wide in
	 * GLUT_BITMAP_9_BY_15 and started at x=505 - so it ran from 505 to
	 * 901 while the black plate stopped at 863, leaving the last few
	 * words sitting on the bare level with nothing behind them.
	 *
	 * The plate is now measured from the string itself with
	 * glutBitmapLength(), plus equal padding on both sides, so it always
	 * contains the whole message no matter how the wording changes. The
	 * plate stays centred on x=675, which is exactly where the old fixed
	 * box was centred (487 + 376/2), so the banner has not moved - it is
	 * only as wide as it should always have been. */
	if (checkpointMsgTimer > 0){
		char *cpMsg = "CHECKPOINT  -  you will return to this switch";
		const int cpPadX = 16, cpPadY = 10;
		int cpTextW = textWidthPx(GLUT_BITMAP_9_BY_15, cpMsg);
		int cpTextH = 15;                            // 9x15 bitmap font
		int cpW = cpTextW + cpPadX * 2;
		int cpH = cpTextH + cpPadY * 2;              // 35, as before
		int cpX = 675 - cpW / 2;                     // unchanged centre
		int cpY = 600;                               // unchanged height

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		float a = (checkpointMsgTimer > 40) ? 0.62f : (0.62f * checkpointMsgTimer / 40.0f);
		glColor4f(0.0f, 0.0f, 0.0f, a);
		iFilledRectangle(cpX, cpY, cpW, cpH);
		glDisable(GL_BLEND);

		iSetcolor(255, 255, 255);
		iText(cpX + cpPadX, cpY + cpPadY + 4, cpMsg, GLUT_BITMAP_9_BY_15);
	}

	/* LEVEL TIMER BAR. Top-centre of the gameplay screen, clear of the
	 * life bar in the top-left corner and of every banner above, and
	 * only ever reached here - i.e. in a gameplay state, after all the
	 * menu/end screens have already returned. */
	drawLevelTimerBar();

	/*Info*//*Start*/
	iSetcolor(255, 255, 255);
	iShowBMP(20, 660, "Assets/UI/life.bmp");
	iRectangle(38, 660, playerMaxHealth + 4, 16);
	iFilledRectangle(40, 662, playerHealth, 12);



	if (playerBlockNo != -1){
		if (playerX - block[playerBlockNo].start < 52) iText(20, 20, "Press \'A\' to Jump left From Block, \'W\' to climb up, \'D\' to move right");
		else if (block[playerBlockNo].end - playerX < 50) iText(20, 20, "Press \'A\' to move left, \'D\' to Jump right from Block, \'W\' to climb up");
		else iText(20, 20, "Press \'A\' to move left and \'D\' to move right");
	}
	/*Info*//*End*/

	/*In-game HOME button*//*Start*/
	// UI overlay, fixed to the top-right corner of the window. Drawn
	// last so it always sits on top of everything else, and NOT offset
	// by cX (the level camera) since it belongs to the screen, not the
	// world. Click handling lives in iMouse().
	if (!homeIconLoaded){
		homeIconImage = iLoadImage("Assets/UI/home_btn.png");
		homeIconLoaded = 1;
		gameHomeBtnX2 = 1350 - homeIconMargin;
		gameHomeBtnX1 = gameHomeBtnX2 - homeIconW;
		gameHomeBtnY2 = 680 - homeIconMargin;
		gameHomeBtnY1 = gameHomeBtnY2 - homeIconH;
	}
	// PNGs with transparency need blending explicitly turned on before
	// iShowImage(), same as drawCharacter()/drawAttack()/drawFree()
	// above - without it the transparent parts of home_btn.png paint
	// as a solid black box instead of letting the game show through.
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	iShowImage(gameHomeBtnX1, gameHomeBtnY1, homeIconW, homeIconH, homeIconImage);
	glDisable(GL_BLEND);
	/*In-game HOME button*//*End*/

	// Only advance game logic at a fixed real-world rate (see the
	// lastUpdateTime/targetUpdateInterval comment above iDraw()) - the
	// screen above this point still redraws every single callback, so
	// there's no stale-buffer blur, but movement itself only steps
	// forward ~60 times/sec regardless of how fast iDraw() is called.
	clock_t nowUpdate = clock();
	if (lastUpdateTime == 0 || (double)(nowUpdate - lastUpdateTime) / CLOCKS_PER_SEC >= targetUpdateInterval){
		update(); //update for next frame//
		lastUpdateTime = nowUpdate;
	}



}

#endif
