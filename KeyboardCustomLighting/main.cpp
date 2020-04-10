#include <CUESDK.h>

#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include <unordered_set>
#include <cmath>
#include <string>
#include <time.h>

#include <Windows.h>

#include "TCHAR.h"
#include "pdh.h"

#include "cpuLoad.h"
#include "constants.h"
#include "ringBuffer.h"
#include "keyboardBarLighting.h"

std::string toString(CorsairError error)
{
    switch (error) {
    case CE_Success:
        return "CE_Success";
    case CE_ServerNotFound:
        return "CE_ServerNotFound";
    case CE_NoControl:
        return "CE_NoControl";
    case CE_ProtocolHandshakeMissing:
        return "CE_ProtocolHandshakeMissing";
    case CE_IncompatibleProtocol:
        return "CE_IncompatibleProtocol";
    case CE_InvalidArguments:
        return "CE_InvalidArguments";
    default:
        return "unknown error";
    }
}

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {
    bool success = false;
    for (unsigned int i = 0; i < retries; i++) {
        CorsairPerformProtocolHandshake();
        if (const auto error = CorsairGetLastError()) {
            std::cout << "Handshake failed: " << toString(error) << "\nPress any key to quit." << std::endl;
        }
        else {
            success = true;
            break;
        }
    }
    if (!success) {
        return -1;
    }
    

    auto deviceCount = CorsairGetDeviceCount();
    auto keyboardIdx = -1;
    if (deviceCount > 0) {
        for (int i = 0; i < deviceCount; i++) {
            CorsairDeviceInfo* deviceInfo = CorsairGetDeviceInfo(i);
            if (strcmp(deviceInfo->model, "K63") == 0) {
                keyboardIdx = i;
            }
        }
    }
    else {
        return -1;
    }

    CorsairLedPositions* ledPositions = CorsairGetLedPositionsByDeviceIndex(keyboardIdx);
    CpuLoad<50> cpuLoad(50);

    unsigned int n = 0;

    KeyboardBarLighting<float, keyboardDivisions, 6> keyboardLighting(keyboardIdx, ledPositions, K63YMax);
    while (1) {
        Sleep(500);
        float var = cpuLoad.getCpuLoad();
        keyboardLighting.addValue(var);
    }
}