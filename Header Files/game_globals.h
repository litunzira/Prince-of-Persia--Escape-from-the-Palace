#ifndef GAME_GLOBALS_H
#define GAME_GLOBALS_H

#include "common.h"
#include "game_types.h"

#define WALL 1001
#define JWALL 1002
#define MWALL 1003

/* ---------------- Player movement ---------------- */
#define MAX_JUMP_HEIGHT 80
#define PLAYER_BASE_SPEED 2
#define PLAYER_SPEED_MULTIPLIER 1.6
#define ENEMY_IMG_SIZE 64

/* Player sprite is 80x80 (see charImgSize in images.h), NOT 64x64 like
 * the enemy sprite. Every place that decides "how close can the player
 * and an enemy get" (chase-stop distance, movement blocking, and the
 * attack-range checks in combat.h) used to reuse ENEMY_IMG_SIZE (64) for
 * this, which is only half the player's own sprite width. That let the
 * two centers get as close as 64px apart even though the sprites need
 * at least half-player-width (40) + half-enemy-width (32) = 72px of
 * separation to just touch without overlapping - so on any block with
 * an enemy actually reachable by the player (e.g. Level 2's enemies),
 * the prince and the enemy visibly overlapped by ~8px or more.
 * COMBAT_RANGE is the correct value and is now used everywhere that
 * used to hardcode 64 for this purpose. */
#define PLAYER_IMG_SIZE 80
#define COMBAT_RANGE (PLAYER_IMG_SIZE / 2 + ENEMY_IMG_SIZE / 2)
double playerMoveAccum = 0.0;

inline int nextMoveStep()
{
    playerMoveAccum += PLAYER_BASE_SPEED * PLAYER_SPEED_MULTIPLIER;
    int step = (int)playerMoveAccum;
    playerMoveAccum -= step;
    return step;
}

/* ---------------- Screen states ---------------- */
#define STATE_SPLASH         0
#define STATE_HOME           1
#define STATE_GAME           2
#define STATE_GAMEOVER       3
#define STATE_LEVELCOMPLETE  4
#define STATE_LEVEL2         9
#define STATE_CREDITS        5
#define STATE_STORY          6
#define STATE_INSTRUCTIONS   7
/* Level 3 ("THE FINAL ESCAPE") and the end-of-game victory screen it
 * leads to. STATE_LEVEL3 behaves like STATE_GAME/STATE_LEVEL2 for all
 * gameplay purposes (see isGameplayState() below, which every gate in
 * controls.h now uses); STATE_VICTORY is a terminal screen - update()
 * never runs there, exactly like STATE_LEVELCOMPLETE. */
#define STATE_LEVEL3        10
#define STATE_VICTORY       11
/* RECORDS: the best-time table, reached from the main menu. Another
 * terminal screen like CREDITS - update() never runs there, so the
 * level timer cannot tick while it is open. */
#define STATE_RECORDS       12

int gameState = STATE_SPLASH;

/* Every "is the player actually playing right now" gate in the game
 * funnels through this, so adding Level 3 did not mean hunting down
 * each `gameState == STATE_GAME || gameState == STATE_LEVEL2` pair
 * separately (and missing one). */
inline int isGameplayState()
{
    return gameState == STATE_GAME ||
           gameState == STATE_LEVEL2 ||
           gameState == STATE_LEVEL3;
}
// Remembers which level (STATE_GAME or STATE_LEVEL2) the player was
// actually in when STATE_GAMEOVER was triggered, so the RETRY button on
// bg50.png restarts that same level instead of always going back to
// Level 1 - see the STATE_GAMEOVER transitions in controls.h and the
// RETRY/MAIN MENU handling in home.h.
int gameOverFromLevel = STATE_GAME;
// Which level the STATE_LEVELCOMPLETE screen was reached FROM, so the
// NEXT LEVEL button on bg30.png can route Level 1 -> Level 2 and
// Level 2 -> Level 3 from the same screen. Level 3 never reaches
// STATE_LEVELCOMPLETE at all - it ends in STATE_VICTORY - so there is
// no way to ask for a Level 4 that does not exist.
int levelCompleteFromLevel = STATE_GAME;
clock_t splashStartTime;
/* homeMenuImage is home_menu.png: the original bg2.png main menu with a
 * fifth RECORDS plate, copied out of that same picture so it carries the
 * identical frame, bevel, finials and gold. bg2.png is kept as the
 * source it was built from. */
int bg1Image, homeMenuImage, bg5Image, bg50Image, bg30Image, bg6Image, creditsImage, instImage;
int bg1Loaded = 0, homeMenuLoaded = 0, bg5Loaded = 0, bg50Loaded = 0;
int bg30Loaded = 0, bg6Loaded = 0, creditsLoaded = 0, instLoaded = 0;
/* bg7.png - LEVEL 3 ONLY: the palace seen from outside. Level 3 used to
 * share Level 1's bg5; it has its own backdrop now. */
int bg7Image;
int bg7Loaded = 0;

