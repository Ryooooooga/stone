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

#include <cassert>
#include <deque>

#include "Lexer.hpp"

namespace stone
{
	class TokenStream
	{
	public:
		explicit TokenStream(std::unique_ptr<Lexer>&& lexer)
			: m_lexer(std::move(lexer))
		{
			assert(m_lexer);
		}

		// Uncopyable, unmovable.
		TokenStream(const TokenStream&) =delete;
		TokenStream(TokenStream&&) =delete;

		TokenStream& operator=(const TokenStream&) =delete;
		TokenStream& operator=(TokenStream&&) =delete;

		~TokenStream() =default;

		std::shared_ptr<Token> read()
		{
			auto token = peek(0);
			m_queue.pop_front();

			return token;
		}

		[[nodiscard]]
		std::shared_ptr<Token> peek(std::size_t position)
		{
			fillQueue(position + 1);

			return m_queue[position];
		}

		void fillQueue(std::size_t amount)
		{
			while (m_queue.size() < amount)
			{
				m_queue.emplace_back(m_lexer->read());
			}
		}

	private:
		std::unique_ptr<Lexer> m_lexer;
		std::deque<std::shared_ptr<Token>> m_queue;
	};
}
