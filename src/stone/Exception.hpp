/*================================================================================
 * The MIT License
 *
 * Copyright (c) 2018 Ryooooooga
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
================================================================================*/

#pragma once

#include <stdexcept>

#include <fmt/ostream.h>

namespace stone
{
	class StoneException
		: public std::runtime_error
	{
	public:
		explicit StoneException(std::size_t line, std::string_view description)
			: runtime_error(fmt::format(u8"error at line {}: {}", line, description))
		{
		}

		// Uncopyable, movable.
		StoneException(const StoneException&) =delete;
		StoneException(StoneException&&) =default;

		StoneException& operator=(const StoneException&) =delete;
		StoneException& operator=(StoneException&&) =default;

		~StoneException() =default;
	};

	class ParseException
		: public StoneException
	{
	public:
		using StoneException::StoneException;
	};

	class EvaluateException
		: public StoneException
	{
	public:
		using StoneException::StoneException;
	};
}
