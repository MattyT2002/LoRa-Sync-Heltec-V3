// FragmentedMessageManager.h
#pragma once
#include <Arduino.h>
#include <vector>
#include <map>

class FragmentedMessageManager {
public:
    static std::vector<String> fragment(const String& message, const String& type, int maxPayload = 180);

    void acceptFragment(const String& raw, int senderId);
    bool isComplete(int senderId);
    String reassemble(int senderId);
    void clear(int senderId);

private:
    struct FragmentSet {
        std::map<int, String> parts;
        int totalParts = 0;
    };

    std::map<int, FragmentSet> fragmentBuffer;
};
