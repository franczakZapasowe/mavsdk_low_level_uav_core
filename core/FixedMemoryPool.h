#pragma once
#include "..//logger/BlackboxPacket.h"

template<typename T, size_t PoolSize>
class FixedMemoryPool {
    struct Node {
        T object;
        Node*next;
    };
    std::array<Node,PoolSize> poolNode;
    Node* m_freeListHead;

public:
    FixedMemoryPool() {
        for (size_t i = 0; i < PoolSize-1; i++) {
            poolNode[i].next = &poolNode[i+1];
        }
        poolNode[PoolSize-1].next = nullptr;
        m_freeListHead = &poolNode[0];
    }
    [[nodiscard]]T* allocate();
    void deallocate(T* ptr);
};

template<typename T, size_t PoolSize>
T * FixedMemoryPool<T, PoolSize>::allocate() {
    if (m_freeListHead == nullptr) {
        return nullptr;
    }
    Node * aktualnyNode = m_freeListHead;
    m_freeListHead = m_freeListHead->next;
    return &(aktualnyNode->object);
}

template<typename T, size_t PoolSize>
void FixedMemoryPool<T, PoolSize>::deallocate(T *ptr) {
    //aktualny pozcyja
    Node* akutalny = reinterpret_cast<Node*>(ptr);
    
}
