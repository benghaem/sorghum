#pragma once

#include <iostream>
#include <vector>
#include <deque>
#include <stack>

#include "sorghum/vm.h"

void dbg_print_vec(const std::vector<int>& vec);

void dbg_print_deque(const std::deque<int>& queue);

void dbg_print_stack(const std::vector<int>& stack);

void dbg_print_progs(const std::vector<std::vector<CGAInst>>& progs);

void dbg_print_prog(const CGAProg& prog);

void dbg_print_cfg1d(CGAConfig1D& cfg1d);
