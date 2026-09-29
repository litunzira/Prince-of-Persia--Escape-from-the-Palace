#ifndef RECORDS_H
#define RECORDS_H

#include "game_globals.h"

/* ==================================================================== */
/* LEVEL TIMER AND BEST-RECORD SYSTEM                                    */
/*                                                                       */
/* One timer and one record table serve every level, so adding Level 4   */
/* or Level 5 later needs no new timing code at all - only a new entry   */
/* in levelNumberForState() below and a bigger RECORD_LEVEL_COUNT.       */
/*                                                                       */
/* Timing uses QueryPerformanceCounter, so the recorded time is real     */
/* elapsed time and is completely independent of the frame rate and of   */
/* the 60Hz update() throttle. GetTickCount is kept only as a fallback   */
/* for the (practically impossible) case of a machine with no            */
/* performance counter.                                                  */
/*                                                                       */
/* Everything is kept in hundredths of a second, which is the precision  */
/* the game displays (MM:SS.hh) and stores.                              */
/* ==================================================================== */

/* Highest level number the record table can hold. Raise this freely -
 * nothing else has to change, and a file holding levels above it is
 * still read back and rewritten untouched for those it does know. */
#define RECORD_LEVEL_COUNT   8

/* How many levels the RECORDS screen lists. The game has three; bump
 * this when a fourth is added. */
#define RECORD_LEVELS_SHOWN  3

/* Relative path, resolved against the working directory - never an
 * absolute path, so the game stays portable. */
#define RECORD_FILE_NAME     "best_records.txt"

/* A level that has never been completed. */
#define RECORD_NONE          (-1L)

/* Timer states. Idle means "not started yet this attempt", which is
 * what lets update() start the clock on the first real gameplay tick
 * without needing a separate "level just began" hook. */
#define LEVEL_TIMER_IDLE     0
#define LEVEL_TIMER_RUNNING  1
#define LEVEL_TIMER_STOPPED  2

long bestRecord[RECORD_LEVEL_COUNT + 1];   /* index by level number, 1..N */

int  levelTimerState = LEVEL_TIMER_IDLE;
int  levelTimerLevel = 0;                  /* level currently being timed */
long levelTimerFrozen = 0;                 /* hundredths, once stopped    */

static LARGE_INTEGER levelTimerFreq;
static LARGE_INTEGER levelTimerOrigin;
static int           levelTimerHaveQpc = 0;
static DWORD         levelTimerTickOrigin = 0;

/* Details of the run that has just finished, for the completion screen. */
int  lastRunLevel      = 0;
long lastRunTime       = 0;
int  lastRunNewRecord  = 0;
long lastRunBest       = RECORD_NONE;      /* best AFTER this run counted */

/* -------------------------------------------------------------------- */
/* Which level number a gameplay state represents.                       */
/* -------------------------------------------------------------------- */
inline int levelNumberForState(int state)
{
	if (state == STATE_GAME)   return 1;
	if (state == STATE_LEVEL2) return 2;
	if (state == STATE_LEVEL3) return 3;
	return 0;                                /* not a gameplay state */
}

/* -------------------------------------------------------------------- */
/* MM:SS.hh - always eight characters, so columns line up.               */
/* out must have room for at least 16 bytes.                             */
/* -------------------------------------------------------------------- */
inline void formatLevelTime(long hundredths, char *out)
{
	long mins, secs, cents;

	if (hundredths < 0){                     /* no record yet */
		strcpy(out, "--:--.--");
		return;
	}

	cents = hundredths % 100;
	secs  = (hundredths / 100) % 60;
	mins  = hundredths / 6000;
	if (mins > 99) mins = 99;                /* keep the field width fixed */

	sprintf(out, "%02ld:%02ld.%02ld", mins, secs, cents);
}

/* -------------------------------------------------------------------- */
/* Timing                                                                */
/* -------------------------------------------------------------------- */
inline void initLevelTimer()
{
	levelTimerHaveQpc = QueryPerformanceFrequency(&levelTimerFreq) &&
	                    levelTimerFreq.QuadPart > 0;
	levelTimerState = LEVEL_TIMER_IDLE;
	levelTimerFrozen = 0;
	levelTimerLevel = 0;
}

