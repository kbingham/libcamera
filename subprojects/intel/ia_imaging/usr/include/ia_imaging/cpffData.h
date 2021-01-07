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

#ifndef _CPFF_SPECIFIC_DATA_H_
#define _CPFF_SPECIFIC_DATA_H_

#include <new>

#include <stdint.h>
#include "ia_types.h"

#pragma pack(push,1)

enum INTERPOLATION_TYPE
{
	Linear_INT = 0,
	NearestNeighbor_INT = 1
};

enum EXTRAPOLATION_TYPE
{
	Native_EXT = 0,
	Replicate_EXT = 1,
	NearestNeighbor_EXT = 2
};

enum GageType
{
	AnalogGain_Gage = 0,
	CCT_Gage = 1,
	Contrast_Gage = 2,
	ExposureTime_Gage = 3,
	Const_Gage = 4,
	ScaleFactor_Gage = 5,
	TotalGain_Gage = 6
};

enum NodeType
{
	int32_NodeType = 0,
	float_NodeType = 1
};

enum Blocks
{
	ISP_DefectPixelCorrection = 3,
	ISP_BayerDownScale = 4,
	ISP_GridBlackLevelSubtraction = 7,
	ISP_Linearization = 10,
	ISP_AWB_Statistics = 12,
	ISP_AF_Statistics = 13,
	ISP_BNR_DynamicDPC = 17,
	ISP_Bayer_ANR = 18,
	ISP_Demosaicing = 19,
	ISP_Gamma = 21,
	ISP_ColorSpaceConversion_ChromaDownsampling = 22,
	ISP_Y_EdgeEnhance_NoiseReduction = 25,
	ISP_ChromaNoiseReduction = 27,
	ISP_XNR = 28,
	ISP_UV_Color_Enhancement = 32,
	ISP_TNR = 34,
	ISP_AWB_Filter_Response_Statistics = 37,
	ISP_IEFD = 38,
	ISP_vHDR = 40
};

enum ISP_DefectPixelCorrection
{
	gradThresh = 0
};

enum ISP_BayerDownScale
{
	hor_enable = 0,
	ver_enable = 1,
	hor_coeffs = 2,
	hor_nf = 3,
	ver_coeffs = 4,
	ver_nf = 5,
	phase_count = 6,
	sequence_len = 7,
	sequence_pat = 8
};

enum ISP_GridBlackLevelSubtraction
{
	GBLS_grid_width = 0,
	GBLS_grid_height = 1,
	GBLS_coeff_R = 2,
	GBLS_coeff_Gr = 3,
	GBLS_coeff_Gb = 4,
	GBLS_coeff_B = 5
};

enum ISP_Linearization
{
	LinGr = 0,
	LinR = 1,
	LinB = 2,
	LinGb = 3
};

enum ISP_AWB_Statistics
{
	Rgbs_thr_gr = 0,
	Rgbs_thr_r = 1,
	Rgbs_thr_b = 2,
	Rgbs_thr_gb = 3,
	Rgbs_incl_sat = 4,
	Rgbs_grid_width = 5,
	Rgbs_grid_height = 6,
	Rgbs_block_width = 7,
	Rgbs_block_height = 8,
	Rgbs_Xstart = 9,
	Rgbs_Ystart = 10
};

enum ISP_AWB_Filter_Response_Statistics
{
	grid_width = 0,
	grid_height = 1,
	block_width = 2,
	block_height = 3,
	Xstart = 4,
	Ystart = 5,
	bayer_filter_coeffs_A = 6,
	bayer_filter_coeffs_sign_vector = 7
};

enum ISP_AF_Statistics
{
	YgenRateGr = 0,
	YgenRateR = 1,
	YgenRateB = 2,
	YgenRateGb = 3,
	Filter_grid_width = 4,
	Filter_grid_height = 5,
	Filter_block_width = 6,
	Filter_block_height = 7,
	Filter_Xstart = 8,
	Filter_Ystart = 9,
	Y1_filter_coeffs_A = 10,
	Y1_filter_coeffs_sign_vector = 11,
	Y2_filter_coeffs_A = 12,
	Y2_filter_coeffs_sign_vector = 13
};

