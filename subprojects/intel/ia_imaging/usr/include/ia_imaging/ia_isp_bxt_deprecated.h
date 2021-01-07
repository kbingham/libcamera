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
 * \file ia_isp_bxt_deprecated.h
 * \brief ia_isp_bxt specific implementation.
*/

#ifndef IA_ISP_BXT_DEPRECATED_H_
#define IA_ISP_BXT_DEPRECATED_H_

#include "ia_aiq_types.h"
#include "ia_types.h"
#include "ia_isp_bxt_types.h"

#ifdef __cplusplus
extern "C" {
#endif

LIBEXPORT ia_err
ia_isp_bxt_statistics_convert_awb_from_binary(
    ia_isp_bxt *ia_isp_bxt,
    const ia_binary_data *statistics,
    ia_aiq_rgbs_grid **out_rgbs_grid);

LIBEXPORT ia_err
ia_isp_bxt_statistics_convert_awb(
    ia_isp_bxt *ia_isp_bxt,
    unsigned int stats_width,
    unsigned int stats_height,
    void *c0_avg,
    void *c1_avg,
    void *c2_avg,
    void *c3_avg,
    void *c4_avg,
    void *c5_avg,
    void *c6_avg,
    void *c7_avg,
    void *sat_ratio_0,
    void *sat_ratio_1,
    void *sat_ratio_2,
    void *sat_ratio_3,
    ia_aiq_rgbs_grid **out_rgbs_grid);

#ifdef __cplusplus
}
#endif
#endif /* IA_ISP_BXT_DEPRECATED_H_ */
