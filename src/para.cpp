#include "sorghum/para.h"
#include <random>
#include <thread>
#include <iostream>
#include "sorghum/util.h"

ParaMCMC::ParaMCMC(int num_instances,
              MCMCProposalDist &proposal_dist,
              int max_stages,
              int max_types,
              int max_inst,
              std::vector<TestCase>& test_cases
        ) : instances() {

    std::random_device rd;
    std::mt19937 rgen(rd());

    for (int i = 0; i < num_instances; i++){
        instances.emplace_back(std::make_unique<MCMCSynth>(proposal_dist,
                                                      max_stages,
                                                      max_types,
                                                      max_inst,
                                                      test_cases,
                                                      rgen()));
        instances.back()->init();
    }
}

ParaMCMC::~ParaMCMC(){
}

void ParaMCMC::get_best_canidate(int n, MCMCSynth& mcmc, MCMCResult& res){

    MCMCResult current_best;
    MCMCResult tmp;
    for (int i = 0; i < n; i++){
        mcmc.gen_next_canidate();
        if(mcmc.get_canidate_valid()){
            tmp = mcmc.get_current_result();
            if (tmp.better_than(current_best)){
                current_best = tmp;
            }
        }
    }

    tmp = mcmc.get_current_result();
    if (tmp.better_than(current_best)){
        current_best = tmp;
    }

    res = current_best;
}

void ParaMCMC::run(){
    std::vector<std::thread> threads;
    std::vector<MCMCResult> results(instances.size());
    for (size_t i = 0; i < instances.size(); i++){
        threads.emplace_back(std::thread(get_best_canidate, 10000000, std::ref(*instances[i]), std::ref(results[i])));
    }
    for (auto& t : threads){
        t.join();
    }

    for (auto& res : results){
        if(res.valid){
            std::cout << res.prob << std::endl;
            dbg_print_prog(res.canidate);
        }
    }
}
