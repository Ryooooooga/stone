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

#include <any>
#include <unordered_map>

#include "Exception.hpp"
#include "Node.hpp"

namespace stone
{
	class Environment
	{
	public:
		explicit Environment() =default;

		// Uncopyable, unmovable.
		Environment(const Environment&) =delete;
		Environment(Environment&&) =delete;

		Environment& operator=(const Environment&) =delete;
		Environment& operator=(Environment&&) =delete;

		~Environment() =default;

		void put(std::string_view name, std::any value)
		{
			m_table[name] = value;
		}

		[[nodiscard]]
		std::any get(std::string_view name)
		{
			return m_table[name];
		}

	private:
		std::unordered_map<std::string_view, std::any> m_table;
	};

	class Interpreter
	{
	public:
		explicit Interpreter() =default;

		// Uncopyable, unmovable.
		Interpreter(const Interpreter&) =delete;
		Interpreter(Interpreter&&) =delete;

		Interpreter& operator=(const Interpreter&) =delete;
		Interpreter& operator=(Interpreter&&) =delete;

		~Interpreter() =default;

		[[nodiscard]]
		std::any evaluate(const ProgramNode& node)
		{
			Environment env;

			return evaluate(node, env);
		}

	private:
		[[nodiscard]]
		static int asInteger(const std::any& value)
		{
			assert(value.type() == typeid(int));

			return std::any_cast<int>(value);
		}

		[[nodiscard]]
		static std::string toString(const std::any& value)
		{
			if (value.type() == typeid(int))
			{
				return std::to_string(asInteger(value));
			}

			return std::any_cast<std::string>(value);
		}

		[[nodiscard]]
		std::any dispatch(const Node& node, Environment& env)
		{
#define STONE_NODE(_name, _format)                                \
			if (const auto p = dynamic_cast<const _name*>(&node)) \
				return evaluate(*p, env);
#include "Node.def.hpp"

			throw EvaluateException { node.lineNumber(), u8"unknown node type." };
		}

		[[nodiscard]]
		std::any evaluate(const ProgramNode& node, Environment& env)
		{
			std::any last;

			for (const auto& child : node.children())
			{
				last = dispatch(*child, env);
			}

			return last;
		}

		[[nodiscard]]
		std::any evaluate(const ParameterNode& node, Environment& env)
		{
			(void)env;
			throw EvaluateException { node.lineNumber(), u8"not implemented" };
		}

		[[nodiscard]]
		std::any evaluate(const ParameterListNode& node, Environment& env)
		{
			(void)env;
			throw EvaluateException { node.lineNumber(), u8"not implemented" };
		}

		[[nodiscard]]
		std::any evaluate(const ArgumentListNode& node, Environment& env)
		{
			(void)env;
			throw EvaluateException { node.lineNumber(), u8"not implemented" };
		}

		[[nodiscard]]
		std::any evaluate(const IfStatementNode& node, Environment& env)
		{
			if (asInteger(dispatch(node.condition(), env)) != 0)
			{
				return dispatch(node.then(), env);
			}
			else if (const auto otherwise = node.otherwise())
			{
				return dispatch(*otherwise, env);
			}

			return {};
		}

		[[nodiscard]]
		std::any evaluate(const WhileStatementNode& node, Environment& env)
		{
			std::any last;

			while (asInteger(dispatch(node.condition(), env)) != 0)
			{
				last = dispatch(node.body(), env);
			}

			return last;
		}

		[[nodiscard]]
		std::any evaluate(const CompoundStatementNode& node, Environment& env)
		{
			std::any last;

			for (const auto& child : node.children())
			{
				last = dispatch(*child, env);
			}

			return last;
		}

		[[nodiscard]]
		std::any evaluate(const ProcedureStatementNode& node, Environment& env)
		{
			(void)env;
			throw EvaluateException {node.lineNumber(), u8"not implemented"};
		}

