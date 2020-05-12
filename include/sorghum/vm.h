#pragma once

#include <array>
#include <map>
#include <ostream>
#include <stack>
#include <vector>

#include "sorghum/synth.h"

enum class CGAOp {
  pull_N,
  pull_W,
  // 2

  send_S_pop,
  send_S_peek,
  send_S_ifz_peek,
  // 5

  add,  // works on reg
  sub,  // works on reg
  mul,  // works on reg
  mac,  // works on reg
  // 9

  gt,  // works on reg
  eq,  // works on reg
  lt,  // works on reg
  jump1c,
  // 13

  pop,   // works on reg
  push,  // works on reg
  peek,  // works on reg
  zero_push,
  // 17

  inc,  // works on reg
  dec,  // works on reg
  // 19

  zero_reg,  // works on reg
  nop,
  undef,
  // 22
};

const unsigned int CGAOp_NUM = 21;

CGAOp int_to_CGAOp(unsigned int v);

struct CGAInst {
  CGAInst(CGAOp op, std::initializer_list<int> args);
  CGAInst(CGAOp op, std::array<int, 2> &args);

  CGAOp op;
  std::array<int, 2> args;
};

std::ostream &operator<<(std::ostream &out, const CGAOp &op);
std::ostream &operator<<(std::ostream &out, const CGAInst &inst);

enum class CGAIterMode {
  north_len,
  west_len,
  pe_depth,
  pe_depth_inv,
  single,
  undef,
};

const unsigned int CGAIterMode_NUM = 5;

CGAIterMode int_to_CGAIterMode(unsigned int v);

std::ostream &operator<<(std::ostream &out, const CGAIterMode imode);

struct CGAProg {
  std::vector<CGAIterMode> iteration_mode;
  std::vector<std::vector<CGAInst>> stages;
};

class CGAConfig1D {
 public:
  CGAConfig1D(unsigned int height, unsigned int num_progs,
              unsigned int num_w_streams);

  CGAProg &get_prog(int prog) { return progs_[prog]; }

  CGAProg &get_assigned_prog(int pe) { return progs_[pe_assignment_[pe]]; }

  void set_assigned_prog(int pe, int prog) { pe_assignment_[pe] = prog; }

  void set_w_stream(int pe, int w_stream) { pe_w_stream_sel_[pe] = w_stream; }

  int get_w_stream(int pe) { return pe_w_stream_sel_[pe]; }

  int get_assigned_prog_id(int pe) { return pe_assignment_[pe]; }
  int get_assigned_stream_id(int pe) { return pe_w_stream_sel_[pe]; }

  unsigned int num_w_streams;
  unsigned int cga_height;
  unsigned int num_progs;

 private:
  std::vector<CGAProg> progs_;
  std::vector<int> pe_assignment_;
  std::vector<int> pe_w_stream_sel_;
};

struct ProgCursor {
  unsigned int stage;
  unsigned int iidx;
};

class CGAVirt {
 public:
  CGAVirt(int num_regs);
  ~CGAVirt();

  bool check(std::vector<int> &north_input,
             std::vector<std::vector<int>> &west_inputs,
             std::vector<int> &south_output,
             std::vector<std::vector<CGAInst>> &progs);

  bool eval(TestCase &tc, CGAConfig1D &cfg, std::vector<int> &output);

  // configuration
  unsigned int stack_limit = 10;

  // statistics
  unsigned int stat_max_stack = 0;
  unsigned int stat_instr_run = 0;

  void reset_stats();

  bool debug = false;

  const int TOTAL_REGS;

 private:
  void reset_regs();

  std::vector<int> hw_stack;
  std::vector<int> hw_regs;
};
