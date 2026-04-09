/** @file
 * Lightweight transient helpers for bike profile.
 *
 * This module intentionally uses low-cost integer arithmetic and updates only
 * on 30Hz ticks, so it does not load critical timing paths.
 */
#include "globals.h"
#include "transients.h"
#include "user_moto_profile.h"

#if MOTO_PROFILE_ENABLE_DFCO_REENTRY
namespace
{
  int16_t lastMap = 0;
  int16_t lastTps = 0;
  uint8_t reentryFuelPct = 0;
  uint8_t reentryHoldTicks = 0;
  uint8_t microTpsTicks = 0;
  uint8_t postDfcoTicks = 0;
  uint8_t leanLambdaFrames = 0;
  uint8_t badLambdaFrames = 0;
  bool releaseRequested = false;
  bool initialised = false;

  static inline uint8_t clampU8(uint16_t value, uint8_t maxValue)
  {
    return (value > maxValue) ? maxValue : (uint8_t)value;
  }

  static inline uint8_t minU8(uint8_t a, uint8_t b)
  {
    return (a < b) ? a : b;
  }

  static inline void initialiseIfNeeded(void)
  {
    if (!initialised)
    {
      lastMap = (int16_t)currentStatus.MAP;
      lastTps = (int16_t)currentStatus.TPS;
      reentryFuelPct = 0U;
      reentryHoldTicks = 0U;
      microTpsTicks = 0U;
      postDfcoTicks = 0U;
      leanLambdaFrames = 0U;
      badLambdaFrames = 0U;
      releaseRequested = false;
      initialised = true;
    }
  }

  static inline uint8_t calculateLambdaX100(void)
  {
    // O2 and stoich are x10, resulting lambda value is scaled x100.
    if (configPage2.stoich == 0U) { return 100U; }
    return (uint8_t)(((uint16_t)currentStatus.O2 * 100U) / configPage2.stoich);
  }

  static inline uint8_t computeHoldTicks(uint16_t score, uint8_t permanence)
  {
    uint16_t hold = MOTO_PROFILE_DFCO_REENTRY_HOLD_BASE;
    hold += (uint16_t)(score >> MOTO_PROFILE_DFCO_REENTRY_HOLD_SCORE_SHIFT);
    hold += (uint16_t)(permanence >> 3U);
    return clampU8(hold, MOTO_PROFILE_DFCO_REENTRY_HOLD_MAX);
  }

  static inline void applyPostDfcoLambdaAdaptation(void)
  {
    #if MOTO_PROFILE_DFCO_REENTRY_LAMBDA_ASSIST
    if (configPage6.egoType != EGO_TYPE_WIDE) { return; }
    if (reentryFuelPct == 0U) { return; }
    if (postDfcoTicks >= MOTO_PROFILE_DFCO_REENTRY_POST_FRAMES) { return; }

    const uint8_t lambdaX100 = calculateLambdaX100();

    if ((lambdaX100 < MOTO_PROFILE_DFCO_REENTRY_LAMBDA_MIN_OK) || (lambdaX100 > MOTO_PROFILE_DFCO_REENTRY_LAMBDA_MAX_OK))
    {
      if (badLambdaFrames < 255U) { badLambdaFrames++; }
      if ((badLambdaFrames <= MOTO_PROFILE_DFCO_REENTRY_BAD_FRAMES) && (reentryFuelPct < MOTO_PROFILE_DFCO_REENTRY_SAFE_FUEL_FLOOR))
      {
        reentryFuelPct = MOTO_PROFILE_DFCO_REENTRY_SAFE_FUEL_FLOOR;
      }
      return;
    }

    badLambdaFrames = 0U;

    if (lambdaX100 > MOTO_PROFILE_DFCO_REENTRY_LEAN_GATE)
    {
      if (leanLambdaFrames < 255U) { leanLambdaFrames++; }
      if (leanLambdaFrames >= MOTO_PROFILE_DFCO_REENTRY_LEAN_FRAMES)
      {
        const uint8_t leanDelta = (uint8_t)(lambdaX100 - MOTO_PROFILE_DFCO_REENTRY_LEAN_GATE);
        const uint8_t leanBonus = minU8((uint8_t)((leanDelta >> MOTO_PROFILE_DFCO_REENTRY_LEAN_SHIFT) + 1U), MOTO_PROFILE_DFCO_REENTRY_LEAN_BONUS_MAX);
        reentryFuelPct = clampU8((uint16_t)reentryFuelPct + leanBonus, MOTO_PROFILE_DFCO_REENTRY_MAX_FUEL);
        reentryHoldTicks = clampU8((uint16_t)reentryHoldTicks + MOTO_PROFILE_DFCO_REENTRY_HOLD_BONUS, MOTO_PROFILE_DFCO_REENTRY_HOLD_MAX);
      }
    }
    else
    {
      leanLambdaFrames = 0U;
    }

    if ((lambdaX100 < MOTO_PROFILE_DFCO_REENTRY_RICH_GATE) && (reentryFuelPct > MOTO_PROFILE_DFCO_REENTRY_RICH_CLAMP))
    {
      reentryFuelPct = MOTO_PROFILE_DFCO_REENTRY_RICH_CLAMP;
      if (reentryHoldTicks > 1U) { reentryHoldTicks = 1U; }
    }
    #endif
  }
}
#endif

