

---

## Team setup first


**Repo structure:**
```
httpserver/
├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── server/        # socket, accept loop
│   ├── threadpool/    # queue + workers
│   ├── http/          # parser, router, response
│   └── utils/         # logger, config
├── tests/
├── benchmarks/
└── docs/
```

**Git workflow:** `main` is always working. Feature branches → PRs to merge. One person owns the thread pool, one owns the HTTP layer — but you both review each other's code. This matters: you need to understand the whole thing for interviews.

**Tools to install now:** `g++`, `cmake`, `make`, `valgrind`, `gdb`, `clang-format`, `wrk` (for benchmarking later).

---
<img width="695" height="550" alt="image" src="https://github.com/user-attachments/assets/54723e95-d89d-4f98-88bd-92db46a32dd4" />


## The 10-week plan

### Week 1–2 — C++ and sockets from zero

Don't touch HTTP yet. Get comfortable with the language and the POSIX API.

**Week 1 — C++ crash course (you both do this independently):**

Write these programs in C++, no Googling the solutions first, just try:
- A linked list class with RAII (destructor frees memory, no `delete` in `main`)
- Read a file into a string using `open()` / `read()` — not `fstream`, the raw syscall
- A `std::vector`-based stack with move semantics

The goal isn't perfection. It's getting your fingers used to C++ error messages, pointers, and `g++ -Wall -Wextra -std=c++17`.

**Week 2 — TCP echo server:**

This is your first real milestone. Write a server that accepts connections and echoes back whatever the client sends.

```cpp
// The four syscalls you must understand cold
int fd = socket(AF_INET, SOCK_STREAM, 0);
bind(fd, (struct sockaddr*)&addr, sizeof(addr));
listen(fd, SOMAXCONN);
int client_fd = accept(fd, nullptr, nullptr);
```

Test it with `nc localhost 8080` — type something, see it echoed back. Then test with two terminals simultaneously and notice it blocks on the second client. That's the problem the thread pool solves. Write that down — you'll explain it in interviews.

**Where AI tools help here:** Paste your compile errors into Claude to understand them. Ask it to explain what `SOMAXCONN` is or why `SO_REUSEADDR` matters. Don't ask it to write the echo server — write it yourself then ask it to review.

---

### Week 3–4 — The thread pool (the heart of the project)

This is the most important thing you'll build. Spend real time here.

**Start naive — thread per connection:**
```cpp
// Week 3, day 1: this is intentionally wrong
int client_fd = accept(...);
std::thread t([client_fd]() { handle(client_fd); });
t.detach();
```

Run `wrk` against it. Watch it die under load. Now you understand *why* thread pools exist.

**Then build the real thing — the bounded task queue:**

```cpp
class TaskQueue {
    std::queue<int> fds;          // client file descriptors
    std::mutex mtx;
    std::condition_variable not_empty;
    std::condition_variable not_full;
    size_t max_size;
    bool shutdown = false;
public:
    void push(int fd);       // blocks if full (back-pressure)
    int  pop();              // blocks if empty (workers wait here)
    void stop();             // wakes all blocked threads to exit
};
```

The `push` blocks when the queue is full — this is back-pressure, the server tells clients to wait rather than crashing. The `pop` blocks when empty — workers sleep here via `condition_variable::wait()`. This producer-consumer pattern is literally a 379 exam topic.

**Then the ThreadPool class:**

```cpp
class ThreadPool {
    std::vector<std::thread> workers;
    TaskQueue queue;
public:
    ThreadPool(size_t n_threads, size_t queue_depth);
    void submit(int client_fd);
    ~ThreadPool();  // graceful shutdown: sets flag, wakes all, joins all
};
```

The destructor is where C++ shines — RAII guarantees every thread is joined when the pool goes out of scope. No `pthread_join` scattered around `main`.

**This week's milestone:** Submit 1000 client fds to the pool, verify all are processed, verify `valgrind` shows zero leaks. Run `gdb`, set a breakpoint, type `info threads` — see all your worker threads waiting.

---

### Week 5–6 — HTTP parsing and routing

**HTTP/1.1 request structure** — you're parsing raw bytes from `read()`:

```
GET /api/users HTTP/1.1\r\n
Host: localhost:8080\r\n
Content-Type: application/json\r\n
\r\n
{"key": "value"}
```

Build a `HttpRequest` struct and a `parse()` function:

