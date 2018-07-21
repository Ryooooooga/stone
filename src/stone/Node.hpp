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
#include <memory>
#include <vector>

#include <fmt/ostream.h>

namespace stone
{
	enum class BinaryOperator
	{
#define STONE_BINARY_OPERATOR(_name, _text) _name,
#include "Node.def.hpp"
	};

	enum class UnaryOperator
	{
#define STONE_UNARY_OPERATOR(_name, _text) _name,
#include "Node.def.hpp"
	};

	inline std::ostream& operator <<(std::ostream& stream, BinaryOperator operation)
	{
		switch (operation)
		{
#define STONE_BINARY_OPERATOR(_name, _text)  \
			case BinaryOperator::_name:      \
				return stream << u8 ## _text;
#include "Node.def.hpp"

			default:
				return stream << u8"unknown";
		}
	}

	inline std::ostream& operator <<(std::ostream& stream, UnaryOperator operation)
	{
		switch (operation)
		{
#define STONE_UNARY_OPERATOR(_name, _text)   \
			case UnaryOperator::_name:       \
				return stream << u8 ## _text;
#include "Node.def.hpp"

			default:
				return stream << u8"unknown";
		}
	}

	class Node
	{
	public:
		// Uncopyable, unmovable.
		Node(const Node&) =delete;
		Node(Node&&) =delete;

		Node& operator=(const Node&) =delete;
		Node& operator=(Node&&) =delete;

		virtual ~Node() =default;

		[[nodiscard]]
		std::size_t lineNumber() const noexcept
		{
			return m_lineNumber;
		}

		[[nodiscard]]
		const std::vector<std::unique_ptr<Node>>& children() const noexcept
		{
			return m_children;
		}

	protected:
		explicit Node(std::size_t lineNumber)
			: m_lineNumber(lineNumber)
			, m_children()
		{
		}

		void addChild(std::unique_ptr<Node>&& node)
		{
			m_children.emplace_back(std::move(node));
		}

	private:
		std::size_t m_lineNumber;
		std::vector<std::unique_ptr<Node>> m_children;
	};

	class StatementNode
		: public Node
	{
	protected:
		using Node::Node;
	};

	class ExpressionNode
		: public StatementNode
	{
	protected:
		using StatementNode::StatementNode;
	};

	class ProgramNode
		: public Node
	{
	public:
		explicit ProgramNode()
			: Node(0)
		{
		}

		void addChild(std::unique_ptr<StatementNode>&& node)
		{
			assert(node);

			Node::addChild(std::move(node));
		}
	};

	class ParameterNode
		: public Node
	{
	public:
		explicit ParameterNode(std::size_t lineNumber, std::string_view name)
			: Node(lineNumber)
			, m_name(name)
		{
		}

		[[nodiscard]]
		std::string_view name() const noexcept
		{
			return m_name;
		}

	private:
		std::string m_name;
	};

	class ParameterListNode
		: public Node
	{
	public:
		explicit ParameterListNode(std::size_t lineNumber)
			: Node(lineNumber)
		{
		}

		void addChild(std::unique_ptr<ParameterNode>&& expression)
		{
			assert(expression);

			Node::addChild(std::move(expression));
		}
	};

	class ArgumentListNode
		: public Node
	{
	public:
		explicit ArgumentListNode(std::size_t lineNumber)
			: Node(lineNumber)
		{
		}

		void addChild(std::unique_ptr<ExpressionNode>&& expression)
		{
			assert(expression);

			Node::addChild(std::move(expression));
		}
	};

	class IfStatementNode
		: public StatementNode
	{
	public:
		explicit IfStatementNode(std::size_t lineNumber, std::unique_ptr<ExpressionNode>&& condition, std::unique_ptr<StatementNode>&& then, std::unique_ptr<StatementNode>&& otherwise)
			: StatementNode(lineNumber)
		{
			assert(condition);
			assert(then);

			addChild(std::move(condition));
			addChild(std::move(then));
			addChild(std::move(otherwise));
		}

		[[nodiscard]]
		ExpressionNode& condition() noexcept
		{
			return static_cast<ExpressionNode&>(*children()[0]);
		}

		[[nodiscard]]
		const ExpressionNode& condition() const noexcept
		{
			return static_cast<ExpressionNode&>(*children()[0]);
		}

		[[nodiscard]]
		StatementNode& then() noexcept
		{
			return static_cast<StatementNode&>(*children()[1]);
		}

		[[nodiscard]]
		const StatementNode& then() const noexcept
		{
			return static_cast<StatementNode&>(*children()[1]);
		}

		[[nodiscard]]
		StatementNode* otherwise() noexcept
		{
			return static_cast<StatementNode*>(children()[2].get());
		}

		[[nodiscard]]
		const StatementNode* otherwise() const noexcept
		{
			return static_cast<StatementNode*>(children()[2].get());
		}
	};

