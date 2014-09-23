#include <iostream>
#include <set>
#include <string>
#include <vector>
#include <unordered_map>
#include <czmq.h>
#include <string.h>
#include <cstdlib> 
#include <ctime> 
#include <iostream>
#include <queue> 
#include <cstdlib>

using namespace std;

//queue<zframe_t*> myqueue;
typedef unordered_map<string, vector<zframe_t*>> WorkerReferences;
typedef unordered_map<string,queue<zframe_t*>> WorkerReferencess;
unordered_map<string, vector<bool>> states;

//WorkerReferences wr;
WorkerReferencess wr;

// typedef vector<zframe_t*> Identifiers;

// unordered_map<string, Identifiers> W;

// void addIdentity(char* op, zframe_t* id) {
//   string t(op);
//   W[t].push_back(zframe_dup(id));
//   cout << "New Identity! for a worker that performs  " << t << "\n";
//   cout << "Server stats:\n";
//   for (const auto& e : W) {
//     cout << "\tOp " << e.first << ": " << e.second.size() << "\n";
//   }
// }

void registerWorker(zframe_t* id, string operation) {
  zframe_print(id, "Id to add");
  zframe_t* dup = zframe_dup(id);
  zframe_print(dup, "Id copy add");
  wr[operation].push(dup);
  states[operation].push_back(0);
  /*cout << "Worker summary" << endl;
  for (const auto& e : wr) {
    cout << e.first << endl;
    for (zframe_t* id : e.second) {
      char* reprId = zframe_strhex(id);
      cout << reprId << " ";
      free(reprId);
    }
    cout << endl;
  }*/
}

zframe_t* getWorkerFor(string operation) {
	
  // Assumes there are at least one worker registered.
  
  
  //zframe_t* wid = wr[operation];
  //funcion para randoms
  //int l=wr[operation].size();
  // int random= rand()%l;
  // zframe_t* wid = wr[operation][random];
  // zframe_print (wid,"aleatorio");
  
  //funcion para balanceo uniforme
 int l=wr[operation].size();
 // zframe_t* wid2;
zframe_t* wid = wr[operation].front();
	  //zframe_t* wid;
if(l > 0){
  for (int i=0; i < l ;i++){  
  
        for(int j = 0; j < states[operation].size(); j++){
  
	zframe_t* wid2 = wr[operation].back();
    //wr[operation].pop(); 
    //wr[operation].push(wid);
	//zframe_t* wid = wid2;
	//}
  }  
  zframe_print (wid,"balancea");
  return zframe_dup(wid);
}
}
}
void handleClientMessage(zmsg_t* msg, void* workers) {
  cout << "Handling the following message" << endl;
  zmsg_print(msg);

  zframe_t* clientId = zmsg_pop(msg);

  char* operation = zmsg_popstr(msg);
  zframe_t* worker = getWorkerFor(operation);

  // char* reprWId = zframe_strhex(worker);
  // cout << "Selected worker to handle request: " << reprWId << endl;
  // free(reprWId);
  zmsg_pushstr(msg, operation);
  zmsg_prepend(msg, &clientId);
  zmsg_prepend(msg, &worker);

  // Prepare and send the message to the worker
  zmsg_send(&msg, workers);

  cout << "End of handling" << endl;
  // zframe_destroy(&clientId);
  free(operation);
  zmsg_destroy(&msg);
}

void handleWorkerMessage(zmsg_t* msg, void* clients) {
  cout << "Handling the following WORKER" << endl;
  zmsg_print(msg);
  // Retrieve the identity and the operation code
  zframe_t* id = zmsg_pop(msg);
  char* opcode = zmsg_popstr(msg);
  if (strcmp(opcode, "register") == 0) {
    // Get the operation the worker computes
    char* operation = zmsg_popstr(msg);
    // Register the worker in the server state
    registerWorker(id, operation);
    free(operation);
  } else if (strcmp(opcode, "answer") == 0) {
  ////////////
    char* s1 = zmsg_popstr(msg);
    char* s2 = zmsg_popstr(msg);
    string w_type = s1;
    int w_num = atoi(s2);  
       states[w_type][w_num] = false;
  ////////////////////
    zmsg_send(&msg, clients);
  } else {
    cout << "Unhandled message" << endl;
  }
  cout << "End of handling" << endl;
  free(opcode);
  zframe_destroy(&id);
  zmsg_destroy(&msg);
}

int main(void) {
  zctx_t* context = zctx_new();
  // Socket to talk to the workers
  void* workers = zsocket_new(context, ZMQ_ROUTER);
  int workerPort = zsocket_bind(workers, "tcp://*:5555");
  cout << "Listen to workers at: "
       << "localhost:" << workerPort << endl;

  // Socket to talk to the clients
  void* clients = zsocket_new(context, ZMQ_ROUTER);
  int clientPort = zsocket_bind(clients, "tcp://*:4444");
  cout << "Listen to clients at: "
       << "localhost:" << clientPort << endl;

  zmq_pollitem_t items[] = {{workers, 0, ZMQ_POLLIN, 0},
                            {clients, 0, ZMQ_POLLIN, 0}};
  cout << "Listening!" << endl;

  while (true) {
    zmq_poll(items, 2, 10 * ZMQ_POLL_MSEC);
    if (items[0].revents & ZMQ_POLLIN) {
      cerr << "From workers\n";
      zmsg_t* msg = zmsg_recv(workers);
      handleWorkerMessage(msg, clients);
    }
    if (items[1].revents & ZMQ_POLLIN) {
      cerr << "From clients\n";
      zmsg_t* msg = zmsg_recv(clients);
      handleClientMessage(msg, workers);
    }
  }

  // TODO: Destroy the identities

  zctx_destroy(&context);
  return 0;
}
