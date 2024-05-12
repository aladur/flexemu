/*
    boption.h


    Basic template class containing an optional value.
    Copyright (C) 2024  W. Schwotzer

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

#ifndef BOPTIONAL_INCLUDED
#define BOPTIONAL_INCLUDED

#include "misc1.h"
#include "flexerr.h"
#include <string>


template<typename T>
class BOptional
{
private:
    T opt_value;
    bool is_valid;

public:
    constexpr BOptional() : opt_value{}, is_valid{false} { };
    constexpr BOptional(const BOptional &src) = default;
    constexpr explicit BOptional(const T &val)
        : opt_value{val}
        , is_valid{true} { };
    ~BOptional() = default;

    constexpr bool has_value() const noexcept { return is_valid; };

    constexpr T& value()
    {
        if (is_valid)
        {
            return opt_value;
        }

        throw FlexException(FERR_BAD_OPTIONAL_ACCESS);
    };
    constexpr const T& value() const
    {
        if (is_valid)
        {
            return opt_value;
        }

        throw FlexException(FERR_BAD_OPTIONAL_ACCESS);
    };
    constexpr T value_or(const T &default_value)
    {
        if (is_valid)
        {
            return opt_value;
        }

        return default_value;

    };
    T value_or(const T &default_value) const
    {
        if (is_valid)
        {
            return opt_value;
        }

        return default_value;
    };
    constexpr const T& operator*() const
    {
        if (is_valid)
        {
            return opt_value;
        }

        throw FlexException(FERR_BAD_OPTIONAL_ACCESS);
    };
    constexpr T& operator*()
    {
        if (is_valid)
        {
            return opt_value;
        }

        throw FlexException(FERR_BAD_OPTIONAL_ACCESS);
    };
    constexpr const T* operator->() const
    {
        if (is_valid)
        {
            return &opt_value;
        }

        throw FlexException(FERR_BAD_OPTIONAL_ACCESS);
    };
    constexpr T* operator->()
    {
        if (is_valid)
        {
            return &opt_value;
        }

        throw FlexException(FERR_BAD_OPTIONAL_ACCESS);
    };
    constexpr void reset() noexcept { is_valid = false; };

    BOptional& operator=(const T &val)
    {
        opt_value = val;
        is_valid = true;

        return *this;
    };
    BOptional& operator=(const BOptional &src)
    {
        if (this != &src)
        {
            opt_value = src.opt_value;
            is_valid = src.is_valid;
        }

        return *this;
    };
};

using BOptionalWord = BOptional<Word>;
using BOptionalString = BOptional<std::string>;

#endif

