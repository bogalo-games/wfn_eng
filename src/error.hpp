#ifndef __WFN_ENG_ERROR_HPP__
#define __WFN_ENG_ERROR_HPP__

#include <stdexcept>
#include <string>

namespace wfn_eng {
    ////
    // WfnError
    //
    // An error used specially for the WFN engine.
    class WfnError : std::exception {
        std::string _module;
        std::string _method;
        std::string _action;

        std::string _message;

    public:
        ////
        // WfnError
        //
        // Constructs a WfnError with the following information (in order of
        // argument list):
        //   - Module    (namespaced module or class name)
        //   - Method    (the name of the method that threw an exception)
        //   - Action    (the section that failed)
        WfnError(std::string, std::string, std::string);

        ////
        // std::string module
        //
        // Provides a copy of the module string.
        std::string module() const;

        ////
        // std::string method
        //
        // Provides a copy of the method string.
        std::string method() const;

        ////
        // std::string action
        //
        // Provides a copy of the action string
        std::string action() const;

        ////
        // const char *what()
        //
        // Provides a full description of the WfnError (via concatenation of the
        // module, method, and action)
        virtual const char *what() const noexcept;
    };
}

#endif
