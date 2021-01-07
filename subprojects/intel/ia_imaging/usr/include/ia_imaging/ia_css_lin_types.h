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

#ifndef _LIN_PUBLIC_H_
#define _LIN_PUBLIC_H_

/** @file
* CSS-API header file for 2500/Skycam linearization lookup (LIN) kernel.
*/

/** Linearization lookup table size. The number of piecewise linear segments is
 * one less than the table size. */
#define LIN_TABLE_SIZE 65

/** Number of piecewise linear segments */
#define LIN_SEGMENTS    (LIN_TABLE_SIZE - 1)

struct ia_css_2500_lin_kernel_config
{
	short curve_lut_GR[LIN_TABLE_SIZE];
	short curve_lut_R[LIN_TABLE_SIZE];
	short curve_lut_B[LIN_TABLE_SIZE];
	short curve_lut_GB[LIN_TABLE_SIZE];
};


#endif // _LIN_PUBLIC_H_
