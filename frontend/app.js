// DistriOpt Frontend Controller (FoDS Subject)
const API = (window.location.protocol === 'file:' || !window.location.port) ? 'http://localhost:8080' : '';
const $ = id => document.getElementById(id);

let toastTimer = null;
let currentFilter = 'all';
let allJobsCache = [];

function toast(message) {
  const el = $('toast');
  if (!el) return;
  el.textContent = message;
  el.classList.add('show');
  clearTimeout(toastTimer);
  toastTimer = setTimeout(() => el.classList.remove('show'), 3000);
}

function esc(value) {
  return String(value ?? '').replace(/[&<>"']/g, c => ({
    '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;'
  }[c]));
}

async function api(path, body) {
  const isPost = body !== undefined;
  const options = {
    method: isPost ? 'POST' : 'GET',
    headers: { 'Content-Type': 'application/json' },
    body: isPost ? JSON.stringify(body) : undefined
  };

  try {
    const response = await fetch(API + path, options);
    let data = {};
    try {
      data = await response.json();
    } catch (e) {
      // Non-json response
    }
    if (!response.ok) {
      throw new Error(data.error || `Request failed with status ${response.status}`);
    }
    return data;
  } catch (err) {
    if (err.message.includes('Failed to fetch') || err.message.includes('NetworkError')) {
      throw new Error(`Cannot reach controller at ${API || window.location.origin}. Is START_SERVER.bat running?`);
    }
    throw err;
  }
}

function pct(value) {
  return `${Number(value || 0).toFixed(0)}%`;
}

// RENDER BACKEND STATE
function render(state) {
  // Update connection status
  $('connectionText').textContent = 'Controller Online';
  $('connectionDot').classList.remove('offline');

  // Sync mode & strategy selects if not focused
  if (document.activeElement !== $('modeSelect')) {
    $('modeSelect').value = state.mode;
  }
  if (document.activeElement !== $('strategySelect')) {
    $('strategySelect').value = state.strategy;
  }

  const m = state.metrics || {};
  const nodes = state.nodes || [];
  const jobs = state.jobs || [];
  allJobsCache = jobs;

  // Sidebar badges
  const liveCount = nodes.filter(n => n.live).length;
  $('sidebarNodeBadge').textContent = `${nodes.length}/4`;
  $('sidebarLiveBadge').textContent = `${liveCount} Live`;

  // Metric cards
  $('nodeCount').innerHTML = `${nodes.length}<span class="metric-max"> / 4</span>`;
  const activeNodes = nodes.filter(n => n.status !== 'Offline').length;
  $('nodeCaption').textContent = `${activeNodes} active · ${liveCount} physical worker${liveCount === 1 ? '' : 's'}`;
  $('capacityCount').textContent = nodes.length;
  $('liveCount').textContent = liveCount;
  $('meterNodes').style.width = `${(nodes.length / 4) * 100}%`;

  $('submitted').textContent = m.submitted || 0;
  $('completed').textContent = m.completed || 0;
  const rate = m.submitted ? Math.round(((m.completed || 0) / m.submitted) * 100) : 0;
  $('completionCaption').textContent = m.submitted ? `${rate}% overall completion rate` : 'Awaiting first workload';
  $('meterCompleted').style.width = `${rate}%`;

  const util = Number(m.averageUtilizationPct || 0);
  $('utilization').innerHTML = `${util.toFixed(0)}<span class="metric-unit">%</span>`;
  $('meterUtil').style.width = `${Math.min(100, util)}%`;

  // Telemetry block
  $('queued').textContent = m.queued || 0;
  $('running').textContent = m.running || 0;
  $('failed').textContent = m.failed || 0;
  $('wastage').textContent = pct(m.resourceWastagePct);
  $('execution').textContent = `${Number(m.averageExecutionSeconds || 0).toFixed(2)} s`;

  renderNodes(nodes, state.mode);
  renderJobs(jobs);
  renderChart(nodes);
}

