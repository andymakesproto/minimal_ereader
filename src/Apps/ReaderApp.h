#pragma once
#include "BaseApp.h"
#include <vector>
#include <map>
#include <Arduino.h>

class ReaderApp : public BaseApp {
private:
    uint32_t currentOffset;
    uint32_t nextOffset;
    std::vector<uint32_t> pageHistory;
    uint32_t lastDrawTime;
    
    // RAM-only Bookmark Storage (resets on power loss during MVP development)
    static std::map<String, uint32_t> ramBookmarks;

    void renderPage(bool fastRefresh);

public:
    ReaderApp(SystemContext* context);
    
    void onStart() override;
    void onLoop() override;
    void onButtonEvent(int btnId, int eventType) override;
    void onExit() override;
};