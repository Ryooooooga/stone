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

#include "stone/Interpreter.hpp"
#include "stone/Parser.hpp"

#include <iostream>

#include <boost/type_index.hpp>
#include <fmt/ostream.h>

int main()
{
	try
	{
		auto lexer = std::make_unique<stone::Lexer>(u8R"(
			even = 0
			odd = 0
			i = 1
			while i < 10 {
				if i % 2 == 0 { // even number?
					even = even + 1
				} else {
					odd = odd + 1
				}
				i = i + 1
			}
			even + odd
		)");

		auto parser = std::make_unique<stone::Parser>(std::move(lexer));

		const auto ast = parser->parse();

		stone::Printer {std::cout}.print(*ast);

		fmt::print(u8"result: {}", std::any_cast<int>(stone::Interpreter {}.evaluate(*ast)));
	}
	catch (const std::exception& e)
	{
		fmt::print(
			stderr,
			u8"*** exception caught ***\n"
			u8"type: {}\n"
			u8"what: {}\n",
			boost::typeindex::type_id_runtime(e).pretty_name(),
			e.what());
	}
}
