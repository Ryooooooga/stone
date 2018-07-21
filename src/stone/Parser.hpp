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

#include "Node.hpp"
#include "TokenStream.hpp"

namespace stone
{
	class Parser
	{
	public:
		explicit Parser(std::unique_ptr<Lexer>&& lexer)
			: m_stream(std::make_unique<TokenStream>(std::move(lexer)))
		{
		}

		// Uncopyable, unmovable.
		Parser(const Parser&) =delete;
		Parser(Parser&&) =delete;

		Parser& operator=(const Parser&) =delete;
		Parser& operator=(Parser&&) =delete;

		~Parser() =default;

		[[nodiscard]]
		std::unique_ptr<ProgramNode> parse()
		{
			return parseProgram();
		}

	private:
		std::shared_ptr<Token> consumeToken()
		{
			return m_stream->read();
		}

		std::shared_ptr<Token> consumeTokenIf(TokenKind acceptable)
		{
			if (peekToken()->kind() == acceptable)
			{
				return consumeToken();
			}

			return nullptr;
		}

		std::shared_ptr<Token> matchToken(TokenKind expected)
		{
			if (const auto token = peekToken(); token->kind() != expected)
			{
				throw ParseException { token->lineNumber(), fmt::format(u8"unexpected token `{}', expected {}.", token->text(), expected) };
			}

			return consumeToken();
		}

		[[nodiscard]]
		std::shared_ptr<Token> peekToken()
		{
			return m_stream->peek(0);
		}


		// program:
		//     statement (separator statement)*
		// separator:
		//     ';' | end-of-line
		[[nodiscard]]
		std::unique_ptr<ProgramNode> parseProgram()
		{
			auto node = std::make_unique<ProgramNode>();

			// statement
			node->addChild(parseStatement());

			// (separator statement)*
			while (peekToken()->kind() != TokenKind::endOfFile)
			{
				// separator
				if (!consumeTokenIf(TokenKind::endOfLine))
				{
					matchToken(TokenKind::semicolon);
				}

				// statement
				node->addChild(parseStatement());
			}

			return node;
		}

		// statement:
		//     if-statement
		//     while-statement
		//     compound-statement
		//     null-statement
		//     expression
		[[nodiscard]]
		std::unique_ptr<StatementNode> parseStatement()
		{
			switch (const auto token = peekToken(); token->kind())
			{
				case TokenKind::keyword_if:
					// if-statement
					return parseIfStatement();

				case TokenKind::keyword_while:
					// while-statement
					return parseWhileStatement();

				case TokenKind::leftBrace:
					// compound-statement
					return parseCompoundStatement();

				case TokenKind::endOfFile:
				case TokenKind::endOfLine:
				case TokenKind::semicolon:
				case TokenKind::rightBrace:
					// null-statement
					return std::make_unique<NullStatementNode>(token->lineNumber());

				default:
					// expression
					return parseExpression();
			}
		}

		// if-statement:
		//     'if' expression compound-statement
		//     'if' expression compound-statement 'else' if-statement
		//     'if' expression compound-statement 'else' compound-statement
		[[nodiscard]]
		std::unique_ptr<StatementNode> parseIfStatement()
		{
			// 'if'
			const auto token = matchToken(TokenKind::keyword_if);

			// expression
			auto condition = parseExpression();

			// compound-statement
			auto then = parseCompoundStatement();

			// 'else'?
			if (!consumeTokenIf(TokenKind::keyword_else))
			{
				return std::make_unique<IfStatementNode>(token->lineNumber(), std::move(condition), std::move(then), nullptr);
			}

			// if-statement | compound-statement
			auto otherwise = (peekToken()->kind() == TokenKind::keyword_if)
				? parseIfStatement()
				: parseCompoundStatement();

			return std::make_unique<IfStatementNode>(token->lineNumber(), std::move(condition), std::move(then), std::move(otherwise));
		}

		// while-statement:
		//     'while' expression compound-statement
		[[nodiscard]]
		std::unique_ptr<StatementNode> parseWhileStatement()
		{
			// 'while'
			const auto token = matchToken(TokenKind::keyword_while);

			// expression
			auto condition = parseExpression();

			// compound-statement
			auto body = parseCompoundStatement();

			return std::make_unique<WhileStatementNode>(token->lineNumber(), std::move(condition), std::move(body));
		}