/* victory.png - the FINAL VICTORY screen, shown once Level 3 is finished.
 * It is a finished 1350x680 piece drawn 1:1 over the whole window, with
 * its HOME button painted into the artwork - so nothing is drawn on top
 * of it at all. The screen used to be the level-complete art dimmed down
 * with several lines of text over it; that is all gone. */
int victoryImage;
int victoryLoaded = 0;

/* The HOME button baked into victory.png, measured off the artwork.
 * It needs its own hit box rather than reusing homeBtn*: that one is
 * positioned for the level-complete screen (bg30.png), which is a
 * different picture with its button somewhere else. */
int victoryHomeBtnX1 = 522, victoryHomeBtnY1 = 108;
int victoryHomeBtnX2 = 824, victoryHomeBtnY2 = 174;

/* ---------------- Story ---------------- */
#define STORY_PAGE_COUNT 4
char *storyPageFiles[STORY_PAGE_COUNT] = {
    "Assets/UI/s1.png", "Assets/UI/s2.png", "Assets/UI/s3.png", "Assets/UI/s4.png"
};
int storyPageImage[STORY_PAGE_COUNT];
int storyPageImgLoaded[STORY_PAGE_COUNT] = { 0 };
int storyPage = 0;
clock_t storyPageStartTime;
const double storyPageDuration = 3.0;

/* ---------------- Screen/button hit boxes ---------------- */
int startBtnX1 = 430, startBtnY1 = 320;
int startBtnX2 = 710, startBtnY2 = 375;

int creditsBtnX1 = 435, creditsBtnY1 = 92;
int creditsBtnX2 = 671, creditsBtnY2 = 156;

int instBtnX1 = 435, instBtnY1 = 166;
int instBtnX2 = 671, instBtnY2 = 220;

int instBackBtnX1 = 540, instBackBtnY1 = 35;
int instBackBtnX2 = 815, instBackBtnY2 = 148;

int creditsHomeBtnX1 = 1130, creditsHomeBtnY1 = 25;
int creditsHomeBtnX2 = 1330, creditsHomeBtnY2 = 70;

/* STORY uses the old HIGH SCORE button position in bg2.png. */
int storyBtnX1 = 440, storyBtnY1 = 240;
int storyBtnX2 = 677, storyBtnY2 = 295;

int retryBtnX1 = 487, retryBtnY1 = 285;
int retryBtnX2 = 860, retryBtnY2 = 327;
int menuBtnX1 = 487, menuBtnY1 = 220;
int menuBtnX2 = 860, menuBtnY2 = 262;
int exitBtnX1 = 487, exitBtnY1 = 155;
int exitBtnX2 = 860, exitBtnY2 = 197;

int homeBtnX1 = 450, homeBtnY1 = 120;
int homeBtnX2 = 900, homeBtnY2 = 200;

/* NEXT LEVEL button on the STATE_LEVELCOMPLETE screen (bg30.png) - sits
   directly above the HOME button there. */
int nextLevelBtnX1 = 450, nextLevelBtnY1 = 205;
int nextLevelBtnX2 = 900, nextLevelBtnY2 = 280;

/* ---------------- RECORDS ---------------- */
/* The RECORDS plate is painted into home_menu.png exactly like the four
 * above it, so - like them - it needs nothing here but a hit box. The
 * plate occupies screen y 18..80; this box is inset the same way the
 * other four are inset inside their plates. */
int recordsBtnX1 = 435, recordsBtnY1 = 24;
int recordsBtnX2 = 695, recordsBtnY2 = 74;

/* The RECORDS screen is drawn over the Instructions artwork, so it gets
 * that page's own BACK plate (the one painted at the bottom of
 * inst.png). These bounds are the visible plate rather than the wider
 * instBackBtn box, so they cannot reach up into the records table. */
int recordsBackBtnX1 = 525, recordsBackBtnY1 = 30;
int recordsBackBtnX2 = 830, recordsBackBtnY2 = 96;

/* ---------------- In-game HOME icon ---------------- */
int homeIconImage;
int homeIconLoaded = 0;
int homeIconW = 140, homeIconH = 46;
int homeIconMargin = 15;
int gameHomeBtnX1, gameHomeBtnY1, gameHomeBtnX2, gameHomeBtnY2;

/* ---------------- Level data ---------------- */
int height[50000];
int blockData[50000];
int doubleS[500];
int doubleE[500];
int doubleUp[500];
int doubleDown[500];
int doubleUpH[500];
int doubleDownH[500];

int EnemyNo = 8;
int mWallNo = 2;
int wallNo = 8;
int jWallNo = 2;
int playerBlockNo;
int buttonNo = 4;

Button button[500];
Enemy enemy[1000]; // for many enemies array
Block block[500];
Wall wall[100];
JumpingWall jWall[100];
MovingWall mWall[100];

int cEnemy = -1; //no valid enemy is selected
int atMode = 0;

int gravitalForce = 0;
int skipFallDamage = 0;
int fallCount = 0;

