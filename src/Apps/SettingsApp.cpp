/*
Description: Shows the settings menu and lets the user change device behavior.
Inputs: sleepTimeoutOptions[], autoSaveOptions[], selectedSetting, ctx->storage save methods, and Boolean flags for edit/reset/info states.
Process: Draws the menu, cycles through options, saves values, and handles confirmation screens.
Output: Updated values for inactivitySleepTimeoutMs and readerAutoSavePageThreshold, or reset info screens.
*/

#include "Apps/SettingsApp.h"
#include "Apps/LibraryMenuApp.h"
#include "DisplayController.h"
#include "StorageManager.h"

extern LibraryMenuApp* libraryApp;

namespace {
const uint32_t sleepTimeoutOptions[] = {
    SLEEP_TIMEOUT_OPTION_1_MS,
    SLEEP_TIMEOUT_OPTION_2_MS,
    SLEEP_TIMEOUT_OPTION_3_MS,
    SLEEP_TIMEOUT_OPTION_4_MS
};
const int sleepTimeoutOptionCount = sizeof(sleepTimeoutOptions) / sizeof(sleepTimeoutOptions[0]);

const uint32_t autoSaveOptions[] = {
    READER_AUTO_SAVE_OPTION_1_PAGES,
    READER_AUTO_SAVE_OPTION_2_PAGES,
    READER_AUTO_SAVE_OPTION_3_PAGES,
    READER_AUTO_SAVE_OPTION_4_PAGES
};
const int autoSaveOptionCount = sizeof(autoSaveOptions) / sizeof(autoSaveOptions[0]);

int findOptionIndex(const uint32_t* options, int optionCount, uint32_t value) {
    for (int i = 0; i < optionCount; ++i) {
        if (options[i] == value) {
            return i;
        }
    }
    return 0;
}
}

SettingsApp::SettingsApp(SystemContext* context) : BaseApp(context) {}

void SettingsApp::loadCurrentSelections() {
    selectedSleepOption = findOptionIndex(
        sleepTimeoutOptions, sleepTimeoutOptionCount, ctx->inactivitySleepTimeoutMs);
    selectedAutoSaveOption = findOptionIndex(
        autoSaveOptions, autoSaveOptionCount, ctx->readerAutoSavePageThreshold);
}

void SettingsApp::onStart() {
    selectedSetting = 0;
    loadCurrentSelections();
    editing = false;
    confirmingReset = false;
    showingWifiPasswordInfo = false;
    saveFailed = false;
    drawSettings();
}

void SettingsApp::onLoop() {}

void SettingsApp::onButtonEvent(int btnId, int eventType) {
    if (eventType != EVENT_CLICK) {
        return;
    }

    if (showingWifiPasswordInfo) {
        if (btnId == 1) {
            showingWifiPasswordInfo = false;
            drawSettings();
        }
        return;
    }

    if (confirmingReset) {
        if (btnId == 1) {
            confirmingReset = false;
            saveFailed = false;
            drawSettings();
        } else if (btnId == 4) {
            if (!ctx->storage->resetUserSettingsToDefaults()) {
                saveFailed = true;
                drawResetConfirmation();
                return;
            }

            ctx->inactivitySleepTimeoutMs = INACTIVITY_SLEEP_MS;
            ctx->readerAutoSavePageThreshold = READER_AUTO_SAVE_PAGE_THRESHOLD;
            loadCurrentSelections();
            confirmingReset = false;
            saveFailed = false;
            drawSettings();
        }
        return;
    }

    if (btnId == 1) {
        if (editing) {
            loadCurrentSelections();
            editing = false;
            saveFailed = false;
            drawSettings();
        } else {
            ctx->appManager->switchApp(libraryApp);
        }
        return;
    }

    if ((btnId == 2 || btnId == 3) && !editing) {
        if (btnId == 2 && selectedSetting > 0) {
            --selectedSetting;
        } else if (btnId == 3 && selectedSetting < 3) {
            ++selectedSetting;
        }
        saveFailed = false;
        drawSettings(true);
        return;
    }

    if (btnId == 2 || btnId == 3) {
        int direction = btnId == 3 ? 1 : -1;
        if (selectedSetting == 0) {
            selectedSleepOption =
                (selectedSleepOption + sleepTimeoutOptionCount + direction) % sleepTimeoutOptionCount;
        } else {
            selectedAutoSaveOption =
                (selectedAutoSaveOption + autoSaveOptionCount + direction) % autoSaveOptionCount;
        }
        saveFailed = false;
        drawSettings(true);
        return;
    }

    if (btnId == 4 && !editing) {
        saveFailed = false;
        if (selectedSetting == 2) {
            showingWifiPasswordInfo = true;
            drawWifiPasswordInfo();
        } else if (selectedSetting == 3) {
            confirmingReset = true;
            drawResetConfirmation();
        } else {
            editing = true;
            drawSettings();
        }
    } else if (btnId == 4) {
        bool saved = selectedSetting == 0
            ? ctx->storage->saveInactivitySleepTimeout(sleepTimeoutOptions[selectedSleepOption])
            : ctx->storage->saveReaderAutoSavePageThreshold(autoSaveOptions[selectedAutoSaveOption]);
        if (!saved) {
            saveFailed = true;
            drawSettings();
            return;
        }

        if (selectedSetting == 0) {
            ctx->inactivitySleepTimeoutMs = sleepTimeoutOptions[selectedSleepOption];
        } else {
            ctx->readerAutoSavePageThreshold = autoSaveOptions[selectedAutoSaveOption];
        }
        editing = false;
        saveFailed = false;
        drawSettings();
    }
}

