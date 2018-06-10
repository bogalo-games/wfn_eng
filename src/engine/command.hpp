#ifndef __WFN_ENG_ENGINE_COMMAND_HPP__
#define __WFN_ENG_ENGINE_COMMAND_HPP__

#include <functional>

namespace wfn_eng::engine {
    ////
    // struct Command
    //
    // An object that encapsulates some code.
    struct Command {
        virtual ~Command() { }
        virtual void execute() = delete;
    };

    ////
    // class FunctionCommand
    //
    // A particular kind of Command that executes an arbitrary function.
    class FunctionCommand {
        std::function<void()> function;

    public:
        ////
        // FunctionCommand(std::function<void()>)
        //
        // Constructs a FunctionCommand by assigning the internal function to
        // the provided function.
        FunctionCommand(std::function<void()>);

        ////
        // void execute()
        //
        // Calls std::function<void()> function.
        virtual void execute();
    };
}

#endif
