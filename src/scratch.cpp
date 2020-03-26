#include <iostream>
#include <vector>

#include "sorghum/vm.h"
#include "sorghum/util.h"


int main(){
    CGAVirt testvm;

    std::vector<int> input_N = {0,2,8,7};
    std::vector<int> input_W_0 = {2,0,4,3};
    //std::vector<int> input_W_1 = {1,0,1,0};

    std::vector<std::vector<int>> inputs_W;

    //setup the west input
    inputs_W.emplace_back(std::move(input_W_0));
    //inputs_W.emplace_back(std::move(input_W_1));
    input_W_0.clear();
    //input_W_1.clear();

    std::vector<int> output;
    /*
    std::vector<CGAInst> el_product_prog = {
        CGAInst::pull_W,
        CGAInst::pull_N,
        CGAInst::pop,
        CGAInst::mul,
        CGAInst::send_S,
        CGAInst::pull_W,
        CGAInst::pull_N,
        CGAInst::pop,
        CGAInst::mul,
        CGAInst::send_S,
        CGAInst::pull_W,
        CGAInst::pull_N,
        CGAInst::pop,
        CGAInst::mul,
        CGAInst::send_S,
        CGAInst::pull_W,
        CGAInst::pull_N,
        CGAInst::pop,
        CGAInst::mul,
        CGAInst::send_S,
    };

    std::vector<CGAInst> broadcast_product_prog = {
        // setup
        CGAInst::pull_W,
        CGAInst::pop,
        // e wise
        CGAInst::pull_N,
        CGAInst::mul,
        CGAInst::send_S,
        CGAInst::pull_N,
        CGAInst::mul,
        CGAInst::send_S,
        CGAInst::pull_N,
        CGAInst::mul,
        CGAInst::send_S,
        CGAInst::pull_N,
        CGAInst::mul,
        CGAInst::send_S,
    };

    std::vector<CGAInst> dotprod_nomac_prog = {
        CGAInst::zero,
        //
        CGAInst::pull_N,
        CGAInst::pull_W,
        CGAInst::pop,
        CGAInst::mul,
        CGAInst::pop,
        CGAInst::add,
        //
        CGAInst::send_S
    };*/

    // this needs to run once, but potentially input elements times
    std::vector<CGAInst> dotprod_mac_OSDF_prog_setup = {
        //CGAInst::zero_reg
    };

    //this needs to run input elements times
    std::vector<CGAInst> dotprod_mac_OSDF_prog_comp = {
        CGAInst::pull_W,
        CGAInst::pull_N,
        CGAInst::send_S_peek,
        CGAInst::mac,
        CGAInst::pop //accumulation is in reg
    };

    //this needs to run output elements times
    std::vector<CGAInst> dotprod_mac_OSDF_prog_cleanup = {
        CGAInst::push,
        CGAInst::send_S_pop,
        CGAInst::pull_N,
        CGAInst::pop
    };

    std::vector<std::vector<CGAInst>> dotprod_mac_OSDF_prog = {
        dotprod_mac_OSDF_prog_setup,
        dotprod_mac_OSDF_prog_comp,
        dotprod_mac_OSDF_prog_cleanup
    };

    std::vector<CGAInst> dotprod_mac_WSDF_prog_setup = {
    };

    std::vector<CGAInst> dotprod_mac_WSDF_prog_comp = {
    };

    std::vector<CGAInst> dotprod_mac_WSDF_prog_cleanup = {
    };

    std::vector<std::vector<CGAInst>> dotprod_mac_WSDF_prog = {
        dotprod_mac_WSDF_prog_setup,
        dotprod_mac_WSDF_prog_comp,
        dotprod_mac_WSDF_prog_cleanup
    };


    testvm.west_input_len = 1;
    testvm.north_input_len = 1;
    testvm.south_output_len = 1;

    testvm.cga_height = 1;

    testvm.eval(input_N,
                inputs_W,
                output,
                dotprod_mac_OSDF_prog);

    std::cout << "MX Stack size: " << testvm.max_stack_size << std::endl;
    dbg_print_vec(output);

 }
