#include "error.hpp"

#include <sstream>

namespace wfn_eng {
    ////
    // WfnError
    //
    // An error used specially for the WFN engine.

    ////
    // WfnError
    //
    // Constructs a WfnError with the following information (in order of
    // argument list):
    //   - Module    (namespaced module or class name)
    //   - Method    (the name of the method that threw an exception)
    //   - Action    (the section that failed)
    WfnError::WfnError(std::string module, std::string method, std::string action) :
            _module(module),
            _method(method),
            _action(action) {
        std::stringstream builder;
        builder << this->module() << " - " << this->method() << " - " << this->action();
        _message = builder.str();
    }

    ////
    // std::string module
    //
    // Provides a copy of the module string.
    std::string WfnError::module() const { return _module; }

    ////
    // std::string method
    //
    // Provides a copy of the method string.
    std::string WfnError::method() const { return _method; }

    ////
    // std::string action
    //
    // Provides a copy of the action string
    std::string WfnError::action() const { return _action; }

    ////
    // const char *what()
    //
    // Provides a full description of the WfnError (via concatenation of the
    // module, method, and action)
    const char *WfnError::what() const noexcept {
        return _message.c_str();
    }
}
