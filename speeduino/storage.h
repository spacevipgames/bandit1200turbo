#ifndef STORAGE_H
#define STORAGE_H

/** @file storage.h
 * @brief Functions for reading and writing user settings to/from EEPROM
 *
 * EEPROM layout (Version 26 - "DIV 1": table axes stored as 16-bit little endian values).
 *
 * Every 3D table is stored as: values (1 byte each), X axis (2 bytes each), Y axis (2 bytes each).
 * Axis values are the real engine values (RPM in rpm, load in 0.5 units), there is no /100 or /2 scaling.
 *
 * | Offset (Dec)|Size (Bytes)| Description                          | Reference                          |
 * | ----------: | :--------: | :----------------------------------: | :--------------------------------- |
 * | 0           |1           | EEPROM version                       | @ref EEPROM_DATA_VERSION           |
 * | 3           |320         | Fuel table (16x16)                   | @ref EEPROM_CONFIG1_MAP            |
 * | 323         |129         | Page 2 settings                      | @ref EEPROM_CONFIG2_START          |
 * | 452         |320         | Ignition table (16x16)               | @ref EEPROM_CONFIG3_MAP            |
 * | 772         |128         | Page 4 settings                      | @ref EEPROM_CONFIG4_START          |
 * | 900         |320         | AFR target table (16x16)             | @ref EEPROM_CONFIG5_MAP            |
 * | 1220        |128         | Page 6 settings                      | @ref EEPROM_CONFIG6_START          |
 * | 1348        |96          | Boost table (8x8)                    | @ref EEPROM_CONFIG7_MAP1           |
 * | 1444        |96          | VVT table (8x8)                      | @ref EEPROM_CONFIG7_MAP2           |
 * | 1540        |96          | Staging table (8x8)                  | @ref EEPROM_CONFIG7_MAP3           |
 * | 1636        |60 x 4      | Trim1..4 tables (6x6)                | @ref EEPROM_CONFIG8_MAP1           |
 * | 1876        |192         | Page 9 settings                      | @ref EEPROM_CONFIG9_START          |
 * | 2068        |198         | Page 10 settings                     | @ref EEPROM_CONFIG10_START         |
 * | 2266        |320         | Fuel2 table (16x16)                  | @ref EEPROM_CONFIG11_MAP           |
 * | 2586        |96          | WMI table (8x8)                      | @ref EEPROM_CONFIG12_MAP           |
 * | 2682        |96          | VVT2 table (8x8)                     | @ref EEPROM_CONFIG12_MAP2          |
 * | 2778        |32          | Dwell table (4x4)                    | @ref EEPROM_CONFIG12_MAP3          |
 * | 2810        |128         | Page 13 settings                     | @ref EEPROM_CONFIG13_START         |
 * | 2938        |320         | Ignition2 table (16x16)              | @ref EEPROM_CONFIG14_MAP           |
 * | 3258        |96          | boostLUT table (8x8)                 | @ref EEPROM_CONFIG15_MAP           |
 * | 3354        |176         | Page 15 settings                     | @ref EEPROM_CONFIG15_START         |
 * | 3530        |144         | EMPTY                                |                                    |
 * | 3674        |4           | CLT Calibration CRC32                |                                    |
 * | 3678        |4           | IAT Calibration CRC32                |                                    |
 * | 3682        |4           | O2 Calibration CRC32                 |                                    |
 * | 3686        |56          | Page CRC32 sums                      | Last first, 14 -> 1                |
 * | 3742        |1           | Baro value saved at init             | @ref EEPROM_LAST_BARO              |
 * | 3743        |352         | O2/IAT/CLT calibration bins & values |                                    |
 * | 4095        |            | END                                  |                                    |
 *
 * Fuel trim tables 5-8 are not stored when MOTO_PROFILE_SHARE_UNUSED_TABLES is set (they do not exist in RAM either).
 */

void writeAllConfig(void);
void writeConfig(uint8_t pageNum);
void EEPROMWriteRaw(uint16_t address, uint8_t data);
uint8_t EEPROMReadRaw(uint16_t address);
void loadConfig(void);
void loadCalibration(void);
void writeCalibration(void);
void writeCalibrationPage(uint8_t pageNum);
void resetConfigPages(void);

//These are utility functions that prevent other files from having to use EEPROM.h directly
byte readLastBaro(void);
void storeLastBaro(byte newValue);
uint8_t readEEPROMVersion(void);
void storeEEPROMVersion(byte newVersion);
void storePageCRC32(uint8_t pageNum, uint32_t crcValue);
uint32_t readPageCRC32(uint8_t pageNum);
void storeCalibrationCRC32(uint8_t calibrationPageNum, uint32_t calibrationCRC);
uint32_t readCalibrationCRC32(uint8_t calibrationPageNum);
uint16_t getEEPROMSize(void);
bool isEepromWritePending(void);

extern uint32_t deferEEPROMWritesUntil;

// Size of a stored 3D table: values (1 byte) + X axis and Y axis (2 bytes per element, "DIV 1")
#define EEPROM_TABLE_SIZE(size) ((size)*(size) + 2U*(size)*2U)
#define EEPROM_TABLE16_SIZE   EEPROM_TABLE_SIZE(16U)
#define EEPROM_TABLE8_SIZE    EEPROM_TABLE_SIZE(8U)
#define EEPROM_TABLE6_SIZE    EEPROM_TABLE_SIZE(6U)
#define EEPROM_TABLE4_SIZE    EEPROM_TABLE_SIZE(4U)

