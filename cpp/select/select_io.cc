//
// Copyright (c) 2018, Gigone Lee, Under the MIT License.
//

#include <mutex>
#include <map>

#include "select_io.h"

namespace c10k {

SelectIO::SelectIO()
    : max_fd_(-1) {
  FD_ZERO(&master_set_);
}


void SelectIO::RegisterHandler(
    const FileDescriptorHandler &fd_handler) {
  fd_handler_ = fd_handler;
}


void SelectIO::AddSession(const int &fd) {
  {
    std::lock_guard<std::mutex> guard(mutex_);
    if (fd > max_fd_) {
      max_fd_ = fd;
    }

    FD_SET(fd, &master_set_);
  }
}


void SelectIO::RemoveSession(const int &fd) {
  {
    std::lock_guard<std::mutex> guard(mutex_);
    FD_CLR(fd, &master_set_);
  }
}


void SelectIO::IOLoop() {
  fd_set working_set = master_set_;

  int nb_fd = select(max_fd_ + 1, &working_set, nullptr, nullptr, nullptr);
  if (nb_fd < 0) {
    Stop();
    return;
  }

  for (int current_fd = 0; current_fd <= max_fd_; ++current_fd) {
    if (FD_ISSET(current_fd, &working_set)) {
      fd_handler_(current_fd);
      IncrRequestsHandled();
      if (--nb_fd <= 0) {
        return;
      }
    }
  }
}

}