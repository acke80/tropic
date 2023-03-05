#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <memory>
#include <cassert>
#include <typeinfo>
#include <any>
#include <mutex>

namespace tropic
{
class ITopic;
class Subscriber;
class Publisher;
struct Message;

using topic_tag_t = std::string;

static std::unordered_map<topic_tag_t, std::unique_ptr<ITopic>> topics;

struct Message
{
    topic_tag_t topic_tag;
    std::any    data;
}; // struct Message

class ITopic
{
public:
    const topic_tag_t TOPIC_TAG;

    ITopic(topic_tag_t topic_tag) : TOPIC_TAG(topic_tag) {};
    virtual ~ITopic() = default;

    virtual std::string getDataType() = 0;

}; // class ITopic

template <typename T>
class Topic : public ITopic
{
private:
    std::unordered_set<Subscriber*> m_subscribers;

public:
    Topic(topic_tag_t topic_tag) : ITopic(topic_tag) {}
    ~Topic() = default;

    std::string getDataType()
    {
        return typeid(T).name();
    }

    void addSubscriber(Subscriber* subscriber)
    {
        m_subscribers.insert(subscriber);
    }

    void pushMessage(T data)
    {
        for (Subscriber* s : m_subscribers)
        {
            s->pushMessage({ TOPIC_TAG, data });
        }
    }

}; // class Topic

class Publisher
{
public:
    Publisher() = default;
    ~Publisher() = default;

    template <typename T>
    void addTopic(const topic_tag_t& topic_tag) const
    {
        assert(topics.count(topic_tag) == 0 && "topic_tag already taken.");

        topics.insert({ topic_tag, std::make_unique<Topic<T>>(topic_tag) });
    }

    bool hasTopic(const topic_tag_t& topic_tag) const
    {
        return topics.count(topic_tag);
    }

    template <typename T>
    void publish(const topic_tag_t& topic_tag, T data) const
    {
        assert(topics.count(topic_tag) == 1 &&
            topics.at(topic_tag)->getDataType() == typeid(T).name() &&
            "Topic does not exist.");
        
        Topic<T>* topic = static_cast<Topic<T>*>(topics.at(topic_tag).get());
        topic->pushMessage(data);
    }

    template <typename T>
    void operator()(const topic_tag_t& topic_tag, T data) const
    {
        publish(topic_tag, data);
    }

};  // class Publisher

class Subscriber
{
private:    
    std::queue<Message> m_message_queue;
    std::mutex m_queue_mutex;

public:
    Subscriber() = default;
    ~Subscriber() = default;

    template <typename T>
    void subscribe(const topic_tag_t& topic_tag)
    {
        assert(topics.count(topic_tag) == 1 &&
            topics.at(topic_tag)->getDataType() == typeid(T).name() &&
            "Topic does not exist.");

        Topic<T>* topic = static_cast<Topic<T>*>(topics.at(topic_tag).get());
        topic->addSubscriber(this);
    }

    bool gotMessage()
    {
        m_queue_mutex.lock();
        bool empty = m_message_queue.empty();
        m_queue_mutex.unlock();

        return !empty;
    }

    Message popMessage()
    {
        assert(m_message_queue.size() > 0 && "Trying to pop empty message queue.");

        m_queue_mutex.lock();
        Message message = m_message_queue.front();
        m_message_queue.pop();
        m_queue_mutex.unlock();

        return message;
    }

    void pushMessage(Message message)
    {
        m_queue_mutex.lock();
        m_message_queue.push(message);
        m_queue_mutex.unlock();
    }

}; // class Subscriber

} // namespace tropic