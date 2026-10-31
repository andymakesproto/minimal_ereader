#include "Apps/TransferApp.h"
#include "Apps/LibraryMenuApp.h"
#include "AppManager.h"
#include "DisplayController.h"

extern LibraryMenuApp* libraryApp;

// Initialize the server on standard port 80
TransferApp::TransferApp(SystemContext* context) : BaseApp(context), server(80) {}

// A lightweight HTML page embedded directly into the code
String TransferApp::generateHTML() {
    String html = "<!DOCTYPE html><html><head><title>Minimal Reader</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<style>";
    html += "body{font-family:sans-serif; text-align:center; padding:20px; background:#f4f4f4;}";
    html += ".container{background:#fff; padding:20px; border-radius:8px; box-shadow:0 2px 4px rgba(0,0,0,0.1); max-width:400px; margin:auto;}";
    html += "h2{color:#333; font-size: 20px;}";
    html += ".file-list{text-align:left; margin-top:20px;}";
    html += ".file-item{display:flex; justify-content:space-between; align-items:center; padding:12px 0; border-bottom:1px solid #eee;}";
    html += ".file-name{word-break: break-all; padding-right: 15px;}";
    html += ".btn{padding:10px 16px; border:none; border-radius:4px; cursor:pointer; font-size:16px;}";
    html += ".btn-upload{background:#000; color:#fff; width:100%; margin-top:10px;}";
    html += ".btn-delete{background:transparent; color:#dc3545; text-decoration:none; font-size:14px; font-weight: bold; white-space:nowrap;}";
    html += "</style></head><body>";
    
    html += "<div class='container'>";
    html += "<h2>Upload Book (.txt)</h2>";
    html += "<form method='POST' action='/upload' enctype='multipart/form-data'>";
    html += "<input type='file' name='update' accept='.txt' required style='margin: 15px 0;'><br>";
    html += "<input type='submit' value='Upload to E-Reader' class='btn btn-upload'>";
    html += "</form>";

    html += "<h2 style='margin-top: 30px;'>Manage Books</h2>";
    html += "<div class='file-list'>";
    
    // Scan LittleFS for existing books
    File root = LittleFS.open("/");
    File file = root.openNextFile();
    bool hasFiles = false;
    
    while (file) {
        if (!file.isDirectory()) {
            hasFiles = true;
            String fileName = String(file.name());
            String filePath = fileName.startsWith("/") ? fileName : "/" + fileName;
            
            html += "<div class='file-item'>";
            html += "<span class='file-name'>" + fileName + "</span>";
            // Adds a Javascript confirmation popup before deleting
            html += "<a href='/delete?file=" + filePath + "' class='btn-delete' onclick=\"return confirm('Delete " + fileName + "?')\">Delete</a>";
            html += "</div>";
        }
        file.close();
        file = root.openNextFile();
    }

    root.close();
    
    if (!hasFiles) {
        html += "<p style='color:#777; text-align:center;'>No books found on device.</p>";
    }
    
    html += "</div></div></body></html>";
    return html;
}

