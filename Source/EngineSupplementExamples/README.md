This directory provides examples for each expected file in the project's `EngineSupplement` directory.

The files in this directory are not built or used in any way.

Example README.md for the base of the EngineSupplement directory:
```
In this directory, the project provides files that will be built as part of the engine.

We use this mechanism for a few things, such as:
* Enum definitions
  * There's no good alternative for providing project enums to the engine.
* Config headers
  * For any configuration constants that we don't want to expose to the end user.
* Type lists and Components
  * We tested an alternative using type erasure, but it performed significantly worse than providing type definitions to the engine and using a variant.

**Note: Since these files are built as part of the engine, if they depend on
        a project file, that file must also be made visible to the relevant 
        engine CMake target.**
```