	class WhileStatementNode
		: public StatementNode
	{
	public:
		explicit WhileStatementNode(std::size_t lineNumber, std::unique_ptr<ExpressionNode>&& condition, std::unique_ptr<StatementNode>&& body)
			: StatementNode(lineNumber)
		{
			assert(condition);
			assert(body);

			addChild(std::move(condition));
			addChild(std::move(body));
		}

		[[nodiscard]]
		ExpressionNode& condition() noexcept
		{
			return static_cast<ExpressionNode&>(*children()[0]);
		}

		[[nodiscard]]
		const ExpressionNode& condition() const noexcept
		{
			return static_cast<ExpressionNode&>(*children()[0]);
		}

		[[nodiscard]]
		StatementNode& body() noexcept
		{
			return static_cast<StatementNode&>(*children()[1]);
		}

		[[nodiscard]]
		const StatementNode& body() const noexcept
		{
			return static_cast<StatementNode&>(*children()[1]);
		}
	};

	class CompoundStatementNode
		: public StatementNode
	{
	public:
		explicit CompoundStatementNode(std::size_t lineNumber)
			: StatementNode(lineNumber)
		{
		}

		void addChild(std::unique_ptr<StatementNode>&& node)
		{
			assert(node);

			Node::addChild(std::move(node));
		}
	};

	class ProcedureStatementNode
		: public StatementNode
	{
	public:
		explicit ProcedureStatementNode(std::size_t lineNumber, std::string_view name, std::unique_ptr<ParameterListNode>&& parameters, std::unique_ptr<StatementNode>&& body)
			: StatementNode(lineNumber)
			, m_name(name)
		{
			assert(parameters);
			assert(body);

			addChild(std::move(parameters));
			addChild(std::move(body));
		}

		[[nodiscard]]
		std::string_view name() const noexcept
		{
			return m_name;
		}

		[[nodiscard]]
		ParameterListNode& parameters() noexcept
		{
			return static_cast<ParameterListNode&>(*children()[0]);
		}

		[[nodiscard]]
		const ParameterListNode& parameters() const noexcept
		{
			return static_cast<ParameterListNode&>(*children()[0]);
		}

		[[nodiscard]]
		StatementNode& body() noexcept
		{
			return static_cast<StatementNode&>(*children()[1]);
		}

		[[nodiscard]]
		const StatementNode& body() const noexcept
		{
			return static_cast<StatementNode&>(*children()[1]);
		}

	private:
		std::string m_name;
	};

	class BinaryExpressionNode
		: public ExpressionNode
	{
	public:
		explicit BinaryExpressionNode(std::size_t lineNumber, BinaryOperator operation, std::unique_ptr<ExpressionNode>&& left, std::unique_ptr<ExpressionNode>&& right)
			: ExpressionNode(lineNumber)
			, m_operation(operation)
		{
			assert(left);
			assert(right);

			addChild(std::move(left));
			addChild(std::move(right));
		}

		[[nodiscard]]
		BinaryOperator operation() const noexcept
		{
			return m_operation;
		}

		[[nodiscard]]
		ExpressionNode& left() noexcept
		{
			return static_cast<ExpressionNode&>(*children()[0]);
		}

		[[nodiscard]]
		const ExpressionNode& left() const noexcept
		{
			return static_cast<ExpressionNode&>(*children()[0]);
		}

		[[nodiscard]]
		ExpressionNode& right() noexcept
		{
			return static_cast<ExpressionNode&>(*children()[1]);
		}

		[[nodiscard]]
		const ExpressionNode& right() const noexcept
		{
			return static_cast<ExpressionNode&>(*children()[1]);
		}

	private:
		BinaryOperator m_operation;
	};

