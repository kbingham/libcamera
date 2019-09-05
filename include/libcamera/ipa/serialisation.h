/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2019, Google Inc.
 *
 * serialisation.h - Serialisation support framework
 */
#ifndef __LIBCAMERA_IPA_SERIALISATION_H__
#define __LIBCAMERA_IPA_SERIALISATION_H__

#include <iostream>
#include <vector>

namespace libcamera {

#if 1
/**
 * https://tuttlem.github.io/2014/08/18/getting-istream-to-work-off-a-byte-array.html
 */
class IPCStreamBuf : public std::basic_streambuf<char>
{
public:
	IPCStreamBuf(const uint8_t *p, size_t l)
	{
		setg((char*)p, (char*)p, (char*)p + l);
	}

	/* This is no good because it's constructed too early. */
	IPCStreamBuf(std::vector<uint8_t> &v)
	{
		char *start = (char*)v.data();
		char *end = start + v.capacity();

		std::cout << "Creating StreamBuf of size "
				<< v.capacity() << std::endl;

		setg(start, start, end);
	}
};
#endif

class IPCStream {
public:
	IPCStream(std::vector<uint8_t> &v) :
			rpos_(0), vector_(v)
	{
	}

	IPCStream& read(char* s, std::streamsize n)
	{
		std::cout << "Reading " << n << " bytes from vector" ;

		uint8_t *b = vector_.data();

		/* Should validate that there are enough
		 * bytes to read from rpos > size(); */

		/* Can the byte read/writes be optimised here ? */
		for (std::streamsize i = 0; i < n; ++i) {
			s[i] = b[rpos_++];
		}

		std::cout << std::endl;
		return *this;
	}

	IPCStream& write(const char* s, std::streamsize n)
	{
		/* Can the byte read/writes be optimised here ? */
		for (std::streamsize i = 0; i < n; ++i)
			vector_.push_back(s[i]);

		return *this;
	}

#define STREAM_SERIALISER_POD(type)		\
	const IPCStream& operator<<(type v) {	\
		return *this;			\
	}					\

	//STREAM_SERIALISER_POD(uint64_t)

private:
	uint32_t rpos_;
	std::vector<uint8_t> &vector_;
};

template<class STREAM_TYPE>
class Archive
{
public:
	Archive(STREAM_TYPE& stream) :
			m_stream(stream)
	{
	}

public:
	/* Writing into the serialised archive. */
	template<class T>
	const Archive& operator<<(const T& v) const
	{
		*this & v;
		return *this;
	}

	template<class T>
	Archive& operator>>(T& v)
	{
		*this & v;
		return *this;
	}

public:
	/* Handle custom class serialisation. */
	template<class T>
	Archive& operator&(T& v)
	{
		v.Serialize(*this);
		return *this;
	}

	/* Handle custom class de-serialisation. */
	template<class T>
	const Archive& operator&(const T& v) const
	{
		((T&)v).Serialize(*this);
		return *this;
	}

	/* Array serialisation. */
	template<class T, size_t N>
	Archive& operator&(T (&v)[N])
	{
		uint32_t len;
		*this & len;
		for (size_t i = 0; i < N; ++i)
			*this & v[i];
		return *this;
	}

	/* Array de-serialisation */
	template<class T, size_t N>
	const Archive& operator&(const T (&v)[N]) const
	{
		uint32_t len = N;
		*this & len;
		for (size_t i = 0; i < N; ++i)
			*this & v[i];
		return *this;
	}

#define SERIALIZER_FOR_POD(type) \
	Archive& operator&(type& v) \
	{ \
	    m_stream.read((char*)&v, sizeof(type)); \
	    return *this; \
	} \
	const Archive& operator&(type v) const \
	{ \
	    m_stream.write((const char*)&v, sizeof(type)); \
	    return *this; \
	}