/* ---------------- Switch checkpoints ---------------- */
/* Every switch in the game doubles as a checkpoint. Pressing one records
 * where it stands, and a fall that would otherwise drop the player back
 * at the very start of the level puts them back at that switch instead.
 *
 * checkpointX is the switch's own x; checkpointY is the surface of the
 * platform it stands on. Every switch in all three levels is placed one
 * half-tile above its platform (button.y == platform height + 48), and
 * all of them are on STATIC platforms, so that surface is always a real,
 * stationary place to stand.
 *
 * -1 means "no switch pressed yet", in which case the old behaviour
 * applies and the player restarts at the beginning of the level. */
int checkpointX = -1;
int checkpointY = 0;
int checkpointMsgTimer = 0;   /* ticks left on the on-screen confirmation */

/* ---------------- Fall damage ---------------- */
/* gravitalForce counts logic ticks spent in the air, and playerHeight()
 * drops the player 5px per tick, so 1 tick is about 5px of fall.
 *
 * FIX: landing used to charge `playerHealth -= gravitalForce / 5` for
 * EVERY landing, no matter how short the drop was. Three things came
 * out of that, all of which felt like the life bar leaking:
 *
 *   1. THE GAME START. The player is placed at playerY = 300, but the
 *      floor under him at x = 300 in Level 1 is at 220 - so he falls
 *      80px (16 ticks) and is charged 3 health before the player has
 *      pressed a single key.
 *   2. STEPPING DOWN OFF ANY BRICK. Every ordinary step from one
 *      platform onto a slightly lower one was charged too, so simply
 *      walking through a level slowly ate the bar.
 *   3. THE DROP AFTER A RESPAWN, on top of the half-health penalty the
 *      respawn had just applied (that one was already handled by
 *      skipFallDamage, which is now also set at the start of a level).
 *
 * Now the first FALL_DAMAGE_FREE_TICKS of a fall cost nothing and only
 * the part beyond that is charged. 24 ticks is about 120px, which is
 * more than the height difference between any two neighbouring
 * platforms anywhere in Levels 1, 2 and 3 - so normal platforming never
 * costs health at all, while a genuine long drop (such as the 300px
 * fall off Level 1's high block) still hurts, as it should.
 *
 * To switch fall damage off completely, set this to a large number
 * (e.g. 100000); nothing else needs to change. */
#define FALL_DAMAGE_FREE_TICKS 24

int playerMaxHealth = 350;
int playerHealth = 350;
int playerX = 300, playerY = 300;
int cX = 0, cY = 0;
int p = 0;
int k = 0;

/* ---------------- Landing / jump / idle state ---------------- */
int landingState = 0;
clock_t landingStartTime;
const double landingPauseSeconds = 0.5;

int jumpEffectTimer = 0;
int jumpEffectX = 0, jumpEffectY = 0;

int hopTimer = 0;
// Both now mutable (not const) - a "nothing to climb" press uses a taller,
// longer hop than a normal idle hop; see the 'w' key handler in
// controls.h, which swaps these two values in before each hopTimer reset.
int hopDuration = 16;
int hopMaxHeight = 8;
const int normalHopDuration = 16;
const int normalHopMaxHeight = 8;
// Bigger, more noticeable hop played specifically when 'w'/Up is pressed
// next to something that ISN'T climbable (too tall, or no ledge there at
// all) - previously this used the same small 14px hop as every other 'w'
// press, which barely showed as any movement at all.
const int noClimbHopDuration = 26;
const int noClimbHopMaxHeight = 45;

int playerIsMoving = 0;
clock_t lastMoveKeyTime = 0;
const double moveIdleTimeout = 0.15;

/* ---------------- Drowning (Level 2 water) ---------------- */
/* Level 2's pits are filled with real water (water.png /
 * water_top.png - see images.h). Falling into one no longer just
 * drops the player to the bare floor: it starts a drowning sequence
 * instead. The player is pinned at the splash point, the drown
 * sprites (drown1..drown6.png) play while the prince sinks, and
 * health drains smoothly from whatever it was down to 0 over
 * DROWN_SECONDS - so the life bar visibly runs out rather than the
 * game cutting straight to GAME OVER. When it hits 0 the normal
 * STATE_GAMEOVER screen takes over, with gameOverFromLevel set so
 * RETRY restarts Level 2. */
#define DROWN_FRAME_COUNT 6
#define DROWN_SECONDS 4.0
#define DROWN_TOTAL_TICKS ((int)(DROWN_SECONDS * 60))

int drowning = 0;              /* 1 while the drown sequence is playing */
int drownSurfaceY = 0;         /* world Y of the water surface that was hit */
int drownY = 0;                /* sprite Y right now - sinks as time passes */
int drownTicks = 0;            /* logic ticks elapsed since the splash */
double drownDrainAccum = 0.0;  /* fractional health-drain carry-over */

