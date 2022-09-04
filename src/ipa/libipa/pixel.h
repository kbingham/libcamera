/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2022, Ideas On Board
 *
 * pixel.h - 
 */

#pragma once

#include <array>
#include <assert.h>
#include <cmath>
#include <functional>
#include <ostream>

namespace libcamera {

namespace ipa {

template<typename Scalar, unsigned int Rows>
class Vector
{
public:
	using iterator = typename std::array<Scalar, Rows>::iterator;
	using const_iterator = typename std::array<Scalar, Rows>::const_iterator;

	Vector()
	{
	}

	Vector(Scalar scalar)
	{
		data_.fill(scalar);
	}

	Vector(std::initializer_list<Scalar> init)
	{
		assert(init.size() == Rows);
		std::copy(init.begin(), init.end(), data_.begin());
	}

	Vector(const Vector &other)
		: data_(other.data_)
	{
	}

	Vector &operator=(const Vector &other)
	{
		data_ = other.data_;

		return *this;
	}

	Vector &operator+=(const Vector &other)
	{
		return apply(other, [](Scalar a, Scalar b) { return a + b; });
	}

	Vector &operator+=(Scalar scalar)
	{
		return apply(scalar, [](Scalar a, Scalar b) { return a + b; });
	}

	Vector &operator-=(const Vector &other)
	{
		return apply(other, [](Scalar a, Scalar b) { return a - b; });
	}

	Vector &operator-=(Scalar scalar)
	{
		return apply(scalar, [](Scalar a, Scalar b) { return a - b; });
	}

	Vector &operator*=(const Vector &other)
	{
		return apply(other, [](Scalar a, Scalar b) { return a * b; });
	}

	Vector &operator*=(Scalar scalar)
	{
		return apply(scalar, [](Scalar a, Scalar b) { return a * b; });
	}

	Vector &operator/=(const Vector &other)
	{
		return apply(other, [](Scalar a, Scalar b) { return a / b; });
	}

	Vector &operator/=(Scalar scalar)
	{
		return apply(scalar, [](Scalar a, Scalar b) { return a / b; });
	}

	Vector min(const Vector &other)
	{
		return apply(other, [](Scalar a, Scalar b) { return std::min(a, b); });
	}

	Vector min(Scalar scalar)
	{
		return apply(scalar, [](Scalar a, Scalar b) { return std::min(a, b); });
	}

	Vector max(const Vector &other)
	{
		return apply(other, [](Scalar a, Scalar b) { return std::max(a, b); });
	}

	Vector max(Scalar scalar)
	{
		return apply(scalar, [](Scalar a, Scalar b) -> Scalar { return std::max(a, b); });
	}

	iterator begin() { return data_.begin(); }
	const_iterator begin() const { return data_.cbegin(); }
	const_iterator cbegin() const { return data_.cbegin(); }
	iterator end() { return data_.end(); }
	const_iterator end() const { return data_.cend(); }
	const_iterator cend() const { return data_.cend(); }

	Scalar *data() { return data_.data(); }
	const Scalar *data() const { return data_.data(); }

