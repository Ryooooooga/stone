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

#include <cstdint>
#include <ostream>
#include <string>

namespace stone
{
	enum class TokenKind
	{
#define STONE_TOKEN(_name, _text) _name,
#include "Token.def.hpp"

		unknown,
	};

	[[nodiscard]]
	const char* toString(TokenKind kind) noexcept
	{
		switch (kind)
		{
#define STONE_TOKEN(_name, _text)   \
			case TokenKind::_name:  \
				return u8 ## _text;
#include "Token.def.hpp"

			default:
				return u8"unknown";
		}
	}

	inline std::ostream& operator <<(std::ostream& stream, TokenKind kind)
	{
		return stream << toString(kind);
	}

	class Token
	{
	public:
		explicit Token(TokenKind kind, std::string_view text, std::size_t lineNumber)
			: m_kind(kind)
			, m_text(text)
			, m_lineNumber(lineNumber)
			, m_integer()
			, m_string()
		{
		}

		explicit Token(TokenKind kind, std::string_view text, std::size_t lineNumber, int integer)
			: m_kind(kind)
			, m_text(text)
			, m_lineNumber(lineNumber)
			, m_integer(integer)
			, m_string()
		{
		}

		explicit Token(TokenKind kind, std::string_view text, std::size_t lineNumber, std::string_view string)
			: m_kind(kind)
			, m_text(text)
			, m_lineNumber(lineNumber)
			, m_integer()
			, m_string(string)
		{
		}

		// Uncopyable, movable.
		Token(const Token&) =delete;
		Token(Token&&) =default;

		Token& operator=(const Token&) =delete;
		Token& operator=(Token&&) =default;

		~Token() =default;

		[[nodiscard]]
		TokenKind kind() const noexcept
		{
			return m_kind;
		}

		[[nodiscard]]
		std::string_view text() const noexcept
		{
			return m_text;
		}

		[[nodiscard]]
		std::size_t lineNumber() const noexcept
		{
			return m_lineNumber;
		}

		[[nodiscard]]
		int integerValue() const noexcept
		{
			return m_integer;
		}

		[[nodiscard]]
		std::string_view stringValue() const noexcept
		{
			return m_string;
		}

	private:
		TokenKind m_kind;
		std::string m_text;
		std::size_t m_lineNumber;
		int m_integer;
		std::string m_string;
	};
}
