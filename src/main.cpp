/*
  This file is part of Gargantua...
*/

#include <iostream>

#include "stockfish_probe/nnue_incremental.h"
#include "stockfish_probe/probe.h"

#include "bitboard.h"
#include "crash_recovery.h"
#include "position.h"
#include "search.h"
#include "tbprobe.h"
#include "tt.h"
#include "uci.h"

#include <csignal>
#include <cstdio>
#include <execinfo.h>
#include <unistd.h>

void crash_handler(int sig) {
  // Check if we crashed inside Syzygy probe
  if (in_syzygy_probe) {
    // Jump back to safe point
    siglongjmp(syzygy_jmp_buf, 1);
  }

  void *array[20];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 20);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);

  // Also write to a file for the user/us to check
  FILE *f = fopen("/tmp/gargantua_crash.log", "w");
  if (f) {
    fprintf(f, "Crash signal %d received\n", sig);
    backtrace_symbols_fd(array, size, fileno(f));
    fclose(f);
  }

  exit(1);
}
int main(int argc, char *argv[]) {
  signal(SIGSEGV, crash_handler);
  signal(SIGABRT, crash_handler);
  signal(SIGBUS, crash_handler);

  cout << EngineName << " " << EngineVersion << " by " << EngineAuthor << endl
       << flush;

  initBitboards();
  initRandomKeys();
  initSearch();
  TT::init(1024);

  // Initialize with the networks present in current directory
  Stockfish::Probe::init("nn-b1a57edbea57.nnue", "nn-baff1ede1f90.nnue");
  Stockfish::Incremental::init();

  cout << endl << flush;
  UCI::loop(argc, argv);

  return 0;
}