	class UnaryExpressionNode
		: public ExpressionNode
	{
	public:
		explicit UnaryExpressionNode(std::size_t lineNumber, UnaryOperator operation, std::unique_ptr<ExpressionNode>&& operand)
			: ExpressionNode(lineNumber)
			, m_operation(operation)
		{
			assert(operand);

			addChild(std::move(operand));
		}

		[[nodiscard]]
		UnaryOperator operation() const noexcept
		{
			return m_operation;
		}

		[[nodiscard]]
		ExpressionNode& operand() noexcept
		{
			return static_cast<ExpressionNode&>(*children()[0]);
		}

		[[nodiscard]]
		const ExpressionNode& operand() const noexcept
		{
			return static_cast<ExpressionNode&>(*children()[0]);
		}

	private:
		UnaryOperator m_operation;
	};

	class CallExpressionNode
		: public ExpressionNode
	{
	public:
		explicit CallExpressionNode(std::size_t lineNumber, std::unique_ptr<ExpressionNode>&& callee, std::unique_ptr<ArgumentListNode>&& arguments)
			: ExpressionNode(lineNumber)
		{
			assert(callee);
			assert(arguments);

			addChild(std::move(callee));
			addChild(std::move(arguments));
		}

		[[nodiscard]]
		ExpressionNode& callee() noexcept
		{
			return static_cast<ExpressionNode&>(*children()[0]);
		}

		[[nodiscard]]
		const ExpressionNode& callee() const noexcept
		{
			return static_cast<ExpressionNode&>(*children()[0]);
		}

		[[nodiscard]]
		ArgumentListNode& arguments() noexcept
		{
			return static_cast<ArgumentListNode&>(*children()[1]);
		}

		[[nodiscard]]
		const ArgumentListNode& arguments() const noexcept
		{
			return static_cast<ArgumentListNode&>(*children()[1]);
		}
	};

	class IdentifierExpressionNode
		: public ExpressionNode
	{
	public:
		explicit IdentifierExpressionNode(std::size_t lineNumber, std::string_view name)
			: ExpressionNode(lineNumber)
			, m_name(name)
		{
		}

		[[nodiscard]]
		std::string_view name() const noexcept
		{
			return m_name;
		}

	private:
		std::string m_name;
	};

	class IntegerExpressionNode
		: public ExpressionNode
	{
	public:
		explicit IntegerExpressionNode(std::size_t lineNumber, int value)
			: ExpressionNode(lineNumber)
			, m_value(value)
		{
		}

		[[nodiscard]]
		int value() const noexcept
		{
			return m_value;
		}

	private:
		int m_value;
	};

	class StringExpressionNode
		: public ExpressionNode
	{
	public:
		explicit StringExpressionNode(std::size_t lineNumber, std::string_view value)
			: ExpressionNode(lineNumber)
			, m_value(value)
		{
		}

		[[nodiscard]]
		std::string_view value() const noexcept
		{
			return m_value;
		}

	private:
		std::string m_value;
	};

	class Printer
	{
	public:
		explicit Printer(std::ostream& stream)
			: m_stream(stream)
		{
		}

		// Uncopyable, unmovable.
		Printer(const Printer&) =delete;
		Printer(Printer&&) =delete;

		Printer& operator=(const Printer&) =delete;
		Printer& operator=(Printer&&) =delete;

		~Printer() =default;

		void print(const Node& node)
		{
			print(node, 0);
		}

	private:
		void print(const Node& node, std::size_t depth)
		{
			using namespace fmt::literals;

			for (std::size_t i = 0; i < depth; i++)
			{
				m_stream << u8"    ";
			}

#define FORMAT(_format, ...) FORMAT_I(_format, __VA_ARGS__)
#define FORMAT_I(_format, ...) fmt::format(u8 ## _format, __VA_ARGS__)
#define EXPAND(...) __VA_ARGS__
#define this p
#define STONE_NODE(_name, _format)                                                          \
			if (const auto p = dynamic_cast<const _name*>(&node))                           \
				m_stream << FORMAT(EXPAND _format, u8"class"_a = u8 ## #_name) << std::endl;
#include "Node.def.hpp"
#undef FORMAT
#undef FORMAT_I
#undef EXPAND
#undef this

			for (const auto& child : node.children())
			{
				if (child)
				{
					print(*child, depth + 1);
				}
			}
		}

		std::ostream& m_stream;
	};
}
