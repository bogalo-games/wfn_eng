#include "message_bus.hpp"

namespace wfn_eng::engine {
    ////
    // struct Message
    //
    // An interface defining the required features of a message.

    ////
    // Message(MessageType)
    //
    // Constructs a message with a given type. Must be called by any Message
    // that inherits from this type.
    Message::Message(MessageType type) :
            type(type) { }

    ////
    // class MessageBus
    //
    // A message bus for forwarding `Message`s of certain `MessageType`s to a
    // list `MessageHandler`s.

    ////
    // MessageBus()
    //
    // A default constructor that constructs `messageHandlers`.
    MessageBus::MessageBus() { }

    ////
    // MessageBus& instance()
    //
    // Provides a singleton reference to the MessageBus.
    MessageBus& MessageBus::instance() {
        static MessageBus messageBus;
        return messageBus;
    }

    ////
    // void subscribe(MessageType, MessageHandler)
    //
    // Adds a `MessageHandler` to the set of `MessageHandler`s that respond
    // to a given kind of `MessageType`.
    void MessageBus::subscribe(MessageType type, MessageHandler handler) {
        messageHandlers[type].push_back(handler);
    }

    ////
    // void alert(Message&)
    //
    // Asks the `MessageBus` to forward a `Message` to a all of the
    // `MessageHandler`s that are subscribed to the `MessageType`.
    void MessageBus::alert(Message& message) {
        auto& handlers = messageHandlers[message.type];
        for (auto& handler: handlers)
            handler(message);
    }
}
