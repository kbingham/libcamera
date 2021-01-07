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

#pragma once

#include "IPU3AICCommon.h"

struct SkyCamAICRuntimeParams
{
	long long time_stamp;
	ia_aiq_frame_use frame_use;
	int mode_index;
    const aic_input_frame_parameters_t *input_frame_params;  /*!< Mandatory. Inputr frame parameters. Describe frame scaling/cropping done in sensor. */
	const aic_resolution_config_parameters_t *frame_resolution_parameters;
	const ia_aiq_output_frame_parameters_t *output_frame_params; /*!< Mandatory. Output frame parameters.  */
    const ia_aiq_exposure_parameters *exposure_results;    /*!< Mandatory. Exposure parameters which are to be used to calculate next ISP parameters. */
    const ia_aiq_hist_weight_grid *weight_grid;
	const ia_aiq_awb_results *awb_results;                 /*!< Mandatory. WB results which are to be used to calculate next ISP parameters (WB gains, color matrix,etc). */
    const ia_aiq_gbce_results *gbce_results;               /*!< Mandatory. GBCE Gamma tables which are to be used to calculate next ISP parameters.
                                                                If NULL pointer is passed, AIC will use static gamma table from the CPF.  */
    const ia_aiq_pa_results *pa_results;                   /*!< Mandatory. Parameter adaptor results from AIQ. */
	const ia_aiq_sa_results *sa_results;                   /*!< Mandatory. Shading adaptor results from AIQ. */
    uint32_t isp_vamem_type;                               /*!< Mandatory. ISP vamem type. */

    char manual_brightness;                                /*!< Optional. Manual brightness value range [-128,127]. */
    char manual_contrast;                                  /*!< Optional. Manual contrast value range [-128,127]. */
    char manual_hue;                                       /*!< Optional. Manual hue value range [-128,127]. */
    char manual_saturation;                                /*!< Optional. Manual saturation value range [-128,127]. */
    char manual_sharpness;                                 /*!< Optional. Manual setting for sharpness [-128,127]. */
    ia_isp_effect effects;                                 /*!< Optional. Manual setting for special effects. Combination of ia_aiq_effect enums.*/
	ia_rectangle *focus_rect;
	sd_dpc_output *scdpc_data;
};
