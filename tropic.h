#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <memory>
#include <cassert>
#include <typeinfo>

#include <iostream>

namespace tropic
{
class IMessage;
class ITopic;
class Subscriber;
class Publisher;

using topic_tag_t = std::string;

static std::unordered_map<topic_tag_t, std::unique_ptr<ITopic>> topics;

class IMessage 
{
public:
    const unsigned int ID;
    
    IMessage(unsigned int id) : ID(id) {};
    ~IMessage() = default;
};

template <typename T>
class Message : public IMessage
{
public:
    const T DATA;

    Message(unsigned int id, T data) : IMessage(id), DATA(data) {}
    ~Message() = default;
};

class ITopic
{
public:
    const topic_tag_t TAG;

    ITopic(topic_tag_t tag) : TAG(tag) {};
    virtual ~ITopic() = default;

    virtual std::string getDataType() = 0;

}; // class ITopic

template <typename T>
class Topic : public ITopic
{
private:
    std::unordered_set<Subscriber*> m_subscribers;

public:
    Topic(topic_tag_t tag) : ITopic(tag) {}
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
            s->pushMessage(Message<T>(1, data));
        }
    }

}; // class Topic

class Publisher
{
public:
    Publisher() = default;
    ~Publisher() = default;

    template <typename T>
    void addTopic(const topic_tag_t& tag) const
    {
        assert(topics.count(tag) == 0 && "Tag already taken.");

        topics.insert({ tag, std::make_unique<Topic<T>>(tag) });
    }

    bool hasTopic(const topic_tag_t& tag) const
    {
        return topics.count(tag);
    }

    template <typename T>
    void publish(const topic_tag_t& tag, T data) const
    {
        assert(topics.count(tag) == 1 &&
            topics.at(tag)->getDataType() == typeid(T).name() &&
            "Topic does not exist.");
        
        Topic<T>* topic = static_cast<Topic<T>*>(topics.at(tag).get());
        topic->pushMessage(data);
    }

    template <typename T>
    void operator()(const topic_tag_t& tag, T data) const
    {
        publish(tag, data);
    }

};  // class Publisher

class Subscriber
{
private:
    std::queue<std::unique_ptr<IMessage>> m_message_queue;

public:
    Subscriber() = default;
    ~Subscriber() = default;

    template <typename T>
    void subscribe(const topic_tag_t& tag)
    {
        assert(topics.count(tag) == 1 &&
            topics.at(tag)->getDataType() == typeid(T).name() &&
            "Topic does not exist.");

        Topic<T>* topic = static_cast<Topic<T>*>(topics.at(tag).get());
        topic->addSubscriber(this);
    }

    void pushMessage(IMessage message)
    {
        m_message_queue.push(std::make_unique<IMessage>(message));
        std::cout << "Got Message: " << message.ID << std::endl;
    }

}; // class Subscriber

} // namespace tropic