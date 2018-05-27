# wfn\_eng

C++ SDL-based mini game engine to learn Vulkan.

### Core

Note on singleton access to Core:

* All members of the Vuklan module (under the namespace `wfn_eng::vulkan`) MUST
  receive explicit reference to any Vulkan resources or wrapper classes.
* Members of the primary namespace (in `main.cpp`) or the engine
  (in `wfn_eng::engine`) may instead access the Core instance via
  `Core::instance`, and do not require explicit reference.
