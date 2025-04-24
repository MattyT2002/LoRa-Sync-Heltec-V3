#include "NodeDirectory.h"
#include <ArduinoJson.h>
#include <set>
NodeDirectory::NodeDirectory() : selfId(0) {}

void NodeDirectory::setSelfId(uint16_t id) {
    selfId = id;
    nodes[selfId].hopCount = 0;
}
// returns the Id of the local node
uint16_t NodeDirectory::getSelfId() const {
    return selfId;
}
// checks to see if node is in the directory
bool NodeDirectory::hasNode(uint16_t nodeId) const {
    return nodes.find(nodeId) != nodes.end();
}
// returns the informatio of the node if in directory, otherwise returns nullptr
const NodeInfo* NodeDirectory::getNode(uint16_t nodeId) const {
    auto it = nodes.find(nodeId);
    return (it != nodes.end()) ? &it->second : nullptr;
}
// returns all nodes in the directory
const std::map<uint16_t, NodeInfo>& NodeDirectory::getAllNodes() const {
    return nodes;
}
// updates the directory based on information from a neighbor node
void NodeDirectory::updateNeighbourNode(uint16_t neighborId, int snr, unsigned long timestamp) {
    nodes[selfId].hopCount = 0;
    nodes[selfId].neighbors[neighborId] = { snr, timestamp };

    nodes[neighborId].hopCount = 1;
    nodes[neighborId].neighbors[selfId] = { snr, timestamp };
}

// merges the directory with another node's directory allowing for nowing 
// the path to nodes more than one hop away
void NodeDirectory::mergeDirectory(const NodeDirectory& other, uint16_t viaNodeId) {
    unsigned long now = millis();
    const int estimatedSNR = 0;

    for (const auto& [nodeId, info] : other.getAllNodes()) {
        if (nodeId == selfId) continue;

        int hopThroughVia = info.hopCount + 1;

        if (!hasNode(nodeId) || hopThroughVia < nodes[nodeId].hopCount) {
            nodes[nodeId].hopCount = hopThroughVia;
            nodes[nodeId].neighbors[viaNodeId] = { estimatedSNR, now };
            nodes[viaNodeId].neighbors[nodeId] = { estimatedSNR, now };
        }

        
        for (const auto& [neighborId, link] : info.neighbors) {
            if (neighborId == selfId) continue;
            nodes[nodeId].neighbors[neighborId] = link;
        }
    }
}
// converts the directory to a JSON format for visualization
std::string NodeDirectory::toVisJson() const {
    StaticJsonDocument<2048> doc;
    JsonArray jNodes = doc.createNestedArray("nodes");
    JsonArray jEdges = doc.createNestedArray("edges");

    for (const auto& [nodeId, info] : nodes) {
        JsonObject jNode = jNodes.createNestedObject();
        jNode["id"] = nodeId;
        jNode["label"] = "Node " + String(nodeId);
        jNode["title"] = "Hop Count: " + String(info.hopCount);
    }

    std::set<std::pair<uint16_t, uint16_t>> addedEdges;

    for (const auto& [fromId, info] : nodes) {
        for (const auto& [toId, link] : info.neighbors) {
            if (fromId == toId) continue;

            auto edge = std::minmax(fromId, toId);
            if (addedEdges.count(edge)) continue;

            JsonObject jEdge = jEdges.createNestedObject();
            jEdge["from"] = fromId;
            jEdge["to"] = toId;
            jEdge["label"] = "SNR: " + String(link.snr);

            addedEdges.insert(edge);
        }
    }

    std::string result;
    serializeJson(doc, result);
    return result;
}
