//
// Copyright (c) 2018, Gigone Lee, Under the MIT License.
//

#ifndef C10KPROBLEM_SELECT_SELECT_IO_H_
#define C10KPROBLEM_SELECT_SELECT_IO_H_

#include <common/c10k_types.h>

#include <csignal>
#include <cstdint>
#include <thread>
#include <map>

#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>

namespace c10k {

class SelectIO {
 public:
  typedef std::function<void (const int &fd)> FileDescriptorHandler;

  SelectIO();

  void RegisterHandler(const FileDescriptorHandler &fd_handler);

  void AddSession(const int &fd);
  void RemoveSession(const int &fd);

  int64_t GetRequestsHandled() const;

  bool Start(const bool &block);
  void Stop();

 private:
  void IOLoop();
  std::thread thread_;
  volatile bool kill_flags_;
  bool block_mode_;
  std::atomic_int64_t requests_handled;

  FileDescriptorHandler fd_handler_;

  std::mutex mutex_;
  int max_fd_;
  fd_set master_set_;
};



}


#endif  // C10KPROBLEM_SELECT_SELECT_IO_H_
