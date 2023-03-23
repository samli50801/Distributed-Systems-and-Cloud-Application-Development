#ifndef RPCS_H
#define RPCS_H

#include <math.h>
#include "chord.h"
#include "rpc/client.h"
#include <iostream>
#include <string>
using std::string;
using std::cout;
using std::endl;

#define DB if (self.id == 1809317512) cout

Node self, successor, predecessor;

Node get_info() { return self; } // Do not modify this line.

const uint64_t RING_BIT = 32;
const uint64_t RING_SIZE = (1UL<<RING_BIT);
const uint64_t dis_base = 28;
uint64_t next = 0;
const int FT_SIZE = 4;
Node fingerTable[FT_SIZE];
bool joined = false;

Node getSuccessor() { return successor; }
Node getPredecessor() { return predecessor; }
//void initFT() { for (size_t i = 0; i < FT_SIZE; ++i) fingerTable[i].id = 0; }
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
    for (int i = FT_SIZE-1; i >= 0; --i) {
      if (fingerTable[i].id != 0 && inRange_wo_equal(fingerTable[i].id, self.id, id)) {
        return fingerTable[i];
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
    cout << "ERROR: find_successor" << endl;
    cout << self.port << ": " << n0.ip << ", " << n0.port << endl;
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
      predecessor = n0;
    }
  } catch (std::exception &e) {
    cout << "ERROR: notify" << endl;
    cout << self.port << ": " << successor.ip << ", " << successor.port << endl;
  }
}

void stabilize() {
  //DB << self.id << ": stabilize " << joined << endl;
  if (joined) {
    //cout << self.id << ": pre: " << predecessor.id << " suc: " << successor.id << endl;  
    try {
      //cout << self.id << ": pre: " << predecessor.id << " suc: " << successor.id << " (stabilize" << endl;  
      rpc::client oldSuc(successor.ip, successor.port);
      Node x = oldSuc.call("getPredecessor").as<Node>();
      if (x.ip != "" && inRange_wo_equal(x.id, self.id, successor.id)) {
        successor = x;
      }
      rpc::client newSuc(successor.ip, successor.port);
      newSuc.call("notify", self);
    } catch (std::exception &e) {
      cout << "---------------------------------------------\n";
      cout << "ERROR: stabilize" << endl;
      cout << self.id << ": pre: " << predecessor.id << " suc: " << successor.id << endl;
      cout << self.port << ": " << successor.ip << ", " << successor.port << endl;
      cout << "---------------------------------------------\n";
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
}

void register_periodics() {
  add_periodic(check_predecessor);
  add_periodic(stabilize);
  add_periodic(fix_fingers);
}

#endif /* RPCS_H */
