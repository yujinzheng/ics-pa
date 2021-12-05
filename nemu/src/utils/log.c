#include <common.h>

extern uint64_t g_nr_guest_instr;
FILE *log_fp = NULL;
FILE *ftrace_log_fp = NULL;

void init_log(const char *log_file) {
  log_fp = stdout;
  if (log_file != NULL) {
    FILE *fp = fopen(log_file, "w");
    Assert(fp, "Can not open '%s'", log_file);
    log_fp = fp;
  }
  Log("Log is written to %s", log_file ? log_file : "stdout");
}

bool log_enable() {
  return MUXDEF(CONFIG_TRACE, (g_nr_guest_instr >= CONFIG_TRACE_START) &&
         (g_nr_guest_instr <= CONFIG_TRACE_END), false);
}

void init_ftrace_log(const char *log_file) {
//    ftrace_log_fp = stdout;
    if (log_file != NULL) {
        FILE *fp = fopen(log_file, "w");
        Assert(fp, "Can not open '%s'", log_file);
        ftrace_log_fp = fp;
    }
    Log("Ftrace log is written to %s", log_file ? log_file : "stdout");
}

bool ftrace_log_enable() {
    return MUXDEF(CONFIG_ITRACE_FUN, (g_nr_guest_instr >= CONFIG_TRACE_START) &&
                                (g_nr_guest_instr <= CONFIG_TRACE_END), false);
}