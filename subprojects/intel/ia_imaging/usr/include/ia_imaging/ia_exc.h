/*
 * Copyright (C) 2015 - 2017 Intel Corporation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*!
 * \file ia_exc.h
 * \brief Definitions of exposure parameters conversions between generic and sensor units.
*/


#ifndef IA_EXC_H_
#define IA_EXC_H_

#include "ia_types.h"
#include "ia_aiq_types.h"
#include "ia_cmc_types.h"

#ifdef __cplusplus
extern "C" {
#endif


/*!
 * \brief Convert exposure time from generic units to sensor units (line & pixel values).
 * AEC will use default formula for conversion, if not given
 * \param[in]  exposure_range          Structure containing coarse and fine integration sensor register ranges. Can be set to NULL if not available.
 * \param[in]  sensor_descriptor       Structure containing coarse and fine integration time limits and step size.
 * \param[in]  exposure_time_us        Exposure time to convert.
 * \param[out] coarse_integration_time Coarse (rows of integration) for rolling shutter cameras.
 * \param[out] fine_integration_time   Fine (pixels of integration remaining after coarse quantization) for rolling shutter cameras.
 * \return                             Error code.
 */
LIBEXPORT ia_err
ia_exc_exposure_time_to_sensor_units(
    const cmc_exposure_range_t *exposure_range,
    const ia_aiq_exposure_sensor_descriptor *sensor_descriptor,
    const int exposure_time_us,
    unsigned short *coarse_integration_time,
    unsigned short *fine_integration_time);

/*!
 * \brief Convert analog gain from generic units to sensor units.
 * Calculate analog gain code from analog gain, limiting it to the sensor specific values.
 * \param[in]  gain_conversion   Structure containing analog gain to gain code conversion tables.
 * \param[in]  analog_gain       Analog gain value to convert to sensor unit.
 * \param[out] analog_gain_code  Calculated analog gain code.
 * \return                       Error code.
 */
LIBEXPORT ia_err
ia_exc_analog_gain_to_sensor_units(
    const cmc_parsed_analog_gain_conversion_t *gain_conversion,
    const float analog_gain,
    unsigned short *analog_gain_code);

/*!
 * \brief Converts digital gain from generic units to sensor units.
 * AEC will use default formulae for conversion, if not given
 * Converts digital gain value to sensor units, limiting the value according to sensor specific limits.
 * \param[in]  gain_conversion   Structure containing digital gain to code mapping information.
 * \param[in]  digital_gain      Digital gain in generic units.
 * \param[out] digital_gain_code Calculated digital gain code.
 * \return                       Error code.
 */
LIBEXPORT ia_err
ia_exc_digital_gain_to_sensor_units(
    const cmc_parsed_digital_gain_t *gain_conversion,
    const float digital_gain,
    unsigned short *digital_gain_code);

/*!
 * \brief Convert exposure time from sensor units to generic units.
 * AEC will use default formula for conversion, if not given
 * \param[in]  sensor_descriptor       Structure containing pixel clock frequency needed in exposure conversion.
 * \param[in]  coarse_integration_time Coarse (rows of integration) for rolling shutter cameras.
 * \param[in]  fine_integration_time   Fine (pixels of integration remaining after coarse quantization) for rolling shutter cameras.
 * \param[out] exposure_time           Calculated exposure value in microseconds.
 * \return                             Error code.
 */
LIBEXPORT ia_err
ia_exc_sensor_units_to_exposure_time(
    const ia_aiq_exposure_sensor_descriptor *sensor_descriptor,
    const unsigned short coarse_integration_time,
    const unsigned short fine_integration_time,
    int *exposure_time);

/*!
 * \brief Convert analog gain from sensor units to generic units.
 * Using the sensor characteristics info, calculate analog gain from sensor register values.
 * \param[in]  gain_conversion   Structure containing analog gain to gain code conversion tables.
 * \param[in]  gain_code         Analog gain code in sensor specific units.
 * \param[out] analog_gain       Calculated analog gain.
 * \return                       Error code.
 */
LIBEXPORT ia_err
ia_exc_sensor_units_to_analog_gain(
    const cmc_parsed_analog_gain_conversion_t *gain_conversion,
    const unsigned short gain_code,
    float *analog_gain);

/*!
 * \brief Converts digital gain from sensor units to generic units.
 * AEC will use default formula for conversion, if not given
 * \param[in]  gain_conversion   Structure containing digital gain to code mapping information.
 * \param[in]  gain_code         Digital gain code in sensor specific units.
 * \param[out] digital_gain      Calculated digital gain.
 * \return                       Error code.
 */
LIBEXPORT ia_err
ia_exc_sensor_units_to_digital_gain(
    const cmc_parsed_digital_gain_t *gain_conversion,
    const unsigned short code,
    float *digital_gain);

/*!
 * \brief Gets analog gain and code based on current code.
 * Offset is used to retrieve previous or next analog gain code pairs from the CMC analog gain conversion tables.
 * \param[in]  gain_conversion   Structure containing analog gain to gain code conversion tables.
 * \param[in]  gain_code         Analog gain code in sensor specific units.
 * \param[in]  gain_code_offset  Offset of code to resolve (-1 or 1).
 * \param[out] indexed_gain_code Analog gain code matching the offset.
 * \return                       Error code.
 */
LIBEXPORT ia_err
ia_exc_get_analog_gain_code(
    const cmc_parsed_analog_gain_conversion_t *gain_conversion,
    const unsigned short gain_code,
    const int gain_code_offset,
    unsigned short *indexed_gain_code);

/*!
 * \brief Converts ISO to analog gain and digital gain and codes.
 * Gains are round down except if given ISO is smaller than corresponding gain 1.0.
 * \param[in]  analog_gain_conversion   Structure containing analog gain to gain code conversion tables.  Can be NULL, if sensor doesn't support analog gain.
 * \param[in]  digital_gain_conversion  Structure containing digital gain to code mapping information. Can be NULL, if sensor doesn't support digital gain.
 * \param[in]  sensitivity              Structure containing sensor sensitivity information.
 * \param[in]  iso                      ISO value to be converted into gains.
 * \param[out] analog_gain              Calculated analog gain.-1.0 if not available.
 * \param[out] analog_gain_code         Calculated analog gain code. -1 if not available.
 * \param[out] digital_gain             Calculated digital gain. -1.0 if not available.
 * \param[out] digital_gain_code        Calculated digital gain code. -1 if not available.
 * \return                              Error code.
 */
LIBEXPORT ia_err
ia_exc_convert_iso_to_gains(
    const cmc_parsed_analog_gain_conversion_t *analog_gain_conversion,
    const cmc_parsed_digital_gain_t *digital_gain_conversion,
    const cmc_sensitivity_t *sensitivity,
    const int iso,
    float *analog_gain,
    int *analog_gain_code,
    float *digital_gain,
    int *digital_gain_code);

/*!
 * \brief Converts analog gain and digital gain codes to ISO.
 * \param[in]  analog_gain_conversion   Structure containing analog gain to gain code conversion tables. Can be NULL, if sensor doesn't support analog gain.
 * \param[in]  digital_gain_conversion  Structure containing digital gain to code mapping information. Can be NULL, if sensor doesn't support digital gain.
 * \param[in]  sensitivity              Structure containing sensor sensitivity information.
 * \param[in]  analog_gain_code         Analog gain code. -1 if not available.
 * \param[in]  digital_gain_code        Digital gain code. -1 if not available.
 * \param[out] iso                      Analog and digital gain codes converted into ISO value. -1 if not available.
 * \return                              Error code.
 */
LIBEXPORT ia_err
ia_exc_convert_gain_codes_to_iso(
    const cmc_parsed_analog_gain_conversion_t *analog_gain_conversion,
    const cmc_parsed_digital_gain_t *digital_gain_conversion,
    const cmc_sensitivity_t *sensitivity,
    const int analog_gain_code,
    const int digital_gain_code,
    int *iso);

#ifdef __cplusplus
}
#endif

#endif /* IA_EXC_H_ */