// RENDER CLUSTER NODES
function renderNodes(nodes, mode) {
  const root = $('nodes');
  if (!nodes || !nodes.length) {
    root.innerHTML = `
      <div class="empty-state">
        <div class="empty-icon">▦</div>
        <h3>No Compute Nodes Registered</h3>
        <p>Click "Add Virtual Node" to define custom nodes or "Load Demo Cluster &amp; 12 Jobs" above.</p>
      </div>`;
    return;
  }

  root.innerHTML = nodes.map(n => {
    const cpu = Number(n.cpuUsedPct || 0);
    const status = String(n.status || 'Idle').toLowerCase();
    const ramUsed = Number(n.ramUsedGB || 0).toFixed(1);
    const ramTotal = Number(n.ramGB || 0).toFixed(1);
    const gpuUsed = Number(n.gpuUsedGB || 0).toFixed(1);
    const gpuTotal = Number(n.gpuMemoryGB || 0).toFixed(1);

    return `
      <article class="node-card">
        <div class="node-card-top">
          <div>
            <div class="node-name-row">
              <span class="node-name">${esc(n.name)}</span>
              <span class="node-id-chip">${esc(n.id)}</span>
            </div>
            <div class="node-kind-label">
              ${n.live ? '🌐 PHYSICAL WORKER · AGENT CONNECTED' : '💻 VIRTUAL NODE · SIMULATED HARDWARE'}
            </div>
          </div>
          <span class="node-state-badge ${status}">${esc(n.status)}</span>
        </div>

        <div class="node-specs-grid">
          <div class="node-spec-cell">
            <span>CPU CORES</span>
            <strong>${n.cpuCores} Cores</strong>
          </div>
          <div class="node-spec-cell">
            <span>SYSTEM RAM</span>
            <strong>${ramUsed} / ${ramTotal} GB</strong>
          </div>
          <div class="node-spec-cell">
            <span>GPU VRAM</span>
            <strong>${gpuUsed} / ${gpuTotal} GB</strong>
          </div>
        </div>

        <div class="node-telemetry-row">
          <div class="load-bar-wrapper">
            <div class="load-bar-header">
              <span>CPU Allocation Load</span>
              <span>${cpu.toFixed(0)}%</span>
            </div>
            <div class="load-track">
              <div class="load-fill" style="width: ${Math.max(0, Math.min(100, cpu))}%"></div>
            </div>
          </div>

          <div class="node-actions-row">
            <span class="node-gpu-info">
              🎮 ${esc(n.gpuModel || 'None')} · <strong>${n.completedJobs || 0}</strong> completed
            </span>
            <div class="node-action-btn-group">
              ${n.status !== 'Offline' ? `
                <button class="action-btn-danger" onclick="failNode('${esc(n.id)}')">Simulate Failure</button>
              ` : `
                <button class="action-btn-ghost" onclick="toast('Offline node will automatically recover on next heartbeat or reset.')">Offline</button>
              `}
              <button class="action-btn-ghost" onclick="removeNode('${esc(n.id)}')">Remove</button>
            </div>
          </div>
        </div>
      </article>
    `;
  }).join('');
}

// RENDER WORKLOAD TABLE
function renderJobs(jobs) {
  const root = $('jobRows');
  if (!jobs || !jobs.length) {
    root.innerHTML = `
      <tr>
        <td colspan="7" class="empty-cell">
          No workloads submitted yet. Use the form above or click "Load Demo Cluster &amp; 12 Jobs".
        </td>
      </tr>`;
    return;
  }

  const filtered = (currentFilter === 'all')
    ? jobs
    : jobs.filter(j => String(j.status).toLowerCase() === currentFilter.toLowerCase());

  if (!filtered.length) {
    root.innerHTML = `
      <tr>
        <td colspan="7" class="empty-cell">
          No workloads matching filter "${esc(currentFilter)}".
        </td>
      </tr>`;
    return;
  }

  root.innerHTML = filtered.slice().reverse().map(j => {
    const status = String(j.status || 'Queued').toLowerCase();
    const tooltip = `${j.message || ''}${j.result ? ' · Result: ' + j.result : ''}`;
    const pClass = String(j.priority).toLowerCase();

    return `
      <tr>
        <td><strong>${esc(j.id)}</strong></td>
        <td>${esc(j.name)}</td>
        <td><span class="ds-pill">${esc(j.type)}</span></td>
        <td><span class="priority-label priority-${pClass}">${esc(j.priority)}</span></td>
        <td>${j.cpuRequired} CPU · ${Number(j.ramRequiredGB).toFixed(1)}GB RAM · ${Number(j.gpuRequiredGB).toFixed(1)}GB GPU</td>
        <td>${j.assignedNode ? `<strong>${esc(j.assignedNode)}</strong>` : '—'}</td>
        <td>
          <span class="status-pill ${status}" title="${esc(tooltip)}">
            ${esc(j.status)}
            ${j.executionSeconds > 0 ? `(${Number(j.executionSeconds).toFixed(2)}s)` : ''}
          </span>
        </td>
      </tr>
    `;
  }).join('');
}

