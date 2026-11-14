#pragma once
#include "models.hpp"
#include "fods.hpp"
#include <vector>
#include <string>
class Cluster {
public:
 bool addNode(const Node&,std::string&); bool failNode(const std::string&); bool removeNode(const std::string&);
 Node* findNode(const std::string&); const std::vector<Node>& nodes()const{return nodes_;} std::vector<Node>& nodes(){return nodes_;}
 void reset(); int capacity()const{return 4;}
private: std::vector<Node> nodes_; FODSLinkedList<std::string> nodeRegistry_; int nextId_{1};
};
