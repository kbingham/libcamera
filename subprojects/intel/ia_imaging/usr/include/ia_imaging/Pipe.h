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

#include "ia_aiq_types.h"
#include "stats_3a_public.h"
#include "rgbpp_public.h"
#include "ia_cp_types.h"


struct ia_cp_output {
    ia_vhdr_config				 *vhdr_config;       /**< KBL: vHDR config */
};

typedef struct aic_output{
    struct ia_css_2500_lin_kernel_config     *lin_2500_config;       /**< Skylake: Linearization config */
    struct ia_css_2500_obgrid_kernel_config  *obgrid_2500_config;    /**< Skylake: OBGRID config */
    struct ia_css_2500_bnr_kernel_config     *bnr_2500_config;       /**< Skylake: bayer denoise config */
    struct ia_css_2500_shd_kernel_config     *shd_2500_config;       /**< Skylake: shading config */
    struct ia_css_2500_dm_kernel_config      *dm_2500_config;        /**< Skylake: demosaic config */
    struct ia_css_2500_rgbpp_kernel_config   *rgbpp_2500_config;     /**< Skylake: RGBPP config */
    struct ia_css_2500_yuvp1_b0_kernel_config *yuvp1_2500_config;     /**< Skylake: yuvp1 config */
    struct ia_css_2500_yuvp2_kernel_config   *yuvp2_2500_config;     /**< Skylake: yuvp2 config */
    struct ia_css_2500_yuvp1_c0_kernel_config *yuvp1_c0_2500_config;		/**< Skylake: yuvp1_c0 config */
    struct ia_css_tnr3_kernel_config         *tnr3_2500_config;	/**< Skylake: TNR3 config */
    struct ia_css_2500_dpc_kernel_config     *dpc_2500_config;       /**< Skylake: DPC config */
    struct ia_css_2500_awb_kernel_config     *awb_2500_config;       /**< Skylake: auto white balance config */
    struct ia_css_2500_awb_fr_kernel_config  *awb_fr_2500_config;    /**< Skylake: auto white balance filter response config */
    struct ia_css_2500_anr_kernel_config     *anr_2500_config;       /**< Skylake: ANR config */
    struct ia_css_2500_af_kernel_config      *af_2500_config;        /**< Skylake: auto focus config */
    struct ia_css_2500_ae_kernel_config      *ae_2500_config;        /**< Skylake: auto exposure config */
    struct ia_css_xnr3_config				 *xnr_2500_config;       /**< Skylake: XNR3 config */
    struct ia_css_2500_rgbir_kernel_config   *rgbir_2500_config;     /**< Skylake: rgbir config */
} aic_output_t;


class ISPPipe
{
public:
	enum pipe_ver{
		Bzero,
		Czero,
        Czero_KBL
	};

    ISPPipe(void)
    {
    }

    virtual ~ISPPipe(void)
    {
    }

	// HACK, not pure virtual function for backward compatibility
	// This function transfers the vHDR paramters to CPlibs vHDR
	virtual void SetCpConfig(const ia_cp_output cp_config) { (void)cp_config; }

	// This function configures the HW/FW pipe via CSS interface
    virtual void SetPipeConfig(const aic_output_t pipe_config) = 0;

	virtual pipe_ver GetPipeVer() = 0;

    virtual const ia_aiq_rgbs_grid* GetAWBStats() = 0;
    virtual const ia_aiq_af_grid* GetAFStats() = 0;
    virtual const ia_aiq_histogram* GetAEStats() = 0;

};