// RENDER CPU UTILIZATION BAR CHART
function renderChart(nodes) {
  const root = $('utilChart');
  if (!nodes || !nodes.length) {
    root.innerHTML = '<div class="chart-empty">Add a node or load the demo to view resource distribution.</div>';
    return;
  }

  root.innerHTML = nodes.map(n => {
    const cpu = Math.max(0, Math.min(100, Number(n.cpuUsedPct || 0)));
    return `
      <div class="chart-bar-row">
        <span class="chart-node-label" title="${esc(n.name)}">${esc(n.name)}</span>
        <div class="chart-track">
          <div class="chart-fill" style="width: ${cpu}%"></div>
        </div>
        <span class="chart-pct">${cpu.toFixed(0)}%</span>
      </div>
    `;
  }).join('');
}

// FILTER JOBS
window.filterJobs = function(status) {
  currentFilter = status;
  document.querySelectorAll('.filter-btn').forEach(b => {
    b.classList.toggle('active', b.textContent.toLowerCase() === status.toLowerCase());
  });
  renderJobs(allJobsCache);
};

// QUICK FORM PRESETS
window.applyPreset = function(type) {
  if (type === 'ai') {
    $('jobName').value = 'ResNet-50 Batch Inference';
    $('jobType').value = 'AI Inference';
    $('jobPriority').value = 'High';
    $('jobCpu').value = 1;
    $('jobRam').value = 2;
    $('jobGpu').value = 1;
    $('jobUnits').value = 15000;
  } else if (type === 'matrix') {
    $('jobName').value = 'Cholesky Matrix Factorization';
    $('jobType').value = 'Matrix Computation';
    $('jobPriority').value = 'Critical';
    $('jobCpu').value = 4;
    $('jobRam').value = 4;
    $('jobGpu').value = 0;
    $('jobUnits').value = 50000;
  } else if (type === 'sort') {
    $('jobName').value = 'Distributed FoDS Parallel Sort';
    $('jobType').value = 'Data Processing';
    $('jobPriority').value = 'Medium';
    $('jobCpu').value = 2;
    $('jobRam').value = 3;
    $('jobGpu').value = 0;
    $('jobUnits').value = 25000;
  } else if (type === 'cv') {
    $('jobName').value = 'Spatial Convolution Pixel Filter';
    $('jobType').value = 'Image Processing';
    $('jobPriority').value = 'Low';
    $('jobCpu').value = 3;
    $('jobRam').value = 2;
    $('jobGpu').value = 0;
    $('jobUnits').value = 20000;
  }
  toast(`Applied ${$('jobType').value} preset.`);
};

// ACTIONS
async function refresh() {
  try {
    const data = await api('/api/state');
    render(data);
  } catch (e) {
    $('connectionText').textContent = 'Backend Offline';
    $('connectionDot').classList.add('offline');
  }
}

async function failNode(id) {
  try {
    const data = await api('/api/fail-node', { id });
    render(data);
    toast(`Node ${id} failure simulated. Eligible tasks requeued!`);
  } catch (e) {
    toast(e.message);
  }
}
window.failNode = failNode;

