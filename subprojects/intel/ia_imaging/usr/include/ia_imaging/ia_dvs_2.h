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
/** @file ia_dvs2.h
 * IA_DVS2. This provides access to the DVS2 Host Library.
 */
#ifndef _IA_DVS2_H_
#define _IA_DVS2_H_

#include <stdio.h>
#include <linux/atomisp.h>
#include "ia_dvs2_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** DVS2 State.
 * Host code access to the DVS2 api with this.
 */
typedef struct t_dvs_facade ia_dvs2_state;

/** @brief Get the DVS2 version.
 *
 * @param[out]  version         text: version
 * @return                      0 for no error, others for error.
 *
 * This function provides the version infomation of DVS2. It is allowed to call this
 * function at anytime include before initialization of DVS2 library.
 */
LIBEXPORT ia_err
dvs_get_version(const char **version);

/** @brief Initialize the DVS2 module.
 *
 * @param[out]  dvs2_state      dvs2 state.
 *                              This is a pointer to a pointer to a module.
 * @param[in]   log_setup       log file setups.
 *                              This is a pointer to a structure which contains
 *                              log settings.
 * @param[in]   debug_env       IA Imaging environment.
 *                              This is a pointer to a structure which contains
 *                              IA Imaging environment.
 * @return                      0 for no error, others for error.
 *
 * This function initializes the DVS2 module. This allocates and initializes
 * internal data structures. This function must always be called before any
 * other dvs2 function except dvs2_get_version is called.
 */
LIBEXPORT ia_err
dvs_init(ia_dvs2_state **dvs2_state,
             const ia_binary_data *a_aiq_tuning_binary,
             const ia_cmc_t *cmc,
             const ia_dvs2_log_setup *log_setup,
             const ia_env *debug_env);

/** @brief Configure the DVS2 module.
 *
 * @param[in]   dvs2_state          dvs2 state.
 * @param[in]   support_config      support functionality configuration
 * @param[in]   gdc_config          gdc configuration
 * @param[in]   characteristics     dvs2 characteristics.
 * @param[in]   digital_zoom_ratio  digital zoom ratio.
 * @param[in]   bd_params           use binary dump.
 *                                  If not needed binary dump, then set this to NULL.
 * @return                          0 for no error, others for error.
 *
 * This function configures the DVS2 module. This allocates and initializes
 * internal data structures. This function must always be called after dvs2_init
 * and before any other dvs2 function is called.
 */
LIBEXPORT ia_err
dvs_config(ia_dvs2_state *dvs2_state,
               const ia_dvs2_configuration *config,
               float digital_zoom_ratio,
               ia_dvs2_binary_dump_params *bd_params);

/** @brief Deinitialize the DVS module.
 *
 * @param[in]   dvs2_state      dvs2 state.
 *                              This is a pointer to a module.
 * @return                      0 for no error, others for error.
 *
 * This function deinitilizes the DVS2 module. This frees the calocated memory.
 */
LIBEXPORT ia_err
dvs_deinit(ia_dvs2_state *dvs2_state);

/** @brief allocate the DVS2 coefficients.
 *
 * @param[in]  grid             grid info for coefficients.
 * @param[out] dvs_coefs        Pointer to Pointer to hor/ver coefficients.
 * @return                      0 for no error, others for error.
 *
 * This function allocates the memory of dvs2 coefficients.
 * allocates the structure: ia_css_dvs2_coefficients, and set the pointer into dvs_coefs.
 * *dvs_coefs->grid field is filled by input.
 * *dvs_coefs->hor_coefs, *dvs_coefs->ver_coefs are outputs.
 * Each member of *dvs_coefs->hor_coefs, *dvs_coefs->ver_coefs are set to allocated memory.
 */
LIBEXPORT ia_err
dvs_allocate_coefficients(const struct atomisp_dvs_grid_info *grid, struct atomisp_dis_coefficients **dvs_coefs);

/** @brief Free the DVS2 coefficients.
 *
 * @param[in]   dvs_coefs       Pointer to hor/ver coefficients.
 * @return                      0 for no error, others for error.
 *
 * This function frees the memory of dvs2 coefficients.
 * Each member of vs_coefs->hor_coefs, dvs_coefs->ver_coefs are freed also.
 */
