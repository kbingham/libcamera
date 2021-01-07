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

#ifndef _RGBIR_PUBLIC_H_
#define _RGBIR_PUBLIC_H_
#include <stdint.h>

/** @file
* CSS-API header file for 2500/Skycam RGBIR Remosaic parameters.
*/

/** max lut table width */
#define IA_CSS_RGBIR_MAX_LUT_WIDTH	17
/** max lut table height */
#define IA_CSS_RGBIR_MAX_LUT_HEIGHT	10
/** max lut table size */
#define IA_CSS_RGBIR_MAX_LUT_SIZE	(IA_CSS_RGBIR_MAX_LUT_WIDTH * IA_CSS_RGBIR_MAX_LUT_HEIGHT)

#define DEFAULT_IR_GAIN 512
#define DEFAULT_OB      64

/**
 * RGBIR Remosaic configuration
 *
 * - ob: Optical black level
 *	u10.0: default: 16
 *
 * - ir_height: height of the weights lut
 *	Valid values: [1, IA_CSS_RGBIR_MAX_LUT_HEIGHT]
 *
 * - ir_width: width of the weights lut
 *	Valid values: [1, IA_CSS_RGBIR_MAX_LUT_WIDTH]
 *
 * - ir_weights_r: lut values for red channel
 *	u3.8: spatial varying IR correction
 *	Amount of IR contamination on Red Channel
 *	Must contain at least 'ir_width * ir_height'
 *	valid elements in row-major order.
 *
 * - ir_weights_g: lut values for green channel
 *	u3.8: spatial varying IR correction
 *	Amount of IR contamination on Green Channel
 *	Must contain at least 'ir_width * ir_height'
 *	valid elements in row-major order.
 *
 * - ir_weights_b: lut values for blue channel
 *	u3.8: spatial varying IR correction
 *	Amount of IR contamination on Blue Channel
 *	Must contain at least 'ir_width * ir_height'
 *	valid elements in row-major order.
 *
 * - ir_gain: Digital Gain
 *	u3.8: digital gain applied on raw to compensate
 *	DR loss caused by IR contamination
 */
struct ia_css_2500_rgbir_kernel_config {
	uint16_t ob;						/**< optical black level */
	uint16_t ir_height;					/**< lut height */
	uint16_t ir_width;					/**< lut width */
	uint16_t ir_weights_r[IA_CSS_RGBIR_MAX_LUT_SIZE];	/**< lut values for red channel */
	uint16_t ir_weights_g[IA_CSS_RGBIR_MAX_LUT_SIZE];	/**< lut values for green channel */
	uint16_t ir_weights_b[IA_CSS_RGBIR_MAX_LUT_SIZE];	/**< lut values for blue channel */
	uint16_t ir_gain;					/**< digital gain */
};


#endif /* _RGBIR_PUBLIC_H_ */
