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

#include <functional>
#include <unordered_map>

#include "Exception.hpp"
#include "Node.hpp"

namespace stone
{
	class Interpreter;
	class StoneObject;

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

		void set(std::string_view name, const std::shared_ptr<StoneObject>& value)
		{
			assert(value);

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

		void put(std::string_view name, const std::shared_ptr<StoneObject>& value)
		{
			m_table[name] = value;
		}

		[[nodiscard]]
		std::shared_ptr<StoneObject> get(std::string_view name, bool recursive = true)
		{
			if (const auto it = m_table.find(name); it != std::cend(m_table))
			{
				return it->second;
			}
			else if (recursive && m_parent)
			{
				return m_parent->get(name);
			}

			return nullptr;
		}

	private:
		std::shared_ptr<Environment> m_parent;
		std::unordered_map<std::string_view, std::shared_ptr<StoneObject>> m_table;
	};

	class StoneObject
	{
	public:
		StoneObject() =default;

		// Uncopyable, unmovable.
		StoneObject(const StoneObject&) =delete;
		StoneObject(StoneObject&&) =delete;

		StoneObject& operator=(const StoneObject&) =delete;
		StoneObject& operator=(StoneObject&&) =delete;

		virtual ~StoneObject() =default;

		[[nodiscard]]
		virtual bool isInteger() const
		{
			return false;
		}

		[[nodiscard]]
		virtual bool isString() const
		{
			return false;
		}

		virtual int asInteger()
		{
			throw EvaluateException { 0, u8"cannot convert to int." };
		}

		virtual std::string asString()
		{
			throw EvaluateException { 0, u8"cannot convert to string." };
		}

		virtual std::shared_ptr<StoneObject> invoke([[maybe_unused]] Interpreter& interpreter, [[maybe_unused]] const std::vector<std::shared_ptr<StoneObject>>& arguments)
		{
			throw EvaluateException { 0, u8"value is not a function." };
		}

		virtual std::shared_ptr<StoneObject> getMember([[maybe_unused]] Interpreter& interpreter, std::string_view memberName)
		{
			throw EvaluateException { 0, fmt::format(u8"invalid member name `{}'.", memberName) };
		}

		virtual void setMember([[maybe_unused]] Interpreter& interpreter, std::string_view memberName, [[maybe_unused]] const std::shared_ptr<StoneObject>& value)
		{
			throw EvaluateException { 0, fmt::format(u8"invalid member assignment `{}'.", memberName) };
		}

		virtual std::shared_ptr<StoneObject> getIndexed([[maybe_unused]] Interpreter& interpreter, [[maybe_unused]] const std::shared_ptr<StoneObject>& index)
		{
			throw EvaluateException { 0, fmt::format(u8"invalid index access.") };
		}

		virtual void setIndexed([[maybe_unused]] Interpreter& interpreter, [[maybe_unused]] const std::shared_ptr<StoneObject>& index, [[maybe_unused]] const std::shared_ptr<StoneObject>& value)
		{
			throw EvaluateException { 0, fmt::format(u8"invalid index assignment.") };
		}
	};

	class IntegerObject
		: public StoneObject
	{
	public:
		explicit IntegerObject(int value)
			: m_value(value)
		{
		}

		[[nodiscard]]
		bool isInteger() const override
		{
			return true;
		}

		int asInteger() override
		{
			return m_value;
		}

		std::string asString() override
		{
			return std::to_string(m_value);
		}

	private:
		int m_value;
	};

	class StringObject
		: public StoneObject
	{
	public:
		explicit StringObject(std::string_view value)
			: m_value(value)
		{
		}

		[[nodiscard]]
		bool isString() const override
		{
			return true;
		}

		std::string asString() override
		{
			return m_value;
		}

	private:
		std::string m_value;
	};

	class StoneFunctionObject
		: public StoneObject
	{
	public:
		explicit StoneFunctionObject(std::size_t line, const ParameterListNode& parameters, const StatementNode& body, const std::shared_ptr<Environment>& env)
			: m_line(line)
			, m_parameters(parameters)
			, m_body(body)
			, m_env(env)
		{
			assert(m_env);
		}

		std::shared_ptr<StoneObject> invoke(Interpreter& interpreter, const std::vector<std::shared_ptr<StoneObject>>& arguments) override;

	private:
		std::size_t m_line;
		const ParameterListNode& m_parameters;
		const StatementNode& m_body;
		std::shared_ptr<Environment> m_env;
	};

