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

#pragma  once


#include "ia_aiq.h"
#include "stats_3a_public.h"


#ifdef __cplusplus
extern "C" {
#endif

    ia_err skycam_statistics_convert(
        const ia_css_4a_statistics* statistics,
        ia_aiq_rgbs_grid *out_rgbs_grid,
        ia_aiq_af_grid *out_af_grid);

#ifdef __cplusplus
}
#endif
