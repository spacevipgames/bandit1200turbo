#include "globals.h"
#include "comms_secondary.h"

uint8_t currentSecondaryCommand = 0;
SECONDARY_SERIAL_T* pSecondarySerial = nullptr;

void secondserial_Command(void)
{
}

void sendCancommand(uint8_t cmdtype, uint16_t canadddress, uint8_t candata1, uint8_t candata2, uint16_t sourcecanAddress)
{
  (void)cmdtype;
  (void)canadddress;
  (void)candata1;
  (void)candata2;
  (void)sourcecanAddress;
}
