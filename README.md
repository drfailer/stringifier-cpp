# C++ serializer generation

The purpose of this project is to automate serialization of C++ classes. In
order for a class to be serializable, it must inherit from the `Serializable`
class. It is a template class where template parameters are the types of the
attributes that have to be serialized. As shown in the following example, we
want to serialize the `Test` class that is composed by two integers `x` and `y`.
In the example, the `Serializable` template is of type `<int, int>`.

To generate the code properly, the `serializable` macro must be used in the
constructor parameter list. This macro will help to generate identifiers of the
attributes. For example, `serializable(x, y)` will allow the serialization of
the attributes `x` and `y` with the identifiers `"x"` and `"y"`. If `this->x`
is used instead of `x`, the identifier will be `"this->x"`. Notice that the
deserialization will work anyway.

Working with composed classes will work fine as long as all the subclasses are
serializable.

## Example

```cpp
#include "serializable.hpp"

class Test : public Serializable<int, int> {
public:
    Test(int _x = 0, int _y = 0) : serializable(x, y), x(_x), y(_y) { }
    Test(const Test& other): Test(other.x, other.y) { }
    ~Test() = default;

private:
    int x;
    int y;
};

int main(int argc, char **argv) {
    Test test1(1, 2);
    Test test2();

    // serialize a test
    std::string test1Str = test1.serialize();

    // we can extract the information from the string
    test2.deserialization(test1Str);

    return 0;
}
```
