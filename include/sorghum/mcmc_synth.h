#pragma once
#include <random>
#include <vector>

#include "sorghum/synth.h"
#include "sorghum/vm.h"

struct MCMCResult {
  CGAConfig1D canidate;
  float prob;
  bool valid;
  int iteration = 0;
  int cycle = 0;

  MCMCResult();
  MCMCResult(CGAConfig1D canidate, float prob, bool valid);

  bool better_than(const MCMCResult& other);
  static bool compare(const MCMCResult& a, const MCMCResult& b);
};

struct MCMCOutGoodness {
  float per_correct_values;
  float per_values_present;
};

struct MCMCCost {
  float correctness;
  float execution;
  bool valid;
};

struct MCMCProposalDist {
  double p_swap;
  double p_insert;
  double p_remove;
  double p_replace;
  double p_inc_stage;
  double p_dec_stage;
  double p_change_imode;
  // new multi-pe-type
  double p_swap_stream;
  double p_swap_pe_type;
};

enum class MCMCxform {
  swap,
  insert,
  remove,
  replace,
  inc_stage,
  dec_stage,
  change_imode,
  swap_stream,
  swap_pe_type
};

class MCMCSynth {
 public:
  MCMCSynth(MCMCProposalDist& proposal_dist, int max_stages, int max_types,
            int max_inst, int num_regs, int num_w_streams, int height,
            std::vector<TestCase>& test_cases, unsigned int seed);
  ~MCMCSynth();

  // initialize the candiate program with a random program
  void init();

  void gen_next_canidate();

  MCMCResult get_current_result();

  float get_canidate_cost() { return canidate_cost; }

  bool get_canidate_valid() { return canidate_valid; }

  CGAConfig1D get_canidate() { return canidate; };

  void set_to_result(MCMCResult& result) {
    canidate = result.canidate;
    canidate_valid = result.valid;
    canidate_cost = result.prob;
  }

 private:
  MCMCOutGoodness compute_output_goodness(TestCase& tc,
                                          std::vector<int>& output);

  MCMCCost unified_cost_fn(TestCase& tc, CGAConfig1D& prog,
                           std::vector<int>& output);

  CGAConfig1D xform_canidate(MCMCxform xform);

  void xform_swap_inst(CGAProg& prog, ProgCursor sel_a, ProgCursor sel_b);
  void xform_replace_inst(CGAProg& prog, ProgCursor sel, CGAInst inst);
  void xform_remove_inst(CGAProg& prog, ProgCursor sel);
  void xform_insert_inst(CGAProg& prog, ProgCursor sel, CGAInst inst);
  void xform_inc_stage(CGAProg& prog, ProgCursor sel);
  void xform_dec_stage(CGAProg& prog, ProgCursor sel);
  void xform_change_imode(CGAProg& prog, ProgCursor sel, CGAIterMode imode);
  void xform_change_stream_assignment(CGAConfig1D& cfg1d, unsigned int pe_id,
                                      int stream_id);
  void xform_change_pe_prog(CGAConfig1D& cfg1d, unsigned int pe_id,
                            unsigned int prog_id);

  // Randomization helper functions
  CGAInst gen_random_inst();
  CGAIterMode gen_random_imode();
  ProgCursor gen_random_cursor(CGAProg& prog);
  CGAProg& sel_random_prog(CGAConfig1D& cfg1d);
  unsigned int sel_random_pe_id(CGAConfig1D& cfg1d);
  unsigned int sel_random_prog_id(CGAConfig1D& cfg1d);
  int sel_random_stream_id(CGAConfig1D& cfg1d);

  // Random Xform Generation
  std::uniform_real_distribution<float> zero_one_dist;
  std::discrete_distribution<unsigned int> pdist;
  std::mt19937 rgen;

  // Configuration
  int max_canidate_stages;
  int max_canidate_types;
  int max_canidate_inst_per_stage;

  // Current canidate info
  float canidate_cost;
  bool canidate_valid;
  CGAConfig1D canidate;

  // Virtual Machine Intstance
  CGAVirt vm;

  // Test cases
  std::vector<TestCase>& test_cases;
};
