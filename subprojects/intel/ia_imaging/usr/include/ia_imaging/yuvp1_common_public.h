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

#ifndef _YUVP1_COMMON_PUBLIC_H_
#define _YUVP1_COMMON_PUBLIC_H_

/** @file
* CSS-API header file for 2500/Skycam specific noice reduction (YUV_P1 component) kernel/accelerator.
*/

/* All structs in this file will be visible to the CSS API User.
 * The User will be able to set each one of the config params separately by
 * specifying a different config type (see sh_config_type in sh_css.h).
 * If one of those config structs is not set explicitly, a default value will
 * be use instead.
 */

struct yuvp1_yds_public_config {

	unsigned char c00;
	unsigned char c01;
	unsigned char c02;
	unsigned char c03;
	unsigned char c10;
	unsigned char c11;
	unsigned char c12;
	unsigned char c13;
	unsigned char norm_factor;
	unsigned char bin_output;

};

struct yuvp1_chnr_coring_public_config {

	unsigned short u;
	unsigned short v;

};

struct yuvp1_chnr_sense_gain_public_config {

	unsigned char vy;
	unsigned char vu;
	unsigned char vv;

	unsigned char hy;
	unsigned char hu;
	unsigned char hv;
};

struct yuvp1_chnr_iir_fir_public_config {

	unsigned char fir_0h;
	unsigned char fir_1h;
	unsigned char fir_2h;
	unsigned short iir_min_prev; /* like in the ATE filter */

};

struct yuvp1_chnr_public_config {

	struct yuvp1_chnr_coring_public_config coring;
	struct yuvp1_chnr_sense_gain_public_config sense_gain;
	struct yuvp1_chnr_iir_fir_public_config iir_fir;

};

#endif
