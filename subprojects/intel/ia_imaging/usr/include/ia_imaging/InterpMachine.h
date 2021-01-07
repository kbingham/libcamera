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

#include "cpffData.h"
#include <math.h>



//#ifdef SKYCAMAIC_DYNAMIC_EXPORTS
//#define SKYCAMAIC_DYNAMIC_API __declspec(dllexport)
//#else
//#define SKYCAMAIC_DYNAMIC_API __declspec(dllimport)
//#endif

#define MAX_3D_INTERPOLATION 3

class VoidBuffer
{
public:
	VoidBuffer(int buffer_size, Type buffer_type, Precision buffer_prec);
	VoidBuffer(int buffer_size, Type buffer_type, Precision buffer_prec, void *buffer_data);
	~VoidBuffer();
	float GetBufferValue(int ind)const;
	void SetBufferValue(float float_val, int ind);
	int Size()const;
	Type Btype()const;
	Precision Prec()const;
	VoidBuffer & operator=(const VoidBuffer &x_in);

protected:

	int buffer_size_;
	Type type_;
	Precision prec_;
	void *val_;
	bool allocated_here_;

	int GetPrecShift(Precision prec)const;
};


class  InterpMachine
{
public:
	enum ia_isp_interpolatio_method {
		AIC_INTERP_METHOD_NN,
		AIC_INTERP_METHOD_LINEAR,
		AIC_INTERP_METHOD_PARABOLA,
		AIC_INTERP_METHOD_HERMIT,
		AIC_INTERP_METHOD_LANCZOS2
	};
	InterpMachine(int GridD, const ISPGage *Gage,int *GInds);
	InterpMachine(void);
	~InterpMachine(void);
	virtual void const Interpolate(const Type ValType, const void *Values, const int ValNdims,
		const int *ValNVals ,float *currentGage, Type resType, void *result)const;
	virtual void Set(int GridD, const ISPGage *Gage,int *GInds);
	bool const IsGridValid() const;
	void const InterpolateCU(const int xValNdims, const int *xValNVals ,
										float *currentGage, Type in_x_type, Type in_a_type,
										Type in_b_type, Precision in_x_prec, Precision in_a_prec,
										Precision in_b_prec, void *x1_val, void *a1_val, void *b1_val,
										Type res_x_type, Type res_a_type, Type res_b_type,
										Precision res_x_prec, Precision res_a_prec,
										Precision res_b_prec, void *x_result,
										void *a_result, void *b_result) const;

protected:
	int grid_dim_;
	const ISPGage *gage_list_; // NofGages in list == GridDim, example: AnalogGain and CCT gaged
	int gage_index_[MAX_3D_INTERPOLATION]; //max 3D interpolation
	int domian_name_;

	void *GetBufferPntr(int buffer_size, Type buffer_type, void *buffer_data, int g_ind)const;
	void const Interpolate1D(INTERPOLATION_TYPE InterpType, double x1, double val1, double x2,
		double val2, double target_x, double  *target_val)const;
	float const GetGageNodeVal(const uint8_t* source, int index, NodeType node_type)const;
	void const findLUboundIndex(int &ind1, int &ind2, int GageInd, float currentGage)const;
	void const Calc_Linear(double x1, double val1, double x2, double val2, double target_x, double * target_val)const;
	void const Calc_NearestNeighbor(double x1, double val1, double x2, double val2, double target_x, double * target_val)const;
	void const Calc_Parabola(float x1, float val1, float x2, float val2, float x3, float val3, float target_x, float * target_val)const;
	void const Calc_Parabola4(float x1, float val1, float x2, float val2,
		float x3, float val3,float x4, float val4, float target_x, float * target_val)const;
	void const Calc_Hermit(float val1, float val2, float val3, float val4, float x2, float x3, float target_x, float * target_val)const;
	float const Sinc(float t) const;
	void const Calc_Lanczos2(float val1, float val2, float val3, float val4, float x2, float x3, float target_x, float * target_val)const;
	inline int Calc_Cubic(float val1, float val2, float val3, float val4,float x2,
		float x3, float target_x, float * target_val, ia_isp_interpolatio_method InterpolationMothod);
	const void InterpolateCU1DNN(VoidBuffer &x1_in, VoidBuffer &a1_in,VoidBuffer &b1_in,
												VoidBuffer &x2_in, VoidBuffer &a2_in,VoidBuffer &b2_in,
												float g1,float g2, float target_g,
												VoidBuffer &x_result, VoidBuffer &a_result, VoidBuffer &b_result)const;
	void const InterpolateCU1DLinear(VoidBuffer &x1_in, VoidBuffer &a1_in,VoidBuffer &b1_in,
												VoidBuffer &x2_in, VoidBuffer &a2_in,VoidBuffer &b2_in,
												float g1,float g2, float target_g,
												VoidBuffer &x_result, VoidBuffer &a_result, VoidBuffer &b_result)const ;
	void CovertBuffer(VoidBuffer &x_in, VoidBuffer &x_res)const;
	void CalcWeightedAvrageBuffer(VoidBuffer &x1, VoidBuffer &x2, float w, VoidBuffer &x_res)const;
	float CalcWeightedAvrage(Type in_type, void* in_1buff, void *in_2buff, float w, int buf_ind)const;
	const void InterpolateCU1D(INTERPOLATION_TYPE InterpType, VoidBuffer &x1_in, VoidBuffer &a1_in,VoidBuffer &b1_in,
												VoidBuffer &x2_in, VoidBuffer &a2_in,VoidBuffer &b2_in,
												float g1,float g2, float target_g,
												VoidBuffer &x_result, VoidBuffer &a_result, VoidBuffer &b_result)const;
	void ConvertXABToXYvectors(VoidBuffer &x, VoidBuffer &a, VoidBuffer &b,
		VoidBuffer &y)const;
	void ConvertXYToXABvectors(VoidBuffer &x, VoidBuffer &y,
		VoidBuffer &a, VoidBuffer &b)const;

};

