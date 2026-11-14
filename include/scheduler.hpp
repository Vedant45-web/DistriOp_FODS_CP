#pragma once
#include "models.hpp"
#include "cluster.hpp"
#include <vector>
#include <string>
class Scheduler {
public:
 void submit(Job); void schedule(Cluster&,const std::string&,const std::string& mode);
 bool completeRemote(const std::string&jobId,const std::string&nodeId,bool success,const std::string&result,double seconds,Cluster&);
 void recoverNode(const std::string&,Cluster&,const std::string&strategy);
 const std::vector<Job>&jobs()const{return jobs_;} std::vector<Job>&jobs(){return jobs_;}
 Metrics metrics(const Cluster&)const; Job* nextForWorker(const std::string&nodeId); void reset();
private: std::vector<Job>jobs_;int nextId_{1}; Node* chooseNode(Cluster&,const Job&,const std::string&,bool liveOnly);
 void release(Job&,Cluster&);
};