LIBEXPORT ia_err
dvs_free_coefficients(struct atomisp_dis_coefficients *dvs_coefs);

/** @brief Get the DVS2 coefficients.
 *
 * @param[in]   dvs2_state      dvs2 state.
 *                              This is a pointer to a module.
 * @param[out]  dvs_coefs       Pointer to hor/ver coefficients
 * @return                      0 for no error, others for error.
 *
 * This function gets the dvs2 coefficients
 */
LIBEXPORT ia_err
dvs_get_coefficients(const ia_dvs2_state *dvs2_state, struct atomisp_dis_coefficients *dvs_coefs);

/** @brief Allocate statistics buffer.
 *
 * @param[in]   grid            grid info for statistics.
 * @param[out]  statistics      Pointer to pointer to Grid statistics from ISP
 * @return                      0 for no error, others for error.
 *
 * Returns allocated DVS statistics.
 */
LIBEXPORT ia_err
dvs_allocate_statistics(const struct atomisp_dvs_grid_info *grid, struct atomisp_dvs2_statistics **statistics);

/** @brief Free statistics buffer.
 *
 * @param[in]   statistics      Pointer to Grid statistics from ISP
 * @return                      0 for no error, others for error.
 *
 * Free the statistics buffer.
 */
LIBEXPORT ia_err
dvs_free_statistics(struct atomisp_dvs2_statistics *statistics);

/** @brief Set the DVS2 statistics.
 *
 * @param[in]   dvs2_state      dvs2 state.
 *                              This is a pointer to a module.
 * @param[in]   statistics      Pointer to Grid statistics from ISP
 * @return                      0 for no error, others for error.
 *
 * This function receives the inner product from the ISP.
 */
LIBEXPORT ia_err
dvs_set_statistics(ia_dvs2_state *dvs2_state, const struct atomisp_dvs2_statistics *statistics);

/** @brief Execute DVS2 main process.
 *
 * @param[in]   dvs2_state      dvs2 state.
 *                              This is a pointer to a module.
 * @return                      0 for no error, others for error.
 *
 * This function processes the DVS2 main functionality. This generates a
 * GDC morphing table.
 */
LIBEXPORT ia_err
dvs_execute(ia_dvs2_state *dvs2_state);

/** @brief allocate the DVS2 morphing table.
 *
 * @param[in]   dvs2_state      dvs2 state.
 *                              This is a pointer to a module.
 * @param[out]  morph_table     Pointer to Pointer to morphing table.
 * @return                      0 for no error, others for error.
 *
 * This function allocates the memory of dvs2 morphing table.
 * This allocates the struct atomisp_dvs_6axis_config itself, and the members in the structure.
 */
LIBEXPORT ia_err
dvs_allocate_morph_table(ia_dvs2_state *dvs2Lib, struct atomisp_dvs_6axis_config **morph_table);

/** @brief Free the DVS2 morphing table.
 *
 * @param[in]   morph_table     Pointer to morphing table.
 * @return                      0 for no error, others for error.
 *
 * This function frees the memory of dvs2 morphing table.
 * Each allocated member in morph_table, and morph_table itself are freed.
 */
LIBEXPORT ia_err
dvs_free_morph_table(struct atomisp_dvs_6axis_config *morph_table);

/** @brief Get the GDC morphing table.
 *
 * @param[in]   dvs2_state      dvs2 state.
 *                              This is a pointer to a module.
 * @param[out]  morph_table     Pointer to the GDC morph table
 * @return                      0 for no error, others for error.
 *
 * This function gets the calculated GDC morphing table.
 */
LIBEXPORT ia_err
dvs_get_morph_table(ia_dvs2_state *dvs2_state,
                        struct atomisp_dvs_6axis_config *morph_table);

/** @brief Output log of the GDC morphing table.
 *
 * @param[in]   dvs2_state      dvs2 state.
 *                              This is a pointer to a module.
 * @param[in]   log_path        log file path.
 *                              This is a pointer to a log file path.
 * @return                      0 for no error, others for error.
 *
 * This function outputs the calculated GDC morphing table log.
 */
