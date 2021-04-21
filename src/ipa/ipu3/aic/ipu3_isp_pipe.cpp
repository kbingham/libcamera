/* SPDX-License-Identifier: Apache-2.0 */
/*
 * IPU3ISPPipe: Derived class from ISPPipe storing the AIC config
 */

#include <string.h>

#include "ipu3_isp_pipe.h"

#include "libcamera/internal/log.h"

namespace libcamera::ipa::ipu3::aic {

LOG_DEFINE_CATEGORY(IPU3ISPPipe)

#define CLEAR(x) memset(&(x), 0, sizeof(x))

IPU3ISPPipe::IPU3ISPPipe()
{
	CLEAR(AicOutput);
	CLEAR(AicConfig);
}

void IPU3ISPPipe::SetPipeConfig(const aic_output_t pipe_config)
{
	AicOutput = pipe_config;

	if (AicOutput.lin_2500_config)
		AicConfig.lin_2500_config = *AicOutput.lin_2500_config;
	if (AicOutput.obgrid_2500_config)
		AicConfig.obgrid_2500_config = *AicOutput.obgrid_2500_config;
	if (AicOutput.bnr_2500_config)
		AicConfig.bnr_2500_config = *AicOutput.bnr_2500_config;
	if (AicOutput.shd_2500_config)
		AicConfig.shd_2500_config = *AicOutput.shd_2500_config;
	if (AicOutput.dm_2500_config)
		AicConfig.dm_2500_config = *AicOutput.dm_2500_config;
	if (AicOutput.rgbpp_2500_config)
		AicConfig.rgbpp_2500_config = *AicOutput.rgbpp_2500_config;
	if (AicOutput.yuvp1_2500_config)
		AicConfig.yuvp1_2500_config = *AicOutput.yuvp1_2500_config;
	if (AicOutput.yuvp1_c0_2500_config)
		AicConfig.yuvp1_c0_2500_config = *AicOutput.yuvp1_c0_2500_config;
	if (AicOutput.yuvp2_2500_config)
		AicConfig.yuvp2_2500_config = *AicOutput.yuvp2_2500_config;
	if (AicOutput.tnr3_2500_config)
		AicConfig.tnr3_2500_config = *AicOutput.tnr3_2500_config;
	if (AicOutput.dpc_2500_config)
		AicConfig.dpc_2500_config = *AicOutput.dpc_2500_config;
	if (AicOutput.awb_2500_config)
		AicConfig.awb_2500_config = *AicOutput.awb_2500_config;
	if (AicOutput.awb_fr_2500_config)
		AicConfig.awb_fr_2500_config = *AicOutput.awb_fr_2500_config;
	if (AicOutput.anr_2500_config)
		AicConfig.anr_2500_config = *AicOutput.anr_2500_config;
	if (AicOutput.af_2500_config)
		AicConfig.af_2500_config = *AicOutput.af_2500_config;
	if (AicOutput.ae_2500_config)
		AicConfig.ae_2500_config = *AicOutput.ae_2500_config;
	if (AicOutput.xnr_2500_config)
		AicConfig.xnr_2500_config = *AicOutput.xnr_2500_config;
	if (AicOutput.rgbir_2500_config)
		AicConfig.rgbir_2500_config = *AicOutput.rgbir_2500_config;
}

void IPU3ISPPipe::dump()
{
	if (AicOutput.ae_2500_config) {
		LOG(IPU3ISPPipe, Debug)
			<< "AicOutput.ae_2500_config->ae.ae_grid_config.ae_en: "
			<< (int)AicOutput.ae_2500_config->ae.ae_grid_config.ae_en;
		LOG(IPU3ISPPipe, Debug)
			<< "AicOutput.ae_2500_config->ae.ae_grid_config.block_height: "
			<< (int)AicOutput.ae_2500_config->ae.ae_grid_config.block_height;
		LOG(IPU3ISPPipe, Debug)
			<< "AicOutput.ae_2500_config->ae.ae_grid_config.block_width: "
			<< (int)AicOutput.ae_2500_config->ae.ae_grid_config.block_width;
		LOG(IPU3ISPPipe, Debug)
			<< "AicOutput.ae_2500_config->ae.ae_grid_config.x_start: "
			<< AicOutput.ae_2500_config->ae.ae_grid_config.x_start;
		LOG(IPU3ISPPipe, Debug)
			<< "AicOutput.ae_2500_config->ae.ae_grid_config.y_start: "
			<< AicOutput.ae_2500_config->ae.ae_grid_config.y_start;
		LOG(IPU3ISPPipe, Debug)
			<< "AicOutput.ae_2500_config->ae.ae_grid_config.grid_height: "
			<< (int)AicOutput.ae_2500_config->ae.ae_grid_config.grid_height;
		LOG(IPU3ISPPipe, Debug)
			<< "AicOutput.ae_2500_config->ae.ae_grid_config.grid_width: "
			<< (int)AicOutput.ae_2500_config->ae.ae_grid_config.grid_width;
	}

	if (AicOutput.af_2500_config) {
		LOG(IPU3ISPPipe, Debug)
			<< "AicOutput.af_2500_config->af.grid.grid_height: "
			<< (int)AicOutput.af_2500_config->af.grid.grid_height;
		LOG(IPU3ISPPipe, Debug)
			<< "AicOutput.af_2500_config->af.grid.grid_width: "
			<< (int)AicOutput.af_2500_config->af.grid.grid_width;
	}

	if (AicOutput.anr_2500_config) {
		LOG(IPU3ISPPipe, Debug)
			<< "AicOutput.anr_2500_config: "
			<< AicOutput.anr_2500_config;
	}

	if (AicOutput.awb_2500_config) {
		LOG(IPU3ISPPipe, Debug)
			<< "AicOutput.awb_2500_config->awb.grid.grid_block_height: "
			<< (int)AicOutput.awb_2500_config->awb.grid.grid_block_height;
		LOG(IPU3ISPPipe, Debug)
			<< "AicOutput.awb_2500_config->awb.grid.grid_block_width: "
			<< (int)AicOutput.awb_2500_config->awb.grid.grid_block_width;
		LOG(IPU3ISPPipe, Debug)
			<< "AicOutput.awb_2500_config->awb.grid.grid_height: "
			<< (int)AicOutput.awb_2500_config->awb.grid.grid_height;
		LOG(IPU3ISPPipe, Debug)
			<< "AicOutput.awb_2500_config->awb.grid.grid_width: "
			<< (int)AicOutput.awb_2500_config->awb.grid.grid_width;
		LOG(IPU3ISPPipe, Debug)
			<< "AicOutput.awb_2500_config->awb.grid.grid_x_start: "
			<< AicOutput.awb_2500_config->awb.grid.grid_x_start;
		LOG(IPU3ISPPipe, Debug)
			<< "AicOutput.awb_2500_config->awb.grid.grid_y_start: "
			<< AicOutput.awb_2500_config->awb.grid.grid_y_start;
	}

	if (AicOutput.awb_fr_2500_config) {
		LOG(IPU3ISPPipe, Debug)
			<< "AicOutput.awb_fr_2500_config: "
			<< AicOutput.awb_fr_2500_config;
	}
	if (AicOutput.bnr_2500_config) {
		LOG(IPU3ISPPipe, Debug)
			<< "AicOutput.bnr_2500_config: "
			<< AicOutput.bnr_2500_config;
	}

	if (AicOutput.dm_2500_config) {
		LOG(IPU3ISPPipe, Debug)
			<< "AicOutput.dm_2500_config: "
			<< AicOutput.dm_2500_config;
	}

	if (AicOutput.dpc_2500_config) {
		LOG(IPU3ISPPipe, Debug)
			<< "AicOutput.dpc_2500_config: " << AicOutput.dpc_2500_config;
	}

	if (AicOutput.lin_2500_config) {
		LOG(IPU3ISPPipe, Debug)
			<< "AicOutput.lin_2500_config: " << AicOutput.lin_2500_config;
	}

	if (AicOutput.obgrid_2500_config) {
		LOG(IPU3ISPPipe, Debug)
			<< "AicOutput.obgrid_2500_config: "
			<< AicOutput.obgrid_2500_config;
	}

	if (AicOutput.rgbir_2500_config) {
		LOG(IPU3ISPPipe, Debug)
			<< "AicOutput.rgbir_2500_config: "
			<< AicOutput.rgbir_2500_config;
	}

	if (AicOutput.rgbpp_2500_config) {
		LOG(IPU3ISPPipe, Debug)
			<< "AicOutput.rgbpp_2500_config: "
			<< AicOutput.rgbpp_2500_config;
	}

	if (AicOutput.shd_2500_config) {
		LOG(IPU3ISPPipe, Debug)
			<< "AicOutput.shd_2500_config: "
			<< AicOutput.shd_2500_config;
	}

	if (AicOutput.tnr3_2500_config) {
		LOG(IPU3ISPPipe, Debug)
			<< "AicOutput.tnr3_2500_config: "
			<< AicOutput.tnr3_2500_config;
	}

	if (AicOutput.xnr_2500_config) {
		LOG(IPU3ISPPipe, Debug)
			<< "AicOutput.xnr_2500_config: "
			<< AicOutput.xnr_2500_config;
	}

	if (AicOutput.yuvp1_2500_config) {
		LOG(IPU3ISPPipe, Debug)
			<< "AicOutput.yuvp1_2500_config: "
			<< AicOutput.yuvp1_2500_config;
	}

	if (AicOutput.yuvp1_c0_2500_config) {
		LOG(IPU3ISPPipe, Debug)
			<< "AicOutput.yuvp1_c0_2500_config: "
			<< AicOutput.yuvp1_c0_2500_config;
	}

	if (AicOutput.yuvp2_2500_config) {
		LOG(IPU3ISPPipe, Debug)
			<< "AicOutput.yuvp2_2500_config: "
			<< AicOutput.yuvp2_2500_config;
	}
}

const ia_aiq_rgbs_grid *IPU3ISPPipe::GetAWBStats()
{
	return nullptr;
}

const ia_aiq_af_grid *IPU3ISPPipe::GetAFStats()
{
	return nullptr;
}

const ia_aiq_histogram *IPU3ISPPipe::GetAEStats()
{
	return nullptr;
}

aic_config *IPU3ISPPipe::GetAicConfig()
{
	return &AicConfig;
}

} /* namespace libcamera::ipa::ipu3::aic */

