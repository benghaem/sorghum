#include "sorghum/synth.h"

TestCase::TestCase(std::initializer_list<int> north,
             std::initializer_list<std::initializer_list<int>> west,
             std::initializer_list<int> south) :
    north_input(north),
    south_output(south){

        //Copy all of the west inputs into the west inputs vector
        for (auto& west_input : west){
            west_inputs.push_back(west_input);
        }

    };
