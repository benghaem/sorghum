#pragma once

#include <stack>
#include <vector>
#include <ostream>
#include <array>

#include "sorghum/synth.h"


enum class CGAInst{
    pull_N,
    pull_W,
    send_S_pop,
    send_S_peek,
    send_S_ifz_peek,
    add,
    sub,
    mul,
    pop,
    push,
    peek,
    mac,
    zero_push,
    zero_reg,
    nop,
    inc,
    dec,
    undef,
};

const unsigned int CGAInst_NUM = 17;

CGAInst int_to_CGAInst(unsigned int v);

std::ostream& operator<<(std::ostream& out, const CGAInst inst);


enum class CGAIterMode{
    north_len,
    west_len,
    pe_depth,
    pe_depth_inv,
    single,
    undef,
};


const unsigned int CGAIterMode_NUM = 5;

CGAIterMode int_to_CGAIterMode(unsigned int v);

std::ostream& operator<<(std::ostream& out, const CGAIterMode imode);

struct CGAProg {
    std::vector<CGAIterMode> iteration_mode;
    std::vector<std::vector<CGAInst>> stages;
};

struct ProgCursor {
    unsigned int stage;
    unsigned int iidx;
};

class CGAVirt {

    public:
        CGAVirt();
        ~CGAVirt();


        bool check(std::vector<int>& north_input,
                   std::vector<std::vector<int>>& west_inputs,
                   std::vector<int>& south_output,
                   std::vector<std::vector<CGAInst>>& progs);

        bool eval(TestCase& tc,
                  CGAProg& prog,
                  std::vector<int>& output);


        //configuration
        unsigned int stack_limit = 10;
        unsigned int cga_height = 1;

        //statistics
        unsigned int stat_max_stack = 0;
        unsigned int stat_instr_run = 0;

        void reset_stats();

        bool debug = false;

    private:
        void reset_regs();

        std::vector<int> hw_stack;
        int hw_reg;
};
