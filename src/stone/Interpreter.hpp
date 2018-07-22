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
#include <functional>
#include <unordered_map>

#include "Exception.hpp"
#include "Node.hpp"

namespace stone
{
	class Interpreter;

	class Environment
	{
	public:
		explicit Environment(const std::shared_ptr<Environment>& parent = nullptr)
			: m_parent(parent)
			, m_table()
		{
		}

		// Uncopyable, unmovable.
		Environment(const Environment&) =delete;
		Environment(Environment&&) =delete;

		Environment& operator=(const Environment&) =delete;
		Environment& operator=(Environment&&) =delete;

		~Environment() =default;

		void set(std::string_view name, const std::any& value)
		{
			for (auto env = this; env; env = env->m_parent.get())
			{
				if (const auto it = env->m_table.find(name); it != std::cend(env->m_table))
				{
					it->second = value;
					return;
				}
			}

			put(name, value);
		}

		void put(std::string_view name, const std::any& value)
		{
			m_table[name] = value;
		}

		[[nodiscard]]
		std::any get(std::string_view name)
		{
			if (const auto it = m_table.find(name); it != std::cend(m_table))
			{
				return it->second;
			}
			else if (m_parent)
			{
				return m_parent->get(name);
			}

			return {};
		}

	private:
		std::shared_ptr<Environment> m_parent;
		std::unordered_map<std::string_view, std::any> m_table;
	};

	class IFunction
	{
	public:
		IFunction() =default;


		// Uncopyable, unmovable.
		IFunction(const IFunction&) =delete;
		IFunction(IFunction&&) =delete;

		IFunction& operator=(const IFunction&) =delete;
		IFunction& operator=(IFunction&&) =delete;

		virtual ~IFunction() =default;

		virtual std::any invoke(Interpreter& interpreter, const std::vector<std::any>& arguments) =0;
	};

	class StoneFunction
		: public IFunction
	{
	public:
		explicit StoneFunction(std::size_t line, const ParameterListNode& parameters, const StatementNode& body, const std::shared_ptr<Environment>& env)
			: m_line(line)
			, m_parameters(parameters)
			, m_body(body)
			, m_env(env)
		{
			assert(m_env);
		}

		std::any invoke(Interpreter& interpreter, const std::vector<std::any>& arguments) override;

	private:
		std::size_t m_line;
		const ParameterListNode& m_parameters;
		const StatementNode& m_body;
		std::shared_ptr<Environment> m_env;
	};

