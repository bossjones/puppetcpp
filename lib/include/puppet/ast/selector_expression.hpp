/**
 * @file
 * Declares the AST selector expression.
 */
#pragma once

#include "expression.hpp"

namespace puppet { namespace ast {

    /**
     * Represents an AST selector case expression.
     */
    struct selector_case_expression
    {
        /**
         * Default constructor for selector case expression.
         */
        selector_case_expression();

        /**
         * Constructs a selector case expression with the given selector and result expressions.
         * @param selector The selector expression.
         * @param result The result expression if the selector matches.
         */
        selector_case_expression(expression selector, expression result);

        /**
         * The selector expression.
         */
        expression selector;

        /**
         * The resulting value.
         */
        expression result;

        /**
         * Gets the position of the expression.
         */
        lexer::position const& position() const;
    };

    /**
     * Stream insertion operator for AST selector case expression.
     * @param os The output stream to write the selector case expression to.
     * @param expr The selector case expression to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, selector_case_expression const& expr);

    /**
     * Represents an AST selector expression.
     */
    struct selector_expression
    {
        /**
         * Default constructor for selector_expression.
         */
        selector_expression();

        /**
         * Constructs a selector expression.
         * @param position The position of the selector expression.
         * @param cases The selector cases.
         */
        selector_expression(lexer::position position, std::vector<selector_case_expression> cases);

        /**
         * The position of the selector expression.
         */
        lexer::position position;

        /**
         * The selector's case expressions.
         */
        std::vector<selector_case_expression> cases;
    };

    /**
     * Stream insertion operator for AST selector expression.
     * @param os The output stream to write the selector expression to.
     * @param expr The selector expression to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, selector_expression const& expr);

}}  // puppet::ast
