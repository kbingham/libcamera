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

#ifndef _RGBPP_PUBLIC_H_
#define _RGBPP_PUBLIC_H_

/** @file
 * \brief CSS-API header file for 2500/Skycam specific RGB Per-pixel Correction (Color Correction) accelerator.
 * \details RGB PPC accelerator performs mainly per pixels calculations, includes 3 fixed functions: CCM, GC, CSC_CDS
 * the accelerator inputs RGB stream at s13.0 format and outputs yuv stream at s13.0
 */
//#include <type_support.h>


/**
 * \brief Color Correction Matrix (CCM) parameters
 * \details CCM transforms sensor RGB color space into standard RGB color space
 */

struct ccm_public_matrix_coeffs {
	signed short m11; /**< ccm 3x3 coeff m11 */
	signed short m12; /**< ccm 3x3 coeff m12 */
	signed short m13; /**< ccm 3x3 coeff m13 */
	signed short m21; /**< ccm 3x3 coeff m21 */
	signed short m22; /**< ccm 3x3 coeff m22 */
	signed short m23; /**< ccm 3x3 coeff m23 */
	signed short m31; /**< ccm 3x3 coeff m31 */
	signed short m32; /**< ccm 3x3 coeff m32 */
	signed short m33; /**< ccm 3x3 coeff m33 */
};

/**
 * enum define the range of ccm coeffs values
 */

enum {
	CCM_M11_MAX = 32767,  /**< max range of ccm 3x3 coeff m11 */
	CCM_M11_MIM = -32768, /**< min range of ccm 3x3 coeff m11 */
	CCM_M12_MAX = 8191,   /**< max range of ccm 3x3 coeff m12 */
	CCM_M12_MIM = -8192,  /**< min range of ccm 3x3 coeff m12 */
	CCM_M13_MAX = 32767,  /**< max range of ccm 3x3 coeff m13 */
	CCM_M13_MIM = -32768, /**< min range of ccm 3x3 coeff m13 */
	CCM_M21_MAX = 32767,  /**< max range of ccm 3x3 coeff m21 */
	CCM_M21_MIM = -32768, /**< min range of ccm 3x3 coeff m21 */
	CCM_M22_MAX = 8191,   /**< max range of ccm 3x3 coeff m22 */
	CCM_M22_MIM = -8192,  /**< min range of ccm 3x3 coeff m22 */
	CCM_M23_MAX = 32767,  /**< max range of ccm 3x3 coeff m23 */
	CCM_M23_MIM = -32768, /**< min range of ccm 3x3 coeff m23 */
	CCM_M31_MAX = 32767,  /**< max range of ccm 3x3 coeff m31 */
	CCM_M31_MIM = -32768, /**< min range of ccm 3x3 coeff m31 */
	CCM_M32_MAX = 8191,   /**< max range of ccm 3x3 coeff m32 */
	CCM_M32_MIM = -8192,  /**< min range of ccm 3x3 coeff m32 */
	CCM_M33_MAX = 32767,  /**< max range of ccm 3x3 coeff m33 */
	CCM_M33_MIM = -32768, /**< min range of ccm 3x3 coeff m33 */
	CCM_OFF_MAX = 8191,   /**< min range of bias 3x1 coeffs */
	CCM_OFF_MIN = -8192,  /**< max range of bias 3x1 coeffs */
};

struct ccm_public_offsets {
	signed short R; /**< ccm bias 3x1 coeff r */
	signed short G; /**< ccm bias 3x1 coeff g */
	signed short B; /**< ccm bias 3x1 coeff b */
};

struct ccm_public_config {
	struct ccm_public_matrix_coeffs matrix_coeffs; /**< ccm 3x3 coeffs matrix */
	struct ccm_public_offsets       offsets;       /**< ccm bias 3x1 coeffs */
};

/**
 * \brief Gamma Correction (GC) parameters
 * \details GC applies gamma correction to all pixels
 */

