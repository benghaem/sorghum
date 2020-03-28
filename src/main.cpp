#include <vector>
#include <iostream>
#include <cassert>

#include "sorghum/esynth.h"
#include "sorghum/vm.h"
#include "sorghum/util.h"
#include "sorghum/mcmc_synth.h"

/*
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
*/

void demo_mcmc_synth(){
    TestCase t0_ewise_mul({0,1,2,3},{{0,1,2,3}},{0,1,4,9});
    TestCase t1_ewise_mul({2,3},{{4,8}},{8,24});

    std::vector<TestCase> tcs_ewise_mul = {t0_ewise_mul, t1_ewise_mul};

    TestCase t0_dotprod({0,1,0,1},{{8,5,2,6}},{11});
    TestCase t1_dotprod({4,4,12},{{0,1,1},{1,0,0}},{16,4});
    std::vector<TestCase> tcs_dotprod = {t0_dotprod, t1_dotprod};

    MCMCProposalDist pdist;
    pdist.p_swap = 0.34;
    pdist.p_insert = 0.33;
    pdist.p_remove = 0.33;
    pdist.p_replace = 0.0;
    pdist.p_inc_stage = 0.0;
    pdist.p_dec_stage = 0.0;

    std::random_device rd;
    unsigned int seed = rd();

    MCMCSynth ms0(pdist,
                  1,
                  1,
                  tcs_ewise_mul,
                  seed);

    ms0.init();

    std::vector<CGAProg> valid_canidates;

    for (int i = 0; i < 1000000000; i++){
        ms0.gen_next_canidate();
        if (ms0.get_canidate_valid()){
            std::cout << "! CANIDATE FOUND ! @ " << i << " c" << ms0.get_canidate_cost() << std::endl;
            CGAProg canidate = ms0.get_canidate();
            dbg_print_prog(canidate);
            valid_canidates.push_back(canidate);
        }
        if (i % 1000000 == 0){
            std::cout << i << ", " << ms0.get_canidate_cost() << std::endl;
            CGAProg canidate = ms0.get_canidate();
            dbg_print_prog(canidate);
        }

        if (valid_canidates.size() == 10){
            break;
        }
    }

    for (CGAProg& vcan : valid_canidates){
        dbg_print_prog(vcan);
    }
}

int main(){
    demo_mcmc_synth();
}