void TransferApp::setupWebServer() {
    // Route 1: Serve the dynamic dashboard
    server.on("/", HTTP_GET, [this]() {
        server.send(200, "text/html", generateHTML());
    });

    // Route 2: Handle deletions
    server.on("/delete", HTTP_GET, [this]() {
        if (server.hasArg("file")) {
            String filename = server.arg("file");
            LittleFS.remove(filename);
        }
        // Force the phone's browser to reload the page instantly
        server.sendHeader("Location", "/");
        server.send(303);
    });

    // Route 3: Handle uploads (UPDATED)
    server.on("/upload", HTTP_POST, [this]() {
        // Instead of a 303 header which breaks Safari, we send a standard 200 OK 
        // with an HTML tag that forces the browser to reload the page after 1 second.
        server.sendHeader("Connection", "close");
        server.send(200, "text/html", "<meta http-equiv='refresh' content='1; url=/' /><body><h2>Success! Reloading...</h2></body>");
    }, [this]() {
        HTTPUpload& upload = server.upload();
        
        if (upload.status == UPLOAD_FILE_START) {
            String rawName = upload.filename;
            String safeName = "";

            // 1. Strip any directory paths attached by mobile browsers
            int lastSlash = rawName.lastIndexOf('/');
            if (lastSlash != -1) rawName = rawName.substring(lastSlash + 1);
            lastSlash = rawName.lastIndexOf('\\');
            if (lastSlash != -1) rawName = rawName.substring(lastSlash + 1);

            // 2. Separate name and extension
            String ext = ".txt";
            int dotIdx = rawName.lastIndexOf('.');
            if (dotIdx != -1) {
                ext = rawName.substring(dotIdx);
                rawName = rawName.substring(0, dotIdx);
            }

            // 3. Keep ONLY safe ASCII letters, numbers, and underscores
            for (size_t i = 0; i < rawName.length(); i++) {
                char c = rawName[i];
                if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) {
                    safeName += c;
                } else if (c == ' ' || c == '-' || c == '_') {
                    safeName += '_';
                }
            }

            // 4. Fallback if the name was entirely non-ASCII symbols (e.g. Japanese/Emojis)
            if (safeName.length() == 0) {
                safeName = "book_" + String(millis() % 1000);
            }

            // 5. Cap filename length at 20 chars max (plus extension)
            if (safeName.length() > 20) {
                safeName = safeName.substring(0, 20);
            }

            safeName = "/" + safeName + ext;

            // 6. Open file for writing
            uploadFile = LittleFS.open(safeName, FILE_WRITE);
            
        } 
        // 7. WRITE THE INCOMING FILE CHUNKS (Missing from your code)
        else if (upload.status == UPLOAD_FILE_WRITE) {
            if (uploadFile) {
                uploadFile.write(upload.buf, upload.currentSize);
            }
        } 
        // 8. CLOSE AND DESTROY THE LOCK TO FIX THE DELETE BUG (Missing from your code)
        else if (upload.status == UPLOAD_FILE_END || upload.status == UPLOAD_FILE_ABORTED) {
            if (uploadFile) {
                uploadFile.close();
                // CRITICAL FIX: Overwrite the object to force LittleFS to release the file lock
                uploadFile = File(); 
            }
        }
    });
}

void TransferApp::onStart() {
    // 1. Turn on WiFi AP Mode
    WiFi.mode(WIFI_AP);
    WiFi.softAP("MinimalReader", "12345678"); // SSID and Password
    
    // 2. Start the Server
    setupWebServer();
    server.begin();
    
    // 3. Draw the UI Instructions
    ctx->display->startFullUpdate();
    do {
        ctx->display->fillWhite();
        ctx->display->setMenuFont();
        
        ctx->display->drawString(0, 30, "Wi-Fi Active");
        ctx->display->drawString(0, 80, "Network:");
        ctx->display->drawString(0, 110, "MinimalReader");
        ctx->display->drawString(0, 160, "Password:");
        ctx->display->drawString(0, 190, "12345678");
        
        // The default IP for ESP32 AP mode is always 192.168.4.1
        ctx->display->drawString(0, 240, "IP: 192.168.4.1");
        
        int bottomY = ctx->display->getHeight() - 20;
        ctx->display->drawString(0, bottomY, "< Back (Btn 1)");
        
    } while (ctx->display->nextPage());
}

void TransferApp::onLoop() {
    // This keeps the web server listening for incoming connections
    server.handleClient(); 
}

void TransferApp::onButtonEvent(int btnId, int eventType) {
    if (eventType != 0) return;
    
    // If the user presses Back (Button 1)
    if (btnId == 1) {
        ctx->appManager->switchApp(libraryApp);
    }
}

void TransferApp::onExit() {
    // Stop the server and kill the Wi-Fi radio to save battery
    server.stop();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);
}