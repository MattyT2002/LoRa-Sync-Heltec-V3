#include "NodeDirectory.h"
#include <ArduinoJson.h>
#include <set>
#include <vector>
#include <queue>
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
void NodeDirectory::updateNeighbourNode(uint16_t neighbourNodeId, float snr, unsigned long timeLastSeen)
{
    // Make sure self node exists
    if (!hasNode(selfId)) {
        nodes[selfId] = NodeInfo();
        nodes[selfId].hopCount = 0;
    }

    // Update or add the neighbor link
    LinkInfo link;
    link.snr = snr;
    link.lastSeen = timeLastSeen;

    nodes[selfId].neighbors[neighbourNodeId] = link;

    
    if (!hasNode(neighbourNodeId)) {
        nodes[neighbourNodeId] = NodeInfo();
    }
}


void NodeDirectory::mergeDirectory(const NodeDirectory& other, uint16_t viaNodeId) {
    unsigned long now = millis();
    const int estimatedSNR = 0;

    for (const auto& [nodeId, info] : other.getAllNodes()) {
        if (nodeId == selfId) continue;  // Skip self node

        // Add the node if it doesn't already exist in this directory
        if (!hasNode(nodeId)) {
            nodes[nodeId] = info;
        }

        // Propagate the neighbor links from the other node to this node and vice versa
        for (const auto& [neighborId, link] : info.neighbors) {
            nodes[nodeId].neighbors[neighborId] = link;      // Add the link to this node's neighbors
            nodes[neighborId].neighbors[nodeId] = link;      // Add the reverse link to the neighbor's neighbors
        }
    }
}



void NodeDirectory::removeStaleNodes(unsigned long timeoutMs) {
    unsigned long now = millis();
    std::vector<uint16_t> toRemove;

    // Check neighbors of each node for stale connections
    for (auto& [nodeId, nodeInfo] : nodes) {
        for (auto& [neighborId, link] : nodeInfo.neighbors) {
            if (now - link.lastSeen > timeoutMs) {
                toRemove.push_back(neighborId); // Mark stale neighbor for removal
            }
        }
    }

    // Remove stale links from all nodes
    for (auto& nodeId : toRemove) {
        for (auto& [key, nodeInfo] : nodes) {
            nodeInfo.neighbors.erase(nodeId); // Remove stale link from all nodes
        }
    }

    // Remove nodes that no longer have neighbors, except for self node
    for (auto it = nodes.begin(); it != nodes.end(); ) {
        if (it->first != selfId && it->second.neighbors.empty()) {
            it = nodes.erase(it); // Remove node without neighbors
        } else {
            ++it;
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

            auto edge = std::make_pair(std::min(fromId, toId), std::max(fromId, toId));
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

int NodeDirectory::getNextHopTo(uint16_t destinationId) const {
    if (destinationId == selfId) return selfId;

    std::map<uint16_t, uint16_t> previous; // current node -> previous node
    std::queue<uint16_t> q;
    std::set<uint16_t> visited;

    q.push(selfId);
    visited.insert(selfId);

    while (!q.empty()) {
        uint16_t current = q.front();
        q.pop();

        auto it = nodes.find(current);
        if (it == nodes.end()) continue;

        for (const auto& [neighborId, link] : it->second.neighbors) {
            if (visited.count(neighborId)) continue;

            visited.insert(neighborId);
            previous[neighborId] = current;
            q.push(neighborId);

            if (neighborId == destinationId) {
                // Found destination; backtrack to find next hop
                uint16_t step = destinationId;
                while (previous[step] != selfId) {
                    step = previous[step];
                }
                return step;
            }
        }
    }

    return -1; // No path found
}
