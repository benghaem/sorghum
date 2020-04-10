#include <vector>
#include <iostream>
#include <cassert>

#include "sorghum/esynth.h"
#include "sorghum/vm.h"
#include "sorghum/util.h"
#include "sorghum/mcmc_synth.h"
#include "sorghum/para.h"

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

    TestCase t0_dotprod({8,5,2,1},{{12,16,99,2},{7,0,1,0}},{376, 58});
    TestCase t1_dotprod({4,4,12},{{0,1,1},{1,1,0}},{16,8});
    TestCase t2_dotprod({1,2,3},{{0,1,1},{1,0,0},{1,1,1}},{1,2,3,5,1,6});
    std::vector<TestCase> tcs_dotprod = {t0_dotprod, t1_dotprod, t2_dotprod};

    std::vector<TestCase>& sel_tcs = tcs_dotprod;

    MCMCProposalDist pdist;
    pdist.p_swap = 0.34;
    pdist.p_insert = 0.34;
    pdist.p_remove = 0.30;
    pdist.p_replace = 0.0;
    pdist.p_inc_stage = 0.01;
    pdist.p_dec_stage = 0.01;

    std::random_device rd;
    unsigned int seed = rd();

    int vm_num_regs = 1;

    MCMCSynth ms0(pdist,
                  3, //2 max stages
                  1, //1 pe type
                  10, //max 7 inst per stage
                  vm_num_regs,
                  sel_tcs,
                  seed);

    ms0.init();

    std::vector<CGAProg> valid_canidates;
    float best_canidate_cost = 0;

    for (int i = 0; i < 1000000000; i++){
        ms0.gen_next_canidate();
        if (ms0.get_canidate_valid()){
            float ms0_cc = ms0.get_canidate_cost();
            std::cout << "! CANIDATE FOUND ! @ " << i << " c" << ms0_cc << std::endl;

            CGAProg canidate = ms0.get_canidate();
            std::vector<int> output;
            CGAVirt vm(vm_num_regs);
            for (int i = 0; i < 2; i++){
                vm.eval(sel_tcs[i],
                        canidate,
                        output);
                dbg_print_prog(canidate);
                dbg_print_vec(output);
                output.clear();
            }
            if (ms0_cc > best_canidate_cost){
                best_canidate_cost = ms0_cc;
                valid_canidates.push_back(canidate);
            }
        }
        if (i % 1000000 == 0){
            std::cout << i << ", " << ms0.get_canidate_cost() << std::endl;
            CGAProg canidate = ms0.get_canidate();
            dbg_print_prog(canidate);
        }

        if (valid_canidates.size() == 2 || best_canidate_cost >= 0.92){
            break;
        }
    }
    
    int i = 0;
    std::cout << "top 3 programs: " << std::endl;
    for (auto itt = valid_canidates.crbegin(); itt != valid_canidates.crend(); itt++){
        std::cout << "prog #" << i + 1 << std::endl;
        dbg_print_prog(*itt);
        if (i == 2){
            break;
        }
        i++;
    }
}

void demo_para_mcmc_synth(){
    TestCase t0_ewise_mul({0,1,2,3},{{0,1,2,3}},{0,1,4,9});
    TestCase t1_ewise_mul({2,3},{{4,8}},{8,24});

    std::vector<TestCase> tcs_ewise_mul = {t0_ewise_mul, t1_ewise_mul};

    TestCase t0_dotprod({8,5,2,1},{{12,16,99,2},{7,0,1,0}},{376, 58});
    TestCase t1_dotprod({4,4,12},{{0,1,1},{1,1,0}},{16,8});
    TestCase t2_dotprod({1,2,3},{{0,1,1},{1,0,0},{1,1,1}},{1,6});
    std::vector<TestCase> tcs_dotprod = {t0_dotprod, t1_dotprod, t2_dotprod};

    TestCase t0_dp_ws({32,-12},{{3,6},{17,2}},{508,-8});
    TestCase t1_dp_ws({10,17},{{4,3,63},{8,7,-22}},{148,121,851});

    int n_a = 17;
    int n_b = 93;
    int w_0a = 3;
    int w_1a = -12; 
    int o_a = n_a*w_0a;
    int o_b = n_b*w_1a;
    TestCase t2_dp_ws({n_b,n_a},{{w_1a},{w_0a}},{o_a,o_b});
    std::vector<TestCase> tcs_dp_ws = {t0_dp_ws};

    TestCase t0_sum_w({},{{1,8,22,14,7},{12,-5}},{52,7});
    TestCase t1_sum_w({},{{-412,66,91},{-929,828,71}},{-255,-30});
    std::vector<TestCase> tcs_sum_w = {t0_sum_w, t1_sum_w};

    TestCase t0_third({},{{1,2,3,4,5,6,7},{8,9,10,11},{12,13,14,15,16}},{3,10,14});
    std::vector<TestCase> tcs_third= {t0_third};

    std::vector<TestCase>& sel_tcs = tcs_dp_ws;

    MCMCProposalDist pdist;
    pdist.p_swap = 0.24;
    pdist.p_insert = 0.34;
    pdist.p_remove = 0.30;
    pdist.p_replace = 0.10;
    pdist.p_inc_stage = 0.02;
    pdist.p_dec_stage = 0.02;
    pdist.p_change_imode = 0.001;

    ParaMCMC pmcmc(14, //threads
                   pdist, //dist
                   2, //max stages
                   1, //max types
                   10, //max instr per stage
                   4, //num regs
                   sel_tcs);

    pmcmc.run(6);

}

int main(){
    demo_para_mcmc_synth();
}
