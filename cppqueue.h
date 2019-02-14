#ifndef CPPQUEUE_H
#define CPPQUEUE_H

#include <atomic>
#include <deque>
#include <mutex>
#include <condition_variable>

/*
 * ConcurrentQueue是一个线程安全的双向队列deque，
 * notify_all: 唤醒所有的wait线程，如果当前没有等待线程，则该函数什么也不做。
 * notify_one：唤醒某个wait线程，如果当前没有等待线程，则该函数什么也不做；如果同时存在多个等待线程，则唤醒某个线程是不确定的(unspecified)。
 * push_front，push_back往队列中插入元素
 * wait_and_pop（&，1000）当队列为空时，阻塞1000毫秒，不为空时弹出数据
 */
template<typename Data>
class ConcurrentQueue
{
private:
    std::atomic<bool> is_active{ false };
    std::deque<Data> the_queue;
    std::mutex queue_mutex;
    std::condition_variable queue_cv;   //条件变量

public:
    // 通知所有等待在队列上的线程
    void notify_all()
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        queue_cv.notify_all();
    }

    //头端存入新的任务
    void push_front(Data const& data)
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        the_queue.push_front(data);
        queue_cv.notify_one();
    }

    //尾端存入新的任务
    void push_back(Data const& data)
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        the_queue.push_back(data);
        queue_cv.notify_one();
    }

    //检查队列是否为空
    bool empty()
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        return the_queue.empty();
    }

    //取出
    bool wait_and_pop(Data& popped_value, int millsecond)
    {
        std::unique_lock<std::mutex> lock(queue_mutex);

        if (the_queue.empty())						//当队列为空时
        {
            // 0： 不阻塞等待
            if (0 == millsecond)
            {
                return false;
            }
            // -1： 阻塞等待
            else if (-1 == millsecond)
            {
                queue_cv.wait(lock);
            }
            // >0： 半阻塞等待
            else
            {
                if (std::cv_status::timeout == queue_cv.wait_for(lock, std::chrono::milliseconds(millsecond)))
                {
                    return false;
                }
            }
        }
        if (the_queue.empty())
        {
            return false;
        }

        popped_value = the_queue.front();		//获取队列中的第一个任务
        the_queue.pop_front();					//删除该任务

        return true;
    }
};

#endif // CPPQUEUE_H
