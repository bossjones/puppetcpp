#include <puppet/runtime/context.hpp>
#include <puppet/runtime/executor.hpp>
#include <puppet/cast.hpp>
#include <boost/format.hpp>

using namespace std;

namespace puppet { namespace runtime {

    class_definition::class_definition(types::klass klass, shared_ptr<compiler::context> context, ast::class_definition_expression const* expression) :
        _klass(rvalue_cast(klass)),
        _context(rvalue_cast(context)),
        _expression(expression)
    {
        if (!_context) {
            throw runtime_error("expected compilation context.");
        }

        _path = _context->path();

        if (_expression) {
            _line = _expression->position().line();
            if (_expression->parent()) {
                _parent = types::klass(_expression->parent()->value());
            }
        }
    }

    types::klass const& class_definition::klass() const
    {
        return _klass;
    }

    types::klass const* class_definition::parent() const
    {
        return _parent.title().empty() ? nullptr : &_parent;
    }

    string const& class_definition::path() const
    {
        return *_path;
    }

    size_t class_definition::line() const
    {
        return _line;
    }

    bool class_definition::evaluated() const
    {
        return !_context || !_expression;
    }

    bool class_definition::evaluate(runtime::context& context, unordered_map<ast::name, values::value> const* arguments)
    {
        if (evaluated()) {
            return true;
        }

        values::hash keywords;
        values::value name = _klass.title();

        // Ensure there is a parameter for each argument
        if (arguments) {
            for (auto const& argument : *arguments) {
                // Check for the name argument
                if (argument.first.value() == "name") {
                    name = argument.second;
                    continue;
                }
                bool found = false;
                for (auto const& parameter : *_expression->parameters()) {
                    if (parameter.variable().name() == argument.first.value()) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    throw evaluation_exception(argument.first.position(), (boost::format("'%1%' is not a valid parameter for class '%2%'.") % argument.first % _klass.title()).str());
                }
                keywords[argument.first.value()] = argument.second;
            }
        }

        // Evaluate the parent
        auto parent_scope = evaluate_parent(context);
        if (!parent_scope) {
            return false;
        }

        // Create a scope for the class
        ostringstream display_name;
        display_name << _klass;
        auto& scope = context.add_scope(_klass.title(), display_name.str(), parent_scope);

        // Set the title and name variables in the scope
        scope.set("title", _klass.title(), _context->path(), _expression->position().line());
        scope.set("name", rvalue_cast(name), _context->path(), _expression->position().line());

        try {
            expression_evaluator evaluator{ _context, context };
            runtime::executor executor(evaluator, _expression->position(), _expression->parameters(), _expression->body());
            executor.execute(keywords, &scope);
        } catch (evaluation_exception const& ex) {
            // The compilation context is probably not the current one, so do not let evaluation exception propagate
            _context->log(logging::level::error, ex.position(), ex.what());
            return false;
        }

        // Copy the path for later use
        _context.reset();
        _expression = nullptr;
        return true;
    }

    runtime::scope* class_definition::evaluate_parent(runtime::context& context)
    {
        // If no parent, return the top scope
        auto parent = this->parent();
        if (!parent) {
            return &context.top();
        }

        if (!context.is_class_defined(*parent)) {
            _context->log(logging::level::error, _expression->parent()->position(), (boost::format("base class '%2%' has not been defined.") % parent->title()).str());
            return nullptr;
        }
        if (!context.declare_class(*parent, _context->path(), _expression->parent()->position())) {
            return nullptr;
        }
        return context.find_scope(parent->title());
    }

    context::context(runtime::catalog& catalog) :
        _catalog(catalog)
    {
        // Add the top scope
        push_scope(add_scope("", "Class[main]"));
    }

    runtime::catalog& context::catalog()
    {
        return _catalog;
    }

    runtime::scope& context::scope()
    {
        return *_scope_stack.back();
    }

