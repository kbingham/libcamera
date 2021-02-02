/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Incorporate equivalent structures for the AIC algorithms without requiring
 * all of the header based implementation provided by the IA AIQ library
 * headers.
 */

#ifndef IPA_IPU3_AIC_H
#define IPA_IPU3_AIC_H

#include <ia_imaging/ia_aiq.h>

#include <ia_imaging/ia_css_lin_types.h>
#include <ia_imaging/obgrid_public.h>
#include <ia_imaging/bnr_public.h>
#include <ia_imaging/shd_public.h>
#include <ia_imaging/dm_public.h>
#include <ia_imaging/rgbpp_public.h>
#include <ia_imaging/yuvp1_b0_public.h>
#include <ia_imaging/yuvp1_c0_public.h>
#include <ia_imaging/yuvp2_public.h>
#include <ia_imaging/ia_css_tnr3_types.h>
#include <ia_imaging/dpc_public.h>
#include <ia_imaging/awb_public.h>
#include <ia_imaging/awb_fr_public.h>
#include <ia_imaging/anr_public.h>
#include <ia_imaging/af_public.h>
#include <ia_imaging/ae_public.h>
#include <ia_imaging/bds_public.h>
#include <ia_imaging/ia_css_xnr3_types.h>
#include <ia_imaging/ia_css_rgbir_types.h>

typedef struct aic_config {
    struct ia_css_2500_lin_kernel_config     lin_2500_config;       /**< Skylake: Linearization config */
    struct ia_css_2500_obgrid_kernel_config  obgrid_2500_config;    /**< Skylake: OBGRID config */
    struct ia_css_2500_bnr_kernel_config     bnr_2500_config;       /**< Skylake: bayer denoise config */
    struct ia_css_2500_shd_kernel_config     shd_2500_config;       /**< Skylake: shading config */
    struct ia_css_2500_dm_kernel_config      dm_2500_config;        /**< Skylake: demosaic config */
    struct ia_css_2500_rgbpp_kernel_config   rgbpp_2500_config;     /**< Skylake: RGBPP config */
    struct ia_css_2500_yuvp1_b0_kernel_config   yuvp1_2500_config;  /**< Skylake: yuvp1 config */
    struct ia_css_2500_yuvp1_c0_kernel_config yuvp1_c0_2500_config;	/**< Skylake: yuvp1_c0 config */
    struct ia_css_2500_yuvp2_kernel_config   yuvp2_2500_config;     /**< Skylake: yuvp2 config */
    struct ia_css_tnr3_kernel_config         tnr3_2500_config;       /**< Skylake: TNR3 config */

    struct ia_css_2500_dpc_kernel_config     dpc_2500_config;       /**< Skylake: DPC config */
    struct ia_css_2500_awb_kernel_config     awb_2500_config;       /**< Skylake: auto white balance config */
    struct ia_css_2500_awb_fr_kernel_config  awb_fr_2500_config;    /**< Skylake: auto white balance filter response config */
    struct ia_css_2500_anr_kernel_config     anr_2500_config;       /**< Skylake: ANR config */
    struct ia_css_2500_af_kernel_config      af_2500_config;        /**< Skylake: auto focus config */
    struct ia_css_2500_ae_kernel_config      ae_2500_config;        /**< Skylake: auto exposure config */
    struct ia_css_2500_bds_kernel_config     bds_2500_config;       /**< Skylake: bayer downscaler config */
    struct ia_css_xnr3_config                xnr_2500_config;       /**< Skylake: XNR3 config */
#ifdef MACRO_KBL_AIC
    struct ia_css_2500_rgbir_kernel_config   rgbir_2500_config;     /**< Skylake: rgbir config */
#endif
} aic_config_t;

#endif /* IPA_IPU3_AIC_H */
