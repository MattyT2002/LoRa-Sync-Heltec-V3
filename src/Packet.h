#ifndef PACKET_H
#define PACKET_H
#include "config.h"
#include <Arduino.h>
#include "NodeDirectory.h" 
class Packet {
public:
    enum Type {
        HELLO,
        DIR_UPDATE,
        MESSAGE,
        MESSAGE_Hop,
        UNKNOWN
    };

    // Core packet fields
    
    // Constructors
    Packet();
    Packet(const String& payload); // Type is optional

   
    String helloMessage();
    String dirUpdateMessage(const String& updatePayload);
    String messageToSend(const String& payload) const;
    String getnodeNumber() const;
    String getPayload(const String& data) const;
    String getMessageType(const String &data) const;
    String getnodeName() const;
    String getPacketType() const;
    String serializeDirectory(const NodeDirectory& directory) const;
    String getDestinationNode(const String& data) const;

    bool deserializeDirectory(NodeDirectory& directory, const String& jsonPayload) const;
    bool deserialize(const String& data);

    // Type helpers
    static Type detectType(const String& rawType);
    static String typeToString(Type type);

private:
    Type type;
    String senderName;
    String senderNumber;

};

#endif // PACKET_H