// Zeroes the clock for a fresh attempt. Called from resetGame(), so every
// start and every retry begins from nothing. It deliberately does NOT
// touch bestRecord[] - a restart never costs the player their record.
inline void resetLevelTimer()
{
	levelTimerState = LEVEL_TIMER_IDLE;
	levelTimerFrozen = 0;
	levelTimerLevel = 0;
}

inline void startLevelTimer(int levelNo)
{
	levelTimerLevel = levelNo;
	levelTimerFrozen = 0;
	levelTimerState = LEVEL_TIMER_RUNNING;

	if (levelTimerHaveQpc) QueryPerformanceCounter(&levelTimerOrigin);
	else                   levelTimerTickOrigin = GetTickCount();
}

inline long getElapsedLevelTime()
{
	if (levelTimerState != LEVEL_TIMER_RUNNING)
		return levelTimerFrozen;             /* 0 when idle, final when stopped */

	if (levelTimerHaveQpc){
		LARGE_INTEGER now;
		QueryPerformanceCounter(&now);
		return (long)(((now.QuadPart - levelTimerOrigin.QuadPart) * 100LL) /
		              levelTimerFreq.QuadPart);
	}
	return (long)((GetTickCount() - levelTimerTickOrigin) / 10);
}

inline void stopLevelTimer()
{
	if (levelTimerState != LEVEL_TIMER_RUNNING) return;
	levelTimerFrozen = getElapsedLevelTime();
	levelTimerState = LEVEL_TIMER_STOPPED;
}

/* ==================================================================== */
/* PHYSICAL LEVEL PROGRESS                                               */
/*                                                                       */
/* Completely separate from the clock above. The clock measures elapsed  */
/* time; this measures how far through the level the Prince actually is, */
/* and the two never read each other. Standing still for a minute moves  */
/* the clock and not this; sprinting east moves this and not the rate of */
/* the clock.                                                            */
/*                                                                       */
/* It is derived from playerX, the WORLD x - not a screen position - so  */
/* the camera scrolling (cX) has no effect on it. All three levels run   */
/* west to east from x=300.                                              */
/*                                                                       */
/* The end point is whatever the level's real completion condition       */
/* measures, so the bar is full exactly when the level is actually won:  */
/*                                                                       */
/*   Level 3  ends at the palace door, a world x (level3ExitX), so that  */
/*            x is the end point directly.                               */
/*   Levels 1 ends when the last guard falls. The guards are strung out  */
/*   and 2   along the level, so the furthest one is the point the       */
/*            player physically has to fight their way to; that x is     */
/*            taken straight from the enemy roster the level just built, */
/*            which means it follows the level design rather than a      */
/*            number copied out of it.                                   */
/*                                                                       */
/* Whatever the span says, the real completion condition is still the    */
/* source of truth: when it fires it calls markLevelProgressComplete()   */
/* and the bar reads exactly 1.0 - never 96% or 98%.                     */
/* ==================================================================== */
int levelProgressStartX = 300;
int levelProgressEndX   = 3250;
int levelProgressDone   = 0;

inline void setLevelProgressSpan(int startX, int endX)
{
	levelProgressStartX = startX;
	levelProgressEndX   = (endX > startX + 1) ? endX : (startX + 1);
	levelProgressDone   = 0;
}

// For the levels that end when the roster is cleared.
inline void setLevelProgressSpanFromEnemies(int startX)
{
	int i, furthest = startX;
	for (i = 0; i < EnemyNo; i++)
		if (enemy[i].x > furthest) furthest = enemy[i].x;
	setLevelProgressSpan(startX, furthest);
}

// Called from the level's own completion code, so the bar is full at the
// same instant the level is won.
inline void markLevelProgressComplete()
{
	levelProgressDone = 1;
}

// Back to empty for a new attempt. A respawn at a checkpoint is NOT a
// reset - the bar simply reads the player's new world x, which is what
// makes it fall back when he returns to an earlier part of the level.
inline void resetLevelProgress()
{
	levelProgressDone = 0;
}

