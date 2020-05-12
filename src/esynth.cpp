#include <iostream>
#include "sorghum/vm.h"
#include "sorghum/esynth.h"
#include "sorghum/util.h"


bool esynth(int max_attempts,
            std::vector<int>& north_input,
            std::vector<std::vector<int>>& west_inputs,
            std::vector<int>& south_output,
            std::vector<std::vector<CGAInst>>& synth_program){


    //only 1 register because we currently don't support generating args here
    CGAVirt vm(1);
    CGAConfig1D cfg(west_inputs.size(), 1, west_inputs.size());


    std::vector<CGAOp> allowed_instr = {
        CGAOp::pull_N, //0
        CGAOp::pull_W, //1
        CGAOp::send_S_pop, //2
        CGAOp::send_S_peek, //3
        CGAOp::push, //4
        CGAOp::zero_reg, //5
        CGAOp::add, //unsafe
        CGAOp::pop,
        CGAOp::peek,
        CGAOp::mul,
        CGAOp::mac,
    };

    std::vector<int> stack_value = {
        1,
        1,
        -1, //optimization for determining if a program is safe quickly
        0,
        1
    };

    int max_safe_instr_id = 5;


    int attempts = 0;
    bool satisfied = false;

    //setup program with single stage
    std::vector<int> int_prog;
    //int_progs.emplace_back(std::vector<int>());
    //int_progs[0].push_back(0);

    std::vector<std::vector<CGAInst>> progs;
    progs.emplace_back(std::vector<CGAInst>());
    progs.emplace_back(std::vector<CGAInst>());

    size_t max_len = 10;

    while(attempts < max_attempts && !satisfied){

        bool rolled_over = increment(int_prog.begin(), int_prog.end(), allowed_instr.size() - 1);
        if (rolled_over){
            //when we roll over increase program size up to max len
            if (int_prog.size() < max_len){
                int_prog.push_back(0);
            } else {
                break;
            }
        }

        //skip unsafe instructions
        if (int_prog[0] > max_safe_instr_id){
            attempts++;
            continue;
        }

        for (size_t i = 0; i < int_prog.size(); i++){
            progs[0].clear();
            progs[1].clear();

            size_t int_prog_idx = 0;
            //TODO: Enumerative synth does not support arguments
            for (int v : int_prog){
                if (int_prog_idx <= i){
                    progs[0].push_back(CGAInst(allowed_instr[v], {}));
                } else {
                    progs[1].push_back(CGAInst(allowed_instr[v], {}));
                }
                int_prog_idx++;
            }

            if (vm.check(north_input,
                         west_inputs,
                         south_output,
                         progs)){
                satisfied = true;
                std::cout << "found on attempt: " << attempts << std::endl;
                dbg_print_progs(progs);
                break;
            }
        }
        

    if (attempts % 1000000 == 0){
        std::cout << attempts << "/" << max_attempts << ", sz: " << int_prog.size() << std::endl;
        std::cout << "current program" << std::endl;

        dbg_print_progs(progs);
    }

        attempts++;
    }

    if (satisfied){

        synth_program.swap(progs);
        return true;
    }
    return false;

}

bool increment(std::vector<int>::iterator begin,
               std::vector<int>::iterator end,
               int max_count){

    //base case is an empty slice
    if (begin == end){
        return true;
    }

    std::vector<int>::iterator before_end = std::prev(end);

    //increment the value at the end
    *before_end += 1;
    if (*before_end > max_count){
        *before_end = 0;
        return increment(begin, before_end, max_count);
    }
    return false;
}