/* ================================================================== */
/* HEALTH HEARTS (Levels 2 and 3)                                      */
/*                                                                     */
/* Hearts hang over the platforms; taking one restores 10% of the life */
/* bar. How many there are and where they hang is rolled fresh every   */
/* time the level is entered or retried, so the layout is never the    */
/* same twice. Level 2 always gets fewer than Level 3 - the ranges     */
/* below do not overlap, so that holds on every single roll.           */
/*                                                                     */
/* WHY YOU HAVE TO JUMP FOR THEM                                       */
/* A heart's centre floats HEART_FLOAT_HEIGHT (104) above the platform */
/* it hangs over, so its lowest pixel is at surface+88. The Prince      */
/* standing on that platform reaches surface+80 (charImgSize) - eight   */
/* pixels short. A 'W' press with nothing to climb adds                */
/* noClimbHopMaxHeight (45) on top of that, which clears it easily.     */
/* So walking underneath one does nothing and jumping takes it.         */
/* ================================================================== */
#define HEART_MAX            9    /* array size - covers the largest roll */
#define HEART_IMG_SIZE       32   /* heart.png is 32x32                   */
#define HEART_HEAL_DIV       10   /* restores playerMaxHealth/10 = 10%     */
#define HEART_FLOAT_HEIGHT   104  /* sprite centre, above the platform     */
#define HEART_REACH_X        40   /* how close horizontally counts as a grab */
#define HEART_MIN_SPACING    280  /* keep them spread out across the level */
#define HEART_CLEAR_SPAN     40   /* needs this much flat floor either side */
#define HEART_PLACE_TRIES    400  /* attempts per heart before giving up   */
#define HEART_MSG_TICKS      100  /* "+10% HEALTH" confirmation, in ticks  */

/* Level 2 gets fewer than Level 3, always - the two ranges are
 * deliberately disjoint so no roll can ever break that. */
#define LEVEL2_HEART_MIN     2
#define LEVEL2_HEART_MAX     3
#define LEVEL3_HEART_MIN     5
#define LEVEL3_HEART_MAX     7

HealthHeart heart[HEART_MAX];
int heartNo = 0;          /* how many slots spawnHearts() actually filled */
int heartSeed = 0;        /* running seed for the count and the placement */
int heartMsgTimer = 0;    /* ticks left on the "+10% HEALTH" confirmation */
int heartBobTick = 0;     /* drives the gentle float of the sprites       */

/* ================================================================== */
/* LEVEL 2 - CAVE BATS                                                 */
/*                                                                     */
/* Every so often a bat sweeps out of the dark and flies straight      */
/* across the cave at the Prince's head height. Ducking under it with  */
/* 'Q' lets it pass harmlessly overhead; standing there costs 20% of   */
/* the life bar.                                                       */
/*                                                                     */
/* Every name here is prefixed level2Bat / LEVEL2_ so none of it can   */
/* collide with the Level 1 / Level 3 state around it, and every       */
/* single read of it is gated on `gameState == STATE_LEVEL2`, so       */
/* Levels 1 and 3 behave exactly as they did before.                   */
/*                                                                     */
/* HOW THE DUCK DODGES IT                                              */
/* The Prince's sprite is charImgSize (80) tall standing and           */
/* LEVEL2_DUCK_HEIGHT (53) tall down on one knee. A bat flies with its */
/* centre LEVEL2_BAT_HEIGHT (76) above his feet and strikes anything   */
/* within LEVEL2_BAT_BAND (12) of that centre - so its strike zone is  */
/* feet+64 to feet+88. That overlaps the standing silhouette (0..80)   */
/* by 16px and clears the kneeling one (0..53) by 11px, which is why   */
/* the duck works and why standing still does not.                     */
/*                                                                    */
/* The bat used to fly lower (centre at 64) because the duck used to   */
/* be a 44px-tall squashed sprite. It is a real kneel now - him down   */
/* on one knee, which is naturally taller than that - so the bat was   */
/* raised to keep a clear margin on both sides of the test rather than */
/* squeezing the pose to fit the old number.                           */
/* ================================================================== */
#define LEVEL2_BAT_MAX        3    /* most bats allowed in the air at once  */
#define LEVEL2_BAT_IMG_SIZE   64   /* bat_l1.png..bat_r3.png are 64x64      */
#define LEVEL2_BAT_FRAMES     3    /* wings up / level / down               */
#define LEVEL2_BAT_ANIM_TICKS 5    /* logic ticks per wing-flap frame       */
#define LEVEL2_BAT_SPEED      7    /* world px travelled per tick           */
#define LEVEL2_BAT_HEIGHT     76   /* it cruises this far above his feet    */
#define LEVEL2_BAT_EASE       3    /* px/tick it closes on that height      */
#define LEVEL2_BAT_BAND       12   /* half-height of its strike zone        */
#define LEVEL2_BAT_REACH      34   /* half-width of its strike zone         */
#define LEVEL2_BAT_MARGIN     110  /* spawns this far outside the view      */

/* Gaps between bats, in logic ticks (60 = 1 second). The actual gap is
 * picked per bat from this range - see level2BatRandom() in controls.h. */
#define LEVEL2_BAT_GAP_MIN    330  /* ~5.5s                                 */
#define LEVEL2_BAT_GAP_MAX    780  /* ~13s                                  */
#define LEVEL2_BAT_FIRST_GAP  420  /* ~7s of quiet before the very first    */

