#pragma once
#include "BaseApp.h"
#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>

class TransferApp : public BaseApp {
private:
    WebServer server;
    File uploadFile;
    
    // Add this new helper function
    String generateHTML();
    
    void setupWebServer();
    
public:
    TransferApp(SystemContext* context);
    void onStart() override;
    void onLoop() override;
    void onButtonEvent(int btnId, int eventType) override;
    void onExit() override;
};