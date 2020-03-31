#include <deque>
#include <iostream>

#include "sorghum/vm.h"
#include "sorghum/util.h"

CGAVirt::CGAVirt(){
}

CGAVirt::~CGAVirt(){
}

void CGAVirt::reset_regs(){
    //clear all internal registers
    hw_stack.clear();
    hw_reg = 0;
}

void CGAVirt::reset_stats(){
    stat_max_stack = 0;
    stat_instr_run = 0;
}

bool CGAVirt::check(std::vector<int>& north_input,
                    std::vector<std::vector<int>>& west_inputs,
                    std::vector<int>& south_output,
                    std::vector<std::vector<CGAInst>>& progs){



    if (west_inputs.size() != cga_height){
        //std::cout << "bad number of W input" << std::endl;
        return false;
    }

    std::vector<int> test_output;
    test_output.clear();

    CGAProg p;
    p.stages = progs;

    TestCase tc({},{},{});

    tc.north_input = north_input;
    tc.west_inputs = west_inputs;
    tc.south_output = south_output;

    bool success = eval(
         tc,
         p,
         test_output);

    if (!success){
        //std::cout << "eval error" << std::endl;
        return false;
    }

    if (test_output.size() == south_output.size()){
        for (size_t i = 0; i < test_output.size(); i++){
            /*if (test_output[i] < 0){
                std::cout << "ERROR" << std::endl;
                std::cout << "EVAL RET: "<< success << std::endl;
                dbg_print_progs(progs);
                this->debug = true;
                test_output.clear();
                eval(north_input,
                 west_inputs,
                 test_output,
                 progs);
                throw;
            }*/
            if (test_output[i] != south_output[i]){
                //std::cout << "output mismatch" << std::endl;
                //dbg_print_vec(test_output);
                //std::cout << " != " << std::endl;
                //dbg_print_vec(south_output);
                return false;
            }
        }
    } else {
        //std::cout << "bad output size" << std::endl;
        return false;
    }

    dbg_print_vec(test_output);
    dbg_print_vec(south_output);

    return true;
}


