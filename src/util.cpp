
#include <iostream>
#include <vector>
#include <deque>
#include <stack>

#include "sorghum/util.h"

void dbg_print_vec(const std::vector<int>& vec){
    std::cout << "v<";
    for (auto & v : vec){
        std::cout << v << ",";
    }
    std::cout << ">" << std::endl;
}

void dbg_print_deque(const std::deque<int>& queue){

    std::deque<int> copy = queue;

    std::cout << "dq<";
    while (!copy.empty()){
        std::cout << copy.front() << ",";
        copy.pop_front();
    }
    std::cout << ">" << std::endl;
}

void dbg_print_stack(const std::vector<int>& stack){
    std::cout << "s<";
    for(auto it = stack.crbegin(); it != stack.crend(); it++){
        std::cout << *it << ",";
    }
    std::cout << ">" << std::endl;
}

void dbg_print_progs(const std::vector<std::vector<CGAInst>>& progs){
        int s = 0;
        for (auto& pstage : progs){
            std::cout << "stage: " << s << std::endl;
            for (auto inst : pstage){
                std::cout << inst.op << ",";
            }
            std::cout << std::endl;
            s++;
        }
}

void dbg_print_prog(const CGAProg& prog){
    int s = 0;
    for (auto& pstage : prog.stages){
        std::cout << "stage: " << s << std::endl;
        std::cout << "imode: " << prog.iteration_mode[s] << std::endl;
        for (auto inst : pstage){
            std::cout <<inst << ",";
        }
        std::cout << std::endl;
        s++;
    }
}

void dbg_print_cfg1d(CGAConfig1D& cfg1d){

    std::cout << "PE Assignment" << std::endl;
    for (unsigned int i = 0; i < cfg1d.cga_height; i++){
        unsigned int prog = cfg1d.get_assigned_prog_id(i);
        int stream_id = cfg1d.get_assigned_stream_id(i);
        std::cout << stream_id << "--> pe" <<  i << "-->" << prog << std::endl;
    }

    for(unsigned int i = 0; i < cfg1d.num_progs; i++){
        std::cout << "!!pe_prog: " << i << std::endl;
        dbg_print_prog(cfg1d.get_prog(i));
    };

}
