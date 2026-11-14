# DistriOpt — Short Project Presentation Guide

## 1. Project in one sentence

DistriOpt is a C++17 workload scheduling prototype that chooses a suitable computer for each computational job based on CPU, RAM, GPU-memory requirements and current load, with both virtual-cluster simulation and live worker-laptop modes.

## 2. Problem statement

When multiple computational or AI-related jobs compete for limited computing resources, assigning every job without checking node capacity can overload some machines while other machines remain underused. A scheduler needs to consider job requirements, node capabilities, job priority and failures.

## 3. Our solution

DistriOpt maintains a cluster of up to four worker nodes. It accepts workloads, filters nodes that cannot satisfy the requested resources, and selects a suitable node using FCFS, Priority or our weighted DistriOpt heuristic. It can simulate a cluster or communicate with real worker laptops over a local network.

## 4. What makes the prototype demonstrable

- Manual mode: add virtual nodes or load four demo nodes and twelve sample jobs.
- Strategy comparison: run the same 300-job scheduler benchmark with FCFS, Priority and DistriOpt; the dashboard shows the measured scheduler routine time and completion/queue counts.
- Live mode: start the C++ Worker Agent on a laptop, register its hardware, submit a supported CPU workload and see the worker return its result.
- Failure handling: disconnect a worker; after heartbeat timeout, the controller marks it offline and requeues restartable work to another eligible worker if one is available.
- FODS: custom data structures and search/sort algorithms are included and unit-tested.

## 5. Two-minute demonstration order

1. Open `http://localhost:8080` after running `BUILD_AND_RUN.bat`.
2. Click **Load demo environment**. Point out the four virtual nodes, twelve jobs and session metrics.
3. Change the strategy between FCFS, Priority and DistriOpt. Submit a workload with different resource requirements.
4. Click **Run algorithm benchmark** and explain that the displayed milliseconds are measured scheduler routine time, not AI model execution time.
5. Reset the session, switch to **Live workers**, and start `RUN_WORKER.bat` on the controller laptop itself using `127.0.0.1` or connect another laptop using the controller's LAN IPv4 address.
6. Submit a CPU workload such as Matrix Computation or AI Inference. Show the job move from Running to Completed and its worker result (hover over the status label for the execution message/result).
7. Explain the failure recovery and security restrictions.

If a second laptop is not available during the presentation, the manual simulation remains a complete demonstration of resource-aware scheduling. Do not present manual virtual nodes as physical computers.

## 6. Architecture explanation

- **Frontend:** HTML, CSS and JavaScript dashboard.
- **Controller:** C++17 HTTP server and central state manager.
- **Cluster manager:** registers virtual nodes and live worker agents.
- **Scheduler:** FCFS, Priority and DistriOpt placement strategies.
- **FODS layer:** custom array, linked list, stack, queue, priority queue, searching and sorting implementations.
- **Worker agent:** detects machine capabilities, sends heartbeats, receives an approved workload and reports its result.

## 7. Honest limitations to mention

This is a second-year academic prototype. Live remote execution uses built-in CPU workloads; NVIDIA GPU information is detected but CUDA execution is not implemented. It does not distribute one AI model across multiple GPUs, train a production AI model, use cloud-scale orchestration or provide production-grade TLS/security. If a worker fails during a job, the job restarts from the beginning rather than resuming from a checkpoint.
