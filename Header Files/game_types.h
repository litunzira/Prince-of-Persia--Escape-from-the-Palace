#ifndef GAME_TYPES_H
#define GAME_TYPES_H

struct Button{
    int x;
    int y;
    int OnOff;
};

struct Enemy{
    int x; //Enemy's horizontal position
    int y; //Enemy's vertical position
    int life; //Enemy health
    int blockNo; //Platform/block where enemy belongs
    int p; //Enemy movement/state information
    int k; //Direction/animation-related state
    int a; //Enemy AI/state information
    int m; //Additional movement/state information
    int dir; //Enemy movement direction
    int atkAnim; //Enemy attack animation state
    int animCounter; //Controls enemy animation timing
};

struct Block{
    int start;
    int end;
};

struct Wall{
    int sPos;
    int ePos;
    int h;
    // Which block[] id this platform's floor is stamped into
    // height[]/blockData[] as.
    //
    // MovingWall and JumpingWall have always carried their block id;
    // static walls did not, which meant nothing at runtime could work
    // out "this rock belongs to block N". That mattered because a
    // moving bridge clears its whole travel rail out of the collision
    // map on every tick, and in Levels 1 and 2 some rails overlap the
    // next rock by 128px - so that strip of rock was drawn but had no
    // floor under it. Now the bridge can put back exactly what it
    // erased. See the Moving Wall section of update() in controls.h.
    int blockNo;
};

struct JumpingWall{
    int Pos;
    int upperLimit;
    int lowerLimit;
    int d;
    int h;
    int buttonNo;
    int blockNo;
    // Same idea as MovingWall::alwaysOn - when set, this elevator moves
    // up/down on its own forever, with no switch gating it at all.
    int alwaysOn;
};

struct MovingWall{
    int left;
    int right;
    int h;
    int d;
    int current;
    int buttonNo;
    int blockNo;
    // When set, this moving wall runs on its own (back and forth,
    // forever) and is never gated by any button/switch at all - used
    // for a bridge whose controlling switch has been removed from the
    // level. Left at its default 0 (global structs are zero-initialized)
    // for every other wall, which still requires its switch as before.
    int alwaysOn;
};


// LEVEL 2 - cave bats.                                               

struct Level2Bat{
    int active;      /* 0 = free slot                                 */
    int x, y;        /* world position; y is the centre of the sprite */
    int dir;         /* +1 = flying right, -1 = flying left           */
    int frame;       /* wing-flap frame, 0..LEVEL2_BAT_FRAMES-1       */
    int animCounter; /* flap throttle (same idea as Enemy::animCounter)*/
    int struck;      /* 1 once it has connected - one hit per bat     */
};

/* ---------------------------------------------------------------- */
/* Health heart - a pickup that restores 10% of the life bar.         */
/*                                                                    */
/* Used by Levels 2 and 3 only. Where they are and how many there are */
/* is decided fresh every time the level is entered or retried (see   */
/* spawnHearts() in controls.h), so no two runs lay them out the same */
/* way. y is the CENTRE of the sprite, and it deliberately floats      */
/* above standing reach: the only way to take one is to jump.         */
/* ---------------------------------------------------------------- */
struct HealthHeart{
    int active;   /* 0 = taken, or this slot is unused */
    int x, y;     /* world position; y is the sprite centre */
};

/* ================================================================== */
/* LEVEL 3 - "THE FINAL ESCAPE"                                        */
/*                                                                     */
/* Two brand-new structures, deliberately kept SEPARATE from Wall /    */
/* JumpingWall / MovingWall above so that none of the existing Level 1 */
/* / Level 2 platform logic had to change at all. Everything that      */
/* reads these lives behind a `gameState == STATE_LEVEL3` check.       */
/* ================================================================== */