inline double getLevelProgress()
{
	double p;

	if (levelProgressDone) return 1.0;
	if (levelProgressEndX <= levelProgressStartX) return 0.0;

	p = (double)(playerX - levelProgressStartX) /
	    (double)(levelProgressEndX - levelProgressStartX);

	if (p < 0.0) p = 0.0;
	if (p > 1.0) p = 1.0;
	return p;
}

/* -------------------------------------------------------------------- */
/* Best records, persisted in best_records.txt                           */
/*                                                                       */
/* One line per level:   LEVEL 1 35.81                                   */
/* (the number is seconds with hundredths, matching what is displayed).  */
/* Only levels that have actually been completed get a line, so a fresh  */
/* install starts with an empty - but present - file.                    */
/* -------------------------------------------------------------------- */
inline long getBestRecord(int levelNo)
{
	if (levelNo < 1 || levelNo > RECORD_LEVEL_COUNT) return RECORD_NONE;
	return bestRecord[levelNo];
}

// Rewrites the whole file from the in-memory table, so records for
// levels other than the one just beaten are carried over untouched.
inline void writeBestRecords()
{
	FILE *f = fopen(RECORD_FILE_NAME, "w");
	int i;

	if (!f) return;                          /* read-only folder: play on */

	for (i = 1; i <= RECORD_LEVEL_COUNT; i++){
		if (bestRecord[i] < 0) continue;
		fprintf(f, "LEVEL %d %ld.%02ld\n", i,
		        bestRecord[i] / 100, bestRecord[i] % 100);
	}
	fclose(f);
}

// Reads the file if it is there, and creates it if it is not. Anything
// that does not parse as a sane "LEVEL <n> <seconds>" line is skipped,
// so a truncated, empty, hand-edited or corrupted file can never stop
// the game starting - it just means those records are missing.
inline void loadBestRecords()
{
	FILE *f;
	char line[256];
	int i;

	for (i = 0; i <= RECORD_LEVEL_COUNT; i++)
		bestRecord[i] = RECORD_NONE;

	f = fopen(RECORD_FILE_NAME, "r");
	if (!f){
		writeBestRecords();                  /* create it, empty */
		return;
	}

	while (fgets(line, sizeof(line), f)){
		int lvl = 0;
		double secs = -1.0;

		if (sscanf(line, "LEVEL %d %lf", &lvl, &secs) != 2) continue;
		if (lvl < 1 || lvl > RECORD_LEVEL_COUNT) continue;
		if (secs < 0.0 || secs > 359999.0) continue;   /* < 100 hours */

		{
			long h = (long)(secs * 100.0 + 0.5);
			/* keep the fastest if the file somehow lists a level twice */
			if (bestRecord[lvl] < 0 || h < bestRecord[lvl])
				bestRecord[lvl] = h;
		}
	}
	fclose(f);
}

// LOWER TIME = BETTER. Returns 1 when this run set a new record (which
// includes the first ever completion of that level), 0 when the existing
// record was faster and is therefore kept.
inline int saveBestRecord(int levelNo, long hundredths)
{
	if (levelNo < 1 || levelNo > RECORD_LEVEL_COUNT) return 0;
	if (hundredths < 0) return 0;

	if (bestRecord[levelNo] >= 0 && bestRecord[levelNo] <= hundredths)
		return 0;                            /* slower - never replace */

	bestRecord[levelNo] = hundredths;
	writeBestRecords();
	return 1;
}

/* -------------------------------------------------------------------- */
/* One call finishes a run: stop the clock, work out whether it beat the */
/* record, save it if so, and leave everything the completion screen     */
/* needs in lastRun*. Used by every level, which is why neither level's  */
/* completion code has any timing logic of its own.                      */
/* -------------------------------------------------------------------- */
inline void finishLevelRun(int levelNo)
{
	stopLevelTimer();

	lastRunLevel     = levelNo;
	lastRunTime      = getElapsedLevelTime();
	lastRunNewRecord = saveBestRecord(levelNo, lastRunTime);
	lastRunBest      = getBestRecord(levelNo);
}

#endif
