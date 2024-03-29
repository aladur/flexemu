/*
    bintervl.h

    Basic class representing a closed interval with lower and upper bound.
    Implementation is a subset of boost::interval and fully compatible.
    Copyright (C) 2020-2024  W. Schwotzer

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef BINTERVAL_INCLUDED
#define BINTERVAL_INCLUDED

#include <algorithm>
#include <vector>
#include <ostream>


template <typename T>
class BInterval
{
public:

    BInterval() : lower_(0), upper_(0) { };
    BInterval(T lower, T upper) : lower_(lower), upper_(upper)
    {
        if (lower_ > upper_)
        {
            throw std::invalid_argument(
                    "lower has to be lower or equal to upper");
        }
    };
    BInterval(const BInterval &src) = default;
    ~BInterval() = default;
	
    T const &lower() const { return lower_; };
    T const &upper() const { return upper_; };
    void assign(T lower, T upper)
    {
        lower_ = lower;
        upper_ = upper;
    }
    BInterval& operator= (const BInterval& src)
    {
        lower_ = src.lower_;
        upper_ = src.upper_;
        return *this;
    }

private:
	T lower_;
	T upper_;
};

template <typename T>
T width(const BInterval<T>& src)
{
    return src.upper() - src.lower();
}

template <typename T>
bool equal(const BInterval<T>& lhs, const BInterval<T>& rhs)
{
    return lhs.lower() == rhs.lower() && lhs.upper() == rhs.upper();
}

template <typename T>
bool singleton(const BInterval<T>& src)
{
    return src.lower() == src.upper();
}

template <typename T>
bool in(T value, const BInterval<T>& src)
{
    return value >= src.lower() && value <= src.upper();
}

template <typename T>
bool subset(const BInterval<T>& lhs, const BInterval<T>& rhs)
{
    return lhs.lower() >= rhs.lower() && lhs.lower() <= rhs.upper() &&
           lhs.upper() >= rhs.lower() && lhs.upper() <= rhs.upper();
}

template <typename T>
bool proper_subset(const BInterval<T>& lhs, const BInterval<T>& rhs)
{
    return !equal(lhs, rhs) &&
           lhs.lower() >= rhs.lower() && lhs.lower() <= rhs.upper() &&
           lhs.upper() >= rhs.lower() && lhs.upper() <= rhs.upper();
}

template <typename T>
bool overlap(const BInterval<T>& lhs, const BInterval<T>& rhs)
{
    if (width(lhs) >= width(rhs))
    {
        return (rhs.lower() >= lhs.lower() && rhs.lower() <= lhs.upper()) ||
               (rhs.upper() >= lhs.lower() && rhs.upper() <= lhs.upper());
    }

    return (lhs.lower() >= rhs.lower() && lhs.lower() <= rhs.upper()) ||
            (lhs.upper() >= rhs.lower() && lhs.upper() <= rhs.upper());
}

template <typename T>
BInterval<T> hull(const BInterval<T>& lhs, const BInterval<T>& rhs)
{
    return BInterval<T>(std::min<T>(lhs.lower(), rhs.lower()),
                        std::max<T>(lhs.upper(), rhs.upper()));
}

template <typename T>
bool cerle(const T& x, const BInterval<T>& src)
{
    return x <= src.lower();
}

template <typename T>
bool cerlt(const T& x, const BInterval<T>& src)
{
    return x < src.lower();
}

template <typename T>
bool cerge(const BInterval<T>& src, const T& y)
{
    return src.lower() >= y;
}

template <typename T>
bool cergt(const BInterval<T>& src, const T& y)
{
    return src.lower() > y;
}

// The following free functions are not part of the boost::interval
// implementation.

template <typename T>
std::ostream &operator<<(std::ostream &os, const BInterval<T> &iv)
{
    os << "[" << iv.lower() << "," << iv.upper() << "]";
    return os;
}

// Compare intervals lower limit values.
template <typename T>
bool cmp_lower(const BInterval<T> &a, const BInterval<T> &b)
{
    return cerlt(a.lower(), b);
}

// Sort all intervals by their lower value.
template <typename T>
void sort_lower(std::vector<BInterval<T> > &bintervals)
{
    std::sort(std::begin(bintervals), std::end(bintervals), cmp_lower<T>);
}

// Join all intervals which either overlap or adjoin.
template <typename T>
void join(std::vector<BInterval<T> > &bintervals)
{
    sort_lower(bintervals);

    for (auto it = bintervals.begin(); it != bintervals.end(); ++it)
    {
        bool done = false;
        while (!done)
        {
            done = true;
            const auto itNext = it + 1;
            if (itNext != bintervals.end() &&
                (overlap(*it, *itNext) || (it->upper() + 1 == itNext->lower())))
            {
                *it = hull(*it, *itNext);
                bintervals.erase(itNext);
                done = false;
            }
        }
    }
}
#endif // #ifndef BINTERVAL_INCLUDED