	template<bool Dependent = false, typename = std::enable_if_t<Dependent || Rows == 3>>
	Scalar &r() { return data()[0]; }
	template<bool Dependent = false, typename = std::enable_if_t<Dependent || Rows == 3>>
	Scalar &g() { return data()[1]; }
	template<bool Dependent = false, typename = std::enable_if_t<Dependent || Rows == 3>>
	Scalar &b() { return data()[2]; }
	template<bool Dependent = false, typename = std::enable_if_t<Dependent || Rows == 3>>
	const Scalar &r() const { return data()[0]; }
	template<bool Dependent = false, typename = std::enable_if_t<Dependent || Rows == 3>>
	const Scalar &g() const { return data()[1]; }
	template<bool Dependent = false, typename = std::enable_if_t<Dependent || Rows == 3>>
	const Scalar &b() const { return data()[2]; }

private:
	template<typename Scalar_, unsigned int Rows_>
	friend Vector<Scalar_, Rows_> operator+(const Vector<Scalar_, Rows_> &lhs, const Vector<Scalar_, Rows_> &rhs);
	template<typename Scalar_, unsigned int Rows_>
	friend Vector<Scalar_, Rows_> operator+(const Vector<Scalar_, Rows_> &lhs, Scalar_ rhs);
	template<typename Scalar_, unsigned int Rows_>
	friend Vector<Scalar_, Rows_> operator+(Scalar_ lhs, const Vector<Scalar_, Rows_> &rhs);
	template<typename Scalar_, unsigned int Rows_>
	friend Vector<Scalar_, Rows_> operator-(const Vector<Scalar_, Rows_> &lhs, const Vector<Scalar_, Rows_> &rhs);
	template<typename Scalar_, unsigned int Rows_>
	friend Vector<Scalar_, Rows_> operator-(const Vector<Scalar_, Rows_> &lhs, Scalar_ rhs);
	template<typename Scalar_, unsigned int Rows_>
	friend Vector<Scalar_, Rows_> operator-(Scalar_ lhs, const Vector<Scalar_, Rows_> &rhs);
	template<typename Scalar_, unsigned int Rows_>
	friend Vector<Scalar_, Rows_> operator*(const Vector<Scalar_, Rows_> &lhs, const Vector<Scalar_, Rows_> &rhs);
	template<typename Scalar_, unsigned int Rows_>
	friend Vector<Scalar_, Rows_> operator*(const Vector<Scalar_, Rows_> &lhs, Scalar_ rhs);
	template<typename Scalar_, unsigned int Rows_>
	friend Vector<Scalar_, Rows_> operator*(Scalar_ lhs, const Vector<Scalar_, Rows_> &rhs);
	template<typename Scalar_, unsigned int Rows_>
	friend Vector<Scalar_, Rows_> operator/(const Vector<Scalar_, Rows_> &lhs, const Vector<Scalar_, Rows_> &rhs);
	template<typename Scalar_, unsigned int Rows_>
	friend Vector<Scalar_, Rows_> operator/(const Vector<Scalar_, Rows_> &lhs, Scalar_ rhs);
	template<typename Scalar_, unsigned int Rows_>
	friend Vector<Scalar_, Rows_> operator/(Scalar_ lhs, const Vector<Scalar_, Rows_> &rhs);

	static Vector apply(const Vector &lhs, const Vector &rhs, std::function<Scalar(Scalar, Scalar)> func)
	{
		Vector result;
		std::transform(lhs.data_.begin(), lhs.data_.end(),
			       rhs.data_.begin(), result.data_.begin(),
			       func);

		return result;
	}

	static Vector apply(const Vector &lhs, Scalar rhs, std::function<Scalar(Scalar, Scalar)> func)
	{
		Vector result;
		std::transform(lhs.data_.begin(), lhs.data_.end(),
			       result.data_.begin(),
			       [&func, rhs](Scalar v) { return func(v, rhs); });

		return result;
	}

	static Vector apply(Scalar lhs, const Vector &rhs, std::function<Scalar(Scalar, Scalar)> func)
	{
		Vector result;
		std::transform(rhs.data_.begin(), rhs.data_.end(),
			       result.data_.begin(),
			       [&func, lhs](Scalar v) { return func(lhs, v); });

		return result;
	}

	Vector &apply(const Vector &other, std::function<Scalar(Scalar, Scalar)> func)
	{
		auto itOther = other.data_.begin();
		std::for_each(data_.begin(), data_.end(),
			      [&func, &itOther](Scalar &v) { v = func(v, *itOther++); });

		return *this;
	}

	Vector &apply(Scalar scalar, std::function<Scalar(Scalar, Scalar)> func)
	{
		std::for_each(data_.begin(), data_.end(),
			      [&func, scalar](Scalar &v) { v = func(v, scalar); });

		return *this;
	}