	template <typename Return, typename... Parameters>
	class NativeFunctionObject
		: public StoneObject
	{
	public:
		explicit NativeFunctionObject(const std::function<Return(Parameters...)>& function)
			: m_function(function)
		{
		}

		std::shared_ptr<StoneObject> invoke([[maybe_unused]] Interpreter& interpreter, const std::vector<std::shared_ptr<StoneObject>>& arguments) override
		{
			if (arguments.size() != sizeof...(Parameters))
			{
				throw EvaluateException { 0, u8"invalid number of arguments." };
			}

			return invoke_impl(arguments, std::index_sequence_for<Parameters...> {});
		}

	private:
		template <std::size_t... Indices>
		std::shared_ptr<StoneObject> invoke_impl(const std::vector<std::shared_ptr<StoneObject>>& arguments, std::index_sequence<Indices...>)
		{
			return m_function(unpack<std::remove_cv_t<std::remove_reference_t<Parameters>>>(arguments[Indices])...);
		}

		template <typename T>
		static auto unpack(const std::shared_ptr<StoneObject>& value)
		{
			if constexpr (std::is_same_v<T, int>)
			{
				return value->asInteger();
			}
			else if constexpr (std::is_same_v<T, std::string>)
			{
				return value->asString();
			}
			else
			{
				return value;
			}
		}

		std::function<Return(Parameters...)> m_function;
	};

	class InstanceObject
		: public StoneObject
		, public std::enable_shared_from_this<InstanceObject>
	{
	public:
		static std::shared_ptr<InstanceObject> create(const std::shared_ptr<Environment>& outerEnv)
		{
			assert(outerEnv);

			struct Helper : InstanceObject {};
			const auto p = std::make_shared<Helper>();
			p->initialize(outerEnv);

			return p;
		}

		std::shared_ptr<StoneObject> getMember(Interpreter& interpreter, std::string_view memberName) override
		{
			if (const auto member = m_env->get(memberName, false))
			{
				return member;
			}

			return StoneObject::getMember(interpreter, memberName);
		}

		void setMember([[maybe_unused]] Interpreter& interpreter, std::string_view memberName, const std::shared_ptr<StoneObject>& value) override
		{
			m_env->put(memberName, value);
		}

	private:
		friend class ClassObject;

		InstanceObject() =default;

		void initialize(const std::shared_ptr<Environment>& outerEnv)
		{
			m_env = std::make_shared<Environment>(outerEnv);
			m_env->put(u8"this", shared_from_this());
		}

		std::shared_ptr<Environment> m_env;
	};

	class ClassObject
		: public StoneObject
	{
	public:
		explicit ClassObject(const ClassStatementNode& node, const std::shared_ptr<Environment>& env, const std::shared_ptr<ClassObject>& superClass)
			: m_node(node)
			, m_env(env)
			, m_superClass(superClass)
		{
			assert(m_env);
		}

		std::string asString() override
		{
			return fmt::format(u8"[class {}]", m_node.name());
		}

		std::shared_ptr<StoneObject> getMember(Interpreter& interpreter, std::string_view memberName) override;

	private:
		const ClassStatementNode& m_node;
		std::shared_ptr<Environment> m_env;
		std::shared_ptr<ClassObject> m_superClass;
	};

