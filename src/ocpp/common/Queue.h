#ifndef QUEUE_H
#define QUEUE_H

#include <limits>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>


namespace ocpp1_6
{
    namespace common
    {
        template <typename ItemType, size_t MAX_SIZE = std::numeric_limits<size_t>::max()>
        class Queue
        {
        public:
            Queue():m_mutex(), m_cond_var(), m_queue(), m_enabled(true){}
            virtual ~Queue() {}

            size_t size() const {return MAX_SIZE;}

            bool empty() const
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                return m_queue.empty();
            }

            bool full() const
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                return (MAX_SIZE == m_queue.size());
            }

            size_t count()
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                return m_queue.size();
            }

            bool push(const ItemType& item)
            {
                bool ret = false;
                std::unique_lock<std::mutex> lock(m_mutex);

                if (MAX_SIZE > m_queue.size())
                {
                    m_queue.push(item);
                    m_cond_var.notify_one();
                    ret = true;
                }

                return ret;
            }

            bool push(ItemType&& item)
            {
                bool ret = false;
                std::unique_lock<std::mutex> lock(m_mutex);

                if (MAX_SIZE > m_queue.size())
                {
                    m_queue.push(std::move(item));
                    m_cond_var.notify_one();
                    ret = true;
                }
                return ret;
            }

            bool pop(ItemType& item, unsigned int ms_timeout = std::numeric_limits<unsigned int>::max())
            {
                bool ret = false;
                std::unique_lock<std::mutex> lock(m_mutex);

                if (m_cond_var.wait_for(lock, std::chrono::milliseconds(ms_timeout),
                                        [this]{ return !m_enabled || !m_queue.empty();}))
                {

                    if (m_enabled)
                    {
                        item = std::move(m_queue.front());
                        m_queue.pop();
                        ret = true;
                    }
                }
                return ret;
            }

            void clear()
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                while (!m_queue.empty())
                {
                    m_queue.pop();
                }
            }

            void setEnable(bool enabled)
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_enabled = enabled;
                m_cond_var.notify_all();
            }

        private:
            mutable std::mutex m_mutex;
            std::condition_variable m_cond_var;
            std::queue<ItemType> m_queue;
            std::atomic<bool> m_enabled;
        };
    }
}

#endif /* QUEUE_H */
