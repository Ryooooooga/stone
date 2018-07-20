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

namespace stone
{
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
		: public Node
	{
	protected:
		using Node::Node;
	};

	class BinaryExpressionNode
		: public ExpressionNode
	{
	public:
		explicit BinaryExpressionNode(std::size_t lineNumber, std::unique_ptr<ExpressionNode>&& left, std::unique_ptr<ExpressionNode>&& right)
			: ExpressionNode(lineNumber)
		{
			assert(left);
			assert(right);
		}

		[[nodiscard]]
		ExpressionNode& left() noexcept
		{
			return static_cast<ExpressionNode&>(*children()[0]);
		}

		[[nodiscard]]
		ExpressionNode& right() noexcept
		{
			return static_cast<ExpressionNode&>(*children()[1]);
		}
	};

	class IntegerExpressionNode
		: public ExpressionNode
	{
	public:
		using ExpressionNode::ExpressionNode;

		[[nodiscard]]
		int value() const noexcept
		{
			return m_value;
		}

	private:
		int m_value;
	};
}
