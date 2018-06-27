//
// Copyright (c) 2018, Gigone Lee, Under the MIT License.
//

#include "poll_io.h"

namespace c10k {

PollIO::PollIO() {
  for (auto &i : fd_set_) {
    i.fd = -1;
  }
}

void PollIO::RegisterHandler(
    const FileDescriptorHandler &fd_handler) {
  fd_handler_ = fd_handler;
}


void PollIO::AddSession(const int &fd) {
  {
    std::lock_guard<std::mutex> guard(mutex_);
    for (auto &i : fd_set_) {
      if (i.fd < 0) {
        i.fd = fd;
        i.events = POLLIN;
        break;
      }
    }
  }
}


void PollIO::RemoveSession(const int &fd) {
  {
    std::lock_guard<std::mutex> guard(mutex_);
    for (auto &i : fd_set_) {
      if (i.fd == fd) {
        i.fd = -1;
        break;
      }
    }
  }
}


void PollIO::IOLoop() {
  int nb_fd = poll(fd_set_, MAX_CONNECTIONS, 1000 /* 1000 ms */);
  if (nb_fd <= 0) {
    return;
  }

  for (auto &i : fd_set_) {
    if (i.fd < 0) {
      continue;
    }

    if (i.revents & (POLLIN | POLLERR)) {
      fd_handler_(i.fd);
      IncrRequestsHandled();
      fd_set_[0].events = POLLIN;
    }
  }
}

}  // namespace c10k