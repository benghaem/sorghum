#include "sorghum/mcmc_synth.h"
#include "sorghum/util.h"
#include <set>

MCMCSynth::MCMCSynth(MCMCProposalDist &proposal_dist, int max_stages,
        int max_types, int max_inst, std::vector<TestCase> &test_cases, unsigned int seed)
    : zero_one_dist(0.0,1.0),
      pdist({proposal_dist.p_swap, proposal_dist.p_insert,
            proposal_dist.p_remove, proposal_dist.p_inc_stage,
            proposal_dist.p_dec_stage}),
      rgen(seed),
      max_canidate_stages(max_stages),
      max_canidate_types(max_types),
      max_canidate_inst_per_stage(max_inst),
      canidate_cost(0),
      canidate_valid(false),
      test_cases(test_cases)
      {
    }

MCMCSynth::~MCMCSynth(){
}

//initalize with a reasonable starting point
void MCMCSynth::init(){

    canidate.stages.push_back(std::vector<CGAInst>());

    canidate.stages[0].push_back(CGAInst::pull_N);
    canidate.stages[0].push_back(CGAInst::pull_W);
    canidate.stages[0].push_back(CGAInst::send_S_peek);
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

MCMCCost MCMCSynth::unified_cost_fn(TestCase& tc, CGAProg& new_canidate, std::vector<int>& output){
    output.clear();
    vm.reset_stats();

    MCMCCost c;

    float correctness_weight = 0.8;
    float execution_weight = 0.2;

    float vp_weight = 0.6;
    float cv_weight = 0.4;

    bool executed = vm.eval(tc, new_canidate, output);

    if (executed){
        MCMCOutGoodness og = compute_output_goodness(tc, output);

        c.correctness = 1 - (og.per_values_present * vp_weight + og.per_correct_values * cv_weight);
        c.valid = (og.per_values_present == 1.0);

        c.execution = (float)vm.stat_max_stack / (float)vm.stack_limit;

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
CGAProg MCMCSynth::xform_canidate(MCMCxform xform){

    CGAProg new_canidate = canidate;

    if (xform == MCMCxform::swap){
        ProgCursor ca = gen_random_cursor(new_canidate);
        ProgCursor cb = gen_random_cursor(new_canidate);
        xform_swap_inst(new_canidate, ca, cb);
    } else if (xform == MCMCxform::replace){
        ProgCursor c = gen_random_cursor(new_canidate);
        CGAInst new_inst = gen_random_inst();
        xform_replace_inst(new_canidate, c, new_inst);
    } else if (xform == MCMCxform::insert){
        ProgCursor c = gen_random_cursor(new_canidate);
        CGAInst new_inst = gen_random_inst();
        xform_insert_inst(new_canidate, c, new_inst);
    } else if (xform == MCMCxform::remove){
        ProgCursor c = gen_random_cursor(new_canidate);
        xform_remove_inst(new_canidate,c);
    } else if (xform == MCMCxform::inc_stage){
        xform_inc_stage(new_canidate);
    } else if (xform == MCMCxform::dec_stage){
        xform_dec_stage(new_canidate);
    }

    return new_canidate;
}

void MCMCSynth::xform_inc_stage(CGAProg& prog){
    if (prog.stages.size() < max_canidate_stages){
        prog.stages.push_back(std::vector<CGAInst>());
        int idx = prog.stages.size() - 1;
        prog.stages[idx].push_back(CGAInst::pull_N);
        prog.stages[idx].push_back(CGAInst::send_S_peek);
    }
}

void MCMCSynth::xform_dec_stage(CGAProg& prog){
    if (prog.stages.size() > 0){
        prog.stages.pop_back();
    }
}

void MCMCSynth::xform_swap_inst(CGAProg& prog, ProgCursor sel_a, ProgCursor sel_b){
    CGAInst a = prog.stages[sel_a.stage][sel_a.iidx];
    CGAInst b = prog.stages[sel_b.stage][sel_b.iidx];
    prog.stages[sel_a.stage][sel_a.iidx] = b;
    prog.stages[sel_b.stage][sel_b.iidx] = a;
}

void MCMCSynth::xform_insert_inst(CGAProg& prog, ProgCursor sel, CGAInst inst){
    if (prog.stages[sel.stage].size() < max_canidate_inst_per_stage){
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

ProgCursor MCMCSynth::gen_random_cursor(CGAProg& prog){
    unsigned int stages = prog.stages.size();

    //randomly select a stage [0, stages-1]
    std::uniform_int_distribution<unsigned int> rand_int(0,stages-1);

    unsigned int stage = rand_int(rgen);
    if (stage > stages){
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

CGAInst MCMCSynth::gen_random_inst(){
    std::uniform_int_distribution<unsigned int> rand_int(0,CGAInst_NUM-1);

    return int_to_CGAInst(rand_int(rgen));
}

MCMCResult MCMCSynth::get_current_result(){
    MCMCResult res;
    res.canidate = canidate;
    res.valid = canidate_valid;
    res.prob = canidate_cost;
    return res;
}

void MCMCSynth::gen_next_canidate() {
    // random sample from proposal distribution to get transformation

    unsigned int raw_xform = pdist(rgen);
    MCMCxform xforms[5] = {MCMCxform::swap, MCMCxform::insert, MCMCxform::remove,
        MCMCxform::inc_stage, MCMCxform::dec_stage};
    MCMCxform xform = xforms[raw_xform];

    // apply transformation

    CGAProg new_canidate = xform_canidate(xform);

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

MCMCResult::MCMCResult(): canidate(), prob(0.0), valid(false){
}

bool MCMCResult::better_than(const MCMCResult& other){
    if (valid && !other.valid){
        return true;
    } else if (!valid && other.valid){
        return false;
    } else {
        return (prob > other.prob);
    }
}
