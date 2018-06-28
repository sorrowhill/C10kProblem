//
// Copyright (c) 2018, Gigone Lee, Under the MIT License.
//

#include "poll_io.h"

namespace c10k {

PollIO::PollIO() : max_index_(-1) {
  for (auto &i : master_set_) {
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
    int i = 0;
    for (auto &pfd : master_set_) {
      if (pfd.fd < 0) {
        pfd.fd = fd;
        pfd.events = POLLIN;
        break;
      }
      ++i;
    }

    if (i > max_index_) {
      max_index_ = i;
    }
  }
}


void PollIO::RemoveSession(const int &fd) {
  {
    std::lock_guard<std::mutex> guard(mutex_);
    int i = 0;
    for (auto &pfd : master_set_) {
      if (pfd.fd == fd) {
        pfd.fd = -1;
        break;
      }
      ++i;
    }

    if (i == max_index_) {
      --max_index_;
    }
  }
}


void PollIO::IOLoop() {
  pollfd working_set_[MAX_CONNECTIONS];

  {
    std::lock_guard<std::mutex> guard(mutex_);
    memcpy(working_set_, master_set_, sizeof(pollfd) * MAX_CONNECTIONS);
  }

  int nb_fd = poll(working_set_, MAX_CONNECTIONS, 1000 /* 1000 ms */);
  if (nb_fd <= 0) {
    return;
  }

  for (int i = 0; i <= max_index_; ++i) {
    if (working_set_[i].fd < 0) {
      continue;
    }

    if (working_set_[i].revents & POLLIN) {
      fd_handler_(working_set_[i].fd);
      IncrRequestsHandled();
      master_set_[i].events = POLLIN;
    }
  }
}

}  // namespace c10k