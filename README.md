# DistriOpt — Distributed AI Workload Optimization

**DistriOpt** is a C++17 academic prototype that schedules compute workloads across a small cluster of heterogeneous computers. It demonstrates resource-aware placement, FODS data structures and algorithms, a manual cluster simulator, live worker registration over a local network, allow-listed remote CPU workload execution, failure detection, recovery and a browser dashboard.

## Project status

The agreed academic MVP scope is implemented in this source package:

- C++17 controller and HTTP API
- Web dashboard served by the C++ backend
- Manual virtual cluster (up to four nodes)
- Live worker-agent registration (up to four nodes total)
- CPU/RAM hardware discovery and NVIDIA GPU/VRAM telemetry when `nvidia-smi` is installed
- FCFS, Priority and DistriOpt resource-aware scheduling
- Resource feasibility checks and resource reservation/release
- Allow-listed remote CPU workloads: synthetic neural-network inference, matrix multiplication, array/data sorting and image-like pixel filtering
- Worker heartbeat and offline detection
- Requeue/restart of in-progress work after worker loss
- Performance metrics and repeatable scheduler runtime benchmark
- Custom FODS data structures, searching and sorting algorithms
- Core tests and Windows build/run scripts

The project is complete as an **academic prototype/MVP**, not a production distributed AI platform. The boundaries below are deliberate and must be described honestly in a presentation.

## Architecture

```text
                  Browser dashboard
                 HTML · CSS · JavaScript
                          |
                          | HTTP / JSON
                          v
              C++17 DistriOpt Controller
        +-----------------+------------------+
        |                 |                  |
   FODS library      Job Scheduler       Cluster Manager
        |                 |                  |
  Arrays, lists,     FCFS / Priority /    Manual virtual nodes
  queue, heap,       DistriOpt scoring    or live worker nodes
  stack, search,                          (maximum 4)
  sorting                                      |
                                               | HTTP over LAN
                           +-------------------+-------------------+
                           |                   |                   |
                     Worker Agent 1      Worker Agent 2      Worker Agent 3/4
                     Hardware report     Heartbeat           Approved CPU jobs
```

The controller and worker agents communicate using HTTP/JSON over the local network. Each worker generates a per-process token at registration; subsequent heartbeat, claim and result requests must present it. Workers execute only built-in workload types. The system never sends arbitrary shell commands to a worker.

## Folder structure

```text
DistriOpt/
  include/
    models.hpp
    cluster.hpp
    scheduler.hpp
    http_server.hpp
    json_utils.hpp
    storage.hpp
    fods.hpp
  src/
    main.cpp
    http_server.cpp
    cluster.cpp
    scheduler.cpp
    storage.cpp
    worker_agent.cpp
  frontend/
    index.html
    style.css
    app.js
  tests/
    core_tests.cpp
  CMakeLists.txt
  BUILD_AND_RUN.bat
  BUILD_WORKER.bat
  RUN_WORKER.bat
  RUN_TESTS.bat
  SHOW_LOCAL_IP.bat
```

## Requirements

- Windows 10/11 (recommended for the team demonstration), macOS or Linux
- CMake 3.16 or newer
- A C++17 compiler (Visual Studio 2022 C++ workload, GCC or Clang)
- For live mode: laptops on a mutually reachable LAN/Wi-Fi network
- Optional: NVIDIA driver and `nvidia-smi` for NVIDIA GPU name/VRAM telemetry

No third-party C++ library is required. The frontend uses browser-native HTML/CSS/JavaScript and does not require Node.js or a package installation.

## Run the controller and dashboard on Windows

1. Extract the project folder.
2. Install Visual Studio 2022 with **Desktop development with C++** and CMake.
3. Double-click `BUILD_AND_RUN.bat`.
4. Wait for CMake to build `distriopt_server`, `distriopt_worker` and `distriopt_tests`.
5. The script opens `http://localhost:8080` in your browser. If it does not, open that address manually.
6. Keep the controller terminal window open while using the dashboard. Do not double-click `frontend/index.html`; that opens a local file without the C++ backend, so API features will not work. The stylesheet now also uses relative paths so the visual design can load when previewing the HTML file directly, but the actual application must be opened through the running server.

