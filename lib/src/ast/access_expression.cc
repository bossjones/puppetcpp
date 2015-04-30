#include <puppet/ast/access_expression.hpp>
#include <puppet/ast/expression_def.hpp>
#include <puppet/ast/utility.hpp>

using namespace std;
using namespace puppet::lexer;

namespace puppet { namespace ast {

    access_expression::access_expression()
    {
    }

    access_expression::access_expression(token_position position, vector<expression> arguments) :
        _position(std::move(position)),
        _arguments(std::move(arguments))
    {
    }

    vector<expression> const& access_expression::arguments() const
    {
        return _arguments;
    }

    token_position const& access_expression::position() const
    {
        return _position;
    }

    ostream& operator<<(ostream& os, access_expression const& expr)
    {
        os << '[';
        pretty_print(os, expr.arguments(), ", ");
        os << ']';
        return os;
    }

}}  // namespace puppet::ast
