#pragma once
#include <atomic>
#include <bit>
#include <array>

template<typename T, size_t Capacity>
class RingBuffer {
    static_assert(std::has_single_bit(Capacity), "Capacity must be a bit set");
    std::array<T, Capacity> buffer;
    alignas(std::hardware_destructive_interference_size) std::atomic<size_t> head{};
    alignas(std::hardware_destructive_interference_size) std::atomic<size_t> tail{};
public:
    void tryPush(const T& element);
    [[nodiscard]] bool tryPop(T& element);
    void push_overwrite();
};

template<typename T, size_t Capacity> // jedyne miejsce w ktorym modyfikowany jest head - producent
void RingBuffer<T, Capacity>::tryPush(const T &element) {
    auto aktualnyHead = head.load(std::memory_order_relaxed);
    if (((aktualnyHead +1) & (Capacity - 1) )== tail.load(std::memory_order_acquire)) return;
    buffer[aktualnyHead] = element;
    head.store((aktualnyHead +1) & (Capacity - 1) , std::memory_order_release);
}

template<typename T, size_t Capacity>
bool RingBuffer<T, Capacity>::tryPop(T& element) {
    auto aktualnyTail = tail.load(std::memory_order_relaxed);
    if (aktualnyTail == head.load(std::memory_order_acquire)) return false;
    element = buffer[aktualnyTail];
    tail.store((aktualnyTail+1) & (Capacity-1), std::memory_order_release);
    return true;
}

template<typename T, size_t Capacity>
void RingBuffer<T, Capacity>::push_overwrite() {
    auto aktualnyTail = tail.load(std::memory_order_acquire);
    auto aktualnyHead = head.load(std::memory_order_acquire);
    if (aktualnyHead == aktualnyTail) {
        // jeszce nie zaimplementowane
    }
}
