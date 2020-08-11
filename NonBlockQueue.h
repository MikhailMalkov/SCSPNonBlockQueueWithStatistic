// NonBlockQueue.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#ifndef NONBLOCKQUEUE_H
#define NONBLOCKQUEUE_H

#include <atomic>
#include <chrono>

static struct Statistic
{
    __int64 enqTime;
    __int64 deqTime;
    __int64 interThreadMoveTime;
};

static const int statArraySize = 1000000;

static Statistic statArray[statArraySize];

#ifdef WIN32
#include <windows.h>
inline int GetProcessorSpeed() // MGHr
{
    const int OFFSET = 5;
    const double MHZ = 1000000.0;
    
    LARGE_INTEGER qwWait, qwStart, qwCurrent;
    
    QueryPerformanceCounter(&qwStart);
    QueryPerformanceFrequency(&qwWait);
    qwWait.QuadPart >>= OFFSET;
    unsigned __int64 Start = __rdtsc();
    do
    {
        QueryPerformanceCounter(&qwCurrent);
    } while (qwCurrent.QuadPart - qwStart.QuadPart < qwWait.QuadPart);
    
    return static_cast<int>(((__rdtsc() - Start) << OFFSET) / MHZ);
}

#elif __linux__
#include <x86intrin.h>
//Starting from GCC 4.5 and later, the __rdtsc() intrinsic is now supported by both MSVCand GCC.

unsigned long long int rdtsc(void) my rtds for any case
{
    unsigned long long int x;
    __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
    return x;
}

inline int GetProcessorSpeed() // MGHr
{
    const int TIMEOUT = 250000;
    const double MHZ = 1000000.0;
    
    struct timezone tz;
    struct timeval tvstart, tvstop;
    unsigned long long int cycles[2];
    unsigned long microseconds;
    

    memset(&tz, 0, sizeof(tz));

    gettimeofday(&tvstart, &tz);
    cycles[0] = __rdtsc();
    gettimeofday(&tvstart, &tz);

    usleep(TIMEOUT);

    gettimeofday(&tvstop, &tz);
    cycles[1] = __rdtsc();
    gettimeofday(&tvstop, &tz);

    microseconds = ((tvstop.tv_sec - tvstart.tv_sec) * MHZ) + (tvstop.tv_usec - tvstart.tv_usec);

    return static_cast<int>((cycles[1] - cycles[0]) / microseconds);
}
#endif



template<typename T>
class TNonBlockQueue
{
public:
    TNonBlockQueue(size_t size)
        : m_buffer( new Item[size]),
          m_bufferSize(size), 
          m_enqPos{ 0 },
          m_deqPos{ 0 }
    {
        for (size_t i = 0; i < m_bufferSize; ++i)
            m_buffer[i].isEmpty.store(1, std::memory_order_relaxed);
        
        m_isEnqBusy.store(0, std::memory_order_relaxed);
        m_isDeqBusy.store(0, std::memory_order_relaxed);
    }

    ~TNonBlockQueue()
    {
        delete[] m_buffer;
    }

    bool Enqueue(T const& data)
    {
        if (uint64_t isBusy = m_isEnqBusy.load(std::memory_order_acquire);  isBusy == 0) // "mutex" to prevent the code from handling more than one producer 
        {
            if (!m_isEnqBusy.compare_exchange_strong(isBusy, 1, std::memory_order_relaxed))
                return false; // another producer was faster
            _int64 startTime = __rdtsc();
            
            if (m_enqPos >= m_bufferSize) // moving enqueue cursor to the start position
                m_enqPos = 0;

            Item* pItem = &m_buffer[m_enqPos];
            bool result{ false };
            
            if (pItem)
            {
                if (pItem->isEmpty.load(std::memory_order_acquire) == 1) // "mutex" for isEmpty item marker to synchronize producer and consumer thread
                {
                    pItem->data = data; // put data
                    ++m_enqPos;
                    pItem->isEmpty.store(0, std::memory_order_release);
                    pItem->enqTime = __rdtsc();
                    if (m_enqPos <= statArraySize)
                        statArray[m_enqPos - 1].enqTime = pItem->enqTime - startTime;
                    result = true;
                }
            }
            
            m_isEnqBusy.store(0, std::memory_order_release);
            return result;
        }
        
        return false;
    }

    bool Dequeue(T& data)
    {
        if (uint64_t isBusy = m_isDeqBusy.load(std::memory_order_acquire); isBusy == 0) // "mutex" to prevent the code from handling more than one consumer 
        {
            if (!m_isDeqBusy.compare_exchange_strong(isBusy, 1, std::memory_order_relaxed))
                return false; // another consumer was faster
            
            _int64 startTime = __rdtsc();
            
            if (m_deqPos >= m_bufferSize) // moving dequeue cursor to the start position
                m_deqPos = 0;

            Item* pItem = &m_buffer[m_deqPos];
            bool result{ false };

            if (pItem)
            {
                if (pItem->isEmpty.load(std::memory_order_acquire) == 0) // "mutex" for isEmpty item marker to synchronize producer and consumer thread
                {
                    data = pItem->data; // get data
                    ++m_deqPos;
                    pItem->isEmpty.store(1, std::memory_order_release);
                    if (m_deqPos <= statArraySize)
                    {
                        statArray[m_deqPos - 1].deqTime = __rdtsc() - startTime;
                        statArray[m_deqPos - 1] .interThreadMoveTime = __rdtsc() - pItem->enqTime;
                    }
                    result = true;
                }

                
            }

            m_isDeqBusy.store(0, std::memory_order_release);
            return result;
        }
        
        return false;
    }
    
    TNonBlockQueue(TNonBlockQueue const&) = delete;
    void operator = (TNonBlockQueue const&) = delete;
   
private:
    
    struct alignas(64) Item
    {
        std::atomic<uint64_t> isEmpty;
        __int64 enqTime;
        T data;
    };
    
    Item* const m_buffer;
    uint64_t const m_bufferSize;

    uint64_t m_enqPos;
    uint64_t m_deqPos;
    
    std::atomic<uint64_t> m_isEnqBusy; 
    std::atomic<uint64_t> m_isDeqBusy;
    
};

#endif

