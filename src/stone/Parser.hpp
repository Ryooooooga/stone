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
		//     top-level-statement (separator top-level-statement)*
		// separator:
		//     ';' | end-of-line
		[[nodiscard]]
		std::unique_ptr<ProgramNode> parseProgram()
		{
			auto node = std::make_unique<ProgramNode>();

			// top-level-statement
			if (auto statement = parseTopLevelStatement())
			{
				node->addChild(std::move(statement));
			}

			// (separator top-level-statement)*
			while (peekToken()->kind() != TokenKind::endOfFile)
			{
				// separator
				if (!consumeTokenIf(TokenKind::endOfLine))
				{
					matchToken(TokenKind::semicolon);
				}

				// top-level-statement
				if (auto statement = parseTopLevelStatement())
				{
					node->addChild(std::move(statement));
				}
			}

			return node;
		}

		// statement:
		//     procedure-statement
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
				case TokenKind::keyword_def:
					// procedure-statement
					return parseProcedureStatement();

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
					return nullptr;

				default:
					// expression
					return parseExpression();
			}
		}

		// top-level-statement:
		//     class-statement
		//     statement
		[[nodiscard]]
		std::unique_ptr<StatementNode> parseTopLevelStatement()
		{
			switch (peekToken()->kind())
			{
				case TokenKind::keyword_class:
					// class-statement
					return parseClassStatement();

				default:
					// statement
					return parseStatement();
			}
		}

		// procedure-statement:
		//     'def' identifier parameter-list compound-statement
		[[nodiscard]]
		std::unique_ptr<StatementNode> parseProcedureStatement()
		{
			// 'def'
			const auto token = matchToken(TokenKind::keyword_def);

			// identifier
			const auto name = matchToken(TokenKind::identifier);

			// parameter-list
			auto parameters = parseParameterList();

			// compound-statement
			auto body = parseCompoundStatement();

			return std::make_unique<ProcedureStatementNode>(token->lineNumber(), name->text(), std::move(parameters), std::move(body));
		}

		// class-statement:
		//     'class' identifier ('extends' identifier)? compound-statement
		[[nodiscard]]
		std::unique_ptr<StatementNode> parseClassStatement()
		{
			// 'class'
			const auto token = matchToken(TokenKind::keyword_class);

			// identifier
			const auto name = matchToken(TokenKind::identifier);

			// ('extends' identifier)?
			const auto super = consumeTokenIf(TokenKind::keyword_extends)
				? matchToken(TokenKind::identifier) : nullptr;

			// compound-statement
			auto body = parseCompoundStatement();

			return std::make_unique<ClassStatementNode>(token->lineNumber(), name->text(), super ? std::make_optional(super->text()) : std::nullopt, std::move(body));
		}

		// parameter-list:
		//     '(' parameter (',' parameter)* ')'
		//     '(' ')'
		[[nodiscard]]
		std::unique_ptr<ParameterListNode> parseParameterList()
		{
			// '('
			const auto token = matchToken(TokenKind::leftParen);

			auto node = std::make_unique<ParameterListNode>(token->lineNumber());

			// (parameter (',' parameter)*)?
			if (peekToken()->kind() != TokenKind::rightParen)
			{
				// parameter
				node->addChild(parseParameter());

				// (',' parameter)*
				while (consumeTokenIf(TokenKind::comma))
				{
					node->addChild(parseParameter());
				}
			}

			// ')'
			matchToken(TokenKind::rightParen);

			return node;
		}

		// parameter:
		//     identifier
		[[nodiscard]]
		std::unique_ptr<ParameterNode> parseParameter()
		{
			// identifier
			const auto token = matchToken(TokenKind::identifier);

			return std::make_unique<ParameterNode>(token->lineNumber(), token->text());
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
			if (auto statement = parseStatement())
			{
				node->addChild(std::move(statement));
			}

			// (separator statement)* '}'
			while (!consumeTokenIf(TokenKind::rightBrace))
			{
				// separator
				if (!consumeTokenIf(TokenKind::endOfLine))
				{
					matchToken(TokenKind::semicolon);
				}

				// statement
				if (auto statement = parseStatement())
				{
					node->addChild(std::move(statement));
				}
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
		//     postfix-expression
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parseUnaryExpression()
		{
			switch (const auto token = peekToken(); token->kind())
			{
				case TokenKind::minus:
					// negative-expression
					return parseNegativeExpression();

				default:
					// postfix-expression
					return parsePostfixExpression();
			}
		}

		// negative-expression:
		//     '-' postfix-expression
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parseNegativeExpression()
		{
			// '-'
			const auto token = matchToken(TokenKind::minus);

			// postfix-expression
			auto operand = parsePostfixExpression();

			return std::make_unique<UnaryExpressionNode>(token->lineNumber(), UnaryOperator::negation, std::move(operand));
		}

		// postfix-expression:
		//     primary-expression postfix
		// postfix:
		//     call-expression-postfix
		//     member-access-expression-postfix
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parsePostfixExpression()
		{
			// primary-expression
			auto operand = parsePrimaryExpression();

			// (call-expression-postfix)*
			while (true)
			{
				switch (peekToken()->kind())
				{
					case TokenKind::leftParen:
						// call-expression-postfix
						operand = parseCallExpressionPostfix(std::move(operand));
						break;

					case TokenKind::period:
						operand = parseMemberAccessPostfix(std::move(operand));
						break;

					default:
						return operand;
				}
			}
		}

		// call-expression-postfix:
		//     '(' expression (',' expression)* ')'
		//     '(' ')'
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parseCallExpressionPostfix(std::unique_ptr<ExpressionNode>&& operand)
		{
			// '('
			const auto token = matchToken(TokenKind::leftParen);

			auto arguments = std::make_unique<ArgumentListNode>(token->lineNumber());

			// (expression (',' expression)*)?
			if (peekToken()->kind() != TokenKind::rightParen)
			{
				// expression
				arguments->addChild(parseExpression());

				// (',' expression)*
				while (consumeTokenIf(TokenKind::comma))
				{
					arguments->addChild(parseExpression());
				}
			}

			// ')'
			matchToken(TokenKind::rightParen);

			return std::make_unique<CallExpressionNode>(operand->lineNumber(), std::move(operand), std::move(arguments));
		}

		// member-access-expression-postfix:
		//     '.' identifier
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parseMemberAccessPostfix(std::unique_ptr<ExpressionNode>&& operand)
		{
			// '.'
			const auto token = matchToken(TokenKind::period);

			// identifier
			const auto member = matchToken(TokenKind::identifier);

			return std::make_unique<MemberAccessExpressionNode>(token->lineNumber(), std::move(operand), member->text());
		}

		// primary-expression:
		//     paren-expression
		//     closure-expression
		//     identifier-expression
		//     integer-expression
		//     string-expression
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parsePrimaryExpression()
		{
			switch (const auto token = peekToken(); token->kind())
			{
				case TokenKind::leftParen:
					// paren-expression
					return parseParenExpression();

				case TokenKind::keyword_fun:
					// closure-expression
					return parseClosureExpression();

				case TokenKind::identifier:
					// identifier-expression
					return parseIdentifierExpression();

				case TokenKind::integer:
					// integer-expression
					return parseIntegerExpression();

				case TokenKind::string:
					// string-expression
					return parseStringExpression();

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

		// closure-expression:
		//     'fun' parameter-list compound-statement
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parseClosureExpression()
		{
			// 'fun'
			const auto token = matchToken(TokenKind::keyword_fun);

			// parameter-list
			auto parameters = parseParameterList();

			// compound-statement
			auto body = parseCompoundStatement();

			return std::make_unique<ClosureExpressionNode>(token->lineNumber(), std::move(parameters), std::move(body));
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

		// string-expression:
		//     string
		[[nodiscard]]
		std::unique_ptr<ExpressionNode> parseStringExpression()
		{
			// string
			const auto token = matchToken(TokenKind::string);

			return std::make_unique<StringExpressionNode>(token->lineNumber(), token->stringValue());
		}

		std::unique_ptr<TokenStream> m_stream;
	};
}