/* A connecting bat costs 20% of the life bar. Written as a divisor of
 * playerMaxHealth rather than a raw number so it stays 20% if the max
 * health is ever retuned. */
#define LEVEL2_BAT_DAMAGE_DIV 5

#define LEVEL2_DUCK_TICKS     42   /* how long one 'Q' press stays crouched */
#define LEVEL2_DUCK_HEIGHT    53   /* kneeling sprite height (standing: 80) */
#define LEVEL2_BAT_FLASH      18   /* ticks of red screen flash after a hit */

Level2Bat level2Bat[LEVEL2_BAT_MAX];

int level2BatTimer = 0;       /* ticks until the next bat is released     */
int level2BatSeed = 0;        /* running seed for the spawn gaps/sides    */
int level2DuckTimer = 0;      /* >0 while the Prince is crouched          */
int level2BatHitFlash = 0;    /* ticks left on the "a bat got you" flash  */

/* The 'Q' duck is a new control and the instructions screen (inst.png)
 * is a fixed image, so the first couple of bats in a run announce
 * themselves with a short on-screen prompt instead. After that the
 * player knows, and the cave goes quiet again. */
#define LEVEL2_BAT_HINT_TICKS 110
#define LEVEL2_BAT_HINT_COUNT 2
int level2BatHintTimer = 0;
int level2BatHintsLeft = LEVEL2_BAT_HINT_COUNT;

/* ================================================================== */
/* LEVEL 3 - "THE FINAL ESCAPE"                                        */
/*                                                                     */
/* Everything below is new and every single name is prefixed level3    */
/* (or LEVEL3_) so it can never collide with the Level 1 / Level 2     */
/* state above. All of it is rebuilt from scratch by setupLevel3() in  */
/* next_level.h, which resetGame() calls on every RETRY, so no Level 3 */
/* state can survive a death and no Level 2 state can leak in.         */
/*                                                                     */
/* The level is a flooding, collapsing palace:                         */
/*   - collapsing platforms (level3Collapse[]) that give way under the */
/*     Prince after a shaking warning,                                 */
/*   - one single rising flood (level3WaterLevel) that reuses the      */
/*     EXISTING Level 2 drowning sequence rather than adding a second  */
/*     water system - see waterSurfaceAt()/startDrowning() in          */
/*     controls.h, which now answer for Level 3 too,                   */
/*   - five stages whose pressure ramps with the player's progress,    */
/*   - one multi-phase final Guardian,                                 */
/*   - and a timed escape run to the palace door.                      */
/* ================================================================== */

/* ---------------- Collapsing platforms ---------------- */
#define LEVEL3_CP_SOLID     0   /* stable, full collision                 */
#define LEVEL3_CP_SHAKING   1   /* warning: still solid, visibly shaking  */
#define LEVEL3_CP_FALLING   2   /* collision ALREADY gone, debris falling */
#define LEVEL3_CP_GONE      3   /* nothing left at all                    */

#define LEVEL3_CP_COUNT       9
#define LEVEL3_CP_FALL_TICKS  42    /* how long the debris keeps dropping */
#define LEVEL3_CP_WIDTH       128   /* every slab is exactly 2 tiles wide */

CollapsePlatform level3Collapse[LEVEL3_CP_COUNT];
int level3CollapseNo = 0;

/* ---------------- Rising flood ---------------- */
/* All heights below are in the same world-Y units as Wall::h, i.e.
 * "the surface the Prince's feet stand on". The Prince starts Level 3
 * on a ledge at height 170 and finishes on the palace roof at 470. */
#define LEVEL3_WATER_START        40
#define LEVEL3_WATER_RISE_TICKS    6   /* logic ticks per 1px of rise     */
/* The escape flood is slower per pixel than the staged rise but has no
 * target to stop at. From the phase-4 level (430) it takes about 27
 * seconds to reach the height that drowns a player standing on the
 * 470-high escape corridor - comfortably more than the ~14 seconds the
 * run to the door actually needs, so it is a real deadline rather than
 * an unfair one. */
#define LEVEL3_WATER_ESCAPE_TICKS 22
/* How far the water has to climb ABOVE the floor the Prince is standing
 * on before it drags him under. 34px is roughly waist-deep on the 80px
 * sprite, so a flooded ledge looks dangerous for a moment before it
 * actually becomes fatal - the water never one-shots the player. */
#define LEVEL3_SUBMERGE_MARGIN    34
/* The slow "the palace never stops flooding" creep that sits underneath
 * the per-stage water targets - 40 seconds of grace, then 1px every
 * 0.8s. That is roughly a tenth of the speed the stage targets move at,
 * so it is invisible to a player who is making progress and only ever
 * matters to one who has stranded themselves. See the creep block in
 * updateLevel3() for why it exists. */
#define LEVEL3_CREEP_GRACE      2400
#define LEVEL3_CREEP_TICKS        48
#define LEVEL3_CREEP_MAX         620

