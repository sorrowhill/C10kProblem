//
// Copyright (c) 2018, Gigone Lee, Under the MIT License.
//

#include <common/c10k_types.h>
#include <common/c10k_memory.h>

#include <boost/lexical_cast.hpp>

#include <netdb.h>
#include <thread>

#include "select/select_io.h"

std::shared_ptr<c10k::VirtualIO> the_io;

int main(int argc, char *argv[]) {
  if (argc < 2) {
    LOG(ERROR) << "Usage: " << argv[0] << " <port>";
    return 1;
  }

  uint16_t remote_port = 0;

  try {
    remote_port = boost::lexical_cast<uint16_t>(argv[1]);
  } catch (boost::bad_lexical_cast &e){
    LOG(INFO) << "e: " << e.what();
    return 1;
  }

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

  int socket_fd;
  char buffer[BUFFER_SIZE];

  LOG_ASSERT(remote_port != 0);

  struct hostent *host = gethostbyname("127.0.0.1");

  sockaddr_in client_address{};
  client_address.sin_family = AF_INET;
  client_address.sin_addr = *((struct in_addr *)host->h_addr);;
  client_address.sin_port = htons(remote_port);

  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0) {
    LOG(ERROR) << "Socket error";
    return 1;
  }

  LOG(INFO) << "Connecting...";
  int ret = connect(socket_fd,
                    (struct sockaddr *)&client_address,
                    sizeof(struct sockaddr));
  if (ret != 0) {
    LOG(ERROR) << "Connect error";
    return 1;
  }


  int index = 0;


  the_io->RegisterHandler([&](const int &fd){
    auto bytes_read = read(fd, buffer, BUFFER_SIZE);
    if (bytes_read == 0) {
      the_io->RemoveSession(fd);
      close(fd);
      return;
    }

    ++index;
    snprintf(buffer, BUFFER_SIZE, "message-%d", index);
    write(fd, buffer, bytes_read);
  });

  the_io->AddSession(socket_fd);

  snprintf(buffer, BUFFER_SIZE, "message-%d", index);
  write(socket_fd, buffer, BUFFER_SIZE);

  the_io->Start(true /*block*/);

  LOG(INFO) << "Echo client has been terminated";
  return 0;
}