//
// Copyright (c) 2018, Gigone Lee, Under the MIT License.
//

#ifndef C10KPROBLEM_INTERFACE_VIRTUAL_IO_H_
#define C10KPROBLEM_INTERFACE_VIRTUAL_IO_H_

#include <common/c10k_types.h>

#include <thread>

namespace c10k {

class VirtualIO {
 public:
  typedef std::function<void (const int &fd)> FileDescriptorHandler;

  VirtualIO() : kill_flags_(false), block_mode_(false), requests_handled(0) {
  }

  virtual void RegisterHandler(const FileDescriptorHandler &fd_handler) = 0;

  virtual void AddSession(const int &fd) = 0;
  virtual void RemoveSession(const int &fd) = 0;

  int64_t GetRequestsHandled() const {
    return requests_handled;
  }

  void IncrRequestsHandled() {
    ++requests_handled;
  }

  bool Start(const bool &block) {
    kill_flags_ = false;
    try {
      thread_ = std::thread([&](){
        while (not kill_flags_) {
          IOLoop();
        }
      });
    } catch (std::exception &e) {
      return false;
    }

    block_mode_ = true;
    if (block_mode_) {
      thread_.join();
    }

    return true;
  }

  void Stop () {
    kill_flags_ = true;
    if (not block_mode_ && thread_.joinable()) {
      thread_.join();
    }
  }

 protected:
  virtual void IOLoop() = 0;
 private:
  std::thread thread_;
  volatile bool kill_flags_;
  bool block_mode_;
  std::atomic_int64_t requests_handled;
};

}  // namespace c10k


#endif  // C10KPROBLEM_INTERFACE_VIRTUAL_IO_H_
