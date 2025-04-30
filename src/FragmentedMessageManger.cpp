#include "FragmentedMessageManager.h"

bool FragmentedMessageManager::FragmentBuffer::complete() const {
    return expectedCount > 0 && countFragments() == expectedCount;
}

String FragmentedMessageManager::FragmentBuffer::reassemble() const {
    String result;
    for (const auto& frag : fragments) {
        result += frag;
    }
    return result;
}

int FragmentedMessageManager::FragmentBuffer::countFragments() const {
    int count = 0;
    for (const auto& frag : fragments) {
        if (frag.length() > 0) count++;
    }
    return count;
}

bool FragmentedMessageManager::isFragment(const String& msg) const {
    return msg.startsWith("FRAG|");
}

bool FragmentedMessageManager::parseFragmentHeader(const String& fragment, String& sessionKey, int& index, int& total, String& data) const {
    int first = fragment.indexOf('|');
    int second = fragment.indexOf('|', first + 1);
    int third = fragment.indexOf('|', second + 1);
    int fourth = fragment.indexOf('|', third + 1);

    if (first == -1 || second == -1 || third == -1 || fourth == -1) return false;

    sessionKey = fragment.substring(first + 1, second);
    index = fragment.substring(second + 1, third).toInt();
    total = fragment.substring(third + 1, fourth).toInt();
    data = fragment.substring(fourth + 1);
    return true;
}

void FragmentedMessageManager::addFragment(const String& rawFragment) {
    String sessionKey;
    int index, total;
    String data;

    if (!parseFragmentHeader(rawFragment, sessionKey, index, total, data)) return;

    FragmentBuffer& buf = fragmentMap[sessionKey];
    if (buf.fragments.empty()) buf.fragments.resize(total);
    buf.fragments[index] = data;
    buf.expectedCount = total;
}

bool FragmentedMessageManager::isComplete(const String& rawFragment) {
    String sessionKey = getSessionKey(rawFragment);
    return fragmentMap.count(sessionKey) && fragmentMap[sessionKey].complete();
}

String FragmentedMessageManager::reassemble(const String& rawFragment) {
    String sessionKey = getSessionKey(rawFragment);
    if (!isComplete(rawFragment)) return "";
    String result = fragmentMap[sessionKey].reassemble();
    fragmentMap.erase(sessionKey);
    return result;
}

String FragmentedMessageManager::getSessionKey(const String& rawFragment) const {
    int first = rawFragment.indexOf('|');
    int second = rawFragment.indexOf('|', first + 1);
    if (first == -1 || second == -1) return "";
    return rawFragment.substring(first + 1, second);
}

std::vector<String> FragmentedMessageManager::fragmentMessage(const String& message, size_t maxLength) {
    std::vector<String> result;
    String sessionKey = String(random(10000, 99999));
    int total = (message.length() + maxLength - 1) / maxLength;

    for (int i = 0; i < total; ++i) {
        int startIdx = i * maxLength;
        String part = message.substring(startIdx, startIdx + maxLength);
        result.push_back("FRAG|" + sessionKey + "|" + String(i) + "|" + String(total) + "|" + part);
    }
    return result;
}
