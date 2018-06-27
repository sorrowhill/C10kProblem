//
// Copyright (c) 2018, Gigone Lee, Under the MIT License.
//

#ifndef C10KPROBLEM_SELECT_SELECT_IO_H_
#define C10KPROBLEM_SELECT_SELECT_IO_H_

#include <common/c10k_types.h>
#include <interface/virtual_io.h>

#include <csignal>
#include <cstdint>
#include <thread>
#include <map>

#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>

namespace c10k {

class SelectIO : public VirtualIO {
 public:

  SelectIO();

  void RegisterHandler(const FileDescriptorHandler &fd_handler) override;

  void AddSession(const int &fd) override;
  void RemoveSession(const int &fd) override;

 protected:
  void IOLoop() override;
 private:
  FileDescriptorHandler fd_handler_;

  std::mutex mutex_;
  int max_fd_;
  fd_set master_set_;
};



}


#endif  // C10KPROBLEM_SELECT_SELECT_IO_H_
