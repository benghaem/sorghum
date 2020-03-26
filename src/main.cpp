#include <vector>
#include <iostream>
#include <cassert>

#include "sorghum/esynth.h"
#include "sorghum/vm.h"
#include "sorghum/util.h"


int demo_dot_prod(){

    std::vector<int> input_N = {0,2,8,7};
    std::vector<int> input_W_0 = {2,0,4,3};
    std::vector<int> input_W_1= {0,1,0,1};
    std::vector<int> output = {7,8,2,0,9,53,0,0};

    std::vector<std::vector<int>> inputs_W;

    //setup the west input
    inputs_W.emplace_back(std::move(input_W_0));
    inputs_W.emplace_back(std::move(input_W_1));
    input_W_0.clear();
    input_W_1.clear();

    //program output
    std::vector<std::vector<CGAInst>> progs_out;

    bool success = esynth(800000000,
           input_N,
           inputs_W,
           output,
           progs_out);

    if (success){
        std::cout << "found program!" << std::endl;
        int s = 0;
        for (auto & prog : progs_out){
            std::cout << "stage: " << s << std::endl;
            for (auto instr : prog){
                std::cout << instr << std::endl;
            }
            s++;
        }
    }

    std::vector<int> val_output;
    std::vector<std::vector<int>> inputs_W_copy = inputs_W;
    CGAVirt vm;
    vm.debug = true;
    vm.cga_height = inputs_W_copy.size();
    bool ret = vm.eval(input_N,
            inputs_W_copy,
            val_output,
            progs_out);

    std::cout << ret << std::endl;
    dbg_print_vec(val_output);

    return 1;
}

int demo_broadcast(){

    std::vector<int> input_N = {2,4,6,8};
    std::vector<int> input_W_0 = {1,2,3,4};
    std::vector<int> output = {2,8,18,24};

    std::vector<std::vector<int>> inputs_W;

    //setup the west input
    inputs_W.emplace_back(std::move(input_W_0));
    input_W_0.clear();

    //program output
    std::vector<std::vector<CGAInst>> progs_out;

    bool success = esynth(800000000,
           input_N,
           inputs_W,
           output,
           progs_out);

    if (success){
        std::cout << "found program!" << std::endl;
        int s = 0;
        for (auto & prog : progs_out){
            std::cout << "stage: " << s << std::endl;
            for (auto instr : prog){
                std::cout << instr << std::endl;
            }
            s++;
        }
    }

    std::vector<int> val_output;
    std::vector<std::vector<int>> inputs_W_copy = inputs_W;
    CGAVirt vm;
    vm.debug = true;
    vm.cga_height = inputs_W_copy.size();
    bool ret = vm.eval(input_N,
            inputs_W_copy,
            val_output,
            progs_out);

    std::cout << ret << std::endl;
    dbg_print_vec(val_output);

    return 1;
}

int main(){
    demo_dot_prod();
}