enum ISP_BNR_DynamicDPC
{
	Cf = 0,
	Cg = 1,
	Ci = 2,
	R_NF = 3,
	bpThresholdGain = 4,
	Defect_mode = 5,
	bpGain = 6,
	w0_coef = 7,
	w1_coef = 8,
	alpha_param = 9,
	beta_param = 10,
	gama_param = 11,
	MaxInf = 12,
	GD_en = 13,
	BPC_En = 14,
	BNR_En = 15,
	GD_red = 16,
	GD_green = 17,
	GD_blue = 18,
	GD_black = 19,
	GD_shading = 20,
	GD_suppoprt = 21,
	GD_Clip = 22,
	GD_Central_Weight = 23,
	ShadCoef_gr = 24,
	ShadCoef_r = 25,
	ShadCoef_b = 26,
	ShadCoef_gb = 27,
	X_Center = 28,
	Y_Center = 29
};

enum ISP_Bayer_ANR
{
	adaptive_treshhold_en = 0,
	basic_transPlane0_Alpha_Gr = 1,
	basic_transPlane0_Alpha_R = 2,
	basic_transPlane0_Alpha_B = 3,
	basic_transPlane0_Alpha_Gb = 4,
	basic_transPlane0_Alpha_DC_Gr = 5,
	basic_transPlane0_Alpha_DC_R = 6,
	basic_transPlane0_Alpha_DC_B = 7,
	basic_transPlane0_Alpha_DC_Gb = 8,
	basic_transPlane1_Alpha_Gr = 9,
	basic_transPlane1_Alpha_R = 10,
	basic_transPlane1_Alpha_B = 11,
	basic_transPlane1_Alpha_Gb = 12,
	basic_transPlane1_Alpha_DC_Gr = 13,
	basic_transPlane1_Alpha_DC_R = 14,
	basic_transPlane1_Alpha_DC_B = 15,
	basic_transPlane1_Alpha_DC_Gb = 16,
	basic_transPlane2_Alpha_Gr = 17,
	basic_transPlane2_Alpha_R = 18,
	basic_transPlane2_Alpha_B = 19,
	basic_transPlane2_Alpha_Gb = 20,
	basic_transPlane2_Alpha_DC_Gr = 21,
	basic_transPlane2_Alpha_DC_R = 22,
	basic_transPlane2_Alpha_DC_B = 23,
	basic_transPlane2_Alpha_DC_Gb = 24,
	basic_transPlane0_Beta_Gr = 25,
	basic_transPlane0_Beta_R = 26,
	basic_transPlane0_Beta_B = 27,
	basic_transPlane0_Beta_Gb = 28,
	basic_transPlane1_Beta_Gr = 29,
	basic_transPlane1_Beta_R = 30,
	basic_transPlane1_Beta_B = 31,
	basic_transPlane1_Beta_Gb = 32,
	basic_transPlane2_Beta_Gr = 33,
	basic_transPlane2_Beta_R = 34,
	basic_transPlane2_Beta_B = 35,
	basic_transPlane2_Beta_Gb = 36,
	R_NormFactor = 37,
	radial_gain_scale_factor = 38,
	X_Center_ANR = 39,
	Y_Center_ANR = 40,
	stitch_pyramid_reg = 41,
	basic_transPlane0_plainColorWMatrix_Gr = 42,
	basic_transPlane0_plainColorWMatrix_R = 43,
	basic_transPlane0_plainColorWMatrix_B = 44,
	basic_transPlane0_plainColorWMatrix_Gb = 45,
	basic_transPlane1_plainColorWMatrix_Gr = 46,
	basic_transPlane1_plainColorWMatrix_R = 47,
	basic_transPlane1_plainColorWMatrix_B = 48,
	basic_transPlane1_plainColorWMatrix_Gb = 49,
	basic_transPlane2_plainColorWMatrix_Gr = 50,
	basic_transPlane2_plainColorWMatrix_R = 51,
	basic_transPlane2_plainColorWMatrix_B = 52,
	basic_transPlane2_plainColorWMatrix_Gb = 53
};

