#include "NonBlockQueue.h"
#include <thread>
#include <iostream>
#include <memory>
int main()
{
    size_t sizeQueue = 10;

    TNonBlockQueue<int> queue(sizeQueue);

   std::shared_ptr<std::thread>(
       new std::thread ([&]()
       { 
           for (size_t i = 0; i < sizeQueue; ++i)
              queue.Enqueue(i);
       }), 
       [](std::thread* pthread) 
       {
           if (pthread->joinable()) pthread->join();
       }
   );
    
   std::shared_ptr<std::thread>(
       new std::thread([&]()
           {
               int item{-1};
               for (size_t i = 0; i < sizeQueue; ++i)
               {
                   queue.Dequeue(item);
               }
           }),
       [&](std::thread* pthread)
       {
            if (pthread->joinable()) pthread->join();
            
            std::cout << std::endl << "Statistics: " << std::endl << std::endl;
            
            __int64 sumEnqTime{ 0 };
            __int64 sumDeqTime{ 0 };
            __int64 sumInterThreadTime{0};
            
            
            __int64 MaxEnqTime{ 0 };
            __int64 MaxDeqTime{ 0 };
            __int64 MaxInterThreadTime{ 0 };

            __int64 MinEnqTime{ 0 };
            __int64 MinDeqTime{ 0 };
            __int64 MinInterThreadTime{ 0 };
            
            const int processorSpeedMHZ = GetProcessorSpeed();
            
            for (size_t i = 0; i < statArraySize && i < sizeQueue; ++i)
            {
                if (MaxEnqTime < statArray[i].enqTime || !i)
                    MaxEnqTime = statArray[i].enqTime;
                
                if(MaxDeqTime < statArray[i].deqTime || !i)
                    MaxDeqTime = statArray[i].deqTime;
                
                if (MaxInterThreadTime < statArray[i].interThreadMoveTime || !i)
                    MaxInterThreadTime = statArray[i].interThreadMoveTime;

                if (MinEnqTime > statArray[i].enqTime || !i)
                    MinEnqTime = statArray[i].enqTime;

                if (MinDeqTime > statArray[i].deqTime || !i)
                    MinDeqTime = statArray[i].deqTime;

                if (MinInterThreadTime > statArray[i].interThreadMoveTime || !i)
                    MinInterThreadTime = statArray[i].interThreadMoveTime;
                
                sumEnqTime += statArray[i].enqTime;
                sumDeqTime += statArray[i].deqTime;
                sumInterThreadTime += statArray[i].interThreadMoveTime;
                
                std::cout << std::endl << "Index of item = " << i << std::endl;
                
                std::cout << std::endl << "Enque time = " << statArray[i].enqTime/(double)processorSpeedMHZ << " nanoseconds " << std::endl;
                std::cout << "Deque time = " << statArray[i].deqTime/(double)processorSpeedMHZ << " nanoseconds " << std::endl;
                std::cout << "Inter-thread moving time = " << statArray[i].interThreadMoveTime/ processorSpeedMHZ << " nanoseconds " << std::endl;
                
               
            }
            
            std::cout << std::endl << "Max enque time = " << MaxEnqTime /(double)processorSpeedMHZ << " nanoseconds " << std::endl;
            std::cout << "Max deque time = " << MaxDeqTime /(double)processorSpeedMHZ << " nanoseconds " << std::endl;
            std::cout << "Max Inter-thread moving time = " << MaxInterThreadTime / processorSpeedMHZ << " nanoseconds " << std::endl;
            
            std::cout << std::endl << "Min enque time = " << MinEnqTime /(double)processorSpeedMHZ << " nanoseconds " << std::endl;
            std::cout << "Min deque time = " << MinDeqTime /(double)processorSpeedMHZ << " nanoseconds " << std::endl;
            std::cout << "Min Inter-thread moving time = " << MinInterThreadTime / processorSpeedMHZ << " nanoseconds " << std::endl;
            
            std::cout << std::endl << "Average enque time = " << (sumEnqTime / sizeQueue)/(double)processorSpeedMHZ << " nanoseconds " << std::endl;
            std::cout << "Average deque time = " << (sumDeqTime / sizeQueue)/ (double)processorSpeedMHZ << " nanoseconds " << std::endl;
            std::cout << "Average Inter-thread moving time = " << (sumInterThreadTime / sizeQueue)/ processorSpeedMHZ << " nanoseconds " << std::endl;
            
       }
   );
    
   
    return 0;
}
