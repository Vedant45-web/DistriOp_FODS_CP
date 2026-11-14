#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <chrono>

enum class NodeStatus { Idle, Busy, Offline };
enum class JobStatus { Queued, Running, Completed, Failed };
enum class Priority { Low=1, Medium=2, High=3, Critical=4 };
struct Node {
 std::string id,name,gpuModel{"None"},workerToken,activeJob;
 int cpuCores{4}; double ramGB{8},gpuMemoryGB{0};
 double cpuUsedPct{0},ramUsedGB{0},gpuUsedGB{0};
 NodeStatus status{NodeStatus::Idle}; bool live{false},claimed{false}; int completedJobs{0};
 std::int64_t lastHeartbeat{0};
};
struct Job {
 std::string id,name,type{"General"},assignedNode,message,result;
 int cpuRequired{1},estimatedSeconds{5},workUnits{10000};
 double ramRequiredGB{1},gpuRequiredGB{0},waitingSeconds{0},executionSeconds{0};
 Priority priority{Priority::Medium}; JobStatus status{JobStatus::Queued};
 bool claimed{false};
};
struct Metrics {
 int submitted{0},completed{0},queued{0},running{0},failed{0};
 double averageWaitingSeconds{0},averageExecutionSeconds{0},averageUtilizationPct{0},resourceWastagePct{0};
};
