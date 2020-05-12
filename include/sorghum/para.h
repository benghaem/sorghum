#pragma once
#include <memory>
#include "sorghum/mcmc_synth.h"



class ParaMCMC{

    public:
        ParaMCMC(int num_instances,
              MCMCProposalDist &proposal_dist,
              int max_stages,
              int max_types,
              int max_inst,
              int num_regs,
              int height,
              int num_w_streams,
              std::vector<TestCase>& test_cases);
        ~ParaMCMC();

        void run(int cycles);
        static void get_best_canidate(int n, int cycle, MCMCSynth& mcmc,  MCMCResult& res);

    private:

        int vm_num_regs;

        std::vector<TestCase> tcs;
        std::vector<std::unique_ptr<MCMCSynth>> instances;
};