The server listens on all network interfaces so worker laptops can connect. The dashboard itself is available on `http://localhost:8080` on the controller laptop.

## Manual mode demonstration

1. Select **Manual simulation**.
2. Click **Load demo environment** to load four virtual nodes and twelve sample workloads, or add your own virtual nodes.
3. Change between **FCFS**, **Priority** and **DistriOpt**.
4. Submit workloads with different CPU, RAM, GPU-memory and priority requirements.
5. Simulate a node failure and observe reassignment/re-evaluation.
6. Run the algorithm benchmark to compare measured scheduler routine time and completion/queue counts on the same node configuration.

Manual mode simulates resource allocation and job completion. Virtual nodes do not represent actual machines and do not execute those workloads on physical hardware.

## Live mode demonstration

### On the controller laptop

1. Start DistriOpt using `BUILD_AND_RUN.bat`.
2. Open the dashboard and select **Live workers**.
3. Run `SHOW_LOCAL_IP.bat` or `ipconfig` in Command Prompt.
4. Find the IPv4 address under the active Wi-Fi adapter, e.g. `192.168.1.10` (example only).
5. If Windows Firewall prompts, allow the controller on **Private networks**. If needed, create a private-network inbound TCP rule for port `8080`.

### On each worker laptop

1. Connect it to the same Wi-Fi/LAN as the controller for the easiest setup.
2. Copy the project folder to the laptop.
3. Install a C++17 compiler and CMake.
4. Run `BUILD_WORKER.bat` once.
5. Run `RUN_WORKER.bat`, enter the controller's IPv4 address and a unique worker name such as `Lenovo-LOQ`.
6. The worker registers its CPU/RAM and, when available, NVIDIA GPU/VRAM telemetry. The dashboard should show it as a physical live worker.
7. Submit a workload that fits its resources. The agent polls for assigned work, executes an approved local CPU workload and reports its result and measured execution time.

Up to four nodes total can be registered in the initial configuration. For a clean live demonstration, reset the session before switching from a four-node manual demo to live mode. A worker with the same name can re-register and reuse its existing slot after restarting the agent.

### Worker workload types

- **Matrix Computation:** CPU matrix multiplication
- **AI Inference:** a small deterministic synthetic feed-forward network calculation (not a trained production model)
- **Data Processing:** generated-array sorting
- **Image Processing:** a generated pixel-grid filter operation (not a user-photo pipeline)

The worker agent is CPU-execution based. NVIDIA GPU detection is hardware telemetry only; live jobs with a non-zero GPU-memory requirement remain queued because CUDA kernels/GPU execution are not enabled. Manual mode can simulate GPU resource fit. CUDA kernels, GPU scheduling and actual model training on GPUs are not included. The scheduler assigns each job to one node; it does not split a single model/job across multiple computers.

## Scheduling strategies

1. **FCFS:** chooses the first eligible node in registration order.
2. **Priority:** processes Critical, High, Medium and Low jobs in priority-queue order, then chooses the least-loaded eligible node.
3. **DistriOpt:** filters out nodes that cannot satisfy the requested CPU, RAM and GPU-memory requirements, then scores eligible nodes using:

   `score = 0.30 × freeCPU + 0.30 × freeRAM + 0.20 × freeGPU + 0.10 × CPUFit + 0.10 × RAMFit`

This is a transparent prototype heuristic, not a claim of globally optimal scheduling. The weights can be changed and benchmarked.

## FODS implementation

The custom implementations are in `include/fods.hpp`:

| Topic | Implemented |
|---|---|
| Dynamic array | `FODSArray<T>` with manual allocation/growth |
| Linked list | Singly linked list with insertion, search and deletion |
| Stack | LIFO stack |
| Queue | FIFO queue |
| Priority queue | Binary max-heap |
| Searching | Linear, sentinel, binary, Fibonacci, indexed sequential |
| Sorting | Bubble, insertion, selection, quick, merge, shell, counting, radix, bucket |