enum ISP_Demosaicing
{
	ChAR_enable = 0,
	FCC_En = 1,
	gamma_sc = 2,
	LC_Ctrl = 3,
	CR_Param1 = 4,
	CR_Param2 = 5,
	Coring_Param = 6
};

enum ISP_Gamma
{
	GammaLUT = 0
};

enum ISP_ColorSpaceConversion_ChromaDownsampling
{
	CSC_matrix = 0,
	bias_coeffs = 1,
	downscaling_filter_coefficients = 2,
	filter_norm_factor = 3
};

enum ISP_Y_EdgeEnhance_NoiseReduction
{
	a_diag = 0,
	a_perif = 1,
	T1 = 2,
	T2 = 3,
	YeeEdgeSense0 = 4,
	DeltaEdgeSense = 5,
	YeeCornerSense0 = 6,
	DeltaCornerSense = 7,
	FcGainPosi0 = 8,
	DeltaGainPosi = 9,
	FcGainNeg0 = 10,
	DeltaGainNeg = 11,
	FcCropPosi0 = 12,
	DeltaCropPosi = 13,
	FcCropNeg0 = 14,
	DeltaCropNeg = 15,
	FcGainExp = 16,
	FcMixRange = 17,
	MinEdge = 18,
	LinSegParam = 19,
	DiagDiscG = 20,
	HVW_hor = 21,
	DW_hor = 22,
	HVW_diag = 23,
	DW_diag = 24,
	FcCoringPosi0 = 25,
	FcCoringPosiDelta = 26,
	FcCoringNega0 = 27,
	FcCoringNegaDelta = 28
};

enum ISP_ChromaNoiseReduction
{
	CnrCoringU = 0,
	CnrCoringV = 1,
	CnrSenseGainVY = 2,
	CnrSenseGainVU = 3,
	CnrSenseGainVV = 4,
	CnrSenseGainHY = 5,
	CnrSenseGainHU = 6,
	CnrSenseGainHV = 7,
	Fir0H = 8,
	Fir1H = 9,
	MinPrev = 10
};

enum ISP_UV_Color_Enhancement
{
	gain_according_to_y_only = 0,
	macc_tables = 1,
	m_gain_pcwl_lut = 2,
	m_Strength = 3
};

enum ISP_XNR
{
	XnrSigma_Y0 = 0,
	XnrSigma_Y1 = 1,
	XnrSigma_U0 = 2,
	XnrSigma_U1 = 3,
	XnrSigma_V0 = 4,
	XnrSigma_V1 = 5,
	CnrCoring_U0 = 6,
	CnrCoring_U1 = 7,
	CnrCoring_V0 = 8,
	CnrCoring_V1 = 9,
	blending_strength = 10
};

enum ISP_IEFD
{
	CU1_x = 0,
	CU1_a = 1,
	CU1_b = 2,
	CU_ED_x = 3,
	CU_ED_a = 4,
	CU_ED_b = 5,
	CU3_x = 6,
	CU3_a = 7,
	CU3_b = 8,
	CU5_x = 9,
	CU5_a = 10,
	CU5_b = 11,
	CU6_x = 12,
	CU6_a = 13,
	CU6_b = 14,
	CU7_x = 15,
	CU7_a = 16,
	CU7_b = 17,
	CUUnSharp_x = 18,
	CUUnSharp_a = 19,
	CUUnSharp_b = 20,
	CURad_x = 21,
	CURad_a = 22,
	CURad_b = 23,
	CUVss_x = 24,
	CUVss_a = 25,
	CUVss_b = 26,
	m_horver_diag_coef = 27,
	m_shrpn_nega_lmt_txt = 28,
	m_shrpn_posi_lmt_txt = 29,
	m_shrpn_nega_lmt_dir = 30,
	m_shrpn_posi_lmt_dir = 31,
	m_clamp_stitch = 32,
	m_dir_far_sharp_w = 33,
	m_dir_far_dns_w = 34,
	m_ndir_dns_powr = 35,
	m_denoise_en = 36,
	m_unsharp_weight = 37,
	m_unsharp_amount = 38,
	m_unsharp_c00 = 39,
	m_unsharp_c01 = 40,
	m_unsharp_c02 = 41,
	m_unsharp_c11 = 42,
	m_unsharp_c12 = 43,
	m_unsharp_c22 = 44,
	m_direct_metric_update = 45,
	m_directional_smooth_en = 46,
	m_vssnlm_enable = 47,
	m_vs_x0 = 48,
	m_vs_x1 = 49,
	m_vs_x2 = 50,
	m_vs_y1 = 51,
	m_vs_y2 = 52,
	m_vs_y3 = 53,
	m_rad_enable = 54,
	m_rad_Xreset = 55,
	m_rad_Yreset = 56,
	m_rad_X2reset = 57,
	m_rad_Y2reset = 58,
	x_Center = 59,
	y_Center = 60,
	m_rad_NF = 61,
	m_rad_inv_r2 = 62,
	m_CU6_pow = 63,
	m_CUUnsharp_pow = 64,
	m_rad_CU6_pow = 65,
	m_rad_CUUnsharp_pow = 66,
	m_radCU6_X1 = 67,
	m_radCUUnSharp_X1 = 68,
	m_ed_horver_diag_coeff = 69,
	m_rad_dir_far_sharp_w = 70,
	m_rad_dir_far_dns_w = 71,
	m_rad_ndir_dns_powr = 72,
	m_iefd_en = 73
};