int level3Active     = 0;   /* 1 while Level 3 is the level being played  */
int level3Timer      = 0;   /* logic ticks since Level 3 started          */
int level3Stage      = 1;   /* 1..5, only ever increases                  */
int level3WaterLevel = LEVEL3_WATER_START;
int level3WaterTarget = LEVEL3_WATER_START;
int level3WaterTick  = 0;   /* sub-pixel accumulator for the rise         */
int level3WaterRiseTicks = LEVEL3_WATER_RISE_TICKS;
int level3MaxPlayerX = 0;   /* furthest x reached - drives the stages     */

/* ---------------- Collapse / drama bookkeeping ---------------- */
int level3CollapseState = 0;  /* how many slabs have fallen so far        */
int level3ShakeLevel    = 0;  /* 0..4 - how violently the palace shakes   */

/* ---------------- Final Guardian ---------------- */
/* 240 life against the player's 15-damage sword swing is 16 swings,
 * i.e. a ~17 second fight - long enough to move through all four
 * phases and feel like a boss, short enough that the arena has not
 * completely flooded by the end of it. */
#define LEVEL3_BOSS_MAX_LIFE      240
#define LEVEL3_BOSS_IMG_SIZE       96   /* bigger than the 64px mooks     */
#define LEVEL3_BOSS_ATK_IMG_SIZE  120
/* Deliberately longer reach than COMBAT_RANGE: half a player sprite +
 * half a (bigger) guardian sprite + a little extra for the sword. */
#define LEVEL3_BOSS_RANGE   (PLAYER_IMG_SIZE / 2 + LEVEL3_BOSS_IMG_SIZE / 2 + 14)
#define LEVEL3_BOSS_ATTACK_TICKS   24
#define LEVEL3_BOSS_DEATH_TICKS   120
#define LEVEL3_BOSS_HIT_FLASH      10
/* Same idea (and the same value) as ENEMY_ANIM_TICK_DIVISOR in
 * images.h: only advance the walk-cycle frame once every N logic ticks
 * so the Guardian does not animate in fast-forward. Declared here
 * rather than reused from images.h because the Guardian's movement
 * runs in controls.h, which is compiled before images.h. */
#define LEVEL3_BOSS_ANIM_DIVISOR    4

Guardian level3Boss;
int level3BossActive = 0;   /* 1 once the arena fight has begun           */
int level3BossPhase  = 1;   /* 1..4, only ever increases                  */

/* ---------------- Final escape ---------------- */
int level3EscapeMode  = 0;  /* 1 after the Guardian falls                 */
int level3EscapeTimer = 0;  /* logic ticks since the escape began         */
int level3ExitX       = 0;  /* world x of the palace door                 */
int level3ExitReached = 0;

/* ---------------- On-screen stage banner ---------------- */
int  level3BannerTimer = 0;
char level3BannerText[80] = "";

/* Level 3 world geometry that more than one file needs to agree on.
 * wallNo grows from _WALLS_MAIN to _WALLS_TOTAL the moment the escape
 * route opens, which is what makes the sealed gate behind the arena
 * become a real, walkable corridor. */
#define LEVEL3_WALLS_MAIN    12
#define LEVEL3_WALLS_TOTAL   15
#define LEVEL3_ARENA_BLOCK   11
#define LEVEL3_GATE_X      6550

/* ================================================================== */
/* LEVEL 3 - SWIMMING, CLIMBING OUT, AND THE PIRANHAS                  */
/*                                                                     */
/* Level 3's water is ordinary water now, not poison, and it does NOT  */
/* drown him: waterSurfaceAt() returns -1 for Level 3, which switches  */
/* the whole existing drowning path off for this level and leaves      */
/* Level 2's pits working exactly as they always did. What replaces it */
/* is this: he swims freely, piranhas hunt him while he is in there,   */
/* and 'R' pulls him out onto a ledge.                                 */
/*                                                                     */
/* Everything here is prefixed level3 / LEVEL3_ and every read of it   */
/* is gated on STATE_LEVEL3, so Levels 1 and 2 are untouched.          */
/* ================================================================== */

/* ---------------- Swimming ---------------- */
#define LEVEL3_SWIM_SPEED         4   /* world px per tick, horizontal      */
#define LEVEL3_SWIM_RISE          4   /* world px per tick, vertical        */
#define LEVEL3_SWIM_ENTER_DEPTH  30   /* how far under before he is swimming*/
#define LEVEL3_SWIM_SURFACE_GAP  16   /* where he floats to, below the top  */
#define LEVEL3_SWIM_INPUT_HOLD    9   /* ticks a stroke suppresses the float*/

/* Pulling himself out of the water with 'R'. */
#define LEVEL3_CLIMB_REACH       96   /* how far either side to look        */
#define LEVEL3_CLIMB_MAX_RISE   150   /* the tallest ledge he can manage    */

/* ---------------- Piranhas ---------------- */
#define LEVEL3_FISH_MAX          24   /* a shoal, not a handful             */
#define LEVEL3_FISH_IMG_SIZE     64   /* fish_l1.png .. fish_r3.png are 64  */
#define LEVEL3_FISH_DRAW         40   /* drawn a little smaller than the art*/
#define LEVEL3_FISH_FRAMES        3
#define LEVEL3_FISH_ANIM_TICKS    6
#define LEVEL3_FISH_SPEED         3   /* cruising                           */
#define LEVEL3_FISH_CHASE_SPEED   6   /* closing in for the bite            */