	template <typename Return, typename... Parameters>
	class NativeFunction
		: public IFunction
	{
	public:
		explicit NativeFunction(const std::function<Return(Parameters...)>& function)
			: m_function(function)
		{
		}

		std::any invoke([[maybe_unused]] Interpreter& interpreter, const std::vector<std::any>& arguments) override
		{
			if (arguments.size() != sizeof...(Parameters))
			{
				throw EvaluateException { 0, u8"invalid number of arguments." };
			}

			return invoke_impl(arguments, std::index_sequence_for<Parameters...> {});
		}

	private:
		template <std::size_t... Indices>
		std::any invoke_impl(const std::vector<std::any>& arguments, std::index_sequence<Indices...>)
		{
			return m_function(unpack<std::remove_cv_t<std::remove_reference_t<Parameters>>>(arguments[Indices])...);
		}

		template <typename T>
		static std::any unpack(const std::any& value)
		{
			if constexpr (std::is_same_v<T, std::any>)
			{
				return value;
			}
			else
			{
				assert(value.type() == typeid(T));
				return std::any_cast<T>(value);
			}
		}

		std::function<Return(Parameters...)> m_function;
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
		std::any evaluate(const ProgramNode& node, const std::shared_ptr<Environment>& env = std::make_shared<Environment>(nullptr))
		{
			std::any last;

			for (const auto& child : node.children())
			{
				last = dispatch(*child, env);
			}

			return last;
		}

	private:
		friend class StoneFunction;

		[[nodiscard]]
		static int asInteger(const std::any& value)
		{
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
		std::any dispatch(const Node& node, const std::shared_ptr<Environment>& env)
		{
#define STONE_NODE(_name, _format)                                \
			if (const auto p = dynamic_cast<const _name*>(&node)) \
				return evaluate(*p, env);
#include "Node.def.hpp"

			throw EvaluateException { node.lineNumber(), u8"unknown node type." };
		}

		[[nodiscard]]
		std::any evaluate([[maybe_unused]] const ParameterNode& node, [[maybe_unused]] const std::shared_ptr<Environment>& env)
		{
			assert(0 && "never reached");
		}

		[[nodiscard]]
		std::any evaluate([[maybe_unused]] const ParameterListNode& node, [[maybe_unused]] const std::shared_ptr<Environment>& env)
		{
			assert(0 && "never reached");
		}

		[[nodiscard]]
		std::any evaluate([[maybe_unused]] const ArgumentListNode& node, [[maybe_unused]] const std::shared_ptr<Environment>& env)
		{
			assert(0 && "never reached");
		}

		[[nodiscard]]
		std::any evaluate(const IfStatementNode& node, const std::shared_ptr<Environment>& env)
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
		std::any evaluate(const WhileStatementNode& node, const std::shared_ptr<Environment>& env)
		{
			std::any last;

			while (asInteger(dispatch(node.condition(), env)) != 0)
			{
				last = dispatch(node.body(), env);
			}

			return last;
		}

		[[nodiscard]]
		std::any evaluate(const CompoundStatementNode& node, const std::shared_ptr<Environment>& env)
		{
			std::any last;

			for (const auto& child : node.children())
			{
				last = dispatch(*child, env);
			}

			return last;
		}

		[[nodiscard]]
		std::any evaluate(const ProcedureStatementNode& node, const std::shared_ptr<Environment>& env)
		{
			const std::shared_ptr<IFunction> function = std::make_shared<StoneFunction>(node.lineNumber(), node.parameters(), node.body(), env);
			env->put(node.name(), function);

			return function;
		}

		[[nodiscard]]
		std::any evaluate(const BinaryExpressionNode& node, const std::shared_ptr<Environment>& env)
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
						env->set(left->name(), right);

						return right;
					}

					throw EvaluateException { node.lineNumber(), u8"invalid assignment." };
				}

				default:
					throw EvaluateException { node.lineNumber(), fmt::format(u8"unknown binary operator {}.", node.operation()) };
			}
		}

		[[nodiscard]]
		std::any evaluate(const UnaryExpressionNode& node, const std::shared_ptr<Environment>& env)
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
		std::any evaluate(const CallExpressionNode& node, const std::shared_ptr<Environment>& env)
		{
			// callee
			const auto function = std::any_cast<std::shared_ptr<IFunction>>(dispatch(node.callee(), env));

			// arguments
			std::vector<std::any> arguments;
			arguments.reserve(node.arguments().children().size());

			for (const auto& arg : node.arguments().children())
			{
				arguments.emplace_back(dispatch(*arg, env));
			}

			return function->invoke(*this, arguments);
		}

		[[nodiscard]]
		std::any evaluate(const ClosureExpressionNode& node, const std::shared_ptr<Environment>& env)
		{
			const std::shared_ptr<IFunction> function = std::make_shared<StoneFunction>(node.lineNumber(), node.parameters(), node.body(), env);

			return function;
		}

		[[nodiscard]]
		std::any evaluate(const IdentifierExpressionNode& node, const std::shared_ptr<Environment>& env)
		{
			return env->get(node.name());
		}

		[[nodiscard]]
		std::any evaluate(const IntegerExpressionNode& node, [[maybe_unused]] const std::shared_ptr<Environment>& env)
		{
			return node.value();
		}

		[[nodiscard]]
		std::any evaluate(const StringExpressionNode& node, [[maybe_unused]] const std::shared_ptr<Environment>& env)
		{
			return node.value();
		}
	};

	inline std::any StoneFunction::invoke(Interpreter& interpreter, const std::vector<std::any>& arguments)
	{
		// Push arguments.
		const auto calleeEnv = std::make_shared<Environment>(m_env);

		if (arguments.size() != m_parameters.children().size())
		{
			throw EvaluateException { m_line, u8"invalid number of arguments." };
		}

		for (std::size_t i = 0; i < arguments.size(); i++)
		{
			const auto& parameter = static_cast<const ParameterNode&>(*m_parameters.children()[i]);

			calleeEnv->put(parameter.name(), arguments[i]);
		}

		return interpreter.dispatch(m_body, calleeEnv);
	}
}
