#pragma once
#include "..//telemetry/TelemetryFrame.h"
#include <array>
#include <iostream>
#include <atomic>

template<typename T, size_t Capacity >
class RingBuffer {
    static_assert(Capacity > 0);
    static_assert(std::has_single_bit(Capacity));
    std::array<T, Capacity> buffer;
    std::atomic<size_t> head = 0;
    std::atomic<size_t> tail = 0;
public:
    [[nodiscard]] bool try_push(const T& element);
    [[nodiscard]] bool try_pull(T& element);
    void push_overwrtie();
};


template<typename T, size_t Capacity>
bool RingBuffer<T, Capacity>::try_push(const T& element) {
    size_t lokalnyHead = head.load(std::memory_order_acquire);
    do {
        if ((lokalnyHead + 1) % Capacity == tail.load(std::memory_order_relaxed)) return false; //sprawdzam czy nie jest poza zasegiem
    }while (!head.compare_exchange_weak(lokalnyHead, (lokalnyHead + 1) %Capacity,std::memory_order_release));
    buffer[lokalnyHead] = element;
    return true;
}

template<typename T, size_t Capacity>
bool RingBuffer<T, Capacity>::try_pull(T&element) {
    size_t lokalnyTail = tail.load(std::memory_order_acquire);
    if (lokalnyTail == head.load(std::memory_order_relaxed)) return false; //pusty
    element = buffer[lokalnyTail];
    tail.store(lokalnyTail+1, std::memory_order_release);
    return true;
}

template<typename T, size_t Capacity>
void RingBuffer<T, Capacity>::push_overwrtie() {

}

