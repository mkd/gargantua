#ifndef THREAD_H
#define THREAD_H

#include "bitboard.h"
#include <cstring>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <atomic>

constexpr int MaxPly = 256;

struct ThreadState {
    // Board state
    Bitboard bitboards[12];
    Bitboard occupancies[3];
    int sideToMove;
    int epsq;
    int castle;
    int fifty;
    int ply;
    uint64_t hash_key;
    bool flip_flag;
    Bitboard repetition_table[1024];
    int repetition_index;

    // Search state
    uint64_t node_count;
    int killers[2][MaxPly];
    int history[12][64];
    int countermoves[12][64];
    int current_move[MaxPly];
    int pv_length[MaxPly];
    int pv_table[MaxPly][MaxPly];
    bool followPV;
    bool scorePV;
    bool allowNull;

    void reset_search_state() {
        node_count = 0;
        memset(killers, 0, sizeof(killers));
        memset(history, 0, sizeof(history));
        memset(countermoves, 0, sizeof(countermoves));
        memset(current_move, 0, sizeof(current_move));
        memset(pv_table, 0, sizeof(pv_table));
        memset(pv_length, 0, sizeof(pv_length));
        followPV = false;
        scorePV = false;
        allowNull = true;
    }

    void copy_board_from(const ThreadState* src) {
        memcpy(bitboards, src->bitboards, sizeof(bitboards));
        memcpy(occupancies, src->occupancies, sizeof(occupancies));
        sideToMove = src->sideToMove;
        epsq = src->epsq;
        castle = src->castle;
        fifty = src->fifty;
        ply = src->ply;
        hash_key = src->hash_key;
        flip_flag = src->flip_flag;
        memcpy(repetition_table, src->repetition_table, sizeof(repetition_table));
        repetition_index = src->repetition_index;
    }
};

class Thread {
public:
    explicit Thread(size_t n);
    virtual ~Thread();

    virtual void search();
    void idle_loop();
    void start_searching();
    void wait_for_search_finished();

    ThreadState state{};
    size_t idx;
    std::mutex mutex;
    std::condition_variable cv;
    std::thread stdThread;
    bool exit = false;
    bool searching = false;
    
    // For syncing NNUE state from MainThread to helper threads
    const void* main_nnue_pos = nullptr;
    const void* main_nnue_setup = nullptr;
    std::atomic<bool> synced{false};
};

class MainThread : public Thread {
public:
    explicit MainThread(size_t n) : Thread(n) {}
    virtual void search() override;
};

class ThreadPool {
public:
    void start_thinking();
    void wait_for_search_finished();
    void clear();
    void set(size_t n);
    uint64_t nodes_searched() const;

    std::vector<Thread*> threads;

    MainThread* main() { return static_cast<MainThread*>(threads.front()); }
};

extern ThreadPool Threads;
extern thread_local ThreadState* current_thread;

#define bitboards (current_thread->bitboards)
#define occupancies (current_thread->occupancies)
#define sideToMove (current_thread->sideToMove)
#define epsq (current_thread->epsq)
#define castle (current_thread->castle)
#define fifty (current_thread->fifty)
#define ply (current_thread->ply)
#define hash_key (current_thread->hash_key)
#define flip (current_thread->flip_flag)
#define repetition_table (current_thread->repetition_table)
#define repetition_index (current_thread->repetition_index)

#define nodes (current_thread->node_count)
#define killers (current_thread->killers)
#define history (current_thread->history)
#define countermoves (current_thread->countermoves)
#define current_move (current_thread->current_move)
#define pv_length (current_thread->pv_length)
#define pv_table (current_thread->pv_table)
#define followPV (current_thread->followPV)
#define scorePV (current_thread->scorePV)
#define allowNull (current_thread->allowNull)

#endif // THREAD_H
