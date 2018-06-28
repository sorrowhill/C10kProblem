//
// Copyright (c) 2018, Gigone Lee, Under the MIT License.
//

#include <common/c10k_types.h>

#include <thread>

#include "select/select_io.h"
#include "poll/poll_io.h"

std::shared_ptr<c10k::VirtualIO> the_io;

int main(int argc, char *argv[]) {
  const uint16_t port = 8040;

  the_io = std::make_shared<c10k::SelectIO>();

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
  std::thread([&](){
    int elapsed = 1;
    while(true) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      int64_t requests_handled = the_io->GetRequestsHandled();
      LOG(INFO) << "nb_of_messages_sent=" << requests_handled
                << ", elapsed=" << elapsed
                << ", TPS=" << requests_handled / elapsed;
      ++elapsed;
    }
  }).detach();
#pragma clang diagnostic pop

  int listen_fd;
  char buffer[BUFFER_SIZE];

  listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd < 0) {
    LOG(ERROR) << "Socket error";
    return 1;
  }

  sockaddr_in server_address{};
  sockaddr_in client_address{};
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = htonl(INADDR_ANY);
  server_address.sin_port = htons(port);

  socklen_t address_len = sizeof(client_address);

  if (bind(listen_fd,
           (struct sockaddr *)&server_address,
           sizeof(server_address)) < 0) {
    LOG(ERROR) << "Bind error";
    return 1;
  }

  if (listen(listen_fd, 5) < 0) {
    LOG(ERROR) << "Bind error";
    return 1;
  }

  the_io->RegisterHandler([&](const int &fd) {
    if (fd == listen_fd) {
      int client_fd =
          accept(listen_fd, (sockaddr *) &client_address, &address_len);
      the_io->AddSession(client_fd);
      return;
    }

    // handle client socket
    auto bytes_read = read(fd, buffer, BUFFER_SIZE);
    if (bytes_read == 0) {
      the_io->RemoveSession(fd);
      close(fd);
      return;
    }

    write(fd, buffer, bytes_read);
  });

  the_io->AddSession(listen_fd);
  the_io->Start(true /*block*/);

  LOG(INFO) << "Echo server has been terminated";
  return 0;
}