```cpp
struct HttpRequest {
    std::string method;   // "GET", "POST", etc.
    std::string path;     // "/api/users"
    std::string version;  // "HTTP/1.1"
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};

HttpRequest parse(int client_fd);
```

Key things to handle: `\r\n` line endings (not just `\n`), the blank line separating headers from body, partial reads (TCP doesn't guarantee one `read()` = one request).

**The router** is simpler — a map from path patterns to handler functions:

```cpp
class Router {
    using Handler = std::function<HttpResponse(const HttpRequest&)>;
    std::unordered_map<std::string, Handler> routes;
public:
    void get(const std::string& path, Handler h);
    void post(const std::string& path, Handler h);
    HttpResponse route(const HttpRequest& req);
};

// Registration looks like:
router.get("/health", [](const HttpRequest&) {
    return HttpResponse{200, "OK"};
});
```

**This week's milestone:** `curl http://localhost:8080/health` returns `200 OK`. `curl http://localhost:8080/missing` returns `404`. Static file serving works for HTML and images.

---

### Week 7–8 — Make it interesting (split between you)

Pick one feature each so you're working in parallel:

**Person A — Logging system (directly teaches 379 IPC)**

A dedicated logger thread that is the *only* thread writing to disk. Workers push log entries to a shared queue (another producer-consumer instance). This teaches you why you isolate I/O — if every thread wrote to the log file directly, the output would be interleaved garbage. Show the broken version first, then fix it.

```cpp
struct LogEntry {
    LogLevel level;
    std::string message;
    std::thread::id thread_id;
    std::chrono::system_clock::time_point timestamp;
};
```

**Person B — Connection keep-alive**

HTTP/1.1 keep-alive means one TCP connection handles multiple requests sequentially. The worker doesn't close `client_fd` after one response — it loops, reads the next request, responds, and only closes when the client sends `Connection: close` or times out. This forces you to handle partial reads and request boundaries correctly, which is a real parsing challenge.

---

### Week 9 — Benchmarking and tuning (do together)

This is where the project becomes impressive to talk about.

```bash
# Install wrk
# Then benchmark
wrk -t4 -c100 -d30s http://localhost:8080/health
```

Run it with thread pool size N=1, N=2, N=4, N=8, N=16. Plot the req/s. You'll see:
- Linear scaling up to some point
- Then diminishing returns (mutex contention)
- Then degradation (context switching overhead)

This is an Amdahl's Law demonstration you built yourself. Screenshot the graph. Put it in the README. Mention it in interviews.

Also run with `-fsanitize=thread` (ThreadSanitizer) — it will catch any data races you missed.

---

### Week 10 — Polish and portfolio

**README must include:**
- Architecture diagram (link to the one above)
- How to build and run (`cmake -B build && cmake --build build`)
- Benchmark results with the graph
- What you learned — specifically call out the 379 concepts (producer-consumer, mutex, condition variables, POSIX threads)

**The resume bullet:**
> Built multithreaded HTTP/1.1 server in C++ from scratch; implemented bounded task queue with mutex/condition variables across N worker threads; benchmarked at X req/s under Y concurrent connections; zero memory leaks under valgrind

---

## Where AI tools genuinely help vs hurt

**Use Claude Code / Copilot for:**
- CMakeLists.txt setup — this is pure boilerplate, don't waste time
- Explaining compiler errors and sanitizer output
- `wrk` Lua scripts for custom load patterns
- Writing unit tests around your parser once it exists
- Code review — paste your mutex logic and ask "can this deadlock?"
- Man page summaries — `read()`, `accept()`, `pthread_cond_wait()`

**Don't use it for:**
- The task queue implementation — you need to own this completely
- Debugging segfaults — use `gdb` yourself first, the process is the learning
- The HTTP parser — string parsing in C++ is where you build instincts

The reason is concrete: in a systems interview, you will be asked "walk me through how your thread pool handles a worker that crashes mid-request" or "what happens if the task queue fills up?" If you didn't write it, you'll stall on the third follow-up. If you wrote every line, you can answer five levels deep.

---

## The 229 connection (worth understanding before you start)

Your MIPS/assembly background from 229 is directly relevant. When you write `int fd = accept(...)`, that's a system call — a trap instruction that switches CPU privilege mode and hands control to the kernel, exactly like the exception handling you saw in 229. `read()` on a socket blocks because the kernel parks the thread in a wait queue until data arrives on the network interface — the same interrupt-driven I/O model from 229, just at the OS layer now. This mental model will make 379 click much faster.
