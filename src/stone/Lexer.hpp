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

#include <boost/container/flat_map.hpp>

#include "Exception.hpp"
#include "Token.hpp"

namespace stone
{
	class Lexer
	{
	public:
		explicit Lexer(std::string_view source)
			: m_source(source)
			, m_pos(0)
			, m_line(1)
		{
		}

		// Uncopyable, unmovable.
		Lexer(const Lexer&) =delete;
		Lexer(Lexer&&) =delete;

		Lexer& operator=(const Lexer&) =delete;
		Lexer& operator=(Lexer&&) =delete;

		~Lexer() =default;

		[[nodiscard]]
		std::unique_ptr<Token> read()
		{
			// \s
			constexpr auto isSpace = [](char c) noexcept
			{
				return c == u8' ' || c == u8'\t' || c == u8'\r';
			};

			// [0-9]
			constexpr auto isDigit = [](char c) noexcept
			{
				return u8'0' <= c && c <= u8'9';
			};

			// [A-Z_a-z]
			constexpr auto isLetter = [](char c) noexcept
			{
				return (u8'A' <= c && c <= u8'Z') || (u8'a' <= c && c <= u8'z') || (c == u8'_');
			};

			// [0-9A-Z_a-z]
			constexpr auto isLetterOrDigit = [](char c) noexcept
			{
				return isLetter(c) || isDigit(c);
			};

			// \s*
			while (isSpace(m_source[m_pos]))
			{
				m_pos++;
			}

			// //.*
			if (m_source.compare(m_pos, 2, u8"//") == 0)
			{
				m_pos += 2;

				while (m_source[m_pos] != u8'\n')
				{
					m_pos++;
				}
			}

			const auto start = m_pos;

			// [EOF]
			if (m_source[m_pos] == u8'\0')
			{
				return std::make_unique<Token>(TokenKind::endOfFile, u8"[EOF]", m_line);
			}

			// [EOL]
			if (m_source[m_pos] == u8'\n')
			{
				m_pos++;
				m_line++;

				return std::make_unique<Token>(TokenKind::endOfLine, u8"[EOL]", m_line - 1);
			}

			// [A-Z_a-z][0-9A-Z_a-z]*
			if (isLetter(m_source[m_pos]))
			{
				while (isLetterOrDigit(m_source[m_pos]))
				{
					m_pos++;
				}

				const auto text = m_source.substr(start, m_pos - start);
				const auto kind =
#define STONE_TOKEN_KEYWORD(_name, _text) \
					(text == u8 ## _text) ? TokenKind::_name :
#include "Token.def.hpp"
					TokenKind::identifier;

				return std::make_unique<Token>(kind, text, m_line);
			}

			// [0-9]+
			if (isDigit(m_source[m_pos]))
			{
				while (isDigit(m_source[m_pos]))
				{
					m_pos++;
				}

				const auto text = m_source.substr(start, m_pos - start);
				const auto value = std::stoi(text);

				return std::make_unique<Token>(TokenKind::integer, text, m_line, value);
			}

			// Punctuator.
			static const auto punctuators = boost::container::flat_map<std::string_view, TokenKind, std::greater<>>
			{
#define STONE_TOKEN_PUNCTUATOR(_name, _text) \
				{ u8 ## _text, TokenKind::_name },
#include "Token.def.hpp"
			};

			for (const auto& [text, kind] : punctuators)
			{
				if (m_source.compare(m_pos, text.size(), text) == 0)
				{
					m_pos += text.size();

					return std::make_unique<Token>(kind, text, m_line);
				}
			}

			// Error.
			throw ParseException { m_line, fmt::format(u8"unexpected character '\\x{:02X}'", m_source[m_pos++]) };
		}

	private:
		std::string m_source;
		std::size_t m_pos;
		std::size_t m_line;
	};
}