/* A piranha far from him swims much harder to close the gap, so wherever
 * he goes in they converge on him quickly instead of the distant ones
 * pottering over. There is no detection range any more: the moment he is
 * in the water the WHOLE shoal comes, from anywhere in the level. */
#define LEVEL3_FISH_FAR_BOOST     9
#define LEVEL3_FISH_FAR_DIST    380

/* With no prey they cruise to a spot picked anywhere in the flooded
 * level, then pick another - so the shoal is spread through the whole
 * body of water rather than each fish bouncing back and forth in the
 * pocket it happened to start in. */
#define LEVEL3_FISH_ROAM_MIN    180
#define LEVEL3_FISH_ROAM_MAX    420
#define LEVEL3_FISH_ROAM_TRIES   60

/* While the flood is still low, stretches of the level genuinely are not
 * joined up by water - a platform whose body reaches the bed is a wall
 * with no way round it. A fish hunting him through one of those would
 * swim at it forever. So: a fish that has spent this long chasing
 * without getting any closer gives up and rejoins the hunt from a fresh
 * spot near him - far enough off (LEVEL3_FISH_REJOIN_DIST) that it swims
 * into view rather than appearing on top of him. That is what makes
 * "wherever he falls in, they come" true even in a cut-off pocket. */
#define LEVEL3_FISH_STALL_TICKS 150
#define LEVEL3_FISH_STALL_GAP   300
#define LEVEL3_FISH_REJOIN_DIST 420
#define LEVEL3_FISH_BITE_X       36   /* contact box, half-width            */
#define LEVEL3_FISH_BITE_Y       32   /* contact box, half-height           */
/* Half a fish. Every limit on where one may sit is this: half a body
 * below the waterline, half a body clear of any stonework. Stating it
 * once, as the real half-height of the sprite, makes the geometry exact
 * - a fish fits in a gap exactly when the gap is as tall as the fish -
 * instead of the two hand-tuned margins it replaces, which between them
 * needed 44px of water and so could not fit into the ~41px the flood
 * leaves over the eastern arena late in the level. */
#define LEVEL3_FISH_HALF     (LEVEL3_FISH_DRAW / 2)

/* THE WATER IS ONE OPEN SWIMMING AREA. THE PLATFORMS ARE NOT FISH
 * BOUNDARIES.
 *
 * Earlier versions treated platforms as obstacles a fish had to route
 * around - first the whole column under height[x], then just the slab a
 * platform is drawn as. Both partitioned the level: Level 3 steps up
 * eastward (platform tops 170, 280, 380, 470), so at most water levels
 * some platform reached from the bed to the surface and cut the water in
 * two, penning the shoal into whichever section it was in.
 *
 * So there is no routing any more. A piranha may be anywhere the water
 * is: the full width of the level, from the bed up to the surface. The
 * platforms are obstacles for the PRINCE - his collision is completely
 * untouched - and for the fish they are only scenery, which is why
 * drawLevel3Fish() is now drawn BEFORE the walls. A fish behind a
 * platform is hidden by it instead of being drawn over the brickwork. */
#define LEVEL3_FISH_MIN_X        60
#define LEVEL3_FISH_MAX_X      6800

/* When he goes in, the shoal turns on him: for a few seconds they swim
 * faster and can smell him from right across the level. */
#define LEVEL3_FISH_FRENZY_TICKS  260
#define LEVEL3_FISH_FRENZY_BOOST    3

/* ---- keeping the shoal from piling up on one pixel ----
 *
 * Every fish used to home on the exact same point - the middle of the
 * Prince - so all two dozen ended up stacked on top of each other in a
 * heap. Two things stop that now:
 *
 * 1. SLOTS. Only LEVEL3_FISH_ATTACKERS of them close in to bite, and
 *    they take up evenly spaced positions on a small ring around him
 *    rather than all aiming at his middle. Everything else holds back
 *    and circles at a stand-off distance. The slot each fish holds
 *    rotates every LEVEL3_FISH_ROTATE ticks, so they take turns at the
 *    front instead of the same five always having him.
 *
 * 2. SEPARATION. After everything has moved, any two fish inside each
 *    other's personal space get pushed apart - the same trick a flocking
 *    model uses, just done as one cheap pass over the pairs. */
#define LEVEL3_FISH_ATTACKERS       5
#define LEVEL3_FISH_RING_X         34   /* attacker ring, inside the bite box */
#define LEVEL3_FISH_RING_Y         24
#define LEVEL3_FISH_STANDOFF      130   /* the rest hold at least this far off */
#define LEVEL3_FISH_STANDOFF_STEP  18
#define LEVEL3_FISH_STANDOFF_MAX  310
#define LEVEL3_FISH_ROTATE        200   /* ticks before the attackers change  */
#define LEVEL3_FISH_SEP_X          46   /* personal space, half-width         */
#define LEVEL3_FISH_SEP_Y          30
#define LEVEL3_FISH_SEP_PUSH        2   /* px each is shoved per tick         */

