// FragmentedMessageManager.cpp
#include "FragmentedMessageManager.h"

std::vector<String> FragmentedMessageManager::fragment(const String& message, const String& type, int maxPayload) {
    std::vector<String> result;
    int total = (message.length() + maxPayload - 1) / maxPayload;

    for (int i = 0; i < total; ++i) {
        int start = i * maxPayload;
        int end = start + maxPayload;
        String chunk = message.substring(start, min(end, (int)message.length()));
        String header = type + "|" + String(i + 1) + "/" + String(total) + "|";
        result.push_back(header + chunk);
    }
    return result;
}

void FragmentedMessageManager::acceptFragment(const String& raw, int senderId) {
    int firstPipe = raw.indexOf('|');
    int secondPipe = raw.indexOf('|', firstPipe + 1);
    if (firstPipe == -1 || secondPipe == -1) return;

    String partInfo = raw.substring(firstPipe + 1, secondPipe);
    String data = raw.substring(secondPipe + 1);

    int slash = partInfo.indexOf('/');
    if (slash == -1) return;

    int partNum = partInfo.substring(0, slash).toInt();
    int total = partInfo.substring(slash + 1).toInt();

    FragmentSet &fs = fragmentBuffer[senderId];
    fs.totalParts = total;
    fs.parts[partNum] = data;
}

bool FragmentedMessageManager::isComplete(int senderId) {
    FragmentSet &fs = fragmentBuffer[senderId];
    return fs.parts.size() == fs.totalParts;
}

String FragmentedMessageManager::reassemble(int senderId) {
    String full;
    FragmentSet &fs = fragmentBuffer[senderId];
    for (int i = 1; i <= fs.totalParts; ++i) {
        full += fs.parts[i];
    }
    return full;
}

void FragmentedMessageManager::clear(int senderId) {
    fragmentBuffer.erase(senderId);
}
