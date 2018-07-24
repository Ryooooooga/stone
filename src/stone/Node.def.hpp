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

#ifndef STONE_NODE
#	define STONE_NODE(_name, _format)
#endif

#ifndef STONE_STATEMENT_NODE
#	define STONE_STATEMENT_NODE(_name, _format) STONE_NODE(_name, _format)
#endif

#ifndef STONE_EXPRESSION_NODE
#	define STONE_EXPRESSION_NODE(_name, _format) STONE_NODE(_name, _format)
#endif

#ifndef STONE_BINARY_OPERATOR
#	define STONE_BINARY_OPERATOR(_name, _text)
#endif

#ifndef STONE_UNARY_OPERATOR
#	define STONE_UNARY_OPERATOR(_name, _text)
#endif

STONE_NODE(ProgramNode, ("{class}"))
STONE_NODE(ParameterNode, ("{class} {name}", u8"name"_a=this->name()))
STONE_NODE(ParameterListNode, ("{class}"))
STONE_NODE(ArgumentListNode, ("{class}"))

STONE_STATEMENT_NODE(IfStatementNode, ("{class}"))
STONE_STATEMENT_NODE(WhileStatementNode, ("{class}"))
STONE_STATEMENT_NODE(CompoundStatementNode, ("{class}"))
STONE_STATEMENT_NODE(ProcedureStatementNode, ("{class} {name}", u8"name"_a=this->name()))
STONE_STATEMENT_NODE(ClassStatementNode, ("{class} {name} {super}", u8"name"_a=this->name(), u8"super"_a=this->superName().value_or(u8"-")))

STONE_EXPRESSION_NODE(BinaryExpressionNode, ("{class} {operator}", u8"operator"_a=this->operation()))
STONE_EXPRESSION_NODE(UnaryExpressionNode, ("{class} {operator}", u8"operator"_a=this->operation()))
STONE_EXPRESSION_NODE(CallExpressionNode, ("{class}"))
STONE_EXPRESSION_NODE(ArrayIndexExpressionNode, ("{class}"))
STONE_EXPRESSION_NODE(MemberAccessExpressionNode, ("{class} {member}", u8"member"_a=this->memberName()))
STONE_EXPRESSION_NODE(ClosureExpressionNode, ("{class}"))
STONE_EXPRESSION_NODE(ArrayExpressionNode, ("{class}"))
STONE_EXPRESSION_NODE(IdentifierExpressionNode, ("{class} {name}", u8"name"_a=this->name()))
STONE_EXPRESSION_NODE(IntegerExpressionNode, ("{class} {value}", u8"value"_a=this->value()))
STONE_EXPRESSION_NODE(StringExpressionNode, ("{class} {value}", u8"value"_a=this->value()))

STONE_BINARY_OPERATOR(addition, "+")
STONE_BINARY_OPERATOR(subtraction, "-")
STONE_BINARY_OPERATOR(multiplication, "*")
STONE_BINARY_OPERATOR(division, "/")
STONE_BINARY_OPERATOR(modulo, "%")
STONE_BINARY_OPERATOR(equal, "==")
STONE_BINARY_OPERATOR(notEqual, "!=")
STONE_BINARY_OPERATOR(lesserThan, "<")
STONE_BINARY_OPERATOR(lesserEqual, "<=")
STONE_BINARY_OPERATOR(greaterThan, ">")
STONE_BINARY_OPERATOR(greaterEqual, ">=")
STONE_BINARY_OPERATOR(assign, "=")

STONE_UNARY_OPERATOR(negation, "-")

#undef STONE_NODE
#undef STONE_STATEMENT_NODE
#undef STONE_EXPRESSION_NODE
#undef STONE_BINARY_OPERATOR
#undef STONE_UNARY_OPERATOR