enum ISP_TNR
{
	TnrMaxFB_Y = 0,
	TnrMaxFB_U = 1,
	TnrMaxFB_V = 2,
	TnrRoundADJ_Y = 3,
	TnrRoundADJ_U = 4,
	TnrRoundADJ_V = 5,
	TnrY0_Sigma_Y = 6,
	TnrY0_Sigma_U = 7,
	TnrY0_Sigma_V = 8,
	TnrY1_Knee_P = 9,
	TnrY1_Sigma_Y = 10,
	TnrY1_Sigma_U = 11,
	TnrY1_Sigma_V = 12,
	TnrY2_Knee_P = 13,
	TnrY2_Sigma_Y = 14,
	TnrY2_Sigma_U = 15,
	TnrY2_Sigma_V = 16,
	TnrY3_Sigma_Y = 17,
	TnrY3_Sigma_U = 18,
	TnrY3_Sigma_V = 19
};
enum ISP_vHDR
{
	GaeBrightThrLow = 0,
	GaeBrightThrHigh = 1,
	GaePyrLow = 2,
	GaePyrHigh = 3,
	GaeZerosThrWeight = 4,
	GaeFlatThrRatio = 5,
	GaeMotionMinAreaLow = 6,
	GaeMotionMinAreaHigh = 7,
	MrgBlendThrLow = 8,
	MrgBlendThrHigh = 9
};
enum Type
{
	uint32_ = 0,
	matrix_int32_ = 1,
	array_uint32_ = 2,
	uint8_ = 3,
	array_int16_ = 4,
	array_uint16_ = 5,
	int16_ = 6,
	uint16_ = 7,
	array_uint8_ = 8,
	matrix_int16_ = 9,
	int8_ = 10,
	array_int8_ = 11
};

enum Precision
{
	NOT_SET = 0,
	uint1_Precision = 1,
	uint2_Precision = 2,
	uint3_Precision = 3,
	uint4_Precision = 4,
	uint5_Precision = 5,
	uint6_Precision = 6,
	uint7_Precision = 7,
	uint8_Precision = 8,
	uint9_Precision = 9,
	uint10_Precision = 10,
	uint11_Precision = 11,
	uint12_Precision = 12,
	uint13_Precision = 13,
	uint14_Precision = 14,
	uint15_Precision = 15,
	int8_Precision = 16,
	int5_Precision = 17,
	int12_Precision = 18,
	int13_Precision = 19,
	uint5q2_Precision = 20,
	uint6q2_Precision = 21,
	uint4q4_Precision = 22,
	uint4q2_Precision = 23,
	int15q14_Precision = 24,
	int16q6_Precision = 25,
	uint16q16_Precision = 26,
	uint32_Precision = 27,
	uint16q13_Precision = 28,
	uint16q8_Precision = 29,
	uint6q6_Precision = 30,
	uint7q3_Precision = 31,
	uint6q3_Precision = 32,
	uint5q4_Precision = 33,
	uint16q4_Precision = 34,
	uint12q10_Precision = 35,
	uint8q6_Precision = 36,
	uint9q4_Precision = 37,
	uint11q4_Precision = 38,
	int9q4_Precision = 39,
	uint7q6_Precision = 40,
	uint9q5_Precision = 41,
	int9q8_Precision = 42,
	uint7q4_Precision = 43,
	uint6q4_Precision = 44,
	int10_Precision = 45,
	uint24_Precision = 46,
	int16q8_Precision = 47,
	uint10q10_Precision = 48,
	uint15q15_Precision = 49,
	int14q13_Precision = 50,
	uint12q11_Precision = 51,
	int12q11_Precision = 52,
	uint8q8_Precision = 53
};

