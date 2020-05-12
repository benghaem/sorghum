#include "sorghum/mcmc_synth.h"
#include "sorghum/util.h"
#include <set>

MCMCSynth::MCMCSynth(MCMCProposalDist &proposal_dist, int max_stages,
        int max_types, int max_inst, int num_regs, int height, int num_w_streams,
        std::vector<TestCase> &test_cases, unsigned int seed)
    : zero_one_dist(0.0,1.0),
      pdist({proposal_dist.p_swap, proposal_dist.p_insert,
            proposal_dist.p_remove, proposal_dist.p_replace,
            proposal_dist.p_inc_stage, proposal_dist.p_dec_stage,
            proposal_dist.p_change_imode, 
            proposal_dist.p_swap_stream, proposal_dist.p_swap_pe_type}),
      rgen(seed),
      max_canidate_stages(max_stages),
      max_canidate_types(max_types),
      max_canidate_inst_per_stage(max_inst),
      canidate_cost(0),
      canidate_valid(false),
      canidate(height, max_types, num_w_streams),
      vm(num_regs),
      test_cases(test_cases)
      {}

MCMCSynth::~MCMCSynth(){
}

//initalize with a reasonable starting point
void MCMCSynth::init(){

    //default iteration mode is north len
    for (unsigned int i = 0; i < canidate.num_progs; i++){
        CGAProg& c_prog = canidate.get_prog(i);
        c_prog.iteration_mode.push_back(CGAIterMode::west_len);

        c_prog.stages.push_back(std::vector<CGAInst>());
        c_prog.stages[0].push_back(CGAInst(CGAOp::pull_N, {0,0}));
        c_prog.stages[0].push_back(CGAInst(CGAOp::pull_W, {0,0}));
        c_prog.stages[0].push_back(CGAInst(CGAOp::send_S_peek, {0,0}));
    }
}


//correctness should be the fraction of correct values in the output independent of order
MCMCOutGoodness MCMCSynth::compute_output_goodness(TestCase& tc, std::vector<int>& output){

    std::set<int> output_v;

    for (int v : output){
        output_v.insert(v);
    }

    int values_found = 0;
    int total_values = tc.south_output.size();
    int output_len = output.size();

    for (int tv : tc.south_output){
        if (output_v.count(tv) == 1){
            values_found++;
        }
    }

    float percent_found = (float)values_found / (float)total_values;
    float percent_good = (float)values_found / (float)output_len;

    MCMCOutGoodness og;
    og.per_correct_values = percent_good;
    og.per_values_present = percent_found;

    return og;
}

MCMCCost MCMCSynth::unified_cost_fn(TestCase& tc, CGAConfig1D& new_canidate, std::vector<int>& output){
    output.clear();
    vm.reset_stats();

    MCMCCost c;

    float correctness_weight = 0.8;
    float execution_weight = 0.2;

    float vp_weight = 0.1;
    float cv_weight = 0.9;

    float ex_stack_weight = 0.7;
    float ex_stages_weight = 0.2;
    float ex_len_weight = 0.1;

    bool executed = vm.eval(tc, new_canidate, output);

    if (executed){
        MCMCOutGoodness og = compute_output_goodness(tc, output);

        c.correctness = 1 - (og.per_values_present * vp_weight + og.per_correct_values * cv_weight);
        c.valid = (og.per_values_present == 1.0);

        int total_stages = 0;
        int total_instr_count = 0;

        for (unsigned int i = 0; i < new_canidate.num_progs; i++){
            CGAProg& nc_prog = new_canidate.get_prog(i);
            total_stages += nc_prog.stages.size();
            int local_instr_count = 0;
            for (auto& stage : nc_prog.stages){
                local_instr_count += stage.size();
            }
            total_instr_count = std::max(local_instr_count, total_instr_count);
        }

        float stack_sz = (float)vm.stat_max_stack / (float)vm.stack_limit;
        float stage_sz = (float)total_stages / (float) max_canidate_stages;
        float instr_sz = (float)total_instr_count / (float)(max_canidate_inst_per_stage * total_stages);

        c.execution = stack_sz * ex_stack_weight
                    + stage_sz * ex_stages_weight
                    + instr_sz * ex_len_weight;

    } else {
        c.correctness = 1;
        c.execution = 1;
        c.valid = false;
    }

    c.correctness *= correctness_weight;
    c.execution *= execution_weight;

    return c;
};

