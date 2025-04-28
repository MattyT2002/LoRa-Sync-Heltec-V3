#include "Packet.h"


Packet::Packet() : type(UNKNOWN) {}

Packet::Packet(const String&)
    : type(UNKNOWN){}

String Packet::helloMessage() {
    return "LoRaMeshNode|" NODE_name "|" + String(NODE_number) + "|";
}

String Packet::dirUpdateMessage(const String& updatePayload) {
    return "DIR_UPDATE|" + String(NODE_number) + "|" + updatePayload;
}

String Packet::messageToSend(const String& payload) const {
    return "MESSAGE|" NODE_name "|" + String(NODE_number) + "|" + payload;
}

String Packet::getnodeName() const {
    return senderName; 
}

String Packet::getnodeNumber() const {
    return senderNumber; 
}



String Packet::serializeDirectory(const NodeDirectory& directory) const {
    return String(directory.toJson().c_str()); // call NodeDirectory::toJson()
}

bool Packet::deserializeDirectory(NodeDirectory& directory, const String& jsonPayload) const {
    directory.fromJson(std::string(jsonPayload.c_str())); // call NodeDirectory::fromJson()
    return true; // You could add error checking if you want
}

bool Packet::deserialize(const String& data) {
    int firstSep = data.indexOf('|');
    int secondSep = data.indexOf('|', firstSep + 1);
    int thirdSep = data.indexOf('|', secondSep + 1);
    if (firstSep == -1 || secondSep == -1 || thirdSep == -1) return false;
    String typeStr = data.substring(0, firstSep);
    senderName = data.substring(firstSep + 1, secondSep);
    senderNumber = data.substring(secondSep + 1, thirdSep);
    type = detectType(typeStr);
    if (type == UNKNOWN) return false;
    return true;
}

Packet::Type Packet::detectType(const String& rawType) {
    if (rawType == "LoRaMeshNode") return HELLO;
    if (rawType == "DIR_UPDATE") return DIR_UPDATE;
    if (rawType == "MESSAGE") return MESSAGE;
    return UNKNOWN;
}

String Packet::typeToString(Packet::Type type) {
    switch (type) {
        case HELLO: return "LoRaMeshNode";
        case DIR_UPDATE: return "DIR_UPDATE";
        default: return "UNKNOWN";
    }
}
