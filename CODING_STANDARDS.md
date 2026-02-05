# Coding Standards

## Code Style

By default, follow the [C++ Core Guidelines](https://github.com/isocpp/CppCoreGuidelines).

## Naming

### Filenames

* Name class files in `PascalCase.cpp`
* Name utility files in `snake_case.cpp`
* Use the following extensions:
  * C++ source files: `.cpp`
  * C++ header files: `.hpp`
  * C source files: `.c`
  * C header files: `.h`

### Namespaces

* Name namespaces in `snake_case`.
* Match the top-level namespaces with the module name, if any.

### Types

* Name all types (classes, structs, typedefs, enums) in `PascalCase`.

### Methods

* Name both free and member methods in `camelCase`.
* Try to avoid getters and setters. If you have a container class and want a method to get its size, call it `size()`, not `getSize()`.
  * If you can modify the size as well, overloading will help you: `size()` and `size(std::size_t)`.

### Variables

* Name all variables in `camelCase`. Constants, too.
* Use descriptive names that are not too long.
* Class member variables may have a suffix underscore `_`.
* Loop indices can be `i`, `j`, `k`, if their meaning is otherwise irrelevant.

### Macros

* Macros are all-caps `SNAKE_CASE`.
  * Note: this does NOT apply to constant variables.
