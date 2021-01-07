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

#include "ia_cmc_types.h"
#include "cpffData.h"
#include "Pipe.h"
#include "SkyCamAICCommon.h"
#include "SkyCamAICVersion.h"

class SkyCamAICImpl;

class  SkyCamAIC
{

public:
    LIBEXPORT SkyCamAIC(ISPPipe *pipe, const ia_cmc_t* cmc_parsed,
/*std::string cpf_file*/ const ia_binary_data* aiqb_, SkyCamAICRuntimeParams runtime_params,
			unsigned int dump_aic_parameters=0, int test_framework_dump=0);
    LIBEXPORT void Run(SkyCamAICRuntimeParams runtime_params);
    LIBEXPORT void Reset(SkyCamAICRuntimeParams runtime_params);
	static LIBEXPORT std::string GetAICVersion(){return SKYCAMAICVERSION;};
    LIBEXPORT ~SkyCamAIC(void);

private:
    SkyCamAICImpl* pimpl_;

	void ConvertRuntimeSkyCamToIPU3(const SkyCamAICRuntimeParams &sklParams, IPU3AICRuntimeParams &ipu3Params);
};