	class ArrayObject
		: public StoneObject
	{
	public:
		explicit ArrayObject(std::vector<std::shared_ptr<StoneObject>>&& elements)
			: m_elements(elements)
		{
		}

		std::string asString() override
		{
			auto s = std::string {};
			auto delim = u8"[";

			for (const auto& element : m_elements)
			{
				s += delim;
				s += element->asString();

				delim = u8", ";
			}

			s += u8"]";

			return s;
		}

		std::shared_ptr<StoneObject> getIndexed([[maybe_unused]] Interpreter& interpreter, const std::shared_ptr<StoneObject>& index) override
		{
			if (const auto n = static_cast<std::size_t>(index->asInteger()); n < m_elements.size())
			{
				return m_elements[n];
			}

			throw EvaluateException { 0, u8"array index out of bounds." };
		}

		void setIndexed([[maybe_unused]] Interpreter& interpreter, const std::shared_ptr<StoneObject>& index, const std::shared_ptr<StoneObject>& value) override
		{
			if (const auto n = static_cast<std::size_t>(index->asInteger()); n < m_elements.size())
			{
				m_elements[n] = value;
				return;
			}

			throw EvaluateException { 0, u8"array index out of bounds." };
		}

	private:
		std::vector<std::shared_ptr<StoneObject>> m_elements;
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
		std::shared_ptr<StoneObject> evaluate(const ProgramNode& node, const std::shared_ptr<Environment>& env = std::make_shared<Environment>(nullptr))
		{
			std::shared_ptr<StoneObject> last;

			for (const auto& child : node.children())
			{
				last = dispatch(*child, env);
			}

			return last;
		}

	private:
		friend class StoneFunctionObject;
		friend class ClassObject;

		std::shared_ptr<StoneObject> dispatch(const Node& node, const std::shared_ptr<Environment>& env)
		{
#define STONE_NODE(_name, _format)                                \
			if (const auto p = dynamic_cast<const _name*>(&node)) \
				return evaluate(*p, env);
#include "Node.def.hpp"

			throw EvaluateException { node.lineNumber(), u8"unknown node type." };
		}

		[[nodiscard]]
		std::shared_ptr<StoneObject> evaluate([[maybe_unused]] const ParameterNode& node, [[maybe_unused]] const std::shared_ptr<Environment>& env)
		{
			assert(0 && "never reached");
		}

		[[nodiscard]]
		std::shared_ptr<StoneObject> evaluate([[maybe_unused]] const ParameterListNode& node, [[maybe_unused]] const std::shared_ptr<Environment>& env)
		{
			assert(0 && "never reached");
		}

		[[nodiscard]]
		std::shared_ptr<StoneObject> evaluate([[maybe_unused]] const ArgumentListNode& node, [[maybe_unused]] const std::shared_ptr<Environment>& env)
		{
			assert(0 && "never reached");
		}

		[[nodiscard]]
		std::shared_ptr<StoneObject> evaluate(const IfStatementNode& node, const std::shared_ptr<Environment>& env)
		{
			if (dispatch(node.condition(), env)->asInteger() != 0)
			{
				return dispatch(node.then(), env);
			}
			else if (const auto otherwise = node.otherwise())
			{
				return dispatch(*otherwise, env);
			}

			return nullptr;
		}

		[[nodiscard]]
		std::shared_ptr<StoneObject> evaluate(const WhileStatementNode& node, const std::shared_ptr<Environment>& env)
		{
			std::shared_ptr<StoneObject> last;

			while (dispatch(node.condition(), env)->asInteger() != 0)
			{
				last = dispatch(node.body(), env);
			}

			return last;
		}

		[[nodiscard]]
		std::shared_ptr<StoneObject> evaluate(const CompoundStatementNode& node, const std::shared_ptr<Environment>& env)
		{
			std::shared_ptr<StoneObject> last;

			for (const auto& child : node.children())
			{
				last = dispatch(*child, env);
			}

			return last;
		}

		[[nodiscard]]
		std::shared_ptr<StoneObject> evaluate(const ProcedureStatementNode& node, const std::shared_ptr<Environment>& env)
		{
			const auto function = std::make_shared<StoneFunctionObject>(node.lineNumber(), node.parameters(), node.body(), env);
			env->put(node.name(), function);

			return function;
		}

		[[nodiscard]]
		std::shared_ptr<StoneObject> evaluate(const ClassStatementNode& node, const std::shared_ptr<Environment>& env)
		{
			const auto superClass = [&]()
			{
				if (!node.superName())
				{
					return std::shared_ptr<ClassObject> {nullptr};
				}

				const auto superClass = env->get(*node.superName());

				if (superClass == nullptr)
				{
					throw EvaluateException { node.lineNumber(), fmt::format(u8"unknown super class `{}'.", *node.superName()) };
				}
				if (std::dynamic_pointer_cast<ClassObject>(superClass) == nullptr)
				{
					throw EvaluateException { node.lineNumber(), fmt::format(u8"`{}' is not a class.", *node.superName()) };
				}

				return std::static_pointer_cast<ClassObject>(superClass);
			}();

			const auto classObject = std::make_shared<ClassObject>(node, env, std::static_pointer_cast<ClassObject>(superClass));
			env->put(node.name(), classObject);

			return classObject;
		}

		[[nodiscard]]
		std::shared_ptr<StoneObject> evaluate(const BinaryExpressionNode& node, const std::shared_ptr<Environment>& env)
		{
			switch (node.operation())
			{
				case BinaryOperator::addition:
				{
					const auto left = dispatch(node.left(), env);
					const auto right = dispatch(node.right(), env);

					if (left->isInteger() && right->isInteger())
						return std::make_shared<IntegerObject>(left->asInteger() + right->asInteger());
					else
						return std::make_shared<StringObject>(left->asString() + right->asString());
				}

				case BinaryOperator::subtraction:
					return std::make_shared<IntegerObject>(dispatch(node.left(), env)->asInteger() - dispatch(node.right(), env)->asInteger());
				case BinaryOperator::multiplication:
					return std::make_shared<IntegerObject>(dispatch(node.left(), env)->asInteger() * dispatch(node.right(), env)->asInteger());
				case BinaryOperator::division:
					return std::make_shared<IntegerObject>(dispatch(node.left(), env)->asInteger() / dispatch(node.right(), env)->asInteger());
				case BinaryOperator::modulo:
					return std::make_shared<IntegerObject>(dispatch(node.left(), env)->asInteger() % dispatch(node.right(), env)->asInteger());

				case BinaryOperator::equal:
				{
					const auto left = dispatch(node.left(), env);
					const auto right = dispatch(node.right(), env);

					if (left->isInteger() && right->isInteger())
						return std::make_shared<IntegerObject>(left->asInteger() == right->asInteger() ? 1 : 0);
					else if (left->isString() || right->isString())
						return std::make_shared<IntegerObject>(left->asString() == right->asString() ? 1 : 0);
					else
						return std::make_shared<IntegerObject>(left == right ? 1 : 0);
				}

				case BinaryOperator::notEqual:
				{
					const auto left = dispatch(node.left(), env);
					const auto right = dispatch(node.right(), env);

					if (left->isInteger() || right->isInteger())
						return std::make_shared<IntegerObject>(left->asInteger() != right->asInteger() ? 1 : 0);
					else if (left->isString() || right->isString())
						return std::make_shared<IntegerObject>(left->asString() != right->asString() ? 1 : 0);
					else
						return std::make_shared<IntegerObject>(left != right ? 1 : 0);
				}

				case BinaryOperator::lesserThan:
					return std::make_shared<IntegerObject>(dispatch(node.left(), env)->asInteger() < dispatch(node.right(), env)->asInteger() ? 1 : 0);
				case BinaryOperator::lesserEqual:
					return std::make_shared<IntegerObject>(dispatch(node.left(), env)->asInteger() <= dispatch(node.right(), env)->asInteger() ? 1 : 0);
				case BinaryOperator::greaterThan:
					return std::make_shared<IntegerObject>(dispatch(node.left(), env)->asInteger() > dispatch(node.right(), env)->asInteger() ? 1 : 0);
				case BinaryOperator::greaterEqual:
					return std::make_shared<IntegerObject>(dispatch(node.left(), env)->asInteger() >= dispatch(node.right(), env)->asInteger() ? 1 : 0);

				case BinaryOperator::assign:
				{
					if (const auto left = dynamic_cast<const IdentifierExpressionNode*>(&node.left()))
					{
						const auto right = dispatch(node.right(), env);
						env->set(left->name(), right);

						return right;
					}
					if (const auto left = dynamic_cast<const MemberAccessExpressionNode*>(&node.left()))
					{
						const auto right = dispatch(node.right(), env);
						const auto operand = dispatch(left->operand(), env);

						operand->setMember(*this, left->memberName(), right);

						return right;
					}
					if (const auto left = dynamic_cast<const ArrayIndexExpressionNode*>(&node.left()))
					{
						const auto right = dispatch(node.right(), env);
						const auto index = dispatch(left->index(), env);
						const auto operand = dispatch(left->operand(), env);

						operand->setIndexed(*this, index, right);

						return right;
					}

					throw EvaluateException { node.lineNumber(), u8"invalid assignment." };
				}

				default:
					throw EvaluateException { node.lineNumber(), fmt::format(u8"unknown binary operator {}.", node.operation()) };
			}
		}

		[[nodiscard]]
		std::shared_ptr<StoneObject> evaluate(const UnaryExpressionNode& node, const std::shared_ptr<Environment>& env)
		{
			switch (node.operation())
			{
				case UnaryOperator::negation:
					return std::make_shared<IntegerObject>(-dispatch(node, env)->asInteger());

				default:
					throw EvaluateException { node.lineNumber(), fmt::format(u8"unknown unary operator {}.", node.operation()) };
			}
		}

		[[nodiscard]]
		std::shared_ptr<StoneObject> evaluate(const CallExpressionNode& node, const std::shared_ptr<Environment>& env)
		{
			// callee
			const auto function = dispatch(node.callee(), env);

			// arguments
			std::vector<std::shared_ptr<StoneObject>> arguments;
			arguments.reserve(node.arguments().children().size());

			for (const auto& arg : node.arguments().children())
			{
				arguments.emplace_back(dispatch(*arg, env));
			}

			return function->invoke(*this, arguments);
		}

		[[nodiscard]]
		std::shared_ptr<StoneObject> evaluate(const ArrayIndexExpressionNode& node, const std::shared_ptr<Environment>& env)
		{
			// index
			const auto index = dispatch(node.index(), env);

			// operand
			const auto operand = dispatch(node.operand(), env);

			return operand->getIndexed(*this, index);
		}

		[[nodiscard]]
		std::shared_ptr<StoneObject> evaluate(const MemberAccessExpressionNode& node, const std::shared_ptr<Environment>& env)
		{
			const auto operand = dispatch(node.operand(), env);

			return operand->getMember(*this, node.memberName());
		}

		[[nodiscard]]
		std::shared_ptr<StoneObject> evaluate(const ClosureExpressionNode& node, const std::shared_ptr<Environment>& env)
		{
			return std::make_shared<StoneFunctionObject>(node.lineNumber(), node.parameters(), node.body(), env);
		}

		[[nodiscard]]
		std::shared_ptr<StoneObject> evaluate(const ArrayExpressionNode& node, const std::shared_ptr<Environment>& env)
		{
			// elements
			std::vector<std::shared_ptr<StoneObject>> elements;
			elements.reserve(node.children().size());

			for (const auto& element : node.children())
			{
				elements.emplace_back(dispatch(*element, env));
			}

			return std::make_shared<ArrayObject>(std::move(elements));
		}

		[[nodiscard]]
		std::shared_ptr<StoneObject> evaluate(const IdentifierExpressionNode& node, const std::shared_ptr<Environment>& env)
		{
			return env->get(node.name());
		}

		[[nodiscard]]
		std::shared_ptr<StoneObject> evaluate(const IntegerExpressionNode& node, [[maybe_unused]] const std::shared_ptr<Environment>& env)
		{
			return std::make_shared<IntegerObject>(node.value());
		}

		[[nodiscard]]
		std::shared_ptr<StoneObject> evaluate(const StringExpressionNode& node, [[maybe_unused]] const std::shared_ptr<Environment>& env)
		{
			return std::make_shared<StringObject>(node.value());
		}
	};

	inline std::shared_ptr<StoneObject> StoneFunctionObject::invoke(Interpreter& interpreter, const std::vector<std::shared_ptr<StoneObject>>& arguments)
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

	inline std::shared_ptr<StoneObject> ClassObject::getMember(Interpreter& interpreter, std::string_view memberName)
	{
		if (memberName == u8"new")
		{
			const auto object = m_superClass
				? std::static_pointer_cast<InstanceObject>(m_superClass->getMember(interpreter, memberName))
				: InstanceObject::create(m_env);

			interpreter.dispatch(m_node.body(), object->m_env);

			return object;
		}

		return StoneObject::getMember(interpreter, memberName);
	}
}
