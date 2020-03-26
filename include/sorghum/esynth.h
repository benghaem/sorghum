#pragma once

#include <vector>
#include "sorghum/vm.h"

bool esynth(int max_attempts,
            std::vector<int>& north_input,
            std::vector<std::vector<int>>& west_inputs,
            std::vector<int>& south_output,
            std::vector<std::vector<CGAInst>>& synth_program);

bool increment(std::vector<int>::iterator begin,
               std::vector<int>::iterator end,
               int max_count);