// Everything is laid out back to back, each address follows from the previous one.
#define EEPROM_CONFIG1_MAP    3U
#define EEPROM_CONFIG2_START  (EEPROM_CONFIG1_MAP + EEPROM_TABLE16_SIZE)
#define EEPROM_CONFIG2_END    (EEPROM_CONFIG2_START + 129U) // config2 is 129 bytes
#define EEPROM_CONFIG3_MAP    EEPROM_CONFIG2_END
#define EEPROM_CONFIG4_START  (EEPROM_CONFIG3_MAP + EEPROM_TABLE16_SIZE)
#define EEPROM_CONFIG4_END    (EEPROM_CONFIG4_START + 128U)
#define EEPROM_CONFIG5_MAP    EEPROM_CONFIG4_END
#define EEPROM_CONFIG6_START  (EEPROM_CONFIG5_MAP + EEPROM_TABLE16_SIZE)
#define EEPROM_CONFIG6_END    (EEPROM_CONFIG6_START + 128U)
#define EEPROM_CONFIG7_MAP1   EEPROM_CONFIG6_END
#define EEPROM_CONFIG7_MAP2   (EEPROM_CONFIG7_MAP1 + EEPROM_TABLE8_SIZE)
#define EEPROM_CONFIG7_MAP3   (EEPROM_CONFIG7_MAP2 + EEPROM_TABLE8_SIZE)
#define EEPROM_CONFIG7_END    (EEPROM_CONFIG7_MAP3 + EEPROM_TABLE8_SIZE)
#define EEPROM_CONFIG8_MAP1   EEPROM_CONFIG7_END
#define EEPROM_CONFIG8_MAP2   (EEPROM_CONFIG8_MAP1 + EEPROM_TABLE6_SIZE)
#define EEPROM_CONFIG8_MAP3   (EEPROM_CONFIG8_MAP2 + EEPROM_TABLE6_SIZE)
#define EEPROM_CONFIG8_MAP4   (EEPROM_CONFIG8_MAP3 + EEPROM_TABLE6_SIZE)
#define EEPROM_CONFIG9_START  (EEPROM_CONFIG8_MAP4 + EEPROM_TABLE6_SIZE)
#define EEPROM_CONFIG9_END    (EEPROM_CONFIG9_START + 192U)
#define EEPROM_CONFIG10_START EEPROM_CONFIG9_END
#define EEPROM_CONFIG10_END   (EEPROM_CONFIG10_START + 198U)
#define EEPROM_CONFIG11_MAP   EEPROM_CONFIG10_END
#define EEPROM_CONFIG11_END   (EEPROM_CONFIG11_MAP + EEPROM_TABLE16_SIZE)
#define EEPROM_CONFIG12_MAP   EEPROM_CONFIG11_END
#define EEPROM_CONFIG12_MAP2  (EEPROM_CONFIG12_MAP + EEPROM_TABLE8_SIZE)
#define EEPROM_CONFIG12_MAP3  (EEPROM_CONFIG12_MAP2 + EEPROM_TABLE8_SIZE)
#define EEPROM_CONFIG12_END   (EEPROM_CONFIG12_MAP3 + EEPROM_TABLE4_SIZE)
#define EEPROM_CONFIG13_START EEPROM_CONFIG12_END
#define EEPROM_CONFIG13_END   (EEPROM_CONFIG13_START + 128U)
#define EEPROM_CONFIG14_MAP   EEPROM_CONFIG13_END
#define EEPROM_CONFIG14_END   (EEPROM_CONFIG14_MAP + EEPROM_TABLE16_SIZE)
#define EEPROM_CONFIG15_MAP   EEPROM_CONFIG14_END
#define EEPROM_CONFIG15_START (EEPROM_CONFIG15_MAP + EEPROM_TABLE8_SIZE)
#define EEPROM_CONFIG15_END   (EEPROM_CONFIG15_START + 176U)

// Trim tables 5-8 only exist (RAM & EEPROM) when MOTO_PROFILE_SHARE_UNUSED_TABLES is off.
// They follow everything else; storage.cpp checks that they fit before the calibration area.
#define EEPROM_CONFIG8_MAP5   EEPROM_CONFIG15_END
#define EEPROM_CONFIG8_MAP6   (EEPROM_CONFIG8_MAP5 + EEPROM_TABLE6_SIZE)
#define EEPROM_CONFIG8_MAP7   (EEPROM_CONFIG8_MAP6 + EEPROM_TABLE6_SIZE)
#define EEPROM_CONFIG8_MAP8   (EEPROM_CONFIG8_MAP7 + EEPROM_TABLE6_SIZE)
#define EEPROM_CONFIG8_END    (EEPROM_CONFIG8_MAP8 + EEPROM_TABLE6_SIZE)

#define EEPROM_CALIBRATION_CLT_CRC  3674
#define EEPROM_CALIBRATION_IAT_CRC  3678
#define EEPROM_CALIBRATION_O2_CRC   3682

//These were the values used previously when all calibration tables were 512 long. They need to be retained so the update process (202005 -> 202008) can work
#define EEPROM_CALIBRATION_O2_OLD   2559
#define EEPROM_CALIBRATION_IAT_OLD  3071
#define EEPROM_CALIBRATION_CLT_OLD  3583

#define EEPROM_DEFER_DELAY          MICROS_PER_SEC //1.0 second pause after large comms before writing to EEPROM

#endif // STORAGE_H