    runtime::scope& context::top()
    {
        return *_scope_stack.front();
    }

    runtime::scope& context::add_scope(string name, string display_name, runtime::scope* parent)
    {
        runtime::scope scope{rvalue_cast(name), rvalue_cast(display_name), parent};
        return _scopes.emplace(make_pair(scope.name(), rvalue_cast(scope))).first->second;
    }

    runtime::scope* context::find_scope(string const& name)
    {
        auto it = _scopes.find(name);
        if (it == _scopes.end()) {
            return nullptr;
        }
        return &it->second;
    }

    void context::push_scope(runtime::scope& current)
    {
        _scope_stack.push_back(&current);
    }

    bool context::pop_scope()
    {
        // Don't pop the top scope
        if (_scope_stack.size() == 1) {
            return false;
        }
        _scope_stack.pop_back();
        return true;
    }

    class_definition const* context::define_class(types::klass klass, shared_ptr<compiler::context> context, ast::class_definition_expression const& expression)
    {
        // Validate the class parameters
        if (expression.parameters()) {
            validate_parameters(true, *expression.parameters());
        }

        auto& definitions = _classes[klass];

        // Check to make sure all existing definitions of the class have the same base or do not inherit
        if (expression.parent()) {
            types::klass parent(expression.parent()->value());
            for (auto const& definition : definitions) {
                auto existing = definition.parent();
                if (!existing) {
                    continue;
                }
                if (parent == *existing) {
                    continue;
                }
                return &definition;
            }
        }
        definitions.push_back(class_definition(rvalue_cast(klass), context, &expression));
        return nullptr;
    }

    runtime::resource* context::declare_class(types::klass const& klass, shared_ptr<string> path, lexer::position const& position, unordered_map<ast::name, values::value> const* arguments)
    {
        // Attempt to find the existing resource
        types::resource resource_type("class", klass.title());
        runtime::resource* resource = _catalog.find_resource(resource_type);
        if (resource) {
            return resource;
        }

        // Lookup the class
        auto it = _classes.find(klass);
        if (it == _classes.end() || it->second.empty()) {
            // TODO: search node for class
            return nullptr;
        }

        // Declare the class in the catalog
        resource = _catalog.add_resource(resource_type, rvalue_cast(path), position.line(), false);
        if (!resource) {
            return nullptr;
        }

        // Evaluate all definitions of the class
        for (auto& definition : it->second) {
            if (!definition.evaluate(*this, arguments)) {
                return nullptr;
            }
        }
        return resource;
    }

    bool context::is_class_defined(types::klass const& klass) const
    {
        auto it = _classes.find(klass);
        return it != _classes.end() && !it->second.empty();
    }

    bool context::is_class_declared(types::klass const& klass) const
    {
        auto it = _classes.find(klass);
        if (it == _classes.end()) {
            return false;
        }

        for (auto& definition : it->second) {
            if (definition.evaluated()) {
                return true;
            }
        }
        return false;
    }

    void context::validate_parameters(bool klass, std::vector<ast::parameter> const& parameters)
    {
        for (auto const& parameter : parameters) {
            auto const& name = parameter.variable().name();
            if (name == "title" || name == "name") {
                throw evaluation_exception(parameter.variable().position(), (boost::format("parameter $%1% is reserved and cannot be used.") % name).str());
            }
            if (parameter.captures()) {
                throw evaluation_exception(parameter.variable().position(), (boost::format("%1% parameter $%2% cannot \"captures rest\".") % (klass ? "class" : "defined type") % name).str());
            }
        }
    }

    local_scope::local_scope(runtime::context& context, runtime::scope* scope) :
        _context(context),
        _scope(&context.scope())
    {
        if (scope) {
            _context.push_scope(*scope);
        } else {
            _context.push_scope(_scope);
        }
    }

    local_scope::~local_scope()
    {
        _context.pop_scope();
    }

}}  // namespace puppet::runtime