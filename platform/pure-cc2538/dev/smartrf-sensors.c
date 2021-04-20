/*
 * Copyright (c) 2012, Texas Instruments Incorporated - http://www.ti.com/
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \addtogroup cc2538-smartrf
 * @{
 *
 * \defgroup cc2538-smartrf-sensors SmartRF06EB Sensors
 *
 * Generic module controlling sensors on the SmartRF06EB
 * @{
 *
 * \file
 * Implementation of a generic module controlling SmartRF06EB sensors
 */
#include "contiki.h"
#include "dev/button-sensor.h"
#include "dev/als-sensor.h"
#include "dev/cc2538-sensors.h"

#include <string.h>

/** \brief Exports a global symbol to be used by the sensor API */
SENSORS(
#if PLATFORM_HAS_BUTTON

#ifdef BUTTON_CANCEL_PORT
&button_cancel_sensor,
#endif

#ifdef BUTTON_SELECT_PORT
&button_select_sensor, 
#endif

#ifdef BUTTON_LEFT_PORT
&button_left_sensor, 
#endif

#ifdef BUTTON_RIGHT_PORT
&button_right_sensor,
#endif

#ifdef BUTTON_UP_PORT
&button_up_sensor, 
#endif

#ifdef BUTTON_DOWN_PORT
&button_down_sensor, 
#endif 

#endif /* PLATFORM_HAS_BUTTON */

#if PLATFORM_HAS_ADC_ALS
&als_sensor,
#endif
&cc2538_temp_sensor, 
&vdd3_sensor);

/**
 * @}
 * @}
 */