//transform the current canidate to create another canidate
CGAConfig1D MCMCSynth::xform_canidate(MCMCxform xform){

    CGAConfig1D new_canidate = canidate;
    CGAProg& nc_prog = sel_random_prog(new_canidate);

    if (xform == MCMCxform::swap){
        ProgCursor ca = gen_random_cursor(nc_prog);
        ProgCursor cb = gen_random_cursor(nc_prog);
        xform_swap_inst(nc_prog, ca, cb);
    } else if (xform == MCMCxform::replace){
        ProgCursor c = gen_random_cursor(nc_prog);
        CGAInst new_inst = gen_random_inst();
        xform_replace_inst(nc_prog, c, new_inst);
    } else if (xform == MCMCxform::insert){
        ProgCursor c = gen_random_cursor(nc_prog);
        CGAInst new_inst = gen_random_inst();
        xform_insert_inst(nc_prog, c, new_inst);
    } else if (xform == MCMCxform::remove){
        ProgCursor c = gen_random_cursor(nc_prog);
        xform_remove_inst(nc_prog,c);
    } else if (xform == MCMCxform::inc_stage){
        ProgCursor c = gen_random_cursor(nc_prog);
        xform_inc_stage(nc_prog, c);
    } else if (xform == MCMCxform::dec_stage){
        ProgCursor c = gen_random_cursor(nc_prog);
        xform_dec_stage(nc_prog, c);
    } else if (xform == MCMCxform::change_imode){
        ProgCursor c = gen_random_cursor(nc_prog);
        CGAIterMode new_imode = gen_random_imode();
        xform_change_imode(nc_prog,c,new_imode);
    } else if (xform == MCMCxform::swap_stream){
        unsigned int pe_id = sel_random_pe_id(new_canidate);
        unsigned int new_stream_id = sel_random_stream_id(new_canidate);
        xform_change_stream_assignment(new_canidate,pe_id,new_stream_id);
    } else if (xform == MCMCxform::swap_pe_type){
        unsigned int pe_id = sel_random_pe_id(new_canidate);
        unsigned int new_prog_id = sel_random_prog_id(new_canidate);
        xform_change_pe_prog(new_canidate,pe_id,new_prog_id);
    }

    return new_canidate;
}

void MCMCSynth::xform_inc_stage(CGAProg& prog, ProgCursor sel){
    if (prog.stages.size() < (unsigned int)max_canidate_stages){
        std::vector<CGAInst> tmp = {CGAInst(CGAOp::pull_N, {}),
                                    CGAInst(CGAOp::pull_W, {}),
                                    CGAInst(CGAOp::send_S_peek, {})};
        prog.stages.insert(prog.stages.cbegin() + sel.stage, tmp);

        //new stage is also in north len mode
        prog.iteration_mode.insert(prog.iteration_mode.cbegin() + sel.stage, CGAIterMode::west_len);
    }
}

void MCMCSynth::xform_dec_stage(CGAProg& prog, ProgCursor sel){
    //only remove if there will still be one stage left
    if (prog.stages.size() > 1){
        prog.stages.erase(prog.stages.cbegin() + sel.stage);
        prog.iteration_mode.erase(prog.iteration_mode.cbegin() + sel.stage);
    }
}

void MCMCSynth::xform_change_imode(CGAProg& prog, ProgCursor sel, CGAIterMode imode){
    prog.iteration_mode[sel.stage] = imode;
}

void MCMCSynth::xform_swap_inst(CGAProg& prog, ProgCursor sel_a, ProgCursor sel_b){
    CGAInst a = prog.stages[sel_a.stage][sel_a.iidx];
    CGAInst b = prog.stages[sel_b.stage][sel_b.iidx];
    prog.stages[sel_a.stage][sel_a.iidx] = b;
    prog.stages[sel_b.stage][sel_b.iidx] = a;
}

void MCMCSynth::xform_insert_inst(CGAProg& prog, ProgCursor sel, CGAInst inst){
    if (prog.stages[sel.stage].size() < (unsigned int)max_canidate_inst_per_stage){
        auto itt = prog.stages[sel.stage].begin();
        prog.stages[sel.stage].insert(itt + sel.iidx, inst);
    }
}

void MCMCSynth::xform_replace_inst(CGAProg& prog, ProgCursor sel, CGAInst inst){
    prog.stages[sel.stage][sel.iidx] = inst;
}

void MCMCSynth::xform_remove_inst(CGAProg& prog, ProgCursor sel){
    //only remove if there is an instuction to remove
    if (prog.stages[sel.stage].size() > 1){
        auto itt = prog.stages[sel.stage].begin();
        prog.stages[sel.stage].erase(itt + sel.iidx);
    }
}

void MCMCSynth::xform_change_stream_assignment(CGAConfig1D& cfg1d, 
                                               unsigned int pe_id,
                                               int stream_id){
    cfg1d.set_w_stream(pe_id,stream_id);
}


void MCMCSynth::xform_change_pe_prog(CGAConfig1D& cfg1d,
                                     unsigned int pe_id,
                                     unsigned int prog_id){
    cfg1d.set_assigned_prog(pe_id,prog_id);
}

unsigned int MCMCSynth::sel_random_pe_id(CGAConfig1D& cfg1d){
    unsigned int pe_count = cfg1d.cga_height;
    std::uniform_int_distribution<unsigned int> rand_int(0,pe_count-1);

    unsigned int pe = rand_int(rgen);
    if (pe >= pe_count){
        throw;
    }

    return pe;
}