void SettingsApp::onExit() {}

void SettingsApp::forceRedraw() {
    if (confirmingReset) {
        drawResetConfirmation();
    } else if (showingWifiPasswordInfo) {
        drawWifiPasswordInfo();
    } else {
        drawSettings();
    }
}

void SettingsApp::drawSettings(bool partialRefresh) {
    if (partialRefresh) {
        ctx->display->startPartialUpdate();
    } else {
        ctx->display->startFullUpdate();
    }
    do {
        ctx->display->fillWhite();
        ctx->display->setMenuFont();
        ctx->display->drawString(8, 40, "Settings");

        int cursorY = 85;
        int itemHeight = LIBRARY_MENU_ITEM_HEIGHT;

        if (!editing) {
            String settingLabels[4] = {
                "Sleep: " + String(sleepTimeoutOptions[selectedSleepOption] / 60000) + " min",
                "Autosave: " + String(autoSaveOptions[selectedAutoSaveOption]) + " pages",
                "Wi-Fi password",
                "Reset to defaults"
            };

            for (int i = 0; i < 4; ++i) {
                String lineText = (selectedSetting == i ? "> " : "  ") + settingLabels[i];
                ctx->display->drawString(8, cursorY + (i * itemHeight), lineText);
            }
            ctx->display->drawString(8, ctx->display->getHeight() - 30, "[2/3] Scroll");
            ctx->display->drawString(8, ctx->display->getHeight() - 10, "[4] Select");
            ctx->display->drawString(8, ctx->display->getHeight() - 52, "[1] Back");
        } else {
            ctx->display->drawString(8, 85, selectedSetting == 0 ? "Sleep timeout:" : "Autosave after:");
            int optionCount = selectedSetting == 0 ? sleepTimeoutOptionCount : autoSaveOptionCount;
            int selectedOption = selectedSetting == 0 ? selectedSleepOption : selectedAutoSaveOption;

            for (int i = 0; i < optionCount; ++i) {
                String label = selectedSetting == 0
                    ? String(sleepTimeoutOptions[i] / 60000) + " min"
                    : String(autoSaveOptions[i]) + " pages";
                String lineText = (i == selectedOption ? "> " : "  ") + label;
                ctx->display->drawString(8, 115 + (i * itemHeight), lineText);
            }
            ctx->display->drawString(8, ctx->display->getHeight() - 30, "[2/3] Scroll");
            ctx->display->drawString(8, ctx->display->getHeight() - 10, "[4] Save");
            ctx->display->drawString(8, ctx->display->getHeight() - 52, "[1] Cancel");
        }
        if (saveFailed) {
            ctx->display->drawString(8, 400, "Save failed");
        }
    } while (ctx->display->nextPage());
}

void SettingsApp::drawResetConfirmation() {
    ctx->display->startFullUpdate();
    do {
        ctx->display->fillWhite();
        ctx->display->setMenuFont();
        ctx->display->drawString(8, 40, "Reset settings?");
        ctx->display->drawString(8, 95, "Sleep: " + String(INACTIVITY_SLEEP_MS / 60000) + " min");
        ctx->display->drawString(8, 130, "Autosave: " + String(READER_AUTO_SAVE_PAGE_THRESHOLD) + " pages");
        ctx->display->drawString(8, 165, "Wi-Fi: " + String(TRANSFER_WIFI_PASSWORD));
        ctx->display->drawString(8, 230, "These will be saved");
        ctx->display->drawString(8, 255, "as default values.");
        ctx->display->drawString(8, ctx->display->getHeight() - 30, "[4] Confirm");
        ctx->display->drawString(8, ctx->display->getHeight() - 10, "[1] Cancel");
        if (saveFailed) {
            ctx->display->drawString(8, ctx->display->getHeight() - 52, "Reset failed");
        }
    } while (ctx->display->nextPage());
}

void SettingsApp::drawWifiPasswordInfo() {
    ctx->display->startFullUpdate();
    do {
        ctx->display->fillWhite();
        ctx->display->setMenuFont();
        ctx->display->drawString(8, 40, "Wi-Fi password");
        ctx->display->drawString(8, 110, "Open Wi-Fi Transfer,");
        ctx->display->drawString(8, 145, "then use its browser");
        ctx->display->drawString(8, 180, "page to change it.");
        ctx->display->drawString(8, ctx->display->getHeight() - 10, "[1] Back");
    } while (ctx->display->nextPage());
}
