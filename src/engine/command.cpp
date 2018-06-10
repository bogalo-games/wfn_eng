#include "command.hpp"

namespace wfn_eng::engine {
    ////
    // class FunctionCommand
    //
    // A particular kind of Command that executes an arbitrary function.

    ////
    // FunctionCommand(std::function<void()>)
    //
    // Constructs a FunctionCommand by assigning the internal function to
    // the provided function.
    FunctionCommand::FunctionCommand(std::function<void()> function) :
            function(function) { }

    ////
    // void execute()
    //
    // Calls std::function<void()> function.
    void FunctionCommand::execute() {
        function();
    }
}
