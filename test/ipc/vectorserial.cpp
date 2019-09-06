/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2019, Google Inc.
 *
 * vectorserial.cpp - Libcamera serialisation to vector tests
 */

#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <vector>

#include "test.h"

#include <libcamera/ipa/serialisation.h>

using namespace std;
using namespace libcamera;

#include <sstream>

// C++ template to print vector container elements
template <typename T>
ostream& operator<<(ostream& os, const vector<T>& v)
{
    os << "[";
    for (unsigned int i = 0; i < v.size(); ++i) {
        os << v[i];
        if (i != v.size() - 1)
            os << ", ";
    }
    os << "]\n";
    return os;
}

template<class Elem, class Traits>
inline void hex_dump(const void* aData, std::size_t aLength, std::basic_ostream<Elem, Traits>& aStream, std::size_t aWidth = 16)
{
	const char* const start = static_cast<const char*>(aData);
	const char* const end = start + aLength;
	const char* line = start;
	while (line != end)
	{
		aStream.width(4);
		aStream.fill('0');
		aStream << std::hex << line - start << " : ";
		std::size_t lineLength = std::min(aWidth, static_cast<std::size_t>(end - line));
		for (std::size_t pass = 1; pass <= 2; ++pass)
		{
			for (const char* next = line; next != end && next != line + aWidth; ++next)
			{
				char ch = *next;
				switch(pass)
				{
				case 1:
					aStream << (ch < 32 ? '.' : ch);
					break;
				case 2:
					if (next != line)
						aStream << " ";
					aStream.width(2);
					aStream.fill('0');
					aStream << std::hex << std::uppercase << static_cast<int>(static_cast<unsigned char>(ch));
					break;
				}
			}
			if (pass == 1 && lineLength != aWidth)
				aStream << std::string(aWidth - lineLength, ' ');
			aStream << " ";
		}
		aStream << std::endl;
		line = line + lineLength;
	}
}



class VectorSerial : public Test
{
protected:

	int init()
	{
		/* Predefine our vector storage space. */
		storage_.reserve(1024);

		return TestPass;
	}

	int runArchive()
	{
		IPCStream ipcstream(storage_);
		stringstream sstream;

		Archive<stringstream> ssArchive(sstream);
		Archive<IPCStream> ipcArchive(ipcstream);

		unsigned int a = 1;
		unsigned int b = 2;
		unsigned int aa = 11;
		unsigned int bb = 22;

		ssArchive << a << b;
		cout << a << " " << b << " " << aa << " " << bb << endl;

		ssArchive >> aa >> bb;
		cout << a << " " << b << " " << aa << " " << bb << endl;

		aa = 0xFF; bb = 0xFF;
		cout << a << " " << b << " " << aa << " " << bb << endl;

		ipcArchive << a << b;
		ipcArchive >> aa >> bb;

		/* Failed if you print 255 (0xFF) */
		cout << a << " " << b << " " << aa << " " << bb << endl;

		//s << "Hello\0";

		//cout << "Data location " << storage_[0] << std::endl;
		return TestPass;
	}

	int runBytestream()
	{
		unsigned int a = 1;
		unsigned int b = 2;
		unsigned int aa = 11;
		unsigned int bb = 22;

		Bytestream stream;

		stream << a << b;
		cout << a << " " << b << " " << aa << " " << bb << endl;

		stream >> aa >> bb;
		cout << a << " " << b << " " << aa << " " << bb << endl;

		cout << "stream size: " << stream.size() << endl;
		hex_dump(stream.data(), stream.size(), std::cout);

		return TestPass;
	}

	int run()
	{
		runBytestream();

		return TestPass;
	}

private:
	std::vector<uint8_t> storage_;

	Bytestream stream_;
};

TEST_REGISTER(VectorSerial)
