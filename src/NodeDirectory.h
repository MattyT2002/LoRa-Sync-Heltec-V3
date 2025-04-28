#pragma once
#include <Arduino.h>
#include <map>
#include <ArduinoJson.h>
#include "NodeDirectory.h"
struct LinkInfo {
    int snr;
    unsigned long lastSeen;
};

struct NodeInfo {
    int hopCount;
    std::map<uint16_t, LinkInfo> neighbors;
};

class NodeDirectory {
public:
    NodeDirectory();

    void setSelfId(uint16_t id);
    uint16_t getSelfId() const;

    void updateNeighbourNode(uint16_t neighborId, int snr, unsigned long timestamp);
    void mergeDirectory(const NodeDirectory& other, uint16_t viaNodeId);
    void fromJson(const std::string& jsonPayload);
    void toJson(std::string& jsonPayload) const;
    std::string toVisJson() const;
    std::string toJson() const;
    // Accessors (optional if needed)
    bool hasNode(uint16_t nodeId) const;
    const NodeInfo* getNode(uint16_t nodeId) const;
    const std::map<uint16_t, NodeInfo>& getAllNodes() const;

private:
    uint16_t selfId;
    std::map<uint16_t, NodeInfo> nodes;
};
