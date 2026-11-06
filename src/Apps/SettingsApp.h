/*
Description: Declares the settings screen for sleep time, autosave, Wi-Fi info, and reset actions.
Inputs: selectedSetting, selectedSleepOption, selectedAutoSaveOption, SystemContext* ctx, and storage save methods.
Process: Exposes the UI state and lifecycle methods used by SettingsApp.cpp.
Output: A settings app that updates device preferences and shows confirm/info screens.
*/

#pragma once
#include "BaseApp.h"
#include "config.h"

class SettingsApp : public BaseApp {
private:
    int selectedSetting = 0;
    int selectedSleepOption = 0;
    int selectedAutoSaveOption = 0;
    bool editing = false;
    bool confirmingReset = false;
    bool showingWifiPasswordInfo = false;
    bool saveFailed = false;

    void drawSettings(bool partialRefresh = false);
    void drawResetConfirmation();
    void drawWifiPasswordInfo();
    void loadCurrentSelections();

public:
    SettingsApp(SystemContext* context);

    void onStart() override;
    void onLoop() override;
    void onButtonEvent(int btnId, int eventType) override;
    void onExit() override;
    void forceRedraw() override;
};
