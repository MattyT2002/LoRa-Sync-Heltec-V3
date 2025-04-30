#pragma once
#include <Arduino.h>
#include <vector>
#include <map>

class FragmentedMessageManager {
public:
    struct FragmentBuffer {
        std::vector<String> fragments;
        int expectedCount = -1;

        bool complete() const;
        String reassemble() const;
        int countFragments() const;
    };

    void addFragment(const String& rawFragment);
    bool isFragment(const String& msg) const;
    bool isComplete(const String& rawFragment);
    String reassemble(const String& rawFragment);
    std::vector<String> fragmentMessage(const String& message, size_t maxLength);

private:
    std::map<String, FragmentBuffer> fragmentMap;

    bool parseFragmentHeader(const String& fragment, String& sessionKey, int& index, int& total, String& data) const;
    String getSessionKey(const String& rawFragment) const;
};
