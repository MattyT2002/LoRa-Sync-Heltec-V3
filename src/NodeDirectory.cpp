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

    // Add the edge for visualization directly here
    // Ensure the edge between the current node and neighbor is created.
    std::set<std::pair<uint16_t, uint16_t>> addedEdges;
    auto edge = std::minmax(selfId, neighborId);
    if (!addedEdges.count(edge)) {
        addedEdges.insert(edge);
    }

    nodes[neighborId].hopCount = 1;
    nodes[neighborId].neighbors[selfId] = { snr, timestamp };

    // Add the edge for the neighbor side as well
    auto newEdge = std::minmax(neighborId, selfId);
    if (!addedEdges.count(newEdge)) {
        addedEdges.insert(newEdge);
    }
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

std::string NodeDirectory::toJson() const {
    StaticJsonDocument<4096> doc; // Maybe bigger depending on your network

    for (const auto& [nodeId, info] : nodes) {
        JsonObject nodeObj = doc.createNestedObject(String(nodeId));
        nodeObj["hopCount"] = info.hopCount;

        JsonObject neighbors = nodeObj.createNestedObject("neighbors");
        for (const auto& [neighborId, link] : info.neighbors) {
            JsonObject neighbor = neighbors.createNestedObject(String(neighborId));
            neighbor["snr"] = link.snr;
            neighbor["timestamp"] = link.lastSeen; // Use lastSeen for timestamp
        }
    }

    std::string result;
    serializeJson(doc, result);
    return result;
}

void NodeDirectory::fromJson(const std::string& json) {
    StaticJsonDocument<4096> doc;
    deserializeJson(doc, json);

    nodes.clear(); // Clear old data

    for (JsonPair node : doc.as<JsonObject>()) {
        uint16_t nodeId = String(node.key().c_str()).toInt();
        JsonObject nodeObj = node.value();
        NodeInfo info;
        info.hopCount = nodeObj["hopCount"] | 255; // default invalid hop count if missing

        JsonObject neighbors = nodeObj["neighbors"];
        for (JsonPair neighbor : neighbors) {
            uint16_t neighborId = String(neighbor.key().c_str()).toInt();
            JsonObject neighborObj = neighbor.value();
            LinkInfo link;
            link.snr = neighborObj["snr"] | 0;
            link.lastSeen = neighborObj["timestamp"] | 0; // default invalid timestamp if missing
            info.neighbors[neighborId] = link;
        }

        nodes[nodeId] = info;
    }
}