	SERIALIZER_FOR_POD(bool)
	SERIALIZER_FOR_POD(char)
	SERIALIZER_FOR_POD(unsigned char)
	SERIALIZER_FOR_POD(short)
	SERIALIZER_FOR_POD(unsigned short)
	SERIALIZER_FOR_POD(int)
	SERIALIZER_FOR_POD(unsigned int)
	SERIALIZER_FOR_POD(long)
	SERIALIZER_FOR_POD(unsigned long)
	SERIALIZER_FOR_POD(long long)
	SERIALIZER_FOR_POD(unsigned long long)
	SERIALIZER_FOR_POD(float)
	SERIALIZER_FOR_POD(double)

#define SERIALIZER_FOR_STL(type) \
	template <class T> \
	Archive& operator&(type<T>& v) \
	{ \
	    uint32_t len; \
	    *this & len; \
	    for(uint32_t i = 0; i < len; ++i) \
	    { \
		T value; \
		*this & value; \
		v.insert(v.end(), value); \
	    } \
	    return *this; \
	} \
	template <class T> \
	const Archive& operator&(const type<T>& v) const \
	{ \
	    uint32_t len = v.size(); \
	    *this & len; \
	    for(typename type<T>::const_iterator it = v.begin(); it != v.end(); ++it) \
	    *this & *it; \
	    return *this; \
	}

#define SERIALIZER_FOR_STL2(type) \
	template <class T1, class T2> \
	Archive& operator&(type<T1, T2>& v) \
	{ \
	    uint32_t len; \
	    *this & len; \
	    for(uint32_t i = 0; i < len; ++i) \
	    { \
		std::pair<T1, T2> value; \
		*this & value; \
		v.insert(v.end(), value); \
	    } \
	    return *this; \
	} \
	template <class T1, class T2> \
	const Archive& operator&(const type<T1, T2>& v) const \
	{ \
	    uint32_t len = v.size(); \
	    *this & len; \
	    for(typename type<T1, T2>::const_iterator it = v.begin(); it != v.end(); ++it) \
	    *this & *it; \
	    return *this; \
	}

	SERIALIZER_FOR_STL(std::vector)
//	SERIALIZER_FOR_STL(std::deque)
//	SERIALIZER_FOR_STL(std::list)
//	SERIALIZER_FOR_STL(std::set)
//	SERIALIZER_FOR_STL(std::multiset)
//	SERIALIZER_FOR_STL2(std::map)
//	SERIALIZER_FOR_STL2(std::multimap)

	template<class T1, class T2>
	Archive& operator&(std::pair<T1, T2>& v)
	{
		*this & v.first & v.second;
		return *this;
	}

	template<class T1, class T2>
	const Archive& operator&(const std::pair<T1, T2>& v) const
	{
		*this & v.first & v.second;
		return *this;
	}

	Archive& operator&(std::string& v)
	{
		uint32_t len;
		*this & len;
		v.clear();
		char buffer[4096];
		uint32_t toRead = len;
		while (toRead != 0)
		{
			uint32_t l = std::min(toRead,
					(uint32_t)sizeof(buffer));
			m_stream.read(buffer, l);
			if (!m_stream)
				throw std::runtime_error(
						"malformed data");
			v += std::string(buffer, l);
			toRead -= l;
		}
		return *this;
	}

	const Archive& operator&(const std::string& v) const
	{
		uint32_t len = v.length();
		*this & len;
		m_stream.write(v.c_str(), len);
		return *this;
	}

private:
	STREAM_TYPE& m_stream;
};

/**
 * Provide streaming operations on top of a vector of bytes.
 */
class BytestreamBuffer {

public:
	BytestreamBuffer() : rpos_(0)
		{}

	BytestreamBuffer& read(char* s, std::streamsize n)
	{
		uint8_t *b = data_.data();

		/* Should validate that there are enough
		 * bytes to read from rpos > size(); */

		/* Can the byte read/writes be optimised here ? */
		for (std::streamsize i = 0; i < n; ++i)
			s[i] = b[rpos_++];

		return *this;
	}

	BytestreamBuffer& write(const char* s, std::streamsize n)
	{
		/* Can the byte read/writes be optimised here ? */
		for (std::streamsize i = 0; i < n; ++i)
			data_.push_back(s[i]);

		return *this;
	}

	void resize(unsigned int s) { data_.resize(s); }
	unsigned int size() const { return data_.size(); }
	uint8_t* data() { return data_.data(); }
	const uint8_t* data() const { return data_.data(); }

private:
	uint32_t rpos_;
	std::vector<uint8_t> data_;
};

/**
 * Operates like a stream, by providing serialisation operations into and out
 * of a vector of bytes.
 *
 * The vector can be pre-populated by calling .resize() and filling from
 * .data() onwards.
 */
class Bytestream {
public:
	Bytestream() : archive_(buffer_)
		{}

	/* Writing into the serialised stream. */
	template<class T>
	const Bytestream& operator<<(const T& v) const
	{
		archive_ & v;
		return *this;
	}

	/* Reading from the serialised stream. */
	template<class T>
	Bytestream& operator>>(T& v)
	{
		archive_ & v;
		return *this;
	}

	int size() const { return buffer_.size(); }
	void resize(unsigned int s) { buffer_.resize(s); }

	uint8_t* data() { return buffer_.data(); }
	const uint8_t* data() const { return buffer_.data(); }
	uint8_t* end() { return buffer_.data() + buffer_.size(); }
	const uint8_t* end() const { return buffer_.data() + buffer_.size(); }

private:
	BytestreamBuffer buffer_;
	Archive<BytestreamBuffer> archive_;
};


} /* namespace libcamera */

#endif /* __LIBCAMERA_IPA_SERIALISATION_H__ */
