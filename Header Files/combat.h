#ifndef COMBAT_H
#define COMBAT_H

#include "game_globals.h"
#include "sounds.h"

#define PLAYER_ATTACK_TICKS 65
#define ENEMY_ATTACK_TICKS 20

// Player sword striking power. 
#define PLAYER_STRIKE_POWER 5

// Enemy striking power reduced
#define ENEMY_BASE_DAMAGE 1
#define ENEMY_DAMAGE_MULTIPLIER (1.0 / 4.0)
//calculates the next amount of damage an ordinary enemy should deal.
inline int nextEnemyDamage() 
{
    enemyDamageAccum += ENEMY_BASE_DAMAGE * ENEMY_DAMAGE_MULTIPLIER;
    int dmg = (int)enemyDamageAccum;
    enemyDamageAccum -= dmg;
    return dmg;
}


// LEVEL 3 - FINAL GUARDIAN COMBAT                                     

inline int level3GuardianInPlayerReach()
{
    if (gameState != STATE_LEVEL3 || !level3Active) return 0;
    if (!level3BossActive) return 0;
    if (level3Boss.dead || level3Boss.dying || level3Boss.life <= 0) return 0;
    /* Must be fighting on the same floor - no hitting it from the
     * elevator or from a ledge above/below the arena. */
    if (abs(playerY - level3Boss.y) > 48) return 0;

    if (k == 0)
        return (playerX >= level3Boss.x && playerX - level3Boss.x <= LEVEL3_BOSS_RANGE);

    return (level3Boss.x >= playerX && level3Boss.x - playerX <= LEVEL3_BOSS_RANGE);
}

/* How hard the Guardian hits, by phase. It gets meaningfully more
 * dangerous as the fight goes on, which (together with the shorter
 * cooldown and faster movement applied in updateLevel3()) is what
 * makes the last quarter of the fight a race rather than a grind.
 *
 * These are per-SWING numbers, landed exactly once per swing on the
 * swing's trigger tick, on the fixed 60Hz logic clock - so unlike an
 * ordinary guard the Guardian's damage does not depend at all on how
 * fast the machine renders. Against its cooldowns that works out at
 * roughly 5 HP/sec in phase 1 rising to 15 HP/sec in phase 4 while
 * the Prince stays in its reach, which is what makes standing toe to
 * toe with it a losing plan and backing off between swings the
 * winning one. */
inline int level3GuardianStrikePower()
{
    switch (level3BossPhase){
    case 1:  return 8;
    case 2:  return 10;
    case 3:  return 13;
    default: return 16;
    }
}

/* Called once per Guardian swing, on the swing's damage-trigger tick. */
inline void applyGuardianStrikeDamage()
{
    if (level3Boss.life <= 0 || level3Boss.dying || level3Boss.dead) return;
    if (drowning) return;                               // already going under - the water has him
    if (abs(playerY - level3Boss.y) > 48) return;

    int hit = 0;
    if (level3Boss.k == 0 &&
        playerX <= level3Boss.x &&
        level3Boss.x - playerX <= LEVEL3_BOSS_RANGE)
    {
        hit = 1;
    }
    else if (level3Boss.k != 0 &&
        playerX >= level3Boss.x &&
        playerX - level3Boss.x <= LEVEL3_BOSS_RANGE)
    {
        hit = 1;
    }

    if (!hit) return;

    playerHealth -= level3GuardianStrikePower();
    if (playerHealth < 0) playerHealth = 0;
}

inline void applyPlayerStrikeDamage()
{
    if (playerBlockNo == -1) return;

    /* LEVEL 3: the Guardian takes priority over the ordinary roster.
     * There are no normal guards in the arena, so this can never steal
     * a hit that was meant for someone else - it just means the sword
     * connects with the boss the moment it is in front of the Prince. */
    if (level3GuardianInPlayerReach()){
        level3Boss.life -= PLAYER_STRIKE_POWER;
        if (level3Boss.life < 0) level3Boss.life = 0;

        level3Boss.hitFlash = LEVEL3_BOSS_HIT_FLASH;   // hit reaction (drawn in images.h)
        /* A small stagger away from the blow. updateLevel3() re-clamps
         * the Guardian to its platform every tick, so this can never
         * knock it off the arena. */
        level3Boss.x += (playerX > level3Boss.x) ? -3 : 3;

        playGuardianHitSound();
        return;
    }

    int target = -1;
    int bestDist = 1000000;
    for (int t = 0; t < EnemyNo; t++){
        if (enemy[t].life <= 0) continue;
        if (enemy[t].blockNo != playerBlockNo) continue;

        if (k == 0 &&
            playerX >= enemy[t].x &&
            playerX - enemy[t].x <= COMBAT_RANGE)
        {
            int dist = playerX - enemy[t].x;
            if (dist < bestDist){ bestDist = dist; target = t; }
        }
        else if (k != 0 &&
            enemy[t].x >= playerX &&
            enemy[t].x - playerX <= COMBAT_RANGE)
        {
            int dist = enemy[t].x - playerX;
            if (dist < bestDist){ bestDist = dist; target = t; }
        }
    }

    if (target == -1) return;

    enemy[target].life -= PLAYER_STRIKE_POWER;
    if (enemy[target].life < 0)
        enemy[target].life = 0;
}

/* Called once when an enemy's attack reaches its damage trigger tick. */
inline void applyEnemyStrikeDamage(int enemyIndex)
{
    if (enemyIndex < 0 || enemyIndex >= EnemyNo) return;
    if (enemy[enemyIndex].life <= 0) return;

    if (enemy[enemyIndex].k == 0 &&
        playerX <= enemy[enemyIndex].x &&
        enemy[enemyIndex].x - playerX <= COMBAT_RANGE)
    {
        playerHealth -= nextEnemyDamage();
    }

    if (enemy[enemyIndex].k != 0 &&
        playerX >= enemy[enemyIndex].x &&
        playerX - enemy[enemyIndex].x <= COMBAT_RANGE)
    {
        playerHealth -= nextEnemyDamage();
    }

    if (playerHealth < 0)
        playerHealth = 0;
}

#endif
