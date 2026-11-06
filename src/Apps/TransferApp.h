/*
Description: Declares the Wi-Fi transfer screen used to load books onto the device.
Inputs: WebServer server, uploadFile, activeWifiPassword, and SystemContext* ctx.
Process: Sets up the file transfer interface and exposes the lifecycle hooks used by TransferApp.cpp.
Output: A transfer app that can open the Wi-Fi upload page and handle new files.
*/

#pragma once
#include "BaseApp.h"
#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>

class TransferApp : public BaseApp {
private:
    WebServer server;
    File uploadFile;
    String activeWifiPassword;
    
    // Helper function
    String generateHTML();
    
    void setupWebServer();
    
public:
    TransferApp(SystemContext* context);
    void onStart() override;
    void onLoop() override;
    void onButtonEvent(int btnId, int eventType) override;
    void onExit() override;
    void prepareForSleep() override;
    void forceRedraw() override {
        onStart();
    }
};