#define GAMMA_CORR_NUM_OF_LUT_ENTRIES 256 /**< number of elements in gamma correction LUT */

struct gamma_corr_public_config {
	unsigned short lut_entries[GAMMA_CORR_NUM_OF_LUT_ENTRIES]; /**< gamma correction LUT array */
	unsigned short enable;                                     /**< enable/disable gamma correction, 0:disable 1:enable */
};


/**
 * \brief Color Space Converter (SCS) parameters
 * \details SCS transforms RGB color space to YUV color space
 */


struct csc_public_C_mat {
	signed short c11; /**< csc 3x3 coeff c11 */
	signed short c12; /**< csc 3x3 coeff c12 */
	signed short c13; /**< csc 3x3 coeff c13 */
	signed short c21; /**< csc 3x3 coeff c21 */
	signed short c22; /**< csc 3x3 coeff c22 */
	signed short c23; /**< csc 3x3 coeff c23 */
	signed short c31; /**< csc 3x3 coeff c31 */
	signed short c32; /**< csc 3x3 coeff c32 */
	signed short c33; /**< csc 3x3 coeff c33 */
};

struct csc_public_b_offset {
	signed short b1; /**< csc bias 3x1 coeff b1 */
	signed short b2; /**< csc bias 3x1 coeff b2 */
	signed short b3; /**< csc bias 3x1 coeff b3 */
};

struct csc_public_config {
	struct csc_public_C_mat mat;       /**< 3x3 conversion coeffs matrix */
	struct csc_public_b_offset offset; /**< ccm bias 3x1 coeffs */
};

/**
 * \brief Chroma Down Scaling (CDS) parameters
 * \details CDS performs down sampling of the Chroma plain
 */

struct cds_public_coeffs {
	unsigned char c00; /**< cds coeff c00 */
	unsigned char c01; /**< cds coeff c01 */
	unsigned char c02; /**< cds coeff c02 */
	unsigned char c03; /**< cds coeff c03 */
	unsigned char c10; /**< cds coeff c10 */
	unsigned char c11; /**< cds coeff c11 */
	unsigned char c12; /**< cds coeff c12 */
	unsigned char c13; /**< cds coeff c13 */
};


struct cds_public_config {
	struct cds_public_coeffs coeffs; /**< 8 coefficients for chroma output downscaling */
	unsigned char nf;                /**< normalization factor for chroma output downscaling */
	/**
	 * note: css fw currently supports only yuv420 format and always applies cds,
	 * as opposed to HW capabilities which supports yuv420 and yuv422 formats, and can enable or disable cds
	 */
};


/**
 * /brief RGBPP parameters
 * /details struct with all parameters for RGBPP kernel that can be
 * applied from the CSS API.
 */

struct ia_css_2500_rgbpp_kernel_config {
	struct ccm_public_config ccm;          /** ccm parameters */
	struct gamma_corr_public_config gamma; /** gamma parameters */
	struct csc_public_config csc;          /** csc parameters */
	struct cds_public_config cds;          /** cds parameters */
};

/** @brief Print RGBPP public configuration
 *
 * @param	cfg	The pointer to configuration data
 * @return	None
 *
 * Print RGBPP public configuration.
 */
void ia_css_rgbpp_public_cfg_dump(const struct ia_css_2500_rgbpp_kernel_config *cfg);

/** @brief Compare two RGBPP public configurations
 *
 * @param	cfg1	The pointer to first configuration data
 * @param	cfg2	The pointer to second configuration data
 * @param	cfg_dump	Configurations are printed in case of
 *   			   mismatch
 * @return	true - match, false - not match
 *
 * Compare two RGBPP public configurations
 */
bool ia_css_rgbpp_public_cfg_compare(
	const struct ia_css_2500_rgbpp_kernel_config *cfg1,
	const struct ia_css_2500_rgbpp_kernel_config *cfg2,
	bool cfg_dump);

#endif /* _RGBPP_PUBLIC_H_ */