void transientsDFCOReentryReset(void)
{
#if MOTO_PROFILE_ENABLE_DFCO_REENTRY
  initialised = false;
#endif
}

void transientsDFCOReentryUpdate(bool dfcoActive)
{
#if MOTO_PROFILE_ENABLE_DFCO_REENTRY
  initialiseIfNeeded();

  releaseRequested = false;

  // Keep this module very cheap by evaluating derivatives on the 30Hz tick.
  if (!BIT_CHECK(LOOP_TIMER, BIT_TIMER_30HZ))
  {
    return;
  }

  const int16_t mapNow = (int16_t)currentStatus.MAP;
  const int16_t tpsNow = (int16_t)currentStatus.TPS;
  const int16_t mapDelta = mapNow - lastMap;
  const int16_t tpsDelta = tpsNow - lastTps;

  lastMap = mapNow;
  lastTps = tpsNow;

  const uint8_t mapRise = (mapDelta > 0) ? (uint8_t)mapDelta : 0U;
  const uint8_t tpsRise = (tpsDelta > 0) ? (uint8_t)tpsDelta : 0U;

  if (dfcoActive)
  {
    if ((tpsNow >= MOTO_PROFILE_DFCO_REENTRY_MICRO_TPS_MIN) && (tpsNow <= MOTO_PROFILE_DFCO_REENTRY_MICRO_TPS_MAX))
    {
      if (microTpsTicks < MOTO_PROFILE_DFCO_REENTRY_PERM_MAX) { microTpsTicks++; }
    }

    if ((mapRise >= MOTO_PROFILE_DFCO_REENTRY_MAP_RISE) || (tpsRise >= MOTO_PROFILE_DFCO_REENTRY_TPS_RISE))
    {
      // Score keeps MAP as the stronger indicator for ITB re-entry while
      // also carrying permanence from micro-TPS region.
      uint16_t score = 0U;
      score += (uint16_t)mapRise * MOTO_PROFILE_DFCO_REENTRY_MAP_WEIGHT;
      score += (uint16_t)tpsRise * MOTO_PROFILE_DFCO_REENTRY_TPS_WEIGHT;
      score += (uint16_t)minU8(microTpsTicks, MOTO_PROFILE_DFCO_REENTRY_PERM_MAX) * MOTO_PROFILE_DFCO_REENTRY_PERM_WEIGHT;

      uint16_t fuelAdd = MOTO_PROFILE_DFCO_REENTRY_BASE_FUEL + (score >> MOTO_PROFILE_DFCO_REENTRY_SCORE_SHIFT);

      #if MOTO_PROFILE_DFCO_REENTRY_LAMBDA_ASSIST
      if (configPage6.egoType == EGO_TYPE_WIDE)
      {
        const uint8_t lambdaX100 = calculateLambdaX100();
        if ((lambdaX100 < MOTO_PROFILE_DFCO_REENTRY_LAMBDA_MIN_OK) || (lambdaX100 > MOTO_PROFILE_DFCO_REENTRY_LAMBDA_MAX_OK))
        {
          if (badLambdaFrames < 255U) { badLambdaFrames++; }
          if ((badLambdaFrames <= MOTO_PROFILE_DFCO_REENTRY_BAD_FRAMES) && (fuelAdd < MOTO_PROFILE_DFCO_REENTRY_SAFE_FUEL_FLOOR))
          {
            fuelAdd = MOTO_PROFILE_DFCO_REENTRY_SAFE_FUEL_FLOOR;
          }
        }
        else
        {
          badLambdaFrames = 0U;
          if (lambdaX100 > MOTO_PROFILE_DFCO_REENTRY_LAMBDA_TARGET)
          {
            uint8_t leanDelta = (uint8_t)(lambdaX100 - MOTO_PROFILE_DFCO_REENTRY_LAMBDA_TARGET);
            uint8_t lambdaAdd = minU8((uint8_t)(leanDelta >> MOTO_PROFILE_DFCO_REENTRY_LAMBDA_SHIFT), MOTO_PROFILE_DFCO_REENTRY_LAMBDA_MAX_ADD);
            fuelAdd += lambdaAdd;
          }
        }
      }
      #endif

      reentryFuelPct = clampU8(fuelAdd, MOTO_PROFILE_DFCO_REENTRY_MAX_FUEL);
      reentryHoldTicks = computeHoldTicks(score, microTpsTicks);
      postDfcoTicks = 0U;
      leanLambdaFrames = 0U;
      releaseRequested = true;
      microTpsTicks = 0U;
    }
  }
  else
  {
    microTpsTicks = 0U;

    if (reentryFuelPct > 0U)
    {
      if (postDfcoTicks < 255U) { postDfcoTicks++; }
      applyPostDfcoLambdaAdaptation();
    }
    else
    {
      postDfcoTicks = 0U;
      leanLambdaFrames = 0U;
      badLambdaFrames = 0U;
    }

    if (reentryHoldTicks > 0U)
    {
      reentryHoldTicks--;
    }
    else if (reentryFuelPct > 0U)
    {
      if (reentryFuelPct > MOTO_PROFILE_DFCO_REENTRY_DECAY_STEP)
      {
        reentryFuelPct -= MOTO_PROFILE_DFCO_REENTRY_DECAY_STEP;
      }
      else
      {
        reentryFuelPct = 0U;
      }
    }
  }
#else
  (void)dfcoActive;
#endif
}

