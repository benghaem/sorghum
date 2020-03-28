#include "sorghum/mcmc_synth.h"
#include "sorghum/util.h"
#include <set>

MCMCSynth::MCMCSynth(MCMCProposalDist &proposal_dist, int max_stages,
        int max_types, std::vector<TestCase> &test_cases, unsigned int seed)
    : zero_one_dist(0.0,1.0),
      pdist({proposal_dist.p_swap, proposal_dist.p_insert,
            proposal_dist.p_remove, proposal_dist.p_inc_stage,
            proposal_dist.p_dec_stage}),
      rgen(seed),
      max_canidate_stages(max_stages),
      max_canidate_types(max_types),
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
float MCMCSynth::compute_correctness(TestCase& tc, std::vector<int> output){

    std::set<int> output_v;

    for (int v : output){
        output_v.insert(v);
    }

    int values_found = 0;
    int total_values = tc.south_output.size();

    for (int tv : tc.south_output){
        if (output_v.count(tv)){
            values_found++;
        }
    }

    return 1 - (float)values_found / (float)total_values;
}

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
    }

    return new_canidate;
}

void MCMCSynth::xform_swap_inst(CGAProg& prog, ProgCursor sel_a, ProgCursor sel_b){
    CGAInst a = prog.stages[sel_a.stage][sel_a.iidx];
    CGAInst b = prog.stages[sel_b.stage][sel_b.iidx];
    prog.stages[sel_a.stage][sel_a.iidx] = b;
    prog.stages[sel_b.stage][sel_b.iidx] = a;
}

void MCMCSynth::xform_insert_inst(CGAProg& prog, ProgCursor sel, CGAInst inst){
    auto itt = prog.stages[sel.stage].begin();
    prog.stages[sel.stage].insert(itt + sel.iidx, inst);
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
    float correctness = 0;
    float execution_cost = 0;
    std::vector<int> output;
    for (TestCase &tc : test_cases) {
        output.clear();
        vm.reset_stats();

        bool executed = vm.eval(tc, new_canidate, output);

        if (executed) {
            // compare output
            correctness += compute_correctness(tc, output);
            // get execution cost
            execution_cost += (float)vm.stat_max_stack / (float)vm.stack_limit;

        } else {
            correctness += 1.0;
            execution_cost += 1.0;
        }

    }

    //compute the cost function (1 is best, 0 is worst)
    float new_cost = 1.0 - (correctness + execution_cost) / 2.0;

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
        if (correctness == 0){
            canidate_valid = true;
        } else {
            canidate_valid = false;
        }
        canidate = new_canidate;
        canidate_cost = new_cost;
    }
}
