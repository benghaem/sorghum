#pragma once
#include <vector>

struct TestCase {

    TestCase(std::initializer_list<int> north,
             std::initializer_list<std::initializer_list<int>> west,
             std::initializer_list<int> south);

    std::vector<int> north_input;
    std::vector< std::vector<int> > west_inputs;
    std::vector<int> south_output;
};
