```cpp
#include <thread> 

#include "tropic.h"

// Create a publisher
tropic::Publisher pub;
pub.addTopic<int>("my_topic");

// Create a subscriber
tropic::Subscriber sub;
sub.subscribe<int>("my_topic");

// Publish data to topic
pub.publish("my_topic", 42);

// Subscriber is busy waiting for messsage on some other thread
std::thread t([]{
    while(true)
    {
        // While there are messages to process
        while(sub.gotMessage())
        {
            // Check what topic the next message has
            if(sub.nextMessageTopic() == "my_topic")
            {
                // Fetch the message
                int message = static_cast<int>(sub.popMessage());
            }
        }
    }
});
```