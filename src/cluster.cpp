#include "cluster.hpp"
#include <algorithm>
bool Cluster::addNode(const Node& input,std::string& error){
 if(nodes_.size()>=4){error="The initial cluster capacity is four nodes.";return false;}
 if(input.cpuCores<1||input.ramGB<=0||input.gpuMemoryGB<0||input.ramUsedGB<0||input.gpuUsedGB<0){error="Invalid CPU, RAM or GPU capacity.";return false;}
 Node n=input;if(n.id.empty())n.id="N"+std::to_string(nextId_++);if(nodeRegistry_.contains(n.id)){error="Duplicate node ID.";return false;}
 nodes_.push_back(n);nodeRegistry_.pushBack(n.id);return true;
}
bool Cluster::failNode(const std::string&id){Node*n=findNode(id);if(!n)return false;n->status=NodeStatus::Offline;return true;}
bool Cluster::removeNode(const std::string&id){auto it=std::find_if(nodes_.begin(),nodes_.end(),[&](const Node&n){return n.id==id;});if(it==nodes_.end())return false;nodeRegistry_.removeFirst(id);nodes_.erase(it);return true;}
Node* Cluster::findNode(const std::string&id){for(auto&n:nodes_)if(n.id==id)return &n;return nullptr;}
void Cluster::reset(){nodes_.clear();nodeRegistry_.clear();nextId_=1;}
