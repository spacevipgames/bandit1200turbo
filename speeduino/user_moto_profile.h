#ifndef USER_MOTO_PROFILE_H
#define USER_MOTO_PROFILE_H

// Project profile for this bike build.
// These flags trim runtime paths for features that are intentionally unused.
#define MOTO_PROFILE_DISABLE_BOOST_CONTROL        1
#define MOTO_PROFILE_DISABLE_NITROUS              1
#define MOTO_PROFILE_DISABLE_WMI                  1
#define MOTO_PROFILE_DISABLE_AIRCON               1
#define MOTO_PROFILE_DISABLE_PROGRAMMABLE_IO      1
#define MOTO_PROFILE_DISABLE_SECONDARY_SERIAL     1
#define MOTO_PROFILE_DISABLE_CAN                  1
#define MOTO_PROFILE_DISABLE_GEAR_DETECTION       1
#define MOTO_PROFILE_DISABLE_SD_LOGGING           1
#define MOTO_PROFILE_DISABLE_RTC                  1
#define MOTO_PROFILE_DISABLE_CTPS                 1
#define MOTO_PROFILE_DISABLE_CAN_WBO             1

// Diagnostic trigger/composite logs are not needed for normal running.
// Keep a short buffer to preserve basic debugging while freeing SRAM.
#define MOTO_PROFILE_TOOTH_LOG_SIZE               16U

// Share RAM for tables belonging to removed features.
#define MOTO_PROFILE_SHARE_UNUSED_TABLES          1U
#define MOTO_PROFILE_SHARE_UNUSED_PINS            1U

// Lightweight DFCO re-entry assist for ITB drivability.
// All values are integer-based and updated at 30Hz in transients.cpp.
#define MOTO_PROFILE_ENABLE_DFCO_REENTRY          1U
#define MOTO_PROFILE_DFCO_REENTRY_MAP_RISE        2U   // MAP rises in 0.5kPa units per 30Hz step
#define MOTO_PROFILE_DFCO_REENTRY_TPS_RISE        1U   // TPS rises in 0.5% units per 30Hz step
#define MOTO_PROFILE_DFCO_REENTRY_MICRO_TPS_MIN   1U   // 0.5% TPS
#define MOTO_PROFILE_DFCO_REENTRY_MICRO_TPS_MAX   4U   // 2.0% TPS
#define MOTO_PROFILE_DFCO_REENTRY_PERM_MAX        18U  // max permanence ticks tracked
#define MOTO_PROFILE_DFCO_REENTRY_PERM_WEIGHT     1U   // permanence contribution in score
#define MOTO_PROFILE_DFCO_REENTRY_BASE_FUEL       6U   // Base fuel add % when re-entry triggers
#define MOTO_PROFILE_DFCO_REENTRY_MAX_FUEL        24U  // Max fuel add % from re-entry assist
#define MOTO_PROFILE_DFCO_REENTRY_HOLD_BASE       2U   // Base hold time (N * 33ms)
#define MOTO_PROFILE_DFCO_REENTRY_HOLD_MAX        7U   // Max hold time (N * 33ms)
#define MOTO_PROFILE_DFCO_REENTRY_DECAY_STEP      2U   // Fuel % removed per 30Hz step after hold
#define MOTO_PROFILE_DFCO_REENTRY_SCORE_SHIFT     2U   // Coarse gain compression for score->fuel
#define MOTO_PROFILE_DFCO_REENTRY_MAP_WEIGHT      2U   // MAP contribution in score
#define MOTO_PROFILE_DFCO_REENTRY_TPS_WEIGHT      1U   // TPS contribution in score
#define MOTO_PROFILE_DFCO_REENTRY_HOLD_SCORE_SHIFT 5U  // score compression for hold shaping

// Optional AFR sanity assist during DFCO re-entry.
// Uses lambda_x100 = (O2*100)/stoich and only integer math.
#define MOTO_PROFILE_DFCO_REENTRY_LAMBDA_ASSIST   1U
#define MOTO_PROFILE_DFCO_REENTRY_LAMBDA_TARGET   88U  // target lambda x100
#define MOTO_PROFILE_DFCO_REENTRY_LAMBDA_MIN_OK   70U  // valid lambda floor x100
#define MOTO_PROFILE_DFCO_REENTRY_LAMBDA_MAX_OK   130U // valid lambda ceiling x100
#define MOTO_PROFILE_DFCO_REENTRY_BAD_FRAMES      2U   // consecutive invalid frames tolerated
#define MOTO_PROFILE_DFCO_REENTRY_SAFE_FUEL_FLOOR 8U   // % add while sensor data is unstable
#define MOTO_PROFILE_DFCO_REENTRY_LAMBDA_SHIFT    3U   // lean correction gain compression
#define MOTO_PROFILE_DFCO_REENTRY_LAMBDA_MAX_ADD  8U   // max extra % from lambda assist

