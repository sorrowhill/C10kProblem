//
// Copyright (c) 2018, Gigone Lee, Under the MIT License.
//

#ifndef C10KPROBLEM_POLL_POLL_IO_H_
#define C10KPROBLEM_POLL_POLL_IO_H_

#include <common/c10k_types.h>
#include <interface/virtual_io.h>

#include <sys/poll.h>

#define MAX_CONNECTIONS 10000

namespace c10k {

class PollIO : public VirtualIO {
 public:

  PollIO();

  void RegisterHandler(const FileDescriptorHandler &fd_handler) override;

  void AddSession(const int &fd) override;
  void RemoveSession(const int &fd) override;

 protected:
  void IOLoop() override;

 private:
  FileDescriptorHandler fd_handler_;

  std::mutex mutex_;
  int max_index_;
  pollfd master_set_[MAX_CONNECTIONS];
};

};


#endif  // C10KPROBLEM_POLL_POLL_IO_H_
