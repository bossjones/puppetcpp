#include <puppet/compiler/evaluation/operators/binary/not_equals.hpp>
#include <puppet/compiler/evaluation/operators/binary/call_context.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace operators { namespace binary {

    descriptor not_equals::create_descriptor()
    {
        binary::descriptor descriptor{ ast::binary_operator::not_equals };

        descriptor.add("Any", "Any", [](call_context& context) {
            return context.left() != context.right();
        });
        return descriptor;
    }

}}}}}  // namespace puppet::compiler::evaluation::operators::binary