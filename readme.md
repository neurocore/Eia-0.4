# Eia v0.4
Chess engine. **Development has been suspended due to wrong design approach**

# What actually was wrong in design?
- Foremost it has been discovered that heavy precalculations in every node (attacks and some position features) expectedly decreases tactical seeing and, what is important, **not bringing alpha or beta cutoff more often** and therefore not increasing playing strength
- Project still contains parts of old code that was taken almost in its original form. In particular it is the most of macrofuctions that can be converted into inline functions easily. Control over execution flow is essential
- For better readability and more convenience of making changes it was decided to stick more to OOP paradigm wherever it doesn't hurt performance much

Work on engine Eia will be continued in the [version 0.5](https://example.com)

# Features
- Bitboards, magics
- Copy-make
