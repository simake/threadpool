# threadpool

#### What is this?

A modest thread pool implementation in C++11. It keeps a queue of tasks to be processed and assigns those tasks to worker threads when they become available.

#### Why is this needed?

Repeatedly spinning up `std::thread`s to handle multiple tasks has significant overhead. A thread pool starts a set of threads and keeps them alive between tasks to avoid that overhead.

### API

**ThreadPool (constructor)**

Creates a thread pool with a set number of worker threads. Defaults to the number of threads supported by the system (or 1 if it's not computable). Note: Throws `std::system_error` if the threads could not be started.

```cpp
ThreadPool pool(4);
```

**push**

Pushes a new task to the processing queue and returns a future used to access the return value. The task will be invoked when there is an available worker thread and any previously pushed tasks have been or are being handled by a worker thread. Note: Reference arguments need to be wrapped with `std::ref`.

```cpp
auto future = pool.push([](int x){ return x*2; }, 42);
std::cout << future.get() << std::endl;  // 84
```

**discard**

Discards all queued tasks and returns the amount. Tasks that are already being handled will run to completion. Note: Calling `future.get()` on the corresponding future of one of the discarded tasks will throw an `std::future_error` exception.

```cpp
size_t n_discarded = pool.discard()
```

**wait**

Blocks until all pushed tasks have been completed. Threads are kept alive. Tip: Blocking can also be accomplished by calling `future.get()`.

```cpp
// bar depends on foo, so let's make sure foo has been completed before starting bar
pool.push(foo)
pool.wait()
pool.push(bar)
```


**stop**

Initiates thread pool shutdown. Worker threads are joined when they are done processing the remaining tasks in the queue. This will be called automatically upon destruction of the thread pool if not done explicitly.

```cpp
pool.stop()
```

### Installation

Everything is contained within the `threadpool.hpp` header, so just drop it into your project.