bool transientsDFCOReentryReleaseRequest(void)
{
#if MOTO_PROFILE_ENABLE_DFCO_REENTRY
  return releaseRequested;
#else
  return false;
#endif
}

uint8_t transientsDFCOReentryFuelMult(void)
{
#if MOTO_PROFILE_ENABLE_DFCO_REENTRY
  if (BIT_CHECK(currentStatus.status1, BIT_STATUS1_DFCO))
  {
    return 100U;
  }
  return (uint8_t)(100U + reentryFuelPct);
#else
  return 100U;
#endif
}

#if MOTO_PROFILE_ENABLE_SPOOL_AE_OVERLAY
namespace
{
  uint8_t spoolAddPct = 0U;
  uint8_t spoolHoldTicks = 0U;
  uint8_t spoolRecoveryTicks = 0U;
  bool spoolInitialised = false;

  static inline uint8_t spoolClampU8(uint16_t value, uint8_t maxValue)
  {
    return (value > maxValue) ? maxValue : (uint8_t)value;
  }

  static inline uint8_t spoolMinU8(uint8_t a, uint8_t b)
  {
    return (a < b) ? a : b;
  }

  static inline uint8_t spoolLambdaX100(void)
  {
    if (configPage2.stoich == 0U) { return 100U; }
    return (uint8_t)(((uint16_t)currentStatus.O2 * 100U) / configPage2.stoich);
  }

  static inline void spoolResetState(void)
  {
    spoolAddPct = 0U;
    spoolHoldTicks = 0U;
    spoolRecoveryTicks = 0U;
  }
}
#endif

void transientsSpoolAeReset(void)
{
#if MOTO_PROFILE_ENABLE_SPOOL_AE_OVERLAY
  spoolInitialised = false;
#endif
}

bool transientsSpoolAeActive(void)
{
#if MOTO_PROFILE_ENABLE_SPOOL_AE_OVERLAY
  return (spoolAddPct > 0U);
#else
  return false;
#endif
}

