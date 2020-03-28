#pragma once
#include <vector>
#include <random>
#include "sorghum/vm.h"
#include "sorghum/synth.h"

struct MCMCProposalDist {
    double p_swap;
    double p_insert;
    double p_remove;
    double p_replace;
    double p_inc_stage;
    double p_dec_stage;
};

enum class MCMCxform{
    swap,
    insert,
    remove,
    replace,
    inc_stage,
    dec_stage
};

class MCMCSynth {

public:
    MCMCSynth(MCMCProposalDist &proposal_dist,
              int max_stages,
              int max_types,
              std::vector<TestCase>& test_cases,
              unsigned int seed
              );
    ~MCMCSynth();

    //initialize the candiate program with a random program
    void init();

    void gen_next_canidate();

    float get_canidate_cost() {
        return canidate_cost;
    }

    bool get_canidate_valid(){
        return canidate_valid;
    }

    CGAProg get_canidate(){
        return canidate;
    };

private:

    float compute_correctness(TestCase& tc, std::vector<int> output);

    CGAProg xform_canidate(MCMCxform xform);

    void xform_swap_inst(CGAProg& prog, ProgCursor sel_a, ProgCursor sel_b);
    void xform_replace_inst(CGAProg& prog, ProgCursor sel, CGAInst inst);
    void xform_remove_inst(CGAProg& prog, ProgCursor sel);
    void xform_insert_inst(CGAProg& prog, ProgCursor sel, CGAInst inst);

    //Randomization helper functions
    CGAInst gen_random_inst();
    ProgCursor gen_random_cursor(CGAProg& prog);

    // Random Xform Generation
    std::uniform_real_distribution<float> zero_one_dist;
    std::discrete_distribution<unsigned int> pdist;
    std::mt19937 rgen;

    //Configuration
    int max_canidate_stages;
    int max_canidate_types;

    // Current canidate info
    float canidate_cost;
    bool canidate_valid;
    CGAProg canidate;

    //Virtual Machine Intstance
    CGAVirt vm;

    //Test cases
    std::vector<TestCase>& test_cases;

};