/* ---------------------------------------------------------------- */
/* CollapsePlatform - a stone slab that gives way underneath the      */
/* Prince. It goes SOLID -> SHAKING (visual warning) -> FALLING       */
/* (collision already removed, debris still dropping) -> GONE.        */
/*                                                                    */
/* The collision for one of these is written into the SAME            */
/* height[]/blockData[] arrays every other platform in the game uses  */
/* (see level3StampCollapse() in controls.h), so the player's own      */
/* collision, the enemy patrol-range scan, and the "where does this   */
/* platform end" bookkeeping all keep using one single source of      */
/* truth - and the instant a slab stops being SOLID/SHAKING its tiles */
/* are cleared back to height 0 / blockData -1, so there is never any */
/* invisible floor left behind.                                       */
/* ---------------------------------------------------------------- */
struct CollapsePlatform{
    int sPos;        /* left world x (inclusive)                       */
    int ePos;        /* right world x (exclusive)                      */
    int h;           /* surface height - same meaning as Wall::h       */
    int blockNo;     /* id stamped into blockData[] for this slab      */
    int warnTicks;   /* how long it shakes before dropping (~60 = 1s)  */
    int enabled;     /* 0 = not part of the world yet (escape route)   */
    int forcedOnly;  /* 1 = the Prince's weight does NOT trigger it;   */
                     /* only the script can (the Guardian fight brings */
                     /* the arena's own ledges down this way), so the  */
                     /* player can cross it safely until then          */
    int state;       /* LEVEL3_CP_SOLID / _SHAKING / _FALLING / _GONE  */
    int timer;       /* ticks left in the current state                */
    int fallY;       /* how far the debris has dropped (visual only)   */
    int shakeSeed;   /* per-slab jitter offset so they never shake in  */
                     /* lockstep with each other                       */
};

/* ---------------------------------------------------------------- */
/* Level3Fish - a piranha in Level 3's water.                         */
/*                                                                    */
/* Kept out of the enemy[] roster for the same reasons the Guardian   */
/* and the Level 2 bats are: the roster drives the "all enemies dead  */
/* -> level complete" rule and the sword-fighting code, and a piranha */
/* is neither killable nor something the Prince can fight - it is a   */
/* hazard he swims away from. Everything that reads this is behind a  */
/* `gameState == STATE_LEVEL3` check.                                 */
/* ---------------------------------------------------------------- */
struct Level3Fish{
    int active;
    int x, y;          /* world position; y is the sprite centre       */
    int dir;           /* +1 swimming right, -1 swimming left          */
    int frame;         /* tail-swish frame                             */
    int animCounter;   /* swish throttle                               */
    int wanderSeed;    /* per-fish phase, so they never shoal in step  */
    int biting;        /* ticks left on its bite effect                */
    int roamX, roamY;  /* where it is cruising to when it has no prey  */
    int roamTimer;     /* ticks until it picks somewhere new           */
    int lastGap;       /* how far from the Prince it was last tick     */
    int stall;         /* ticks spent hunting without getting closer   */
};

/* ---------------------------------------------------------------- */
/* Guardian - the single final boss of Level 3. It is NOT part of the */
/* enemy[] roster on purpose: the roster drives the "all enemies dead */
/* -> LEVEL COMPLETE" rule and draws a life bar whose pixel length is */
/* the raw life value, neither of which suits a 300+ HP boss whose    */
/* death has to start the escape sequence instead of ending the       */
/* level. It still reuses the existing enemy ART (drawEnemy() /       */
/* drawEnemyAttack()) and the existing enemyPatrolRange() floor scan, */
/* so it can never walk off the arena it is standing on.              */
/* ---------------------------------------------------------------- */
struct Guardian{
    int x, y;
    int life, maxLife;
    int blockNo;      /* the arena block it is allowed to stand on     */
    int k;            /* 0 = facing left, 13 = facing right (Enemy::k) */
    int p;            /* walk-cycle frame counter                      */
    int animCounter;  /* walk-cycle throttle (same idea as Enemy)      */
    int a;            /* 1 while a swing is playing                    */
    int atkAnim;      /* ticks left in the current swing               */
    int m;            /* attack cooldown counter                       */
    int dir;          /* patrol direction when idle                    */
    int active;       /* 1 once the Prince has entered the arena       */
    int hitFlash;     /* ticks left of the "just took a hit" reaction  */
    int dying;        /* 1 while the death animation plays             */
    int deathTimer;   /* ticks left of the death animation             */
    int dead;         /* 1 once the death animation has finished       */
};

#endif