char* GET_PTR(char *startAddress,long long int offset);

union INT32PTR
{
public:
	long long int offset;
	int32_t      *address;

	const int32_t  * operator->() const
	{
		return address;
	}

	const int32_t &  operator[](int i) const
	{
		return address[i];
	}

	const int32_t *  GetPtr() const
	{
		return address;
	}

};

union UINT8PTR
{
public:
	long long int offset;
	uint8_t      *address;

	const uint8_t  * operator->() const
	{
		return address;
	}

	const uint8_t &  operator[](int i) const
	{
		return address[i];
	}

	const uint8_t *  GetPtr() const
	{
		return address;
	}

};

union INT8PTR
{
public:
	long long int offset;
	int8_t      *address;
	const int8_t  * operator->() const
	{
		return address;
	}

	const int8_t &  operator[](int i) const
	{
		return address[i];
	}

	const int8_t *  GetPtr() const
	{
		return address;
	}

};
class globalHeader
{
public:
	uint32_t 	 tag;
	uint32_t 	 data_size;
	uint32_t 	 system_version;
	uint32_t 	 enum_revision;
	uint16_t 	 sensor_model_id;
	uint8_t 	 module_model_id;
	uint8_t 	 revision_number;
	uint8_t 	 manufacturer_id;
	uint32_t 	 config_bits;
	uint32_t 	 checksum;
};

class ISPGage
{
public:
	int32_t				ID;
	GageType			gage_type;
	NodeType			node_type;
	INT8PTR				min;
	INT8PTR				max;
	uint8_t	 			num_of_nodes;
	int32_t				type_size;
	UINT8PTR 			nodes_values;
	INTERPOLATION_TYPE	interpolation_type;
	EXTRAPOLATION_TYPE	extrapolation_type;
	ISPGage();
};
union UISPGage
{
public:
   long long int offset;
   ISPGage *address;

   const ISPGage  * operator->() const
	{
		return address;
	}

	const ISPGage &  operator[](int i) const
	{
		return address[i];
	}

};

class ISPDomain
{
public:
	int32_t	 ID;
	uint8_t	 num_of_gages;
	INT8PTR	 gage_ids;
	ISPDomain();
};

union UISPDomain
{
public:
	long long int offset;
   ISPDomain *address;

   const ISPDomain  * operator->() const
	{
		return address;
	}

	const ISPDomain &  operator[](int i) const
	{
		return address[i];
	}

};
class RecordParam
{
public:
    int		    paramID;
    Type		type;
    Precision	precision;
    int32_t		domain_name;
    INT8PTR		min;
    INT8PTR		max;
    int32_t		valueSize;
	int32_t		numberOfDims;
	INT32PTR	numOfValues;
	int32_t		nOfNodes;
    INT8PTR		value;
	RecordParam();
};

union URecordParam
{
public:
	long long int  offset;
	RecordParam   *address;

   const RecordParam  * operator->() const
	{
		return address;
	}

	const RecordParam &  operator[](int i) const
	{
		return address[i];
	}

};
class ISPRecord
{
public:
	Blocks			     NameId; // from recordHeader
	int32_t				 nOfRecordParmas;
	URecordParam		 paramList;
	ISPRecord();
	const RecordParam * Param(int paramID) const
	{
		for (int32_t i = 0; i < nOfRecordParmas; ++i)
		{
			if (paramList[i].paramID == paramID)
			{
				return &paramList[i];
			}
		}
		return 0;
	}
};
union UISPRecord
{
public:
	long long int offset;
   ISPRecord *address;

