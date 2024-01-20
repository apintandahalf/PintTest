# PintTest
A header only unit test library for C++.  Not at all fully featured, but very easy to include in your project.

Put the single header in your project, or somewhere where you project can find it, and add some tests. 

Call PintTest::runAllTests to execute all the test cases. 

See the header file for supported expects and asserts.

Currently developed with Visual Studio 2022

    Future Plans:

    Test suites (groups of tests with a common name prefix and common setup/tear down)

    Filtering to allowing running a subset of tests (matching on a part of the test name)

    (Optional) order randomisation

    Exception handling

    Test with gcc and clang