/* Being in the water costs health the whole time he is in it, whether or
 * not a piranha has actually got hold of him this instant - the shoal is
 * always worrying at him. This is the slow background rate:
 * max health / 25 per second = 4%/sec, so a full bar lasts 25 seconds of
 * swimming. Getting properly bitten replaces it with the much faster
 * LEVEL3_BITE_PERCENT_DIV rate below. If it runs out before he reaches a
 * wall and climbs, the existing `playerHealth <= 0` check ends the game
 * exactly as any other cause of death does. */
#define LEVEL3_WATER_DRAIN_DIV     25

/* Being bitten costs 20% of the life bar per second.
 *
 * This is a TOTAL, not a per-fish rate: with a shoal this size, four or
 * five piranhas reaching him at once would otherwise be 80-100% a second
 * and the water would be instant death rather than something he can
 * survive and swim out of. One set of teeth in him or six, it drains at
 * the same 20%/sec - so he has about five seconds to reach a ledge.
 * The fraction is carried between ticks in level3BiteAccum, exactly like
 * the enemy and drown damage already do. */
#define LEVEL3_BITE_PERCENT_DIV   5
#define LEVEL3_BITE_TICKS        60
#define LEVEL3_BITE_FLASH        14

/* Player body offset used by the fish contact test: playerY is his feet,
 * and a fish aims for the middle of him. */
#define LEVEL3_BODY_MID          34

Level3Fish level3Fish[LEVEL3_FISH_MAX];
int level3FishNo = 0;
int level3FishSeed = 0;

int level3Swimming = 0;        /* 1 while he is in the water            */
int level3SwimInput = 0;       /* ticks left suppressing the float-up   */
int level3ClimbX = -1;         /* ledge 'R' would take, -1 = none       */
int level3ClimbY = 0;
int level3BiteFlash = 0;       /* ticks left on the "bitten" flash      */
int level3FishFrenzy = 0;      /* ticks left of the shoal turning on him*/
double level3BiteAccum = 0.0;  /* fractional bite damage carried over   */

/* ---------------- Reset backups ---------------- */
Enemy enemyBackup[1000];
int EnemyNoBackup;

Button buttonBackup[500];
JumpingWall jWallBackup[100];
MovingWall mWallBackup[100];

/* ---------------- Combat runtime ---------------- */
int playerAtkLastFrame = -1;
double enemyDamageAccum = 0.0;

/* ---------------- Fixed logic timing ---------------- */
clock_t lastUpdateTime = 0;
const double targetUpdateInterval = 1.0 / 60.0;

/* Cross-header declarations */
void update();
void playerHeight();
int getHeight(int tx, int ty);
void resetGame();
void setupLevel2();
void setupLevel1();
void setupLevel3();

void iKeyboard(unsigned char key);
void iSpecialKeyboard(unsigned char key);
void iMouseMove(int mx, int my);
void iMouse(int mouseButton, int state, int mx, int my);

void preloadAttackImages();
void advanceEnemyAnim(Enemy *e);

void applyPlayerStrikeDamage();
void applyEnemyStrikeDamage(int enemyIndex);

void handlePButton();
void handleOButton();
void handleHomeMouse(int mouseButton, int state, int mx, int my);

/* Level 2 bats - logic lives in controls.h, drawing lives in images.h,
 * the per-level reset lives in next_level.h. Same split as Level 3.
 *
 * charImgSize is defined in images.h, which main.cpp includes AFTER
 * controls.h; declared here so the bat's strike test can measure
 * itself against the real sprite height instead of hard-coding 80 in a
 * second place. */
extern int charImgSize;
void updateLevel2Bats();
void clearLevel2Bats();
void drawLevel2Bats();
void drawDuck(int centerX, int y, int k);

/* Health hearts - same split: logic in controls.h, drawing in images.h,
 * spawned per level from next_level.h. */
int  nextRandom(int *seed, int span);
void clearHearts();
void spawnHearts(int count, int minX, int maxX);
void updateHearts();
void drawHearts();

/* Level 3 - logic lives in controls.h, drawing lives in images.h,
 * level data lives in next_level.h. Same split the rest of the game
 * already uses. */
void level3StampCollapse();
void updateLevel3();
int  level3GuardianBlocksPlayer(int dir);
void level3SetBanner(const char *text, int ticks);
void drawLevel3WaterBody();
void drawLevel3CollapsePlatforms();
void drawLevel3Structures();
void drawLevel3Guardian();
void drawLevel3Effects();
int  level3ShakeOffset();

/* Level 3 water: swimming, the climb out, and the piranhas. */
void level3UpdateSwim();
void level3SpawnFish();
void level3PickRoam(int i);
void clearLevel3Water();
void drawLevel3Fish();
void drawLevel3SeaTint(int left, int right, int surfaceY);

/* Drowning helpers - defined in controls.h, used by images.h/reset.h */
int waterSurfaceAt(int x);
void startDrowning(int surfaceY);
void updateDrowning();
void clearDrowning();

#endif
