#ifndef EE382V_EPP_RUNTIME_H
#define EE382V_EPP_RUNTIME_H

extern "C" void init_path_reg(int loopId);
extern "C" void inc_path_reg(int loopId, int val);
extern "C" void finalize_path_reg(int loopId);
extern "C" void dump_path_regs();

#endif
