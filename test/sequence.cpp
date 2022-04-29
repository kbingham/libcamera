/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2022, Ideas on Board Oy.
 *
 * sequence.cpp - Sequence observer tests
 */

#include <iostream>

#include <libcamera/sequence.h>

#include "test.h"

using namespace std;
using namespace libcamera;

class SequenceTest : public Test
{
private:
	int Fail(std::string m)
	{
		cout << m << endl;
		return TestFail;
	}

protected:
	int run()
	{
		Sequence seq;
		int diff;

		/* Validate non-zero initialization */
		diff = seq.update(10);
		if (diff)
			return Fail("Initialisation test failed");

		diff = seq.update(11);
		if (diff)
			return Fail("Sequential sequence failure");

		/* Validate 1 drop */
		diff = seq.update(13);
		if (diff != 1)
			return Fail("Sequence gap not detected");

		/* Validate 10 drops - currently expect sequence 14 */
		diff = seq.update(24);
		if (diff != 10)
			return Fail("Large sequence gap not detected");

		/* Validate reset */
		seq.reset();
		diff = seq.update(50);
		if (diff)
			return Fail("Reset failed");

		/* Validate reverse sequence detected */
		diff = seq.update(49);
		if (diff == 0)
			return Fail("Reverse sequence detection error");

		/* Validate integer wrap around (Shouldn't ever happen but...) */
		seq.reset();
		diff = seq.update(-2);
		if (diff)
			return Fail("Integer wrap test reset failed");

		diff = seq.update(-1);
		if (diff)
			return Fail("Negative sequence failed");

		diff = seq.update(0);
		if (diff)
			return Fail("Integer wrap test failed");

		return TestPass;
	}
};

TEST_REGISTER(SequenceTest)
