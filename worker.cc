#include <iostream>
#include <czmq.h>
#include <string.h>

using namespace std;

int operate(char* operation, char* op1, char* op2) {
  int o1 = atoi(op1);
  int o2 = atoi(op2);
  if (strcmp(operation, "ADD") == 0)
    return o1 + o2;
  if (strcmp(operation, "SUB") == 0)
    return o1 - o2;
  if (strcmp(operation, "MUL") == 0)
    return o1 * o2;
  if (strcmp(operation, "DIV") == 0)
    return o1 / o2;
  return 0;
}

void handleServerMessage(zmsg_t* msg, void* server) {
  cout << "Handling the following from SERVER" << endl;
  zmsg_print(msg);

  zframe_t* clientId = zmsg_pop(msg);

  // TODO: check that  the operation is the correct one for this worker
  char* operation = zmsg_popstr(msg);
  char* op1 = zmsg_popstr(msg);
  char* op2 = zmsg_popstr(msg);
  int res = operate(operation, op1, op2);
  cout << "Result: " << res << endl;

  zmsg_t* result = zmsg_new();

  zmsg_addstr(msg, "answer");
  zmsg_append(msg, &clientId);
  zmsg_addstrf(msg, "%d", res);
  zmsg_send(&msg, server);

  cout << "End of handling" << endl;
  free(operation);
  free(op1);
  free(op2);
  zmsg_destroy(&msg);
}

int main(int argc, char** argv) {
  cout << "Worker agent starting..." << endl;
  if (argc != 2) {
    cerr << "Wrong call\n";
    return 1;
  }

  zctx_t* context = zctx_new();
  void* server = zsocket_new(context, ZMQ_DEALER);

  int c = zsocket_connect(server, "tcp://localhost:5555");
  cout << "connecting to server: " << (c == 0 ? "OK" : "ERROR") << endl;

  // Register the worker with the server
  zmsg_t* regmsg = zmsg_new();
  zmsg_addstr(regmsg, "register");
  zmsg_addstr(regmsg, argv[1]);
  zmsg_send(&regmsg, server);
  // zstr_send(server, argv[1]);

  zmq_pollitem_t items[] = {{server, 0, ZMQ_POLLIN, 0}};

  while (true) {
    zmq_poll(items, 1, 10 * ZMQ_POLL_MSEC);
    if (items[0].revents & ZMQ_POLLIN) {
      zmsg_t* msg = zmsg_recv(server);
      handleServerMessage(msg, server);
    }
  }
  zctx_destroy(&context);
  return 0;
}
