#ifdef NDEBUG
#undef NDEBUG
#endif
#include "fods.hpp"
#include "cluster.hpp"
#include "scheduler.hpp"
#include <cassert>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
int main(){
 FODSArray<int>a;a.push(4);a.push(7);assert(a.size()==2&&a.at(1)==7);
 FODSLinkedList<int>list;list.pushBack(2);list.pushBack(5);assert(list.contains(5));assert(list.removeFirst(2));assert(!list.contains(2));
 FODSStack<int>st;st.push(3);st.push(9);assert(st.pop()==9);
 FODSQueue<int>q;q.push(1);q.push(2);assert(q.pop()==1);
 FODSPriorityQueue<int>pq;pq.push(10,1);pq.push(20,4);assert(pq.pop()==20);
 std::vector<int>v={9,1,6,2,4};bubbleSort(v);assert((v==std::vector<int>{1,2,4,6,9}));assert(binarySearch(v,4)==2);assert(linearSearch(v,9)==4);
 std::vector<int>w={8,3,7,1};insertionSort(w);assert((w==std::vector<int>{1,3,7,8}));std::vector<int>x={7,2,5,1};selectionSort(x);assert((x==std::vector<int>{1,2,5,7}));std::vector<int>y={9,4,2,8,1};quickSort(y,0,(int)y.size()-1);assert((y==std::vector<int>{1,2,4,8,9}));std::vector<int>z={6,4,3,9,1};mergeSort(z);assert((z==std::vector<int>{1,3,4,6,9}));
 std::vector<int>sh={12,34,54,2,3};shellSort(sh);assert((sh==std::vector<int>{2,3,12,34,54}));std::vector<int>ct={-2,5,0,-2,3};countingSort(ct);assert((ct==std::vector<int>{-2,-2,0,3,5}));std::vector<int>rd={170,45,75,90,802,24,2,66};radixSort(rd);assert((rd==std::vector<int>{2,24,45,66,75,90,170,802}));std::vector<double>bk={.42,.32,.23,.52,.25,.47,.51};bucketSort(bk);assert(std::is_sorted(bk.begin(),bk.end()));
 std::vector<int>sorted={1,3,5,7,9,11,13,15,17,19};assert(sentinelSearch(sorted,11)==5);assert(fibonacciSearch(sorted,15)==7);assert(indexedSequentialSearch(sorted,17)==8);
 Cluster c;std::string err;Node n;n.name="test-node";n.cpuCores=4;n.ramGB=8;assert(c.addNode(n,err));assert(c.nodes().size()==1);assert(c.findNode("N1")!=nullptr);
 Scheduler sched;Job j;j.name="unit-test-job";j.cpuRequired=2;j.ramRequiredGB=2;sched.submit(j);sched.schedule(c,"DistriOpt","Manual");assert(sched.jobs().front().status==JobStatus::Completed);assert(sched.metrics(c).completed==1);
 Cluster live;Node worker;worker.name="worker";worker.live=true;worker.workerToken="token-token-token";worker.cpuCores=4;worker.ramGB=8;assert(live.addNode(worker,err));Node worker2=worker;worker2.id.clear();worker2.name="worker-two";assert(live.addNode(worker2,err));Scheduler remote;Job r;r.name="remote-test";remote.submit(r);remote.schedule(live,"DistriOpt","Live");assert(remote.jobs().front().status==JobStatus::Running);Job*claimed=remote.nextForWorker("N1");assert(claimed!=nullptr);remote.recoverNode("N1",live,"DistriOpt");assert(remote.jobs().front().status==JobStatus::Running);assert(remote.jobs().front().assignedNode=="N2");Job*claimedAgain=remote.nextForWorker("N2");assert(claimedAgain!=nullptr);assert(remote.completeRemote(claimedAgain->id,"N2",true,"ok",0.02,live));assert(remote.metrics(live).completed==1);
 Cluster gpuCluster;Node gpuWorker;gpuWorker.name="gpu-worker";gpuWorker.live=true;gpuWorker.cpuCores=8;gpuWorker.ramGB=16;gpuWorker.gpuMemoryGB=8;assert(gpuCluster.addNode(gpuWorker,err));Scheduler gpuScheduler;Job gpuJob;gpuJob.name="gpu-only";gpuJob.gpuRequiredGB=2;gpuScheduler.submit(gpuJob);gpuScheduler.schedule(gpuCluster,"DistriOpt","Live");assert(gpuScheduler.jobs().front().status==JobStatus::Queued);
 std::cout<<"All DistriOpt core tests passed.\n";return 0;
}
