#pragma once

#include <iostream>
#include <vector>
#include <deque>
#include <stack>

#include "sorghum/vm.h"

void dbg_print_vec(std::vector<int>& vec);

void dbg_print_deque(std::deque<int>& queue);

void dbg_print_stack(std::vector<int>& stack);

void dbg_print_progs(std::vector<std::vector<CGAInst>>& progs);
