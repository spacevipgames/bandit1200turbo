/** @file
 * Lightweight transient helpers for bike profile.
 */
#ifndef TRANSIENTS_H
#define TRANSIENTS_H

#include <stdint.h>

void transientsDFCOReentryUpdate(bool dfcoActive);
bool transientsDFCOReentryReleaseRequest(void);
uint8_t transientsDFCOReentryFuelMult(void);
void transientsDFCOReentryReset(void);
uint16_t transientsSpoolAeOverlay(uint16_t baseAeValue);
bool transientsSpoolAeActive(void);
void transientsSpoolAeReset(void);

#endif
