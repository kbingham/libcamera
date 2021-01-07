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
 * \mainpage IA DVS documentation
 *
 * \section general General info
 *
 * Digital video stabilization API.
 *
 */

/*!
 * \file ia_dvs.h
 * \brief Definitions and declarations of Intel DVS library.
 */

#ifndef _IA_DVS_H_
#define _IA_DVS_H_

#include <stdint.h>
#include "ia_dvs_types.h"
#include "ia_aiq_types.h"
#include "ia_cmc_types.h"
#include "ia_aiq.h"
#ifdef __cplusplus
extern "C" {
#endif

/*! \brief Initialize the DVS module.
 *
 * \param[out]  dvs_state            dvs state.
 *                                   This is a pointer to a pointer to a module.
 * \param[in]   a_aiq_tuning_binary  aiqb binary tuning parameter.
 * \param[in]   cmc                  cmc_t parameter.
 * \return                           0 for no error, others for error.
 *
 * This function initializes the DVS module. This allocates and initializes
 * internal data structures. This function must always be called before any
 * other ia_dvs function.
 */
LIBEXPORT ia_err
ia_dvs_init(ia_dvs_state **dvs_state,
            const ia_binary_data *a_aiq_tuning_binary,
            const ia_cmc_t *cmc);

/*! \brief Configure the DVS module.
 *
 * \param[in]   dvs_state           dvs state.
 * \param[in]   config              dvs configuration
 * \param[in]   digital_zoom_ratio  digital zoom ratio.
 * \return                          0 for no error, others for error.
 *
 * This function configures the DVS module. This allocates and initializes
 * internal data structures. This function must always be called after ia_dvs_init
 * and before any other ia_dvs function is called.
 */
LIBEXPORT ia_err
ia_dvs_config(ia_dvs_state *dvs_state,
            const ia_dvs_configuration *config,
            float digital_zoom_ratio);

/*! \brief Deinitialize the DVS module.
 *
 * \param[in]   dvs_state       dvs state.
 *                              This is a pointer to a module.
 *
 * This function deinitilizes the DVS module. This frees the allocated memory.
 */
LIBEXPORT void
ia_dvs_deinit(ia_dvs_state *dvs_state);

/*! \brief Set the DVS statistics.
 *
 * \param[in]   dvs_state       dvs state.
 *                              This is a pointer to a module.
 * \param[in]   statistics      Pointer to generic DVS statistics
 * \param[in]   ae_results      Optional. Pointer to AIQ AE results.
 * \param[in]   af_results      Optional. Pointer to AIQ AF results.
 * \param[in]   sensor_events   Optional. Pointer to sensor events data which contains accelerometer,
 *                              gravity and gyroscope events.
 * \param[in]   frame_exposure_start    Frame exposure start time. Optional for image based DVS. Mandatory for gyro based DVS
 * \param[in]   frame_readout_end       Frame readout end time. Optional for image based DVS. Mandatory for gyro based DVS
 * \return                      0 for no error, others for error.
 *
 * This function receives DVS statistics in generic format.
 */
LIBEXPORT ia_err
ia_dvs_set_statistics(ia_dvs_state *dvs_state,
                      const ia_dvs_statistics *statistics,
                      const ia_aiq_ae_results *ae_results,
                      const ia_aiq_af_results *af_results,
                      const ia_aiq_sensor_events *sensor_events,
                      const unsigned long long frame_exposure_start,
                      const unsigned long long frame_readout_end);

/*! \brief Execute DVS main process.
 *
 * \param[in]   dvs_state       dvs state.
 *                              This is a pointer to a module.
 * \param[in]   focus_position  Focus motor position in terms of those used by the sensor module.
 *                              Value 0 will use the first LDC grid from the CPF.
 * \return                      0 for no error, others for error.
 *
 * This function processes the DVS main functionality. This generates a
 * lens distortion configuration and calculates global motion.
 */
LIBEXPORT ia_err
ia_dvs_execute(ia_dvs_state *dvs_state,
               uint16_t focus_position);

/*! \brief allocate the DVS morphing table.
 *
 * \param[in]   dvs_state       dvs state.
 *                              This is a pointer to a module.
 * \param[out]  morph_table     Pointer to Pointer to morphing table.
 * \return                      0 for no error, others for error.
 *
 * This function allocates the memory of dvs morphing table.
 * This allocates the struct ia_dvs_morph_table itself, and the members in the structure.
 */
LIBEXPORT ia_err
ia_dvs_allocate_morph_table(ia_dvs_state *dvs_state,
                            ia_dvs_morph_table **morph_table);

/*! \brief Free the DVS morphing table.
 *
 * \param[in]   morph_table     Pointer to morphing table.
 * \return                      0 for no error, others for error.
 *
 * This function frees the memory of dvs morphing table.
 * Each allocated member in morph_table, and morph_table itself are freed.
 */
LIBEXPORT ia_err
ia_dvs_free_morph_table(ia_dvs_morph_table *morph_table);

/*! \brief Get the GDC morphing table.
 *
 * \param[in]   dvs_state       dvs state.
 *                              This is a pointer to a module.
 * \param[out]  morph_table     Pointer to the GDC morph table
 * \return                      0 for no error, others for error.
 *
 * This function calculates GDC morphing table from lens distortion configuration, digital zoom and global motion.
 */
LIBEXPORT ia_err
ia_dvs_get_morph_table(ia_dvs_state *dvs_state,
                       ia_dvs_morph_table *morph_table);

/*! \brief Set non blanking ratio.
 *
 * \param[in]   dvs_state               dvs_state.
 *                                      This is a pointer to a module.
 * \param[in]   nonblanking_ratio       non blanking ratio.
 *                                      Value 0.0 means no rolling shutter correction.
 * \return                              0 for no error, others for error.
 *
 * This function specifies the rolling shutter correction effect.
 */
LIBEXPORT ia_err
ia_dvs_set_non_blank_ratio(ia_dvs_state *dvs_state,
                           float nonblanking_ratio);

/*! \brief Set digital zoom magnitude.
 *
 * \param[in]   dvs_state       dvs state.
 *                              This is a pointer to a module.
 * \param[in]   magnitude       digital zoom magnitude
 * \return                      0 for no error, others for error.
 *
 * This function specifies the digital zoom magnitude
 */
LIBEXPORT ia_err
ia_dvs_set_digital_zoom_magnitude(ia_dvs_state *dvs_state,
                                  float magnitude);

/*!
 * \brief Set the distortion configuration.
 *
 * This function specifies lens distortion correction grid. This will override LDC defined in CPF.
 *
 * \param[in]   dvs_state           dvs state.
 *                                  This is a pointer to a module.
 * \param[in]   distortion_config   Distortion grid configuration.
 * \return                          0 for no error, others for error.
 */
LIBEXPORT ia_err
ia_dvs_set_distortion_config(ia_dvs_state *dvs_state,
                             const ia_dvs_distortion_config *distortion_config);

/*!
 * \brief Set digital zoom mode.
 *
 * This function specifies the digital zoom mode.
 *
 * In mode ia_dvs_zoom_mode_center ia_dvs_set_digital_zoom_magnitude() is used to control
 * digital zoom. Zooming is performed to the center of the image.
 *
 * In mode ia_dvs_zoom_mode_region ia_dvs_set_digital_zoom_region() is used to control
 * zooming position and magnitude.
 *
 * By default mode ia_dvs_zoom_mode_center is used.
 *
 * \param[in]   dvs_state           dvs state.
 *                                  This is a pointer to a module.
 * \param[in]   zoom_mode           digital zoom mode
 * \return                          0 for no error, others for error.
 */
LIBEXPORT ia_err
ia_dvs_set_digital_zoom_mode(ia_dvs_state *dvs_state,
                             ia_dvs_zoom_mode zoom_mode);

/*!
 * \brief Set digital zoom region.
 *
 * This function specifies the digital zoom region. It requires setting
 * the zoom mode to ia_dvs_zoom_mode_region.
 *
 * \param[in]   dvs_state           dvs state.
 *                                  This is a pointer to a module.
 * \param[in]   zoom_region         Rectangle which is zoomed in.
 *                                  This region is cropped and scaled
 *                                  to the size of the output image.
 *                                  Coordinates are given in BQs.
 * \return                          0 for no error, others for error.
 */
LIBEXPORT ia_err
ia_dvs_set_digital_zoom_region(ia_dvs_state *dvs_state,
                               ia_rectangle *zoom_region);

/*!
 * \brief Set digital zoom coordinate.
 *
 * This function specifies the digital zoom coordinate. It requires setting
 * the zoom mode to ia_dvs_zoom_mode_coordinate.
 *
 * \param[in]   dvs_state           dvs state.
 *                                  This is a pointer to a module.
 * \param[in]   zoom_coordinate     Coordinate which is zoomed in.
 *                                  Coordinate is given in BQs.
 * \return                          0 for no error, others for error.
 */
LIBEXPORT ia_err
ia_dvs_set_digital_zoom_coordinate(ia_dvs_state *dvs_state,
                               ia_coordinate *zoom_coordinate);

/*!
 * \brief Get version.
 * Get version from version header.
 *
 * \return                          Version string.
 */
LIBEXPORT const char* ia_dvs_get_version(void);

#ifdef __cplusplus
}
#endif

#endif /* _IA_DVS_H_ */
