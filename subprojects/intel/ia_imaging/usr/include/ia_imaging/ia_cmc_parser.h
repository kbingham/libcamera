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
 * \file ia_cmc_parser.h
 * \brief Parser for CMC data.
*/


#ifndef IA_CMC_PARSER_H_
#define IA_CMC_PARSER_H_

#include "ia_types.h"
#include "ia_cmc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

LIBEXPORT ia_cmc_t*
ia_cmc_parser_init(const ia_binary_data *aiqb_binary);

LIBEXPORT void
ia_cmc_parser_deinit(ia_cmc_t *ia_cmc);

#ifdef __cplusplus
}
#endif

#endif /* IA_CMC_PARSER_H_ */