int MCMCSynth::sel_random_stream_id(CGAConfig1D& cfg1d){
    int streams = cfg1d.num_w_streams;
    std::uniform_int_distribution<int> rand_int(-1,streams-1);

    int stream_id = rand_int(rgen);
    if (stream_id >= streams || stream_id < -1){
        throw;
    }

    return stream_id;
}

unsigned int MCMCSynth::sel_random_prog_id(CGAConfig1D& cfg1d){
    unsigned int progs = cfg1d.num_progs;

    std::uniform_int_distribution<unsigned int> rand_int(0,progs-1);

    unsigned int prog = rand_int(rgen);
    if (prog >= progs){
        throw;
    }

    return prog;
}

CGAProg& MCMCSynth::sel_random_prog(CGAConfig1D& cfg1d){
    unsigned int prog = sel_random_prog_id(cfg1d); 

    return cfg1d.get_prog(prog);
}

ProgCursor MCMCSynth::gen_random_cursor(CGAProg& prog){
    unsigned int stages = prog.stages.size();

    //randomly select a stage [0, stages-1]
    std::uniform_int_distribution<unsigned int> rand_int(0,stages-1);

    unsigned int stage = rand_int(rgen);
    if (stage >= stages){
        throw;
    }

    //randomly select an instruction [0, total_insts-1]
    unsigned int total_insts = prog.stages[stage].size();

    std::uniform_int_distribution<unsigned int>::param_type z_ti(0,total_insts-1);
    rand_int.param(z_ti);

    unsigned int iidx = rand_int(rgen);

    if (iidx > total_insts){
        throw;
    }

    ProgCursor sel = {stage, iidx};

    return sel;
}

//this uses information from
CGAInst MCMCSynth::gen_random_inst(){
    std::uniform_int_distribution<unsigned int> op_int(0,CGAOp_NUM-1);
    std::uniform_int_distribution<unsigned int> reg_int(0,vm.TOTAL_REGS-1);

    CGAOp op = int_to_CGAOp(op_int(rgen));

    std::array<int,2> args;
    args[0] = reg_int(rgen);
    args[1] = reg_int(rgen);

    return CGAInst(op, args);
}

CGAIterMode MCMCSynth::gen_random_imode(){
    std::uniform_int_distribution<unsigned int> rand_int(0,CGAIterMode_NUM-1);

    return int_to_CGAIterMode(rand_int(rgen));
}

MCMCResult MCMCSynth::get_current_result(){
    MCMCResult res(canidate, canidate_cost, canidate_valid);
    return res;
}

void MCMCSynth::gen_next_canidate() {
    // random sample from proposal distribution to get transformation

    unsigned int raw_xform = pdist(rgen);

    constexpr MCMCxform xforms[9] = {MCMCxform::swap,
                                    MCMCxform::insert,
                                    MCMCxform::remove,
                                    MCMCxform::replace,
                                    MCMCxform::inc_stage,
                                    MCMCxform::dec_stage,
                                    MCMCxform::change_imode,
                                    MCMCxform::swap_stream,
                                    MCMCxform::swap_pe_type};

    MCMCxform xform = xforms[raw_xform];

    // apply transformation

    CGAConfig1D new_canidate = xform_canidate(xform);

    // evaluate new program on all test_cases and compute
    // cost function
    float correctness_cost = 0;
    float execution_cost = 0;
    bool valid = true;
    std::vector<int> output;
    for (TestCase &tc : test_cases) {

        MCMCCost c = unified_cost_fn(tc, new_canidate, output);

        correctness_cost += c.correctness;
        execution_cost += c.execution;
        valid &= c.valid;
    }

    //compute the cost function (1 is best, 0 is worst)
    float total_tests = test_cases.size();
    float cc_avg = correctness_cost / total_tests;
    float ec_avg = execution_cost / total_tests;
    //float ec_avg = 0;
    float new_cost = 1.0 - (cc_avg + ec_avg);

    // choose to keep or revert transformation
    float p_accept = new_cost / canidate_cost;

    bool accept = (p_accept > 1.0);
    if (!accept) {
        float choice = zero_one_dist(rgen);
        if (choice < p_accept){
            accept = true;
        } else {
            accept = false;
        }
    }
    if (accept){
        if (valid){
            canidate_valid = true;
        } else {
            canidate_valid = false;
        }
        canidate = new_canidate;
        canidate_cost = new_cost;
    }
}

MCMCResult::MCMCResult(CGAConfig1D canidate, float prob, bool valid): 
    canidate(canidate), prob(prob), valid(valid){
}

MCMCResult::MCMCResult():
    canidate(0,0,0), prob(0.0), valid(false){
}

bool MCMCResult::better_than(const MCMCResult& other){
    return compare(other, *this);
}

bool MCMCResult::compare(const MCMCResult& a, const MCMCResult& b){
    if (a.valid && !b.valid){
        return false;
    } else if (!a.valid && b.valid){
        return true;
    } else {
        return (a.prob < b.prob);
    }
}
