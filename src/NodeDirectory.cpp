#include "NodeDirectory.h"
#include <ArduinoJson.h>
#include <set>
#include <vector>
#include <queue>

NodeDirectory::NodeDirectory() : selfId(0) {}

void NodeDirectory::setSelfId(uint16_t id) {
    selfId = id;
    nodes[selfId] = NodeInfo(); // Initialize self node
}

uint16_t NodeDirectory::getSelfId() const {
    return selfId;
}

bool NodeDirectory::hasNode(uint16_t nodeId) const {
    return nodes.find(nodeId) != nodes.end();
}

const NodeInfo* NodeDirectory::getNode(uint16_t nodeId) const {
    auto it = nodes.find(nodeId);
    return (it != nodes.end()) ? &it->second : nullptr;
}

const std::map<uint16_t, NodeInfo>& NodeDirectory::getAllNodes() const {
    return nodes;
}

void NodeDirectory::updateNeighbourNode(uint16_t neighbourNodeId, float snr, unsigned long timeLastSeen) {
    if (neighbourNodeId == 65535 || neighbourNodeId == 0) return;

    LinkInfo link{snr, timeLastSeen};
    nodes[selfId].neighbors[neighbourNodeId] = link;

    if (!hasNode(neighbourNodeId)) {
        nodes[neighbourNodeId] = NodeInfo();
    }
}

void NodeDirectory::mergeDirectory(const NodeDirectory& other, uint16_t viaNodeId) {
    for (const auto& [nodeId, info] : other.getAllNodes()) {
        if (nodeId == selfId) continue;

        if (!hasNode(nodeId)) {
            nodes[nodeId] = info;
        }

        for (const auto& [neighborId, link] : info.neighbors) {
            nodes[nodeId].neighbors[neighborId] = link;
            nodes[neighborId].neighbors[nodeId] = link;
        }
    }
}

void NodeDirectory::removeStaleNodes(unsigned long timeoutMs) {
    unsigned long now = millis();
    std::vector<uint16_t> toRemove;

    for (auto& [nodeId, nodeInfo] : nodes) {
        for (auto& [neighborId, link] : nodeInfo.neighbors) {
            if (now - link.lastSeen > timeoutMs) {
                toRemove.push_back(neighborId);
            }
        }
    }

    for (auto& nodeId : toRemove) {
        for (auto& [_, nodeInfo] : nodes) {
            nodeInfo.neighbors.erase(nodeId);
        }
    }

    for (auto it = nodes.begin(); it != nodes.end(); ) {
        if (it->first != selfId && it->second.neighbors.empty()) {
            it = nodes.erase(it);
        } else {
            ++it;
        }
    }
}

std::string NodeDirectory::toVisJson() const {
    StaticJsonDocument<2048> doc;
    JsonArray jNodes = doc.createNestedArray("nodes");
    JsonArray jEdges = doc.createNestedArray("edges");

    for (const auto& [nodeId, _] : nodes) {
        JsonObject jNode = jNodes.createNestedObject();
        jNode["id"] = nodeId;
        jNode["label"] = "Node " + String(nodeId);
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
    StaticJsonDocument<4096> doc;
    for (const auto& [nodeId, info] : nodes) {
        JsonObject nodeObj = doc.createNestedObject(String(nodeId));
        JsonObject neighbors = nodeObj.createNestedObject("neighbors");
        for (const auto& [neighborId, link] : info.neighbors) {
            JsonObject neighbor = neighbors.createNestedObject(String(neighborId));
            neighbor["snr"] = link.snr;
            neighbor["timestamp"] = link.lastSeen;
        }
    }

    std::string result;
    serializeJson(doc, result);
    return result;
}

void NodeDirectory::fromJson(const std::string& json) {
    StaticJsonDocument<4096> doc;
    deserializeJson(doc, json);
    nodes.clear();

    for (JsonPair node : doc.as<JsonObject>()) {
        uint16_t nodeId = String(node.key().c_str()).toInt();
        NodeInfo info;
        JsonObject neighbors = node.value()["neighbors"];
        for (JsonPair neighbor : neighbors) {
            uint16_t neighborId = String(neighbor.key().c_str()).toInt();
            JsonObject neighborObj = neighbor.value();
            LinkInfo link{neighborObj["snr"] | 0, neighborObj["timestamp"] | 0};
            info.neighbors[neighborId] = link;
        }
        nodes[nodeId] = info;
    }
}

int NodeDirectory::getNextHopTo(uint16_t destinationId) const {
    if (destinationId == selfId) return selfId;

    std::map<uint16_t, uint16_t> previous;
    std::queue<uint16_t> q;
    std::set<uint16_t> visited;

    q.push(selfId);
    visited.insert(selfId);

    while (!q.empty()) {
        uint16_t current = q.front(); q.pop();

        auto it = nodes.find(current);
        if (it == nodes.end()) continue;

        for (const auto& [neighborId, link] : it->second.neighbors) {
            if (visited.count(neighborId)) continue;
            visited.insert(neighborId);
            previous[neighborId] = current;
            q.push(neighborId);

            if (neighborId == destinationId) {
                uint16_t step = destinationId;
                while (previous[step] != selfId) {
                    step = previous[step];
                }
                return step;
            }
        }
    }

    return -1;
}

String NodeDirectory::encodeDirectorySimplified() {
    String result = "DIR_UPDATE";
    for (const auto& [nodeId, info] : nodes) {
        result += "|" + String(nodeId);
        for (const auto& [neighborId, link] : info.neighbors) {
            result += "|" + String(neighborId) + "," + String(link.snr);
        }
    }
    return result;
}

void NodeDirectory::decodeDirectorySimplified(const String& data) {
    int startIdx = 0;
    while (startIdx < data.length()) {
        int nextPipe = data.indexOf('|', startIdx);
        if (nextPipe == -1) break;
        String nodeIdStr = data.substring(startIdx, nextPipe);
        int nodeId = nodeIdStr.toInt();
        startIdx = nextPipe + 1;

        NodeInfo info;
        while (true) {
            nextPipe = data.indexOf('|', startIdx);
            if (nextPipe == -1) break;
            String neighborData = data.substring(startIdx, nextPipe);
            int commaPos = neighborData.indexOf(',');
            if (commaPos == -1) break;
            int neighborId = neighborData.substring(0, commaPos).toInt();
            int snr = neighborData.substring(commaPos + 1).toInt();
            LinkInfo link{(float)snr, millis()};
            info.neighbors[neighborId] = link;
            startIdx = nextPipe + 1;
        }

        nodes[nodeId] = info;
    }
}