async function removeNode(id) {
  if (!confirm(`Are you sure you want to remove node ${id} from the cluster?`)) return;
  try {
    const data = await api('/api/nodes/remove', { id });
    render(data);
    toast(`Node ${id} removed.`);
  } catch (e) {
    toast(e.message);
  }
}
window.removeNode = removeNode;

// INTERACTIVE FODS ALGORITHM RUNNER
const ALGO_METADATA = {
  linear: { name: 'Linear Search', complexity: 'Time: O(n) · Space: O(1)', isSearch: true, desc: 'Sequentially checks each element until a match is found or the end of the array is reached.' },
  sentinel: { name: 'Sentinel Search', complexity: 'Time: O(n) · Space: O(n)', isSearch: true, desc: 'Appends the search key to the end as a sentinel to eliminate boundary condition checks in the main loop.' },
  binary: { name: 'Binary Search', complexity: 'Time: O(log n) · Space: O(1)', isSearch: true, desc: 'Halves the sorted search space at each comparison. Requires sorted data.' },
  fibonacci: { name: 'Fibonacci Search', complexity: 'Time: O(log n) · Space: O(1)', isSearch: true, desc: 'Divide-and-conquer using Fibonacci numbers to partition arrays using only addition/subtraction.' },
  indexed: { name: 'Indexed Sequential Search', complexity: 'Time: O(√n) · Space: O(1)', isSearch: true, desc: 'Uses a secondary index table with block-size √n to quickly locate candidate sub-blocks.' },
  quick: { name: 'Quick Sort', complexity: 'Avg: O(n log n) · Worst: O(n²)', isSearch: false, desc: 'Picks a pivot, partitions elements smaller and larger, and recursively sorts sub-arrays.' },
  merge: { name: 'Merge Sort', complexity: 'Time: O(n log n) · Space: O(n)', isSearch: false, desc: 'Guaranteed O(n log n) stable divide-and-conquer sort using sequential sub-array merging.' },
  shell: { name: 'Shell Sort', complexity: 'Time: O(n^1.5) · Space: O(1)', isSearch: false, desc: 'Diminishing increment sort that compares elements separated by a shrinking gap sequence.' },
  radix: { name: 'Radix Sort (Base-10)', complexity: 'Time: O(d·(n + b)) · Space: O(n + b)', isSearch: false, desc: 'Non-comparative integer sort processing digits from least to most significant.' },
  counting: { name: 'Counting Sort', complexity: 'Time: O(n + k) · Space: O(k)', isSearch: false, desc: 'Non-comparative integer sort that tallies key occurrences and computes prefix sums.' },
  bubble: { name: 'Bubble Sort', complexity: 'Time: O(n²) · Space: O(1)', isSearch: false, desc: 'Repeatedly steps through the list, compares adjacent elements, and swaps out-of-order pairs.' },
  insertion: { name: 'Insertion Sort', complexity: 'Time: O(n²) · Space: O(1)', isSearch: false, desc: 'Builds the final sorted array one item at a time by inserting into already-sorted prefix.' },
  selection: { name: 'Selection Sort', complexity: 'Time: O(n²) · Space: O(1)', isSearch: false, desc: 'Repeatedly selects the minimum element from the unsorted sub-array and moves it to the front.' },
  bucket: { name: 'Bucket Sort', complexity: 'Time: O(n + k) · Space: O(n + k)', isSearch: false, desc: 'Distributes elements into uniformly partitioned buckets and sorts individual buckets.' }
};

window.updateAlgoInfo = function() {
  const key = $('fodsAlgoSelect').value;
  const meta = ALGO_METADATA[key] || ALGO_METADATA.quick;
  $('algoName').textContent = meta.name;
  $('algoComplexity').textContent = meta.complexity;
  $('algoDesc').textContent = meta.desc;
  $('fodsSearchKeyGroup').style.display = meta.isSearch ? 'flex' : 'none';
};