uint16_t transientsSpoolAeOverlay(uint16_t baseAeValue)
{
#if MOTO_PROFILE_ENABLE_SPOOL_AE_OVERLAY
  if (!spoolInitialised)
  {
    spoolResetState();
    spoolInitialised = true;
  }

  // Keep overlay constrained to the intended strategy.
  if ((configPage2.aeMode != AE_MODE_TPS) || (configPage2.aeApplyMode != AE_MODE_ADDER))
  {
    spoolResetState();
    return baseAeValue;
  }

  // Avoid overlay while engine is not in a valid running transient state.
  if (!BIT_CHECK(currentStatus.engine, BIT_ENGINE_RUN) ||
      BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) ||
      BIT_CHECK(currentStatus.engine, BIT_ENGINE_DCC) ||
      BIT_CHECK(currentStatus.status1, BIT_STATUS1_DFCO))
  {
    spoolResetState();
    return baseAeValue;
  }

  if (BIT_CHECK(LOOP_TIMER, BIT_TIMER_30HZ))
  {
    const uint8_t tpsNow = currentStatus.TPS;
    const uint16_t mapNow = currentStatus.MAP;
    const uint16_t mapDotPos = (currentStatus.mapDOT > 0) ? (uint16_t)currentStatus.mapDOT : 0U;
    const uint16_t tpsDotPos = (currentStatus.tpsDOT > 0) ? (uint16_t)currentStatus.tpsDOT : 0U;

    const bool spoolWindow = (tpsNow >= MOTO_PROFILE_SPOOL_AE_TPS_MIN) && (mapNow >= MOTO_PROFILE_SPOOL_AE_MAP_MIN);
    const bool spoolRamp = (mapDotPos >= MOTO_PROFILE_SPOOL_AE_MAPDOT_MIN) &&
                           ((tpsDotPos >= MOTO_PROFILE_SPOOL_AE_TPSDOT_MIN) || (mapNow >= MOTO_PROFILE_SPOOL_AE_MAP_STRONG));

    if (spoolWindow && spoolRamp)
    {
      uint32_t score = (uint32_t)mapDotPos * MOTO_PROFILE_SPOOL_AE_MAP_WEIGHT;
      score += (uint32_t)tpsDotPos * MOTO_PROFILE_SPOOL_AE_TPS_WEIGHT;

      uint16_t addPct = MOTO_PROFILE_SPOOL_AE_BASE_ADD + (uint16_t)(score >> MOTO_PROFILE_SPOOL_AE_SCORE_SHIFT);
      addPct = spoolClampU8(addPct, MOTO_PROFILE_SPOOL_AE_MAX_ADD);

      #if MOTO_PROFILE_SPOOL_AE_LAMBDA_ASSIST
      if (configPage6.egoType == EGO_TYPE_WIDE)
      {
        const uint8_t lambdaX100 = spoolLambdaX100();
        if (lambdaX100 > MOTO_PROFILE_SPOOL_AE_LAMBDA_LEAN_GATE)
        {
          const uint8_t leanDelta = (uint8_t)(lambdaX100 - MOTO_PROFILE_SPOOL_AE_LAMBDA_LEAN_GATE);
          const uint8_t leanBonus = spoolMinU8((uint8_t)((leanDelta >> MOTO_PROFILE_SPOOL_AE_LAMBDA_SHIFT) + 1U), MOTO_PROFILE_SPOOL_AE_LAMBDA_BONUS_MAX);
          addPct = spoolClampU8((uint16_t)addPct + leanBonus, MOTO_PROFILE_SPOOL_AE_MAX_ADD);
        }
        else if ((lambdaX100 < MOTO_PROFILE_SPOOL_AE_LAMBDA_RICH_GATE) && (addPct > MOTO_PROFILE_SPOOL_AE_RICH_CLAMP))
        {
          addPct = MOTO_PROFILE_SPOOL_AE_RICH_CLAMP;
        }
      }
      #endif

      if (addPct > spoolAddPct) { spoolAddPct = (uint8_t)addPct; }

      uint16_t holdTicks = MOTO_PROFILE_SPOOL_AE_HOLD_BASE + (uint16_t)(score >> MOTO_PROFILE_SPOOL_AE_HOLD_SCORE_SHIFT);
      holdTicks = spoolClampU8(holdTicks, MOTO_PROFILE_SPOOL_AE_HOLD_MAX);
      if (holdTicks > spoolHoldTicks) { spoolHoldTicks = (uint8_t)holdTicks; }

      spoolRecoveryTicks = MOTO_PROFILE_SPOOL_AE_RECOVERY_WINDOW;
    }
    else
    {
      if (spoolRecoveryTicks > 0U)
      {
        spoolRecoveryTicks--;
        if ((mapNow >= MOTO_PROFILE_SPOOL_AE_RECOVERY_MAP) && (currentStatus.rpmDOT <= MOTO_PROFILE_SPOOL_AE_RECOVERY_RPMDOT))
        {
          if (spoolAddPct < MOTO_PROFILE_SPOOL_AE_RECOVERY_FLOOR) { spoolAddPct = MOTO_PROFILE_SPOOL_AE_RECOVERY_FLOOR; }
          if (spoolHoldTicks < MOTO_PROFILE_SPOOL_AE_RECOVERY_HOLD) { spoolHoldTicks = MOTO_PROFILE_SPOOL_AE_RECOVERY_HOLD; }
        }
      }

      if (spoolHoldTicks > 0U)
      {
        spoolHoldTicks--;
      }
      else if (spoolAddPct > 0U)
      {
        if (spoolAddPct > MOTO_PROFILE_SPOOL_AE_DECAY_STEP) { spoolAddPct -= MOTO_PROFILE_SPOOL_AE_DECAY_STEP; }
        else { spoolAddPct = 0U; }
      }
    }
  }

  if (spoolAddPct == 0U) { return baseAeValue; }

  uint16_t aeWithOverlay = (uint16_t)(baseAeValue + spoolAddPct);
  if (aeWithOverlay > MOTO_PROFILE_SPOOL_AE_TOTAL_MAX) { aeWithOverlay = MOTO_PROFILE_SPOOL_AE_TOTAL_MAX; }
  return aeWithOverlay;
#else
  return baseAeValue;
#endif
}
