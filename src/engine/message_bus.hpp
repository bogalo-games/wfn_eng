#ifndef __WFN_ENG_ENGINE_MESSAGE_BUS_HPP__
#define __WFN_ENG_ENGINE_MESSAGE_BUS_HPP__

#include <vector>
#include <map>

namespace wfn_eng::engine {
    ////
    // enum MessageType
    //
    // An enumeration of all message types.
    enum MessageType {
    };

    ////
    // struct Message
    //
    // An interface defining the required features of a message.
    struct Message {
        ////
        // MessageType type
        //
        // The type of the Message. Used in the MessageBus to forward messages
        // to entities that subscribe to an event.
        const MessageType type;

        ////
        // Message(MessageType)
        //
        // Constructs a message with a given type. Must be called by any Message
        // that inherits from this type.
        Message(MessageType);
    };

    ////
    // using MessageHandler
    //
    // A type synonym for a function that performs some action given a Message.
    using MessageHandler = std::function<void(Message&)>;

    ////
    // class MessageBus
    //
    // A message bus for forwarding `Message`s of certain `MessageType`s to a
    // list `MessageHandler`s.
    class MessageBus {
        std::map<MessageType, std::vector<MessageHandler>> messageHandlers;

        ////
        // MessageBus()
        //
        // A default constructor that constructs `messageHandlers`.
        MessageBus();

    public:
        ////
        // MessageBus& instance()
        //
        // Provides a singleton reference to the MessageBus.
        static MessageBus& instance();

        ////
        // void subscribe(MessageType, MessageHandler)
        //
        // Adds a `MessageHandler` to the set of `MessageHandler`s that respond
        // to a given kind of `MessageType`.
        void subscribe(MessageType, MessageHandler);

        ////
        // void alert(Message&)
        //
        // Asks the `MessageBus` to forward a `Message` to a all of the
        // `MessageHandler`s that are subscribed to the `MessageType`.
        void alert(Message&);


        // Rule of 5's
        MessageBus(const MessageBus&) = delete;
        MessageBus(Message&&) = delete;
        MessageBus& operator=(const MessageBus&) = delete;
        MessageBus& operator=(Message&&) = delete;
    };
}

#endif
