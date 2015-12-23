#include <puppet/compiler/lexer/number_token.hpp>
#include <puppet/cast.hpp>
#include <iomanip>

using namespace std;

namespace puppet { namespace compiler { namespace lexer {

    ostream& operator<<(ostream& os, numeric_base base)
    {
        switch (base) {
            case numeric_base::decimal:
                os << "decimal";
                break;

            case numeric_base::octal:
                os << "octal";
                break;

            case lexer::numeric_base::hexadecimal:
                os << "hexadecimal";
                break;

            default:
                throw runtime_error("unexpected numeric base.");
        }
        return os;
    }

    number_token::number_token(lexer::range range, int64_t value, numeric_base base) :
        _range(rvalue_cast(range)),
        _value(value),
        _base(base)
    {
    }

    number_token::number_token(lexer::range range, double value) :
        _range(rvalue_cast(range)),
        _value(value),
        _base(numeric_base::decimal)
    {
    }

    lexer::range const& number_token::range() const
    {
        return _range;
    }

    number_token::value_type const& number_token::value() const
    {
        return _value;
    }

    numeric_base number_token::base() const
    {
        return _base;
    }

    struct insertion_visitor : boost::static_visitor<ostream&>
    {
        insertion_visitor(ostream& os, numeric_base base) :
            _os(os),
            _base(base)
        {
        }

        result_type operator()(int64_t value) const
        {
            switch (_base) {
                case numeric_base::decimal:
                    _os << value;
                    break;

                case numeric_base::octal:
                    _os << "0" << oct << value << dec;
                    break;

                case numeric_base::hexadecimal:
                    _os << "0x" << hex << value << dec;
                    break;

                default:
                    throw runtime_error("unexpected numeric base for number token.");
            }
            return _os;
        }

        result_type operator()(double value) const
        {
            return (_os << value);
        }

     private:
        ostream& _os;
        numeric_base _base;
    };

    ostream& operator<<(ostream& os, number_token const& token)
    {
        return boost::apply_visitor(insertion_visitor(os, token.base()), token.value());
    }

}}}  // namespace puppet::compiler::lexer
