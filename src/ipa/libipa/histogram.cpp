/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (C) 2019, Raspberry Pi (Trading) Limited
 *
 * histogram.cpp - histogram calculations
 */
#include <math.h>

#include "histogram.h"

/**
 * \file histogram.h
 * \brief Class to represent Histograms and manipulate them
 */

namespace libcamera {

namespace ipa {

/**
 * \class Histogram
 * \brief The base class for creating histograms
 *
 * The Histogram class defines a standard interface for IPA algorithms. By
 * abstracting histograms, it makes possible the implementation of generic code
 * to manage algorithms regardless of their specific type.
 */

/**
 * \brief Cumulative frequency up to a (fractional) point in a bin.
 * \param[in] bin the number of bins
 * \return The number of bins cumulated
 */
uint64_t Histogram::cumulativeFreq(double bin) const
{
	if (bin <= 0)
		return 0;
	else if (bin >= bins())
		return total();
	int b = (int)bin;
	return cumulative_[b] +
	       (bin - b) * (cumulative_[b + 1] - cumulative_[b]);
}

/**
 * \brief Return the (fractional) bin of the point through the histogram
 * \param[in] q the desired point (0 <= q <= 1)
 * \param[in] first low limit (optionnal if -1)
 * \param[in] last high limit (optionnal if -1)
 * \return The fractionnal bin of the point
 */
double Histogram::quantile(double q, int first, int last) const
{
	if (first == -1)
		first = 0;
	if (last == -1)
		last = cumulative_.size() - 2;
	assert(first <= last);
	uint64_t items = q * total();
	/* Binary search to find the right bin */
	while (first < last) {
		int middle = (first + last) / 2;
		/* Is it between first and middle ? */
		if (cumulative_[middle + 1] > items)
			last = middle;
		else
			first = middle + 1;
	}
	assert(items >= cumulative_[first] && items <= cumulative_[last + 1]);
	double frac = cumulative_[first + 1] == cumulative_[first] ? 0
								   : (double)(items - cumulative_[first]) /
									     (cumulative_[first + 1] - cumulative_[first]);
	return first + frac;
}

/**
 * \brief Calculate the mean between two quantiles
 * \param[in] lowQuantile low Quantile
 * \param[in] highQuantile high Quantile
 * \return The average histogram bin value between the two quantiles
 */
double Histogram::interQuantileMean(double lowQuantile, double highQuantile) const
{
	assert(highQuantile > lowQuantile);
	double lowPoint = quantile(lowQuantile);
	double highPoint = quantile(highQuantile, (int)lowPoint);
	double sumBinFreq = 0, cumulFreq = 0;
	for (double p_next = floor(lowPoint) + 1.0; p_next <= ceil(highPoint);
	     lowPoint = p_next, p_next += 1.0) {
		int bin = floor(lowPoint);
		double freq = (cumulative_[bin + 1] - cumulative_[bin]) *
			      (std::min(p_next, highPoint) - lowPoint);
		sumBinFreq += bin * freq;
		cumulFreq += freq;
	}
	/* add 0.5 to give an average for bin mid-points */
	return sumBinFreq / cumulFreq + 0.5;
}

} /* namespace ipa */

} /* namespace libcamera */
