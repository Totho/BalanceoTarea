#include <iostream>
#include <czmq.h>
#include <string.h>

using namespace std;

int main(int argc, char** argv) {
  if (argc != 4) {
    cerr << "Wrong call\n";
    return 1;
  }

  zctx_t* context = zctx_new();
  void* server = zsocket_new(context, ZMQ_DEALER);

  zsocket_connect(server, "tcp://localhost:4444");

  // Send some work to the server
  // argv[1]: {add, sub, mul, div}
  // argv[2]: first operand
  // argv[3]: second operand
  zmsg_t* msg = zmsg_new();
  for (int i = 1; i <= 3; i++)
    zmsg_addstr(msg, argv[i]);
  zmsg_send(&msg, server);

  // Wait for the answer
  cout << "Waiting for answer..." << endl;
  zmq_pollitem_t items[] = {{server, 0, ZMQ_POLLIN, 0}};

  while (true) {
    zmq_poll(items, 1, 10 * ZMQ_POLL_MSEC);
    if (items[0].revents & ZMQ_POLLIN) {
      cout << "Answer!!!:\n";
      zmsg_t* msg = zmsg_recv(server);
      zmsg_print(msg);
      zmsg_destroy(&msg);
      break;
    }
  }
  zctx_destroy(&context);
  return 0;
}