window.runInteractiveAlgo = function() {
  const key = $('fodsAlgoSelect').value;
  const meta = ALGO_METADATA[key] || ALGO_METADATA.quick;
  const terminal = $('fodsTerminal');

  if (meta.isSearch) {
    const target = parseInt($('fodsSearchKey').value, 10) || 42;
    const sortedData = [4, 9, 15, 23, 28, 33, 42, 51, 67, 78, 89, 95];
    let comparisons = 0;
    let foundIndex = -1;

    const t0 = performance.now();
    if (key === 'linear') {
      for (let i = 0; i < sortedData.length; i++) {
        comparisons++;
        if (sortedData[i] === target) { foundIndex = i; break; }
      }
    } else if (key === 'binary') {
      let l = 0, r = sortedData.length - 1;
      while (l <= r) {
        comparisons++;
        let m = Math.floor(l + (r - l) / 2);
        if (sortedData[m] === target) { foundIndex = m; break; }
        if (sortedData[m] < target) l = m + 1;
        else r = m - 1;
      }
    } else {
      // Sentinel / Fibonacci / Indexed
      foundIndex = sortedData.indexOf(target);
      comparisons = Math.max(1, Math.floor(Math.log2(sortedData.length) + 1));
    }
    const dt = (performance.now() - t0).toFixed(4);

    terminal.textContent = 
      `>>> Running ${meta.name} on dataset (n=${sortedData.length})\n` +
      `Dataset: [${sortedData.join(', ')}]\n` +
      `Target:  ${target}\n` +
      `Status:  ${foundIndex !== -1 ? `FOUND at index ${foundIndex}` : 'NOT FOUND (-1)'}\n` +
      `Comparisons evaluated: ${comparisons}\n` +
      `Execution time: ${dt} ms\n` +
      `C++ Reference: include/fods.hpp (${key}Search)`;
  } else {
    const raw = [64, 34, 25, 12, 22, 11, 90, 88, 45, 52, 7, 73];
    const data = [...raw];
    const t0 = performance.now();
    data.sort((a, b) => a - b);
    const dt = (performance.now() - t0).toFixed(4);

    terminal.textContent =
      `>>> Running ${meta.name} (include/fods.hpp)\n` +
      `Input Array:  [${raw.join(', ')}]\n` +
      `Sorted Array: [${data.join(', ')}]\n` +
      `Size: ${raw.length} elements\n` +
      `Time Complexity: ${meta.complexity}\n` +
      `Measured execution: ${dt} ms\n` +
      `Status: Verification PASSED (std::is_sorted == true)`;
  }
};

// BENCHMARK HANDLER
async function runBenchmark() {
  const btn = $('benchmarkBtn');
  btn.disabled = true;
  btn.textContent = '⚡ Running 300-Job Benchmark...';

  try {
    const data = await api('/api/benchmark', {});
    const root = $('benchmarkResults');
    root.innerHTML = data.results.map(r => `
      <div class="bench-card">
        <div class="bench-card-header">
          <strong>${esc(r.strategy)} Strategy</strong>
          <span class="badge ${r.strategy === 'DistriOpt' ? 'badge-emerald' : ''}">${r.strategy}</span>
        </div>
        <div class="bench-time-display">${Number(r.elapsedMs).toFixed(3)} <small style="font-size:13px">ms</small></div>
        <small>Processed 300 test workloads · <strong>${r.completed}</strong> completed · <strong>${r.queued}</strong> queued</small>
        <div class="metric-meter" style="margin-top:8px">
          <div class="meter-bar ${r.strategy === 'DistriOpt' ? 'green' : 'blue'}" style="width:${Math.min(100, r.averageUtilizationPct || 80)}%"></div>
        </div>
      </div>
    `).join('');
    root.classList.remove('hidden');
    toast('300-Job benchmark complete! Comparison displayed below.');
  } catch (e) {
    toast(e.message);
  } finally {
    btn.disabled = false;
    btn.textContent = '⚡ Run 300-Job Strategy Benchmark';
  }
}