// Post-DFCO adaptive window for light-throttle recovery.
#define MOTO_PROFILE_DFCO_REENTRY_POST_FRAMES      10U  // adaptation window after DFCO release
#define MOTO_PROFILE_DFCO_REENTRY_LEAN_GATE        120U // lambda x100 considered lean in the window
#define MOTO_PROFILE_DFCO_REENTRY_LEAN_FRAMES      2U   // consecutive lean frames before boosting
#define MOTO_PROFILE_DFCO_REENTRY_LEAN_BONUS_MAX   6U   // max extra % from post-window lean boost
#define MOTO_PROFILE_DFCO_REENTRY_LEAN_SHIFT       2U   // lean bonus compression
#define MOTO_PROFILE_DFCO_REENTRY_HOLD_BONUS       1U   // extra hold ticks on lean re-entry
#define MOTO_PROFILE_DFCO_REENTRY_RICH_GATE        75U  // lambda x100 considered too rich in the window
#define MOTO_PROFILE_DFCO_REENTRY_RICH_CLAMP       6U   // clamp fuel add when rich overshoot is detected

// Spool-Sync AE overlay for turbo ITB drivability.
// Works on top of native AE using existing TPSdot/MAPdot signals.
#define MOTO_PROFILE_ENABLE_SPOOL_AE_OVERLAY        1U
#define MOTO_PROFILE_SPOOL_AE_TPS_MIN               16U  // 8.0% TPS (TPS is 0.5% resolution)
#define MOTO_PROFILE_SPOOL_AE_MAP_MIN               90U  // kPa
#define MOTO_PROFILE_SPOOL_AE_MAP_STRONG            105U // kPa, allows spool trigger with low TPSdot
#define MOTO_PROFILE_SPOOL_AE_MAPDOT_MIN            35U  // kPa/s
#define MOTO_PROFILE_SPOOL_AE_TPSDOT_MIN            8U   // %/s equivalent scale
#define MOTO_PROFILE_SPOOL_AE_MAP_WEIGHT            2U
#define MOTO_PROFILE_SPOOL_AE_TPS_WEIGHT            1U
#define MOTO_PROFILE_SPOOL_AE_BASE_ADD              4U   // % add over AE baseline
#define MOTO_PROFILE_SPOOL_AE_MAX_ADD               28U  // max % add over AE baseline
#define MOTO_PROFILE_SPOOL_AE_SCORE_SHIFT           5U
#define MOTO_PROFILE_SPOOL_AE_HOLD_BASE             2U   // ticks at 30Hz
#define MOTO_PROFILE_SPOOL_AE_HOLD_MAX              10U  // ticks at 30Hz
#define MOTO_PROFILE_SPOOL_AE_HOLD_SCORE_SHIFT      6U
#define MOTO_PROFILE_SPOOL_AE_DECAY_STEP            2U   // % removed per 30Hz tick after hold
#define MOTO_PROFILE_SPOOL_AE_TOTAL_MAX             220U // hard ceiling for combined AE (%)

// Recovery support for shifts / transient rpm drop during spool.
#define MOTO_PROFILE_SPOOL_AE_RECOVERY_WINDOW       12U  // ticks at 30Hz (~400ms)
#define MOTO_PROFILE_SPOOL_AE_RECOVERY_MAP          95U  // kPa
#define MOTO_PROFILE_SPOOL_AE_RECOVERY_RPMDOT      -250  // rpm/s
#define MOTO_PROFILE_SPOOL_AE_RECOVERY_FLOOR        8U   // minimum % add during recovery
#define MOTO_PROFILE_SPOOL_AE_RECOVERY_HOLD         3U   // minimum hold ticks during recovery

// Optional lambda sanity assist during spool overlay.
#define MOTO_PROFILE_SPOOL_AE_LAMBDA_ASSIST         1U
#define MOTO_PROFILE_SPOOL_AE_LAMBDA_LEAN_GATE      118U // lambda x100
#define MOTO_PROFILE_SPOOL_AE_LAMBDA_RICH_GATE      75U  // lambda x100
#define MOTO_PROFILE_SPOOL_AE_LAMBDA_BONUS_MAX      6U   // max extra % from lean detection
#define MOTO_PROFILE_SPOOL_AE_LAMBDA_SHIFT          3U
#define MOTO_PROFILE_SPOOL_AE_RICH_CLAMP            8U   // clamp add% if too rich

#endif
