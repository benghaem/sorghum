#include <iostream>
#include <vector>

#include "sorghum/vm.h"
#include "sorghum/util.h"


int main(){
    //single reg version
    CGAVirt testvm(1);


    TestCase tc0({0,2,8,7},{{2,0,4,3}},{});
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
        CGAInst(CGAOp::pull_W,{}),
        CGAInst(CGAOp::pull_N,{}),
        CGAInst(CGAOp::send_S_peek,{}),
        CGAInst(CGAOp::mac,{}),
        CGAInst(CGAOp::pop,{}), //accumulation is in reg
    };

    //this needs to run output elements times
    std::vector<CGAInst> dotprod_mac_OSDF_prog_cleanup = {
        CGAInst(CGAOp::push,{}),
        CGAInst(CGAOp::send_S_pop,{}),
        CGAInst(CGAOp::pull_N,{}),
        CGAInst(CGAOp::pop,{}),
    };


    CGAProg dotprod_mac_OSDF_prog;
    dotprod_mac_OSDF_prog.stages.push_back(dotprod_mac_OSDF_prog_setup);
    dotprod_mac_OSDF_prog.stages.push_back(dotprod_mac_OSDF_prog_comp);
    dotprod_mac_OSDF_prog.stages.push_back(dotprod_mac_OSDF_prog_cleanup);

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


    testvm.cga_height = 1;


    testvm.eval(tc0,
                dotprod_mac_OSDF_prog,
                output);

    std::cout << "MX Stack size: " << testvm.stat_max_stack << std::endl;
    dbg_print_vec(output);

 }
