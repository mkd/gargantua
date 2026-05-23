#ifdef USE_NEW_NNUE
#include "stockfish_probe/nnue_incremental.h"
#endif
#include "search.h"
#include "thread.h"
#include <cassert>

// Global thread pool
ThreadPool Threads;

// Thread local state pointer
thread_local ThreadState* current_thread = nullptr;

// Thread constructor
Thread::Thread(size_t n) : idx(n) {
    state.reset_search_state();
    stdThread = std::thread(&Thread::idle_loop, this);
}

// Thread destructor
Thread::~Thread() {
    {
        std::unique_lock<std::mutex> lk(mutex);
        exit = true;
        cv.notify_one();
    }
    if (stdThread.joinable()) {
        stdThread.join();
    }
}

// idle_loop is executed by the std::thread and waits for work
void Thread::idle_loop() {
    current_thread = &state;

    while (true) {
        std::unique_lock<std::mutex> lk(mutex);
        cv.wait(lk, [&]{ return searching || exit; });

        if (exit) {
            return;
        }

        lk.unlock();

#ifdef USE_NEW_NNUE
        // ALL threads sync their NNUE state from the UCI thread's NNUE state
        if (main_nnue_pos && main_nnue_setup) {
            Stockfish::Incremental::sync_from_main_thread(
                *static_cast<const Stockfish::Position*>(main_nnue_pos),
                *static_cast<const std::deque<Stockfish::StateInfo>*>(main_nnue_setup)
            );
        }
#endif
        
        synced = true;

        search();
        lk.lock();

        searching = false;
        cv.notify_one(); // Notify Threads.wait_for_search_finished()
    }
}

// start_searching signals the thread to begin a search
void Thread::start_searching() {
    std::unique_lock<std::mutex> lk(mutex);
    searching = true;
    cv.notify_one();
}

// wait_for_search_finished waits until the thread has finished its search
void Thread::wait_for_search_finished() {
    std::unique_lock<std::mutex> lk(mutex);
    cv.wait(lk, [&]{ return !searching; });
}

// search is a wrapper around the global search() function
void Thread::search() {
    ::search();
}

void MainThread::search() {
    ::search();
}

// ThreadPool methods

void ThreadPool::start_thinking() {
#ifdef USE_NEW_NNUE
    const void* main_pos = &Stockfish::Incremental::get_global_pos();
    const void* main_setup = &Stockfish::Incremental::get_setup_states();
#else
    const void* main_pos = nullptr;
    const void* main_setup = nullptr;
#endif

    // Copy the root board state from the MainThread to all helper threads
    for (size_t i = 0; i < threads.size(); i++) {
        if (i > 0) {
            threads[i]->state.copy_board_from(&main()->state);
            threads[i]->state.reset_search_state();
        }
        threads[i]->main_nnue_pos = main_pos;
        threads[i]->main_nnue_setup = main_setup;
    }

    // Start helper threads
    for (size_t i = 1; i < threads.size(); i++) {
        threads[i]->synced = false;
        threads[i]->start_searching();
    }
    
    // Wait for helper threads to finish copying NNUE state
    for (size_t i = 1; i < threads.size(); i++) {
        while (!threads[i]->synced) {}
    }

    // Start main thread
    main()->start_searching();
}

void ThreadPool::clear() {
    for (Thread* th : threads) {
        delete th;
    }
    threads.clear();
}

void ThreadPool::set(size_t n) {
    if (n == 0) n = 1;
    if (threads.size() > 0) {
        // Stop current threads if we are resizing
        // Not implemented strictly here, usually resize happens during idle
    }

    // Create main thread if pool is empty
    if (threads.empty()) {
        threads.push_back(new MainThread(0));
    }

    // Add new threads
    while (threads.size() < n) {
        threads.push_back(new Thread(threads.size()));
    }

    // Remove old threads
    while (threads.size() > n) {
        delete threads.back();
        threads.pop_back();
    }
}

uint64_t ThreadPool::nodes_searched() const {
    uint64_t sum = 0;
    for (Thread* th : threads) {
        sum += th->state.node_count;
    }
    return sum;
}
