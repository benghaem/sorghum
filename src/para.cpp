#include <random>
#include <thread>
#include <iostream>
#include <algorithm>
#include <cassert>
#include "sorghum/util.h"
#include "sorghum/para.h"

ParaMCMC::ParaMCMC(int num_instances,
              MCMCProposalDist &proposal_dist,
              int max_stages,
              int max_types,
              int max_inst,
              int num_regs,
              int height,
              int num_w_streams,
              std::vector<TestCase>& test_cases
        ) : vm_num_regs(num_regs), tcs(test_cases), instances()  {

    std::random_device rd;
    std::mt19937 rgen(rd());

    for (auto& tc : test_cases){
        assert(tc.west_inputs.size() == (unsigned long)num_w_streams);
    }

    for (int i = 0; i < num_instances; i++){
        instances.emplace_back(std::make_unique<MCMCSynth>(proposal_dist,
                                                      max_stages,
                                                      max_types,
                                                      max_inst,
                                                      vm_num_regs,
                                                      height,
                                                      num_w_streams,
                                                      test_cases,
                                                      rgen()));
        instances.back()->init();
    }
}

ParaMCMC::~ParaMCMC(){
}

void ParaMCMC::get_best_canidate(int n, int cycle, MCMCSynth& mcmc, MCMCResult& res){

    MCMCResult current_best = mcmc.get_current_result();
    MCMCResult tmp;
    for (int i = 0; i < n; i++){
        mcmc.gen_next_canidate();
        if(mcmc.get_canidate_valid()){
            tmp = mcmc.get_current_result();
            tmp.iteration = i;
            tmp.cycle = cycle;
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

void ParaMCMC::run(int cycles){
    std::vector<std::thread> threads;
    std::vector<MCMCResult> results(instances.size());

    for (int c = 0; c < cycles; c++){
        threads.clear();
        for (size_t i = 0; i < instances.size(); i++){
            threads.emplace_back(std::thread(get_best_canidate, 5000000, c, std::ref(*instances[i]), std::ref(results[i])));
        }
        for (auto& t : threads){
            t.join();
        }

        std::sort(results.begin(), results.end(), MCMCResult::compare);

        std::cout << "CYCLE " << c << "/" << cycles << std::endl;
        for (auto& res : results){
            std::cout << "---------" << std::endl;
            std::cout << "valid: " << res.valid << ", prob: " << res.prob << std::endl;
            std::cout << "iteration: " << res.iteration << ", cycle: " << res.cycle << std::endl;
            dbg_print_cfg1d(res.canidate);
            for (auto& tc : tcs){
                CGAVirt vm(vm_num_regs);
                std::vector<int> out;
                vm.eval(tc,
                        res.canidate,
                        out);
                std::cout << "ref: " << std::endl;
                dbg_print_vec(tc.south_output);
                std::cout << "can: " << std::endl;
                dbg_print_vec(out);
            }

        }

        //set all of the instances to the best result
        for (auto& instance : instances){
            instance->set_to_result(results.back());
        }
    }
}
