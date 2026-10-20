/** 
 * @addtogroup table_3d 
 *  @{
 */

/** \file
 * @brief 3D table axis I/O support 
 */

#pragma once

#include "table3d_axes.h"

/** @brief Byte type. This is not defined in any C or C++ standard header. */
typedef uint8_t byte;

/** @brief Number of bytes each axis element occupies on the wire (TunerStudio page) and in EEPROM.
 *
 * Axis values are stored at full 16-bit resolution (little endian) in every domain
 * (RPM in rpm, load in 0.5 kPa/% units, ...). There is no scaling: what is in RAM is
 * what is sent/stored. This replaces the old 8-bit scheme that divided RPM by 100 
 * and load by 2.
 */
static constexpr uint8_t table3d_axis_io_size = (uint8_t)sizeof(table3d_axis_t);
static_assert(table3d_axis_io_size==2U, "Axis I/O code assumes 16-bit axis elements");

/** @brief Get the low (byte_index==0) or high (byte_index==1) byte of an axis value */
static inline byte table3d_axis_get_byte(table3d_axis_t value, uint8_t byte_index)
{
    return (byte)(((uint16_t)value >> (8U*byte_index)) & 0xFFU);
}

/** @brief Set the low (byte_index==0) or high (byte_index==1) byte of an axis value */
static inline table3d_axis_t table3d_axis_set_byte(table3d_axis_t value, uint8_t byte_index, byte new_byte)
{
    const uint16_t mask = (uint16_t)(0xFFU << (8U*byte_index));
    return (table3d_axis_t)(((uint16_t)value & (uint16_t)~mask) | (uint16_t)((uint16_t)new_byte << (8U*byte_index)));
}

/** @brief Assemble an axis value from its low & high bytes */
static inline table3d_axis_t table3d_axis_from_bytes(byte low, byte high)
{
    return (table3d_axis_t)(((uint16_t)high << 8U) | low);
}

/** @} */
