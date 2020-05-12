#include <deque>
#include <iostream>

#include "sorghum/vm.h"
#include "sorghum/util.h"


CGAConfig1D::CGAConfig1D(unsigned int height, unsigned int num_progs,
              unsigned int num_w_streams) :
      num_w_streams(num_w_streams),
      cga_height(height),
      num_progs(num_progs),
      progs_(num_progs),
      pe_assignment_(height),
      pe_w_stream_sel_(height)
    {
        std::fill(pe_assignment_.begin(),pe_assignment_.end(),0);
        std::fill(pe_w_stream_sel_.begin(),pe_w_stream_sel_.end(),-1);

        for (unsigned int i = 0; i < height; i++){
            if (i >= num_w_streams){
                pe_w_stream_sel_[i] = -1;
            } else {
                pe_w_stream_sel_[i] = i;
            }
        }
    };



CGAInst::CGAInst(CGAOp op, std::initializer_list<int> args): 
    op(op)
{
    if (args.size() > 2){
        throw;
    }
    this->args.fill(0);
    int c = 0;
    for ( int v : args ){
        this->args[c] = v;
        c++;
    }
}

CGAInst::CGAInst(CGAOp op, std::array<int,2>& args): 
    op(op),
    args(args){
};


CGAVirt::CGAVirt(int num_regs) : TOTAL_REGS(num_regs), hw_regs(num_regs){
    std::fill(hw_regs.begin(), hw_regs.end(), 0);
}

CGAVirt::~CGAVirt(){
}

void CGAVirt::reset_regs(){
    //clear all internal registers
    hw_stack.clear();
    std::fill(hw_regs.begin(), hw_regs.end(), 0);
}

void CGAVirt::reset_stats(){
    stat_max_stack = 0;
    stat_instr_run = 0;
}

bool CGAVirt::check(std::vector<int>& north_input,
                    std::vector<std::vector<int>>& west_inputs,
                    std::vector<int>& south_output,
                    std::vector<std::vector<CGAInst>>& progs){

    return false;
}