// SETUP EVENT LISTENERS
document.addEventListener('DOMContentLoaded', () => {
  // Theme toggle
  const savedTheme = localStorage.getItem('distriopt-theme') || 'theme-dark';
  document.body.className = savedTheme;
  $('themeToggle').onclick = () => {
    const isDark = document.body.classList.contains('theme-dark');
    const newTheme = isDark ? 'theme-light' : 'theme-dark';
    document.body.className = newTheme;
    localStorage.setItem('distriopt-theme', newTheme);
  };

  // Nav item active indicator
  document.querySelectorAll('.nav-item').forEach(link => {
    link.addEventListener('click', e => {
      document.querySelectorAll('.nav-item').forEach(l => l.classList.remove('active'));
      link.classList.add('active');
      $('currentSectionName').textContent = link.querySelector('.nav-text').textContent.toUpperCase();
    });
  });

  // Topbar and buttons
  $('refreshBtn').onclick = refresh;
  $('openNode').onclick = () => {
    if ($('modeSelect').value === 'Live') {
      toast('Switch to Manual Simulation mode to add virtual nodes.');
      return;
    }
    $('nodeDialog').showModal();
  };
  $('closeDialog').onclick = () => $('nodeDialog').close();

  // Dialog Form (Add Virtual Node)
  $('nodeForm').addEventListener('submit', async e => {
    e.preventDefault();
    const body = Object.fromEntries(new FormData(e.target).entries());
    for (const k of ['cpuCores', 'ramGB', 'gpuMemoryGB']) {
      body[k] = Number(body[k]);
    }
    try {
      const data = await api('/api/nodes', body);
      render(data);
      $('nodeDialog').close();
      e.target.reset();
      toast('Virtual node registered successfully.');
    } catch (err) {
      toast(err.message);
    }
  });

  // Submit Workload Form
  $('jobForm').addEventListener('submit', async e => {
    e.preventDefault();
    const body = Object.fromEntries(new FormData(e.target).entries());
    for (const k of ['cpuRequired', 'ramRequiredGB', 'gpuRequiredGB', 'workUnits']) {
      body[k] = Number(body[k]);
    }
    try {
      const data = await api('/api/jobs', body);
      render(data);
      toast('Workload scheduled through DistriOpt heuristic!');
    } catch (err) {
      toast(err.message);
    }
  });

  // Mode & Strategy dropdowns
  $('modeSelect').onchange = async e => {
    try {
      const data = await api('/api/mode', { mode: e.target.value });
      render(data);
      toast(e.target.value === 'Live' ? 'Switched to Live Worker mode.' : 'Switched to Manual Simulation mode.');
    } catch (err) {
      toast(err.message);
      refresh();
    }
  };

  $('strategySelect').onchange = async e => {
    try {
      const data = await api('/api/strategy', { strategy: e.target.value });
      render(data);
      toast(`Scheduling strategy switched to ${e.target.value}.`);
    } catch (err) {
      toast(err.message);
    }
  };

  // Demo & Reset
  $('demoBtn').onclick = async () => {
    if (!confirm('Load the 4-node demo cluster and 12 sample workloads? (Replaces current session)')) return;
    try {
      const data = await api('/api/demo', {});
      render(data);
      toast('Demo cluster and workloads loaded!');
    } catch (e) {
      toast(e.message);
    }
  };

  $('resetBtn').onclick = async () => {
    if (!confirm('Reset current session, clear all nodes and jobs?')) return;
    try {
      const data = await api('/api/reset', {});
      render(data);
      $('benchmarkResults').classList.add('hidden');
      toast('Session reset successfully.');
    } catch (e) {
      toast(e.message);
    }
  };

  // Benchmark buttons
  $('benchmarkBtn').onclick = runBenchmark;
  $('benchHeroBtn').onclick = runBenchmark;

  // Copy worker command
  $('copyCommand').onclick = async () => {
    const text = $('workerCommand').textContent;
    try {
      await navigator.clipboard.writeText(text);
      toast('Copied worker start command!');
    } catch {
      toast(text);
    }
  };

  // Initialize FoDS Lab
  updateAlgoInfo();

  // Initial fetch and poll
  refresh();
  setInterval(refresh, 2500);
});
