
#include <iostream>
#include <vector>
#include <deque>
#include <stack>

#include "sorghum/util.h"

void dbg_print_vec(std::vector<int>& vec){
    std::cout << "v<";
    for (auto & v : vec){
        std::cout << v << ",";
    }
    std::cout << ">" << std::endl;
}

void dbg_print_deque(std::deque<int>& queue){

    std::deque<int> copy = queue;

    std::cout << "dq<";
    while (!copy.empty()){
        std::cout << copy.front() << ",";
        copy.pop_front();
    }
    std::cout << ">" << std::endl;
}

void dbg_print_stack(std::vector<int>& stack){
    std::cout << "s<";
    for(auto it = stack.crbegin(); it != stack.crend(); it++){
        std::cout << *it << ",";
    }
    std::cout << ">" << std::endl;
}

void dbg_print_progs(std::vector<std::vector<CGAInst>>& progs){
        int s = 0;
        for (auto& pstage : progs){
            std::cout << "stage: " << s << std::endl;
            for (auto inst : pstage){
                std::cout << inst << ",";
            }
            std::cout << std::endl;
            s++;
        }
}
