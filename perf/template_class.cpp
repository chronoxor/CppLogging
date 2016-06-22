//
// Created by Ivan Shynkarenka on 26.05.2016.
//

#include "cppbenchmark.h"

#include "template/class.h"

BENCHMARK("Template class", 10000000)
{
    CppTemplate::Template instance(10);

    instance.Method(100);
    instance.StaticMethod(1000);
}

BENCHMARK_MAIN()
