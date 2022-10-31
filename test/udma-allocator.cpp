/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2022, Ideas on Board Oy.
 *
 * udma-allocator.cpp - UdmaBuf Allocator Test
 */

#include <fcntl.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "libcamera/internal/udma_allocator.h"

#include "test.h"

using namespace libcamera;
using namespace std;

class UdmaBufTest : public Test
{
protected:
	int init() override
	{
		allocator_ = new UdmaBuf();

		return !allocator_->isValid();
	}

	int run() override
	{
		UniqueFD buf = allocator_->allocate("Test", 4096 * 10);

		std::cout << "Allocation " << (buf.isValid() ? "valid" : "failed") << std::endl;

		if (!buf.isValid())
			return TestFail;

		return TestPass;
	}

	void cleanup() override
	{
		delete allocator_;
	}

private:
	UdmaBuf *allocator_;
};

TEST_REGISTER(UdmaBufTest)
