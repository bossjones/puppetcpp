/**
 * @file
 * Declares the function descriptor.
 */
#pragma once

#include "../../ast/ast.hpp"
#include "../../../runtime/values/value.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace puppet { namespace compiler { namespace evaluation { namespace functions {

    // Forward declaration of function call_context.
    struct call_context;

    /**
     * Responsible for describing a Puppet function.
     */
    struct descriptor
    {
        /**
         * The callback type to call when the function call is dispatched.
         */
        using callback_type = std::function<runtime::values::value(call_context&)>;

        /**
         * Constructs a function descriptor.
         * @param name The name of the function.
         * @param statement The function statement if the function was defined in Puppet source code.
         */
        explicit descriptor(std::string name, ast::function_statement const* statement = nullptr);

        /**
         * Gets the function's name.
         * @return Returns the function's name.
         */
        std::string const& name() const;

        /**
         * Gets the associated function statement if the function was defined in Puppet source code.
         * @return Returns the function statement or nullptr if the function is a built-in function.
         */
        ast::function_statement const* statement() const;

        /**
         * Determines if the function has dispatch descriptors.
         * @return Returns true if the function has dispatch descriptors or false if not.
         */
        bool dispatchable() const;

        /**
         * Adds a dispatch descriptor for the function.
         * @param signature The signature for function call dispatch.
         * @param callback The callback to invoke when the function call is dispatched.
         */
        void add(std::string const& signature, callback_type callback);

        /**
         * Dispatches a function call to the matching dispatch descriptor.
         * @param context The call context to dispatch.
         * @return Returns the result of the function call.
         */
        runtime::values::value dispatch(call_context& context) const;

     private:
        struct dispatch_descriptor
        {
            runtime::types::callable signature;
            callback_type callback;
        };

        std::vector<dispatch_descriptor const*> check_argument_count(call_context const& context) const;
        void check_block_parameters(call_context const& context, std::vector<dispatch_descriptor const*> const& invocable) const;
        void check_parameter_types(call_context const& context, std::vector<dispatch_descriptor const*> const& invocable) const;

        std::string _name;
        std::vector<dispatch_descriptor> _dispatch_descriptors;
        std::shared_ptr<ast::syntax_tree> _tree;
        ast::function_statement const* _statement;
    };

}}}}  // puppet::compiler::evaluation::functions
