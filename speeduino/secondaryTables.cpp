#include "globals.h"
#include "secondaryTables.h"
#include "corrections.h"

void calculateSecondaryFuel(void)
{
  // Fuel 2 is always active in this build as a fixed MAP-based trim table.
  BIT_SET(currentStatus.status3, BIT_STATUS3_FUEL2_ACTIVE);
  currentStatus.VE2 = getVE2();
  uint16_t combinedVE = ((uint16_t)currentStatus.VE1 * (uint16_t)currentStatus.VE2) / 100;
  if(combinedVE <= UINT8_MAX) { currentStatus.VE = combinedVE; }
  else { currentStatus.VE = UINT8_MAX; }
}


void calculateSecondarySpark(void)
{
  //Same as above but for the secondary ignition table
  BIT_CLEAR(currentStatus.status5, BIT_STATUS5_SPARK2_ACTIVE); //Clear the bit indicating that the 2nd spark table is in use. 
  if(configPage10.spark2Mode == SPARK2_MODE_MULTIPLY)
  { 
    BIT_SET(currentStatus.status5, BIT_STATUS5_SPARK2_ACTIVE);
    currentStatus.advance2 = getAdvance2();
    if(currentStatus.advance2 < 0) { currentStatus.advance2 = 0; }
    int16_t combinedAdvance = ((int16_t)currentStatus.advance1 * (int16_t)currentStatus.advance2) / 100;
    if(combinedAdvance <= 127) { currentStatus.advance = combinedAdvance; }
    else { currentStatus.advance = 127; }
  }
  else if(configPage10.spark2Mode == SPARK2_MODE_ADD)
  {
    BIT_SET(currentStatus.status5, BIT_STATUS5_SPARK2_ACTIVE);
    currentStatus.advance2 = getAdvance2();
    int16_t combinedAdvance = (int16_t)currentStatus.advance1 + (int16_t)currentStatus.advance2;
    if(combinedAdvance <= 127) { currentStatus.advance = combinedAdvance; }
    else { currentStatus.advance = 127; }
  }

  if(configPage10.spark2Mode > 0)
  {
    //Apply the fixed timing correction manually. This has to be done again here if any of the above conditions are met to prevent any of the seconadary calculations applying instead of fixec timing
    currentStatus.advance = correctionFixedTiming(currentStatus.advance);
    currentStatus.advance = correctionCrankingFixedTiming(currentStatus.advance); //This overrides the regular fixed timing, must come last
  }
}

/**
 * @brief Looks up and returns the VE value from the secondary fuel table
 * 
 * This performs largely the same operations as getVE() however the lookup is of the secondary fuel table and uses the secondary load source
 * @return byte 
 */
byte getVE2(void)
{
  byte tempVE = 100;
  currentStatus.fuelLoad2 = currentStatus.MAP;
  tempVE = get3DTableValue(&fuelTable2, currentStatus.fuelLoad2, currentStatus.RPM); //Perform lookup into fuel map for RPM vs MAP value

  return tempVE;
}

/**
 * @brief Performs a lookup of the second ignition advance table. The values used to look this up will be RPM and whatever load source the user has configured
 * 
 * @return byte The current target advance value in degrees
 */
byte getAdvance2(void)
{
  byte tempAdvance = 0;
  currentStatus.ignLoad2 = currentStatus.TPS * 2;
  tempAdvance = get3DTableValue(&ignitionTable2, currentStatus.ignLoad2, currentStatus.RPM) - OFFSET_IGNITION; //As above, but for ignition advance

  return tempAdvance;
}
