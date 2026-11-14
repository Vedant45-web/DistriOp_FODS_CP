#include "storage.hpp"
#include <fstream>
void saveSnapshot(const std::vector<Node>& nodes,const std::vector<Job>& jobs){std::ofstream f("distriopt_snapshot.txt",std::ios::trunc);if(!f)return;f<<"DistriOpt snapshot\nNodes: "<<nodes.size()<<"\nJobs: "<<jobs.size()<<"\n";for(const auto&n:nodes)f<<n.id<<","<<n.name<<","<<n.cpuCores<<","<<n.ramGB<<","<<n.gpuMemoryGB<<"\n";for(const auto&j:jobs)f<<j.id<<","<<j.name<<","<<static_cast<int>(j.status)<<","<<j.assignedNode<<"\n";}
