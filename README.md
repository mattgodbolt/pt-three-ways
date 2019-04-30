# Path tracing, done three ways

Some code sketchings for an idea I have to do a presentation on the effects of different
coding styles on design and performance. It's a trivial path tracer (an extended homage to smallpt.cpp)
implemented three different ways.

Tentative "ways" of storing the scene:

1. Traditional OO with interfaces and virtual calls
2. Functional style with std::variant
3. `constexpr` all the things (maybe), or pure C ish?