		// compound-statement:
		//     '{' statement (separator statement)* '}'
		[[nodiscard]]
		std::unique_ptr<StatementNode> parseCompoundStatement()
		{
			// '{'
			const auto token = matchToken(TokenKind::leftBrace);

			auto node = std::make_unique<CompoundStatementNode>(token->lineNumber());

			// statement
			node->addChild(parseStatement());

			// (separator statement)* '}'
			while (!consumeTokenIf(TokenKind::rightBrace))
			{
				// separator
				if (!consumeTokenIf(TokenKind::endOfLine))
				{
					matchToken(TokenKind::semicolon);
				}

				// statement
				node->addChild(parseStatement());
			}

			return node;
		}

		// expression:
		//     binary-expression
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parseExpression()
		{
			// binary-expression
			return parseBinaryExpression(0);
		}

		// binary-expression:
		//     unary-expression (binary-operator unary-expression)*
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parseBinaryExpression(int minLevel)
		{
			// unary-expression
			auto left = parseUnaryExpression();

			// (binary-operator unary-expression)*
			while (true)
			{
				const auto [operation, prec, assoc] = [](TokenKind kind)
				{
					switch (kind)
					{
						case TokenKind::star: return std::make_tuple(BinaryOperator::multiplication, 5, false);
						case TokenKind::slash: return std::make_tuple(BinaryOperator::division, 5, false);
						case TokenKind::percent: return std::make_tuple(BinaryOperator::modulo, 5, false);
						case TokenKind::plus: return std::make_tuple(BinaryOperator::addition, 4, false);
						case TokenKind::minus: return std::make_tuple(BinaryOperator::subtraction, 4, false);
						case TokenKind::lesserThan: return std::make_tuple(BinaryOperator::lesserThan, 3, false);
						case TokenKind::lesserEqual: return std::make_tuple(BinaryOperator::lesserEqual, 3, false);
						case TokenKind::greaterThan: return std::make_tuple(BinaryOperator::greaterThan, 3, false);
						case TokenKind::greaterEqual: return std::make_tuple(BinaryOperator::greaterEqual, 3, false);
						case TokenKind::equal: return std::make_tuple(BinaryOperator::equal, 2, false);
						case TokenKind::notEqual: return std::make_tuple(BinaryOperator::notEqual, 2, false);
						case TokenKind::assign: return std::make_tuple(BinaryOperator::assign, 1, true);
						default: return std::make_tuple(BinaryOperator {}, -1, false);
					}
				}(peekToken()->kind());

				if (prec < minLevel)
				{
					return left;
				}

				// binary-operator
				const auto token = consumeToken();

				// right-hand-side
				auto right = parseBinaryExpression(prec + (assoc ? 0 : 1));

				left = std::make_unique<BinaryExpressionNode>(token->lineNumber(), operation, std::move(left), std::move(right));
			}
		}

		// unary-expression:
		//     negative-expression
		//     primary-expression
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parseUnaryExpression()
		{
			switch (const auto token = peekToken(); token->kind())
			{
				case TokenKind::minus:
					// negative-expression
					return parseNegativeExpression();

				default:
					// primary-expression
					return parsePrimaryExpression();
			}
		}

		// negative-expression:
		//     '-' primary-expression
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parseNegativeExpression()
		{
			// '-'
			const auto token = matchToken(TokenKind::minus);

			// primary-expression
			auto operand = parsePrimaryExpression();

			return std::make_unique<UnaryExpressionNode>(token->lineNumber(), UnaryOperator::negation, std::move(operand));
		}

		// primary-expression:
		//     paren-expression
		//     identifier-expression
		//     integer-expression
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parsePrimaryExpression()
		{
			switch (const auto token = peekToken(); token->kind())
			{
				case TokenKind::leftParen:
					// paren-expression
					return parseParenExpression();

				case TokenKind::identifier:
					// identifier-expression
					return parseIdentifierExpression();

				case TokenKind::integer:
					// integer-expression
					return parseIntegerExpression();

				default:
					throw ParseException { token->lineNumber(), fmt::format(u8"unexpected token `{}', expected expression.", token->text()) };
			}
		}

		// paren-expression:
		//     '(' expression ')'
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parseParenExpression()
		{
			// '('
			matchToken(TokenKind::leftParen);

			// expression
			auto expression = parseExpression();

			// ')'
			matchToken(TokenKind::rightParen);

			return expression;
		}

		// identifier-expression:
		//     identifier
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parseIdentifierExpression()
		{
			// identifier
			const auto token = matchToken(TokenKind::identifier);

			return std::make_unique<IdentifierExpressionNode>(token->lineNumber(), token->text());
		}

		// integer-expression:
		//     integer
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parseIntegerExpression()
		{
			// integer
			const auto token = matchToken(TokenKind::integer);

			return std::make_unique<IntegerExpressionNode>(token->lineNumber(), token->integerValue());
		}

		std::unique_ptr<TokenStream> m_stream;
	};
}
