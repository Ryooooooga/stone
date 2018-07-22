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

#ifndef STONE_TOKEN
#	define STONE_TOKEN(_name, _text)
#endif

#ifndef STONE_TOKEN_KEYWORD
#	define STONE_TOKEN_KEYWORD(_name, _text) STONE_TOKEN(_name, _text)
#endif

#ifndef STONE_TOKEN_KEYWORD1
#	define STONE_TOKEN_KEYWORD1(_keyword) STONE_TOKEN_KEYWORD(keyword_ ## _keyword, #_keyword)
#endif

#ifndef STONE_TOKEN_PUNCTUATOR
#	define STONE_TOKEN_PUNCTUATOR(_name, _text) STONE_TOKEN(_name, _text)
#endif

STONE_TOKEN(endOfFile, "end of file")
STONE_TOKEN(endOfLine, "end of line")
STONE_TOKEN(identifier, "identifier")
STONE_TOKEN(integer, "integer literal")
STONE_TOKEN(string, "string literal")

STONE_TOKEN_KEYWORD1(if)
STONE_TOKEN_KEYWORD1(else)
STONE_TOKEN_KEYWORD1(while)
STONE_TOKEN_KEYWORD1(def)
STONE_TOKEN_KEYWORD1(fun)
STONE_TOKEN_KEYWORD1(return)

STONE_TOKEN_PUNCTUATOR(plus, "+")
STONE_TOKEN_PUNCTUATOR(minus, "-")
STONE_TOKEN_PUNCTUATOR(star, "*")
STONE_TOKEN_PUNCTUATOR(slash, "/")
STONE_TOKEN_PUNCTUATOR(percent, "%")
STONE_TOKEN_PUNCTUATOR(assign, "=")
STONE_TOKEN_PUNCTUATOR(equal, "==")
STONE_TOKEN_PUNCTUATOR(notEqual, "!=")
STONE_TOKEN_PUNCTUATOR(lesserThan, "<")
STONE_TOKEN_PUNCTUATOR(lesserEqual, "<=")
STONE_TOKEN_PUNCTUATOR(greaterThan, ">")
STONE_TOKEN_PUNCTUATOR(greaterEqual, ">=")
STONE_TOKEN_PUNCTUATOR(period, ".")
STONE_TOKEN_PUNCTUATOR(comma, ",")
STONE_TOKEN_PUNCTUATOR(semicolon, ";")
STONE_TOKEN_PUNCTUATOR(leftParen, "(")
STONE_TOKEN_PUNCTUATOR(rightParen, ")")
STONE_TOKEN_PUNCTUATOR(leftBrace, "{")
STONE_TOKEN_PUNCTUATOR(rightBrace, "}")

#undef STONE_TOKEN
#undef STONE_TOKEN_KEYWORD
#undef STONE_TOKEN_KEYWORD1
#undef STONE_TOKEN_PUNCTUATOR