bool CGAVirt::eval(
        TestCase& tc,
        CGAConfig1D& cfg,
        std::vector<int>& output
        ){

    unsigned int instr_count = 0;
    unsigned int run_max_stack = 0;

    std::deque<int> pe_link_N;
    std::deque<int> pe_link_S;
    std::deque<int> pe_link_W;

    unsigned int cga_height = cfg.cga_height;

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
        int sel_stream = cfg.get_w_stream(pe);
        if (sel_stream != -1){
            for (int v : tc.west_inputs[sel_stream]){
                pe_link_W.push_front(v);
            }
        }

        //run each program in the programs vector (setup, comp, cleanup)
        CGAProg& prog = cfg.get_assigned_prog(pe);
        int prog_stage_id = 0;
        for (std::vector<CGAInst>& stage : prog.stages){
            //we run each program based on the mode
            int base_iterations = 0;
            unsigned int total_iterations = 0;
            switch(prog.iteration_mode[prog_stage_id]){
                case CGAIterMode::north_len:
                    total_iterations = north_input_sz;
                    break;
                case CGAIterMode::west_len:
                    total_iterations = tc.west_inputs[pe].size();
                    break;
                case CGAIterMode::pe_depth:
                    total_iterations = pe + 1;
                    break;
                case CGAIterMode::pe_depth_inv:
                    total_iterations = cga_height - pe;
                    break;
                case CGAIterMode::single:
                    total_iterations = 1;
                    break;
                case CGAIterMode::undef:
                    //this is an error case
                    return false;
            }
            for (unsigned int i = base_iterations; i < total_iterations; i++){
                //interpret the program
                int stage_len = stage.size();
                for (int stage_pc = 0; stage_pc < stage_len; stage_pc++){

                    CGAInst& inst = stage[stage_pc];
                    CGAOp op = inst.op;
                    instr_count++;
                    if (debug){
                        std::cout << "---" << std::endl;
                        std::cout << "Stage: " << prog_stage_id << std::endl;
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
                        std::cout << "HW REGS" << std::endl;
                        dbg_print_vec(hw_regs);
                        std::cout << "NEXT op" << std::endl;
                        std::cout << op << std::endl;
                    }

                    if (op == CGAOp::pull_N){
                        if (!pe_link_N.empty()){
                            hw_stack.push_back(pe_link_N.front());
                            pe_link_N.pop_front();
                        } else {
                            hw_stack.push_back(0);
                        }
                    } else if (op == CGAOp::pull_W){
                        if (!pe_link_W.empty()){
                            hw_stack.push_back(pe_link_W.front());
                            pe_link_W.pop_front();
                        } else {
                            hw_stack.push_back(0);
                        }

                    } else if (op == CGAOp::send_S_pop){
                        //send top of stack or zero
                        if (!hw_stack.empty()){
                            pe_link_S.push_back(hw_stack.back());
                            hw_stack.pop_back();
                        } else {
                            //pe_link_S.push_back(0);
                            return false;
                        }

                    } else if (op == CGAOp::send_S_peek){
                        //send top of stack or zero
                        if (!hw_stack.empty()){
                            pe_link_S.push_back(hw_stack.back());
                        } else {
                            //pe_link_S.push_back(0);
                            return false;
                        }
                    } else if (op == CGAOp::send_S_ifz_peek){
                        //send top of stack or zero
                        if (!hw_stack.empty()){
                            if (!hw_regs[inst.args[0]]){
                                pe_link_S.push_back(hw_stack.back());
                            }
                        } else {
                            //pe_link_S.push_back(0);
                            return false;
                        } 
                    } else if (op == CGAOp::add){
                        if (hw_stack.empty()){
                            return false;
                        }
                        int op_stack = hw_stack.back();
                        int reg_v = hw_regs[inst.args[0]];
                        hw_stack.pop_back();
                        hw_stack.push_back(op_stack + reg_v);
                    } else if (op == CGAOp::sub){
                        if (hw_stack.empty()){
                            return false;
                        }
                        int op_stack = hw_stack.back();
                        int reg_v = hw_regs[inst.args[0]];
                        hw_stack.pop_back();
                        hw_stack.push_back(op_stack - reg_v);
                    } else if (op == CGAOp::mul){
                        if (hw_stack.empty()){
                            return false;
                        }
                        int op_stack = hw_stack.back();
                        int reg_v = hw_regs[inst.args[0]];
                        hw_stack.pop_back();
                        hw_stack.push_back(op_stack * reg_v);
                    } else if (op == CGAOp::inc){
                        hw_regs[inst.args[0]] += 1;
                    } else if (op == CGAOp::dec){
                        hw_regs[inst.args[0]] -= 1;
                    } else if (op == CGAOp::gt){
                        if (hw_stack.empty()){
                            return false;
                        }
                        int reg_v = hw_regs[inst.args[0]];
                        int stack_v = hw_stack.back();
                        hw_stack.push_back(stack_v > reg_v);
                    } else if (op == CGAOp::lt){
                        if (hw_stack.empty()){
                            return false;
                        }
                        int reg_v = hw_regs[inst.args[0]];
                        int stack_v = hw_stack.back();
                        hw_stack.push_back(stack_v < reg_v);
                    } else if (op == CGAOp::eq){
                        if (hw_stack.empty()){
                            return false;
                        }

                        int reg_v = hw_regs[inst.args[0]];
                        int stack_v = hw_stack.back();
                        hw_stack.push_back(stack_v == reg_v);
                    } else if (op == CGAOp::pop){
                        if (hw_stack.empty()){
                            return false;
                        }
                        //just trying something out
                        //hw_regs[inst.args[0]] = hw_stack.back();
                        hw_stack.pop_back();
                    } else if (op == CGAOp::push){
                        int reg_v = hw_regs[inst.args[0]];
                        hw_stack.push_back(reg_v);
                    } else if (op == CGAOp::peek){
                        if (hw_stack.empty()){
                            return false;
                        }
                        hw_regs[inst.args[0]] = hw_stack.back();
                    } else if (op == CGAOp::mac){
                        if (hw_stack.size() < 2){
                            return false;
                        }
                        int op_a = hw_stack.back();
                        hw_stack.pop_back();
                        int op_b = hw_stack.back();
                        hw_stack.pop_back();

                        int reg_v = hw_regs[inst.args[0]];

                        int mac_res = op_a * op_b + reg_v;
                        hw_stack.push_back(mac_res);
                    } else if (op == CGAOp::zero_push){
                        hw_stack.push_back(0);
                    } else if (op == CGAOp::zero_reg){
                        hw_regs[inst.args[0]] = 0;
                    } else if (op == CGAOp::jump1c){
                        if (hw_stack.empty()){
                            return false;
                        }
                        int stack_v = hw_stack.back();
                        if (stack_v > 0){
                            //jump over next instruction
                            stage_pc++;
                        }
                    }
                    else {
                        return false;
                    }

                    //record the maximum stack size
                    unsigned int hw_stack_size = hw_stack.size();
                    if (hw_stack_size > run_max_stack){
                        run_max_stack = hw_stack_size;
                    }
                }
            }

            prog_stage_id++;
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


CGAOp int_to_CGAOp(unsigned int v){
    if (v < CGAOp_NUM){
        return static_cast<CGAOp>(v);
    } else {
        return CGAOp::undef;
    }
}


std::ostream& operator<<(std::ostream& out, const CGAInst& inst){
    return out << inst.op << "<" << inst.args[0] << "," << inst.args[1] << ">";
}

std::ostream& operator<<(std::ostream& out, const CGAOp& op){
    switch (op){
        case CGAOp::pull_N:
            return out << "pull_N";
        case CGAOp::pull_W:
            return out << "pull_W";
        case CGAOp::add:
            return out << "add";
        case CGAOp::sub:
            return out << "sub";
        case CGAOp::send_S_pop:
            return out << "send_S_pop";
        case CGAOp::pop:
            return out << "pop";
        case CGAOp::mul:
            return out << "mul";
        case CGAOp::mac:
            return out << "mac";
        case CGAOp::peek:
            return out << "peek";
        case CGAOp::send_S_peek:
            return out << "send_S_peek";
        case CGAOp::send_S_ifz_peek:
            return out << "send_S_ifz_peek";
        case CGAOp::zero_reg:
            return out << "zero_reg";
        case CGAOp::zero_push:
            return out << "zero_push";
        case CGAOp::push:
            return out << "push";
        case CGAOp::nop:
            return out << "nop";
        case CGAOp::inc:
            return out << "inc";
        case CGAOp::dec:
            return out << "dec";
        case CGAOp::jump1c:
            return out << "jump1c";
        case CGAOp::gt:
            return out << "gt";
        case CGAOp::lt:
            return out << "lt";
        case CGAOp::eq:
            return out << "eq";
        case CGAOp::undef:
            return out << "undef";
    }
    return out << "unknown instr";
}


CGAIterMode int_to_CGAIterMode(unsigned int v){
    if (v < CGAIterMode_NUM){
        return static_cast<CGAIterMode>(v);
    } else {
        return CGAIterMode::undef;
    }
}

std::ostream& operator<<(std::ostream& out, const CGAIterMode imode){
    switch (imode){
        case CGAIterMode::north_len:
            return out << "north len";
        case CGAIterMode::west_len:
            return out << "west len";
        case CGAIterMode::pe_depth:
            return out << "pe depth";
        case CGAIterMode::pe_depth_inv:
            return out << "pe depth (inv)";
        case CGAIterMode::single:
            return out << "single";
        case CGAIterMode::undef:
            return out << "undef";
        default:
            return out << "undefined";
    }
}