LIBEXPORT ia_err
dvs_morph_table_log(ia_dvs2_state *dvs2_state,
                        const char *log_path);

/** @brief Set no blanking ratio.
 *
 * @param[in]   dvs2_state              dvs2 state.
 *                                      This is a pointer to a module.
 * @param[in]   nonblanking_ratio       non blanking ratio.
 *                                      Value 0.0 means no rolling shutter correction.
 * @return                              0 for no error, others for error.
 *
 * This function specifies the rolling shutter correction effect.
 */
LIBEXPORT ia_err
dvs_set_non_blank_ratio(ia_dvs2_state *dvs2_state, float nonblanking_ratio);

/** @brief Set the local motion coring threshold.
 *
 * @param[in]   dvs2_state              dvs2 state.
 *                                      This is a pointer to a module.
 * @param[in]   min_local_motion        local motion coring threshold.
 * @return                              0 for no error, others for error.
 *
 * This function specifies the local motion coring threshold.
 */
LIBEXPORT ia_err
dvs_set_min_local_motion(ia_dvs2_state *dvs2_state, float min_local_motion);

/** @brief Set Cut off frequency (for video only).
 *
 * @param[in]   dvs2_state              dvs2 state.
 *                                      This is a pointer to a module.
 * @param[in]   cutoff_frequency        cut off frequency.
 * @return                              0 for no error, others for error.
 *
 * This function specifies the cut off frequency.
 */
LIBEXPORT ia_err
dvs_set_cut_off_frequency(ia_dvs2_state *dvs2_state, float *cutoff_frequency);

/** @brief Set digital zoom magnitude.
 *
 * @param[in]   dvs2_state      dvs2 state.
 *                              This is a pointer to a module.
 * @param[in]   magnitude       digital zoom magnitude
 * @return                      0 for no error, others for error.
 *
 * This function specifies the digital zoom magnitude
 */
LIBEXPORT ia_err
dvs_set_digital_zoom_magnitude(ia_dvs2_state *dvs2_state, float magnitude);

/*  */
/** @brief Set Distortion Coefficient.
 *
 * @param[in]   dvs2_state      dvs2 state.
 *                              This is a pointer to a module.
 * @param[in]   distortion      distortion coefficients.
 * @return                      0 for no error, others for error.
 *
 * This function specifies the distortion coefficient.
 */
LIBEXPORT ia_err
dvs_set_distortion_coeff(ia_dvs2_state *dvs2_state, const ia_dvs2_distortion_coefs *distortion);

/** @brief Set the wavelength of detector vector in BQs.
 *
 * @param[in]   dvs2_state      dvs2 state.
 *                              This is a pointer to a module.
 * @param[in]   wave_length     wave length of detector vector in BQs.
 *                              recommended: 1.0*AreaSize for DVS
 *                                           0.5*AreaSize for DIS
 * @return                      0 for no error, others for error.
 *
 * This function specifies the avelength of detector vector.
 * This re-calculates the coefficients, so should call dvs2_get_coefficients()
 * after this was called.
 */
LIBEXPORT ia_err
dvs_set_wave_length(ia_dvs2_state *dvs2_state, float wave_length);

/** @brief Reset the inter_frame parameter.
 *
 * @param[in]   dvs2_state      dvs2 state.
 *                              This is a pointer to a module.
 * @return                      0 for no error, others for error.
 *
 * This function resets the inter_frame parameter.
 */
LIBEXPORT ia_err
dvs_reset_interframe_parameter(ia_dvs2_state *dvs2_state);

/** @brief Get the pointer to inner product.
 * @param[in]   dvs2_state      dvs2 state.
 *                              This is a pointer to a module.
 * @param[out]  v_product       pointer to a pointer to the virtical inner product.
 * @param[out]  h_product       pointer to a pointer to the horizontal inner product.
 * @return                      0 for no error, others for error.
 *
 * This function provides the pointer to the inner product.
 */
