#ifndef RPCS_H
#define RPCS_H

#include <math.h>
#include "chord.h"
#include "rpc/client.h"
#include <iostream>
#include <string>
#include <vector>
using std::string;
using std::cout;
using std::endl;
using std::vector;

bool kill = false;

#define DB if (self.id == 1184248810) cout
#define KILL if (kill == true) cout

Node self, successor, predecessor;

Node get_info() { return self; } // Do not modify this line.

const uint64_t RING_BIT = 32;
const uint64_t RING_SIZE = (1UL<<RING_BIT);
const uint64_t dis_base = 28;
uint64_t next = 0;
const int FT_SIZE = 4;
const int SL_SIZE = 4;  // logN
Node fingerTable[FT_SIZE];
vector<Node> sucList(SL_SIZE);
int suc_idx = 0;
bool joined = false;
int repeat_count = 0;

Node getSuccessor() { return successor; }
Node getPredecessor() { return predecessor; }
vector<Node> getSucList() { return sucList; }
bool killed(Node n) {
  try {
    rpc::client client(n.ip, n.port);
    client.call("getPredecessor").as<Node>();
  } catch (std::exception &e) {
    return true;
  }
  return false;
}

void printFT() {
  for (int i = 0; i < FT_SIZE; ++i)
    DB << self.id << "[" << i << "]: " << fingerTable[i].id << endl;
}

bool inRange(uint64_t tar, uint64_t a, uint64_t b) {
  if (a < b) 
    return a < tar && tar <= b;
  else 
    return a < tar || tar <= b;
}

bool inRange_wo_equal(uint64_t tar, uint64_t a, uint64_t b) {
  if (a < b)
    return a < tar && tar < b;
  else
    return a < tar || tar < b;
}

void create() {
  joined = true;
  predecessor.ip = "";
  successor = self;
}

void join(Node n) {
  joined = true;
  predecessor.ip = "";
  rpc::client client(n.ip, n.port);
  successor = client.call("find_successor", self.id).as<Node>();
}

Node closest_preceding_node(uint64_t id) {
  //DB << self.id << ": closest_preceding_node \n";
  //try {
    Node FTnode, SLnode;
    for (int i = FT_SIZE-1; i >= 0; --i) {
      if (fingerTable[i].id != 0 && inRange_wo_equal(fingerTable[i].id, self.id, id)) {
        //return fingerTable[i];
        FTnode = fingerTable[i];
        break;
      }
    }

    try {
      rpc::client client(FTnode.ip, FTnode.port);
      client.call("getPredecessor").as<Node>();
      return FTnode;
    } catch (std::exception &e) {
      for (int i = SL_SIZE-1; i >= 0; --i) {
        if (sucList[i].id != FTnode.id && inRange_wo_equal(sucList[i].id, self.id , id)) {
          try {
            rpc::client client1(sucList[i].ip, sucList[i].port);
            client1.call("getPredecessor").as<Node>();
            return sucList[i];
          } catch (std::exception &e) {
            continue;
          }
          return sucList[i];
        }
      }
    }

    return self;

  /*} catch (std::exception &e) {
    cout << "ERROR: closest_preceding_node" << endl;
  }*/
}

Node find_successor(uint64_t id) {
  // TODO: implement your `find_successor` RPC
  //return self;
  Node n0;
  try {
    //cout << "find_successor: " << self.id << " suc: " << successor.id << endl;
    //cout << self.id << "pre: " << predecessor.id << " suc: " << successor.id << endl;
    if (inRange(id, self.id, successor.id)) {
      return successor;
    }
    else {
      n0 = closest_preceding_node(id);
      if (n0.id == self.id) n0 = successor;
      rpc::client client(n0.ip, n0.port);
      return client.call("find_successor", id).as<Node>();
    }
  } catch (std::exception &e) {
    cout << "ERROR: find_successor " << self.id << endl;
  }
}

// refreshes finger table entries
void fix_fingers() {
  if (joined) {
    try {
      uint64_t dis = 1 << (dis_base+next);
      uint64_t tar = ((self.id) + (dis % RING_SIZE)) % RING_SIZE;
      fingerTable[next] = find_successor(tar);
      //cout << self.id << "[" << tar << "] = " << fingerTable[next].id << endl;
      next = (next+1) % FT_SIZE;
    } catch (std::exception &e) {
      cout << "ERROR: fix_fingers" << endl;
    }
  }
}

void notify(Node n0) {
  //cout << self.port << " notify " << n0.port << endl;
  try {
    if (predecessor.ip == "" || inRange_wo_equal(n0.id, predecessor.id, self.id)) {
      cout << "*********** " << self.id << " predecessor update from " << predecessor.id << " to " << n0.id << endl;
      predecessor = n0;
    }
  } catch (std::exception &e) {
    cout << "ERROR: notify" << endl;
  }
}

void stabilize() {
  if (joined) {
    try {
      //cout << self.id << ": pre: " << predecessor.id << " suc: " << successor.id << endl;
      Node x, test;
      try {
        //DB << "stabilize: " << self.id << " suc: " << successor.ip << ", " << successor.port << endl;
        rpc::client oldSuc1(successor.ip, successor.port); // work
        x = oldSuc1.call("getPredecessor").as<Node>(); // not work
      } catch (std::exception &e) {
        suc_idx = (suc_idx+1) % SL_SIZE;
        cout << "*********** " << self.id << " successor update from " << successor.id << " to " << sucList[suc_idx].id << endl;
        successor = sucList[suc_idx];
        try {
          rpc::client oldSuc2(successor.ip, successor.port); // work
          test = oldSuc2.call("getPredecessor").as<Node>(); // not work
        } catch (std::exception &e) {
          suc_idx = (suc_idx+1) % SL_SIZE;
          cout << "*********** " << self.id << " successor update from " << successor.id << " to " << sucList[suc_idx].id << endl;
          successor = sucList[suc_idx];
        }
      }
      if (x.ip != "" && inRange_wo_equal(x.id, self.id, successor.id)) {
        successor = x;
      }
      rpc::client newSuc(successor.ip, successor.port);
      newSuc.call("notify", self);

      vector<Node> suc_sucList = newSuc.call("getSucList").as<vector<Node>>();
      sucList[0] = successor;
      std::copy(suc_sucList.begin(), suc_sucList.end()-1, sucList.begin()+1);
      suc_idx = 0;

    } catch (std::exception &e) {
      cout << "ERROR: stabilize" << endl;
    }
  }
}

void check_predecessor() {
  //DB << self.id << ": check_predecessor \n";
  try {
    rpc::client client(predecessor.ip, predecessor.port);
    Node n = client.call("get_info").as<Node>();
  } catch (std::exception &e) {
    predecessor.ip = "";
  }
}

void register_rpcs() {
  add_rpc("get_info", &get_info); // Do not modify this line.
  add_rpc("create", &create);
  add_rpc("join", &join);
  add_rpc("find_successor", &find_successor);
  add_rpc("notify", &notify);
  add_rpc("getSuccessor", &getSuccessor);
  add_rpc("getPredecessor", &getPredecessor);
  add_rpc("getSucList", &getSucList);
}

void register_periodics() {
  add_periodic(check_predecessor);
  add_periodic(stabilize);
  add_periodic(fix_fingers);
}

#endif /* RPCS_H */