	std::array<Scalar, Rows> data_;
};

template<typename Scalar>
using RGB = Vector<Scalar, 3>;

template<typename Scalar, unsigned int Rows>
bool operator==(const Vector<Scalar, Rows> &lhs, const Vector<Scalar, Rows> &rhs)
{
	return lhs.data_ == rhs.data_;
}

template<typename Scalar, unsigned int Rows>
bool operator!=(const Vector<Scalar, Rows> &lhs, const Vector<Scalar, Rows> &rhs)
{
	return !(lhs == rhs);
}

template<typename Scalar, unsigned int Rows>
Vector<Scalar, Rows> operator+(const Vector<Scalar, Rows> &lhs, const Vector<Scalar, Rows> &rhs)
{
	return Vector<Scalar, Rows>::apply(lhs, rhs, [](Scalar a, Scalar b) { return a + b; });
}

template<typename Scalar, unsigned int Rows>
Vector<Scalar, Rows> operator+(const Vector<Scalar, Rows> &lhs, Scalar rhs)
{
	return Vector<Scalar, Rows>::apply(lhs, rhs, [](Scalar a, Scalar b) { return a + b; });
}

template<typename Scalar, unsigned int Rows>
Vector<Scalar, Rows> operator+(Scalar lhs, const Vector<Scalar, Rows> &rhs)
{
	return Vector<Scalar, Rows>::apply(lhs, rhs, [](Scalar a, Scalar b) { return a + b; });
}

template<typename Scalar, unsigned int Rows>
Vector<Scalar, Rows> operator-(const Vector<Scalar, Rows> &lhs, const Vector<Scalar, Rows> &rhs)
{
	return Vector<Scalar, Rows>::apply(lhs, rhs, [](Scalar a, Scalar b) { return a - b; });
}

template<typename Scalar, unsigned int Rows>
Vector<Scalar, Rows> operator-(const Vector<Scalar, Rows> &lhs, Scalar rhs)
{
	return Vector<Scalar, Rows>::apply(lhs, rhs, [](Scalar a, Scalar b) { return a - b; });
}

template<typename Scalar, unsigned int Rows>
Vector<Scalar, Rows> operator-(Scalar lhs, const Vector<Scalar, Rows> &rhs)
{
	return Vector<Scalar, Rows>::apply(lhs, rhs, [](Scalar a, Scalar b) { return a - b; });
}

template<typename Scalar, unsigned int Rows>
Vector<Scalar, Rows> operator*(const Vector<Scalar, Rows> &lhs, const Vector<Scalar, Rows> &rhs)
{
	return Vector<Scalar, Rows>::apply(lhs, rhs, [](Scalar a, Scalar b) { return a * b; });
}

template<typename Scalar, unsigned int Rows>
Vector<Scalar, Rows> operator*(const Vector<Scalar, Rows> &lhs, Scalar rhs)
{
	return Vector<Scalar, Rows>::apply(lhs, rhs, [](Scalar a, Scalar b) { return a * b; });
}

template<typename Scalar, unsigned int Rows>
Vector<Scalar, Rows> operator*(Scalar lhs, const Vector<Scalar, Rows> &rhs)
{
	return Vector<Scalar, Rows>::apply(lhs, rhs, [](Scalar a, Scalar b) { return a * b; });
}

template<typename Scalar, unsigned int Rows>
Vector<Scalar, Rows> operator/(const Vector<Scalar, Rows> &lhs, const Vector<Scalar, Rows> &rhs)
{
	return Vector<Scalar, Rows>::apply(lhs, rhs, [](Scalar a, Scalar b) { return a / b; });
}

template<typename Scalar, unsigned int Rows>
Vector<Scalar, Rows> operator/(const Vector<Scalar, Rows> &lhs, Scalar rhs)
{
	return Vector<Scalar, Rows>::apply(lhs, rhs, [](Scalar a, Scalar b) { return a / b; });
}

template<typename Scalar, unsigned int Rows>
Vector<Scalar, Rows> operator/(Scalar lhs, const Vector<Scalar, Rows> &rhs)
{
	return Vector<Scalar, Rows>::apply(lhs, rhs, [](Scalar a, Scalar b) { return a / b; });
}

} /* namespace ipa */

template<typename Scalar, unsigned int Rows>
std::ostream &operator<<(std::ostream &out, const ipa::Vector<Scalar, Rows> &vec)
{
	bool first = true;

	out << "(";

	for (Scalar v : vec) {
		if (!first)
			out << ", ";
		else
			first = false;
		out << v;
	}

	out << ")";

	return out;
}

} /* namespace libcamera */