LIBEXPORT ia_err
dvs_get_prod_ptr(const ia_dvs2_state *dvs2_state, float **v_product, float **h_product);

/** @brief Set enable flag to main log module.
 *
 * @param[in]   dvs2_state      dvs2 state.
 *                              This is a pointer to a module.
 * @param[in]   enable          Enable flag.
 * @return                      0 for no error, others for error.
 *
 * This function sets enable flag to main log module.
 */
LIBEXPORT ia_err
dvs_set_enable_info_log(ia_dvs2_state *dvs2_state, int enable);

/** @brief Set enable flag to GDC log module.
 *
 * @param[in]   dvs2_state      dvs2 state.
 *                              This is a pointer to a module.
 * @param[in]   enable          Enable flag.
 * @return                      0 for no error, others for error.
 *
 * This function sets enable flag to GDC log module.
 */
LIBEXPORT ia_err
dvs_set_enable_gdc_log(ia_dvs2_state *dvs2_state, int enable);

/** @brief Set enable flag to FP log module.
 *
 * @param[in]   dvs2_state      dvs2 state.
 *                              This is a pointer to a module.
 * @param[in]   enable          Enable flag.
 * @return                      0 for no error, others for error.
 *
 * This function sets enable flag to FP log module.
 */
LIBEXPORT ia_err
dvs_set_enable_fp_log(ia_dvs2_state *dvs2_state, int enable);

/** @brief Check gdc constraints
 *
 * @param[in]   dvs2_state      dvs2 state.
 *                              This is a pointer to a module.
 * @param[out]  valid           1 for gdc setup is valid, 0 for invalid.
 * @param[in]   gdc_hw_config   GDC hardware configuration info.
 * @return                      0 for no error, others for error.
 *
 * This function checks whether gdc setup is valid or not.
 */
LIBEXPORT ia_err
dvs_check_gdc_constraints(ia_dvs2_state *dvs2_state, int *valid, ia_dvs2_gdc_hw_configuration *gdc_hw_config);

/** @brief Start dvs2 debug log.
 *
 * @param[in]   endless        true for endess mode, false for full-end mode.
 * @return                     0 for no error, others for error.
 *
 * This function sets enable to debug log.
 * Endless mode dumps log into buffer circularly.
 * Full-end mode dumps log into buffer by full.
 * If buffer is full, stops to dump in full-end mode.
 */
LIBEXPORT ia_err
dvs_start_dbglog(bool endless);

/** @brief Stop dvs2 debug log.
 *
 * @return                     0 for no error, others for error.
 *
 * This function sets disable to debug log.
 */
LIBEXPORT ia_err
dvs_stop_dbglog(void);

/** @brief Resets dvs2 debug log.
 *
 * @return                     0 for no error, others for error.
 *
 * This function resets the buffer counter to 0 and clear the buffer (fill by 0).
 * In full-end mode, log is activated again.
 */
LIBEXPORT ia_err
dvs_reset_dbglog(void);

/** @brief Denitiaize dvs2 debug log.
 *
 * @return                     0 for no error, others for error.
 *
 * This function deinitializes dvs2 debug log.
 */
LIBEXPORT ia_err
dvs_deinit_dbglog(void);

/** @brief Set enable flag to item of binary dump.
 *
 * @param[in]   item           binary dump item.
 * @param[in]   enable         Enable flag.
 * @return                     0 for no error, others for error.
 *
 * This function sets enable flag to item of binary dump.
 */
LIBEXPORT ia_err
dvs_set_enable_dbglog_item(ia_dvs2_binary_dump_item item, bool enable);

/** @brief Writes binary dump data into the file.
 *
 * @param[in]   fp             file pointer to be writen log.
 * @return                     0 for no error, others for error.
 *
 * This function writes log into the file.
 */
LIBEXPORT ia_err
dvs_fwrite_dbglog(FILE *fp);

LIBEXPORT ia_err
dvs_dump_prods(ia_dvs2_state *dvs2_state, const struct atomisp_dvs2_statistics *dvs_stats);

#ifdef __cplusplus
}
#endif

#endif /* _IA_DVS2_H_ */