   const ISPRecord  * operator->() const
	{
		return address;
	}

	const ISPRecord &  operator[](int i) const
	{
		return address[i];
	}

};
class ParamsInDomain
{
public:
	int			 paramID;
	Blocks           recordID;
	URecordParam	 paramPtr;
	ParamsInDomain();
};
union UParamsInDomain
{
public:
	long long int offset;
    ParamsInDomain *address;

    const ParamsInDomain  * operator->() const
	{
		return address;
	}

	const ParamsInDomain &  operator[](int i) const
	{
		return address[i];
	}

};
class DomainsTable
{
public:
	int32_t		        domainID;
	int32_t				nOfParams = 0;
	UParamsInDomain     params;
    DomainsTable();
};
union UDomainsTable
{
public:
	long long int offset;
   DomainsTable *address;

   const DomainsTable  * operator->() const
	{
		return address;
	}

	const DomainsTable &  operator[](int i) const
	{
		return address[i];
	}

};
class ISP
{
public:
	globalHeader    gh;
	int32_t			nOfGages;
	UISPGage		gagesList;
	int32_t			nOfDomains;
	UISPDomain	    domainList;
	int32_t			nOfRecords;
	UISPRecord	    recordList;
	int32_t			numOfNonConstDomians;
	UDomainsTable   domainsTables;
	ISP();
	const ISPGage*			Gage(int32_t gageId)	     const
	{
		for (int i = 0; i < nOfGages; ++i)
		{
			if (gagesList[i].ID == gageId)
			{
				return &gagesList[i];
			}
		}
		return 0;
	}

	const ISPDomain*			Domain(int32_t domainId)	     const
	{
		for (int i = 0; i < nOfDomains; ++i)
		{
			if (domainList[i].ID == domainId)
			{
				return &domainList[i];
			}
		}
		return 0;
	}

	const ISPRecord*			Record(Blocks recordId)	     const
	{
		for (int i = 0; i < nOfRecords; ++i)
		{
			if (recordList[i].NameId == recordId)
			{
				return &recordList[i];
			}
		}

		return 0;
	}

	const DomainsTable * DomainInDomainTables(int32_t DomainID) const
	{
		for (int i = 0; i < numOfNonConstDomians; ++i)
		{
			if (domainsTables[i].domainID == DomainID)
			{
				return &domainsTables[i];
			}
		}

		return 0;
	}

	const ParamsInDomain * ParamsInDomainTables(int32_t DomainID) const
	{
		for (int i = 0; i < numOfNonConstDomians; ++i)
		{
			if (domainsTables[i].domainID == DomainID)
			{
				return domainsTables[i].params.address;
			}
		}

		return 0;
	}



	const RecordParam * ParamsInDomainTables(int32_t DomainID,Blocks RecordID,int ParamID) const
	{
		for (int i = 0; i < numOfNonConstDomians; ++i)
		{
			if (domainsTables[i].domainID == DomainID)
			{
				for (int j = 0; j < domainsTables[i].nOfParams; ++j)
				{
					if (domainsTables[i].params[j].paramID == ParamID && domainsTables[i].params[j].recordID == RecordID)
					{
						return domainsTables.address[i].params[j].paramPtr.address;
					}
				}
			}
		}

		return 0;
	}


};

union UISP
{
public:
	long long int offset;
   ISP *address;

   const ISP  * operator->() const
	{
		return address;
	}

	const ISP &  operator[](int i) const
	{
		return address[i];
	}

};
class CPFF
{
public:
	UISP  IspPreviewVideo;
	UISP  IspStills;
    CPFF();
};
union UCPFF
{
public:
	long long int  offset;
    CPFF		  *address;

   const CPFF  * operator->() const
	{
		return address;
	}

	const CPFF &  operator[](int i) const
	{
		return address[i];
	}

};

LIBEXPORT CPFF * ReadCpff(char * buffer);

#pragma pack(pop)


#endif //_CPFF_SPECIFIC_DATA_H_