bool CGAVirt::eval(
        TestCase& tc,
        CGAProg& prog,
        std::vector<int>& output
        ){

    unsigned int instr_count = 0;
    unsigned int run_max_stack = 0;

    std::deque<int> pe_link_N;
    std::deque<int> pe_link_S;
    std::deque<int> pe_link_W;

    cga_height = tc.west_inputs.size();

    unsigned int north_input_sz = tc.north_input.size();

    //init the queue with the north input (in reverse order) to match the W
    //input access pattern
    for (auto v : tc.north_input){
        pe_link_N.push_front(v);
    }
    pe_link_S.clear();

    //run each pe from top to bottom
    for (unsigned int pe = 0; pe < cga_height; pe++){

        //reset VM state
        reset_regs();

        //load west PE link
        pe_link_W.clear();
        for (int v : tc.west_inputs[pe]){
            pe_link_W.push_front(v);
        }

        //run each program in the programs vector (setup, comp, cleanup)
        int prog_id = 0;
        for (std::vector<CGAInst>& stage : prog.stages){
            //we run each program cga height times
            int base_iterations = 0;
            //if (prog_id == 0){
            //    base_iterations=pe;
            //}
            for (unsigned int i = base_iterations; i < north_input_sz; i++){
                //interpret the program
                for (CGAInst inst : stage){

                    instr_count++;
                    if (debug){
                        std::cout << "---" << std::endl;
                        std::cout << "Prog: " << prog_id << std::endl;
                        std::cout << "Iter: " << i << std::endl;
                        std::cout << "PE: " << pe << std::endl;
                        std::cout << "PE LINK: NORTH" << std::endl;
                        dbg_print_deque(pe_link_N);
                        std::cout << "PE LINK: SOUTH" << std::endl;
                        dbg_print_deque(pe_link_S);
                        std::cout << "PE LINK: WEST" << std::endl;
                        dbg_print_deque(pe_link_W);
                        std::cout << "STACK" << std::endl;
                        dbg_print_stack(hw_stack);
                        std::cout << "HW REG" << std::endl;
                        std::cout << hw_reg << std::endl;
                        std::cout << "NEXT INST" << std::endl;
                        std::cout << inst << std::endl;
                    }

                    if (inst == CGAInst::pull_N){
                        if (!pe_link_N.empty()){
                            hw_stack.push_back(pe_link_N.front());
                            pe_link_N.pop_front();
                        } else {
                            hw_stack.push_back(0);
                        }
                    } else if (inst == CGAInst::pull_W){
                        if (!pe_link_W.empty()){
                            hw_stack.push_back(pe_link_W.front());
                            pe_link_W.pop_front();
                        } else {
                            hw_stack.push_back(0);
                        }

                    } else if (inst == CGAInst::send_S_pop){
                        //send top of stack or zero
                        if (!hw_stack.empty()){
                            pe_link_S.push_back(hw_stack.back());
                            hw_stack.pop_back();
                        } else {
                            //pe_link_S.push_back(0);
                            return false;
                        }

                    } else if (inst == CGAInst::send_S_peek){
                        //send top of stack or zero
                        if (!hw_stack.empty()){
                            pe_link_S.push_back(hw_stack.back());
                        } else {
                            //pe_link_S.push_back(0);
                            return false;
                        }

                    } else if (inst == CGAInst::add){
                        if (hw_stack.empty()){
                            return false;
                        }
                        int op_stack = hw_stack.back();
                        hw_stack.pop_back();
                        hw_stack.push_back(op_stack + hw_reg);
                    } else if (inst == CGAInst::mul){
                        if (hw_stack.empty()){
                            return false;
                        }
                        int op_stack = hw_stack.back();
                        hw_stack.pop_back();
                        hw_stack.push_back(op_stack * hw_reg);
                    } else if (inst == CGAInst::pop){
                        if (hw_stack.empty()){
                            return false;
                        }
                        hw_reg = hw_stack.back();
                        hw_stack.pop_back();
                    } else if (inst == CGAInst::push){
                        hw_stack.push_back(hw_reg);
                    } else if (inst == CGAInst::peek){
                        if (hw_stack.empty()){
                            return false;
                        }
                        hw_reg = hw_stack.back();
                    } else if (inst == CGAInst::mac){
                        if (hw_stack.size() < 2){
                            return false;
                        }
                        int op_a = hw_stack.back();
                        hw_stack.pop_back();
                        int op_b = hw_stack.back();
                        hw_stack.pop_back();

                        int mac_res = op_a * op_b + hw_reg;
                        hw_stack.push_back(mac_res);
                    } else if (inst == CGAInst::zero_push){
                        hw_stack.push_back(0);
                    } else if (inst == CGAInst::zero_reg){
                        hw_reg = 0;
                    } else {
                        return false;
                    }

                    //record the maximum stack size
                    unsigned int hw_stack_size = hw_stack.size();
                    if (hw_stack_size > run_max_stack){
                        run_max_stack = hw_stack_size;
                    }
                }
            }

            prog_id++;
        }

        //we are switching PEs any unused elements in the N LINK must be
        //dropped
        pe_link_N.clear();
        //swap the north and the south links
        pe_link_N.swap(pe_link_S);
    }

    //add all the values in the pe link to the output vector

    //after the last pe is evaluated we will swap its south pe link to the
    //north. We reverse this here so that we can read from the south link
    //not required, but easier to follow the code

    while(!pe_link_N.empty()){
        output.push_back(pe_link_N.front());
        pe_link_N.pop_front();
    }

    // Update statistics
    if (run_max_stack > stat_max_stack){
        stat_max_stack = run_max_stack;
    }

    stat_instr_run += instr_count;

    return true;
}


CGAInst int_to_CGAInst(unsigned int v){
    if (v < CGAInst_NUM){
        return static_cast<CGAInst>(v);
    } else {
        return CGAInst::undef;
    }
}


std::ostream& operator<<(std::ostream& out, const CGAInst inst){
    switch (inst){
        case CGAInst::pull_N:
            return out << "pull_N";
        case CGAInst::pull_W:
            return out << "pull_W";
        case CGAInst::add:
            return out << "add";
        case CGAInst::send_S_pop:
            return out << "send_S_pop";
        case CGAInst::pop:
            return out << "pop";
        case CGAInst::mul:
            return out << "mul";
        case CGAInst::mac:
            return out << "mac";
        case CGAInst::peek:
            return out << "peek";
        case CGAInst::send_S_peek:
            return out << "send_S_peek";
        case CGAInst::zero_reg:
            return out << "zero_reg";
        case CGAInst::zero_push:
            return out << "zero_push";
        case CGAInst::push:
            return out << "push";
        case CGAInst::nop:
            return out << "nop";
        default:
            return out << "undefined";
    }
}
