#pragma once

#include <stack>
#include <vector>
#include <ostream>

enum class CGAInst{
    pull_N,
    pull_W,
    send_S_pop,
    send_S_peek,
    add,
    mul,
    pop,
    push,
    peek,
    mac,
    zero_push,
    zero_reg,
    nop,
};


std::ostream& operator<<(std::ostream& out, const CGAInst inst);



class CGAVirt {

    public:
        CGAVirt();
        ~CGAVirt();

        void reset();

        int stack_size();
        unsigned int max_stack_size = 0;
        int set_stack_limit();

        unsigned int cga_height = 1;

        bool check(std::vector<int>& north_input,
                   std::vector<std::vector<int>>& west_inputs,
                   std::vector<int>& south_output,
                   std::vector<std::vector<CGAInst>>& progs);

        bool eval(std::vector<int>& north_input,
                   std::vector<std::vector<int>>& west_inputs,
                   std::vector<int>& south_output,
                   std::vector<std::vector<CGAInst>>& progs);

        int stack_limit = 10;

        unsigned int north_input_len = 1;
        unsigned int south_output_len = 1;
        unsigned int west_input_len = 1;

        bool debug = false;

    private:

        std::vector<int> hw_stack;
        int hw_reg;
};