The cluster registry, FCFS/priority queues and failure/retry workflow use the custom FODS structures. Fibonacci and indexed sequential search require sorted input; radix sort accepts non-negative integers; bucket sort accepts values in the inclusive range [0,1]. `std::vector` and other standard-library utilities are used where appropriate for storage and calculations.

Typical algorithm complexities (average/worst case where applicable):

| Algorithm | Time complexity | Extra space |
|---|---:|---:|
| Linear search | O(n) | O(1) |
| Sentinel search | O(n) | O(n) for the helper copy |
| Binary / Fibonacci search | O(log n) | O(1) |
| Indexed sequential search | O(√n) average with √n block size | O(1) |
| Bubble / insertion / selection sort | O(n²) | O(1) |
| Shell sort | Gap-sequence dependent; commonly O(n²) worst | O(1) |
| Quick sort | O(n log n) average, O(n²) worst | O(log n) average recursion |
| Merge sort | O(n log n) | O(n) |
| Counting sort | O(n + k), k = value range | O(k) |
| Radix sort | O(d(n + b)), d = digits, b = base | O(n + b) |
| Bucket sort | O(n + k) expected for evenly distributed values | O(n + k) |

## API overview

- `GET /api/state` — nodes, jobs, strategy and metrics
- `POST /api/mode` — select `Manual` or `Live`
- `POST /api/nodes` — add a manual virtual node
- `POST /api/nodes/remove` — remove a node by ID
- `POST /api/jobs` — submit a workload
- `POST /api/strategy` — select `FCFS`, `Priority` or `DistriOpt`
- `POST /api/fail-node` — simulate node failure
- `POST /api/demo` — reset and load the demo environment
- `POST /api/benchmark` — benchmark the three scheduling strategies
- `POST /api/reset` — reset the in-memory session
- `POST /api/workers/register` — register a worker agent
- `POST /api/workers/heartbeat` — worker heartbeat
- `POST /api/workers/claim` — worker claims its next assigned job
- `POST /api/workers/result` — worker reports execution result

## Build and run tests

```powershell
cmake -S . -B build
cmake --build build --config Release
.\build\Release\distriopt_tests.exe
```

With a single-configuration generator, the executable may be at `build/distriopt_tests.exe` instead. `RUN_TESTS.bat` handles both layouts.

## Metrics and interpretation

- Submitted/completed/queued/running/failed workload counts are read from the controller's current session.
- Manual-mode CPU load is a simulated scheduling/load indicator.
- Live-mode CPU/RAM/GPU specifications are detected/reported by the worker agent; the current worker agent does not stream per-second OS CPU utilization telemetry.
- Live remote execution time is measured by the worker process and reported to the controller.
- Algorithm benchmark milliseconds measure the controller's scheduler routine over a generated 300-job test set; they are not workload execution times or end-to-end cluster throughput.
- Resource wastage is a simple estimate derived from the CPU-load indicator, not a full energy/cost measurement.

## Known academic-prototype boundaries

These are outside the agreed 1–2 day MVP and must not be claimed as implemented:

- Production security (TLS, certificate management, enterprise identity and hardened authorization)
- Persistent database-backed history across controller restarts (the project writes a lightweight session snapshot)
- CUDA/GPU job execution or multi-GPU scheduling
- Distributed model training, job splitting, gradient synchronization or model checkpointing
- Mid-task checkpoint resume; failed in-progress work is requeued and restarts from the beginning
- WAN/cloud federation, NAT traversal and internet-wide worker discovery
- Production-grade CPU/RAM/GPU telemetry sampling and large-scale stress validation

Within the defined academic scope, the project demonstrates the core concepts of heterogeneous node management, resource-aware scheduling, data structures, remote worker execution and failure recovery.
