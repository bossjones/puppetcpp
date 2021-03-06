/**
 * @file
 * Declares the less than or equal to operator.
 */
#pragma once

#include "descriptor.hpp"

namespace puppet { namespace compiler { namespace evaluation { namespace operators { namespace binary {

    /**
     * Implements the less than or equal to operator.
     */
    struct less_equal
    {
        /**
         * Create a binary operator descriptor.
         * @return Returns the binary operator descriptor representing this operator.
         */
        static descriptor create_descriptor();
    };

}}}}}  // namespace puppet::compiler::evaluation::operators::binary
