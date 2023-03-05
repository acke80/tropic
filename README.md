# :palm_tree: Tropic 
## A single header Publish-subscribe implementation in C++17

```cpp
#include <iostream>
#include <thread> 
#include <chrono>

#include "tropic.h"

int main()
{
    // Create a publisher
    tropic::Publisher pub;
    pub.addTopic<int>("my_topic");

    // Create a subscriber
    tropic::Subscriber sub;
    sub.subscribe<int>("my_topic");

    // Subscriber is busy waiting for messsages on some other thread
    std::thread sub_thread([&]()
    {
        while (true)
        {
            // While there are messages to process
            while (sub.gotMessage())
            {
                std::cout << "Got Message!";

                // Pop the message
                tropic::Message message = sub.popMessage();

                // Check the message topic
                if (message.topic_tag == "my_topic")
                {
                    // Cast the data properly
                    int data = std::any_cast<int>(message.data);

                    std::cout << " Data: " << data << std::endl;
                }
            }
        }
    });

    // Publish some data on main thread
    using namespace std::chrono_literals;
    for (int i = 0; i < 10; ++i)
    {
        pub("my_topic", i); // pub.publish("my_topic", i);
        std::this_thread::sleep_for(1s);
    }

    sub_thread.join();
}
```

Produces the following output:
```code
Got Message! Data: 0
Got Message! Data: 1
Got Message! Data: 2
Got Message! Data: 3
Got Message! Data: 4
Got Message! Data: 5
Got Message! Data: 6
Got Message! Data: 7
Got Message! Data: 8
Got Message! Data: 9
```
