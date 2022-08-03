/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2022, Ideas on Board Oy.
 *
 * shd.cpp - IPU3 Lens Shading Correction
 */

#include "shd.h"

#include <string.h>

#include <libcamera/base/log.h>

#include "libcamera/internal/yaml_parser.h"


/**
 * \file shd.h
 * \brief IPU3 Lens Shading Correction
 */

namespace libcamera {

namespace ipa::ipu3::algorithms {

LOG_DEFINE_CATEGORY(IPU3SHD)

/**
 * \class LensShadingCorrection
 * \brief A class to handle lens shading correction
 *
 * Due to the optical characteristics of the lens, the light intensity received
 * by the sensor is not uniform.
 *
 * The Lens Shading Correction algorithm applies multipliers to all pixels
 * to compensate for the lens shading effect. The coefficients are
 * specified in a downscaled table in the YAML tuning file.
 */

LensShadingCorrection::LensShadingCorrection()
{
}

static Size getSize(const YamlObject &tuningData,
		    const char *prop)
{
	std::vector<double> data = tuningData[prop].getList<double>().value_or(std::vector<double>{});

	if (data.size() != 2)
		return Size(0, 0);

	return Size(data[0], data[1]);
}

static std::vector<uint16_t> getTable(const YamlObject &tuningData,
                    const char *prop, Size size)
{
	unsigned int expectedTableSize = size.width * size.height;

	std::vector<uint16_t> table =
                tuningData[prop].getList<uint16_t>().value_or(std::vector<uint16_t>{});
        if (table.size() != expectedTableSize) {
                LOG(IPU3SHD, Error)
                        << "Invalid '" << prop << "' values: expected "
                        << expectedTableSize
                        << " elements, got " << table.size();
        }

	return table;
}

/**
 * \copydoc libcamera::ipa::Algorithm::init
 */
int LensShadingCorrection::init([[maybe_unused]] IPAContext &context,
                                const YamlObject &tuningData)
{
	gridSize_ = getSize(tuningData, "gridSize");
	gridBlockSize_ = getSize(tuningData, "gridBlockSize");

	// Clamp to restrict sizes then validate

	gain_ = tuningData["gain"].get<unsigned int>().value_or(1);
	x_ = tuningData["x"].get<unsigned int>().value_or(0);
	y_ = tuningData["y"].get<unsigned int>().value_or(0);

	// Clamp to restrict gain

	const YamlObject &yamlSets = tuningData["sets"];
	if (!yamlSets.isList()) {
		LOG(IPU3SHD, Error)
			<< "'sets' parameter not found in tuning file";
		return -EINVAL;
	}

	double lastCt = -1;

	for (std::size_t i = 0; i < yamlSets.size(); ++i) {
		const YamlObject &yamlSet = yamlSets[i];

		sets_.push_back({});
		componentData &set = sets_.back();

		set.ct = yamlSet["ct"].get<uint16_t>(0);
		if (set.ct <= lastCt) {
			LOG(IPU3SHD, Error)
				<< "Sets must be in increasing ct order";
			return -EINVAL;
		}
		lastCt = set.ct;

		set.r = getTable(yamlSet, "r", gridSize_);
		set.gr = getTable(yamlSet, "gr", gridSize_);
		set.gb = getTable(yamlSet, "gb", gridSize_);
		set.b = getTable(yamlSet, "b", gridSize_);

		if (set.r.empty() || set.gr.empty() ||
		    set.b.empty() || set.gb.empty())
			return -EINVAL;
	}

	LOG(IPU3SHD, Warning) << "GridSize: " << gridSize_;
	LOG(IPU3SHD, Warning) << "GridBlockSize: " << gridBlockSize_;
	LOG(IPU3SHD, Warning) << "Gain: " << gain_;
	
	return 0;
}

/**
 * \brief Fill in the parameter structure, and enable black level correction
 * \param context The shared IPA context
 * \param params The IPU3 parameters
 *
 * Populate the IPU3 parameter structure with the correction values for each
 * channel and enable the corresponding ImgU block processing.
 */
void LensShadingCorrection::prepare([[maybe_unused]] IPAContext &context,
			      ipu3_uapi_params *params)
{
	if (initialized_)
		return;

	ipu3_uapi_shd_config_static *shd = &params->acc_param.shd.shd;
	ipu3_uapi_shd_lut *shd_lut = &params->acc_param.shd.shd_lut;
	
	ipu3_uapi_shd_grid_config *grid = &shd->grid;
	ipu3_uapi_shd_general_config *general = &shd->general;
	ipu3_uapi_shd_black_level_config *blackLevel = &shd->black_level;

	/* Grid Configuration */

	grid->width =  gridSize_.width;
	grid->height =  gridSize_.height;
	grid->block_width_log2 = gridBlockSize_.width;
	grid->block_height_log2 = gridBlockSize_.height;
	/*
	 * @grid_height_per_slice:      SHD_MAX_CELLS_PER_SET/width.
	 *                              (with SHD_MAX_CELLS_PER_SET = 146).
	 */
	grid->grid_height_per_slice = IPU3_UAPI_SHD_MAX_CELLS_PER_SET / gridSize_.width;
	/*
	 * @x_start:    X value of top left corner of sensor relative to ROI
	 *              s13, [-4096, 0], default 0, only negative values.
	 * @y_start:    Y value of top left corner of sensor relative to ROI
	 *              s13, [-4096, 0], default 0, only negative values.
	 */
	grid->x_start = 0;
	grid->y_start = 0;

	/* General Configuration */

	general->init_set_vrt_offst_ul = grid->y_start >> grid->block_height_log2
	       			       % grid->grid_height_per_slice;
	general->shd_enable = 1;
	general->gain_factor = gain_;

	/* Black Level Configuration */

	/* Bios values for each compoent: s11 range [-2048, 2047]. */
	blackLevel->bl_r = 0;
	blackLevel->bl_gr = 0;
	blackLevel->bl_gb = 0;
	blackLevel->bl_b = 0;

	/* Prepare the tables */

	unsigned int pos = 0;
	for (unsigned int s = 0; s < grid->height / grid->grid_height_per_slice; s++) {
		for (unsigned int c = 0; c < grid->width * grid->grid_height_per_slice; c++) {
			if (pos >= bData_.size()) {
				LOG(IPU3SHD, Error) << "Reached the end of the tables: " << pos;
				break;
			}

			LOG(IPU3SHD, Error) << rData_[pos];

			shd_lut->sets[s].r_and_gr[c].r = rData_[pos];
			shd_lut->sets[s].r_and_gr[c].gr = grData_[pos];
			shd_lut->sets[s].gb_and_b[c].gb = gbData_[pos];
			shd_lut->sets[s].gb_and_b[c].b = bData_[pos];
			pos++;
		}
	}

	if (x_ || y_) {
		LOG(IPU3SHD, Error) << "Using x: " << x_ << " y: " << y_;
		unsigned int val = 0xffff;
		shd_lut->sets[x_].r_and_gr[y_].r = val;
		shd_lut->sets[x_].r_and_gr[y_].gr = val;
		shd_lut->sets[x_].gb_and_b[y_].gb = val;
		shd_lut->sets[x_].gb_and_b[y_].b = val;
	}

	/* Enable the shading parameters */
	params->use.acc_shd = 1;

	initialized_ = true;
}

REGISTER_IPA_ALGORITHM(LensShadingCorrection, "LensShadingCorrection")

} /* namespace ipa::ipu3::algorithms */

} /* namespace libcamera */