		[[nodiscard]]
		std::any evaluate(const BinaryExpressionNode& node, Environment& env)
		{
			switch (node.operation())
			{
				case BinaryOperator::addition:
				{
					const auto left = dispatch(node.left(), env);
					const auto right = dispatch(node.right(), env);

					if (left.type() == typeid(int) || right.type() == typeid(int))
						return asInteger(left) + asInteger(right);
					else
						return toString(left) + toString(right);
				}

				case BinaryOperator::subtraction:
					return asInteger(dispatch(node.left(), env)) - asInteger(dispatch(node.right(), env));
				case BinaryOperator::multiplication:
					return asInteger(dispatch(node.left(), env)) * asInteger(dispatch(node.right(), env));
				case BinaryOperator::division:
					return asInteger(dispatch(node.left(), env)) / asInteger(dispatch(node.right(), env));
				case BinaryOperator::modulo:
					return asInteger(dispatch(node.left(), env)) % asInteger(dispatch(node.right(), env));

				case BinaryOperator::equal:
				{
					const auto left = dispatch(node.left(), env);
					const auto right = dispatch(node.right(), env);

					if (left.type() == typeid(int) || right.type() == typeid(int))
						return asInteger(left) == asInteger(right) ? 1 : 0;
					else if (left.type() == typeid(std::string) || right.type() == typeid(std::string))
						return toString(left) == toString(right) ? 1 : 0;
					else
						return 0;
				}

				case BinaryOperator::notEqual:
				{
					const auto left = dispatch(node.left(), env);
					const auto right = dispatch(node.right(), env);

					if (left.type() == typeid(int) || right.type() == typeid(int))
						return asInteger(left) != asInteger(right) ? 1 : 0;
					else if (left.type() == typeid(std::string) || right.type() == typeid(std::string))
						return toString(left) != toString(right) ? 1 : 0;
					else
						return 1;
				}

				case BinaryOperator::lesserThan:
					return asInteger(dispatch(node.left(), env)) < asInteger(dispatch(node.right(), env)) ? 1 : 0;
				case BinaryOperator::lesserEqual:
					return asInteger(dispatch(node.left(), env)) <= asInteger(dispatch(node.right(), env)) ? 1 : 0;
				case BinaryOperator::greaterThan:
					return asInteger(dispatch(node.left(), env)) > asInteger(dispatch(node.right(), env)) ? 1 : 0;
				case BinaryOperator::greaterEqual:
					return asInteger(dispatch(node.left(), env)) >= asInteger(dispatch(node.right(), env)) ? 1 : 0;

				case BinaryOperator::assign:
				{
					if (const auto left = dynamic_cast<const IdentifierExpressionNode*>(&node.left()))
					{
						const auto right = dispatch(node.right(), env);
						env.put(left->name(), right);

						return right;
					}

					throw EvaluateException { node.lineNumber(), u8"invalid assignment." };
				}

				default:
					throw EvaluateException { node.lineNumber(), fmt::format(u8"unknown binary operator {}.", node.operation()) };
			}
		}

		[[nodiscard]]
		std::any evaluate(const UnaryExpressionNode& node, Environment& env)
		{
			switch (node.operation())
			{
				case UnaryOperator::negation:
					return -asInteger(dispatch(node, env));

				default:
					throw EvaluateException { node.lineNumber(), fmt::format(u8"unknown unary operator {}.", node.operation()) };
			}
		}

		[[nodiscard]]
		std::any evaluate(const CallExpressionNode& node, Environment& env)
		{
			(void)env;
			throw EvaluateException { node.lineNumber(), u8"not implemented" };
		}

		[[nodiscard]]
		std::any evaluate(const IdentifierExpressionNode& node, Environment& env)
		{
			return env.get(node.name());
		}

		[[nodiscard]]
		std::any evaluate(const IntegerExpressionNode& node, [[maybe_unused]] Environment& env)
		{
			return node.value();
		}

		[[nodiscard]]
		std::any evaluate(const StringExpressionNode& node, [[maybe_unused]] Environment& env)
		{
			return node.value();
		}
	};
}
