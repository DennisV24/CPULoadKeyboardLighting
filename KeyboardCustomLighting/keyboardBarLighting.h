#pragma once

#include <vector>
#include <array>
#include <cmath>

#include "utilities.h"
#include "ringBuffer.h"
#include "constants.h"

#include <CUESDK.h>

template <class T, unsigned int WIDTH_DIVISIONS, unsigned int HEIGHT_DIVISIONS>
class KeyboardBarLighting
{
public:
    KeyboardBarLighting(const unsigned deviceIdx, CorsairLedPositions* ledPositions, const unsigned int manualTopIdx = 0) :
        buffer(),
        keys(),
        deviceIdx(deviceIdx),
        leftmostPos(static_cast<unsigned int>(getLeftPosition(ledPositions))),
        topmostPos((manualTopIdx == 0) ? static_cast<unsigned int>(getTopPosition(ledPositions)) : manualTopIdx),
        keyboardWidth(static_cast<unsigned int>(getKeyboardWidth(ledPositions))),
        keyboardHeight(static_cast<unsigned int>(getKeyboardHeight(ledPositions)))
    {

        for (auto i = 0; i < ledPositions->numberOfLed; i++) {
            auto& led = ledPositions->pLedPosition[i];
            unsigned int divIdx = static_cast<unsigned int>(positionToDivisionIndex(led.left));
            unsigned int heightIdx = static_cast<unsigned int>(positionToHeightIndex(led.top));
            auto ledColor = CorsairLedColor();
            ledColor.ledId = led.ledId;
            ledColor.r = 0;
            ledColor.g = 0;
            ledColor.b = 0;

            unsigned int invertedHeightIdx = keyboardHeightDivisions - heightIdx - 1;
            keys[divIdx][invertedHeightIdx].push_back(ledColor);
        }
    }

    double positionToDivisionIndex(double position) const 
    {
        double max = static_cast<double>(leftmostPos) + static_cast<double>(keyboardWidth);
        double result = invLerp(leftmostPos, max, position);
        result = result * keyboardWidthDivisions;
        return result;
    }

    double positionToHeightIndex(double position) const 
    {
        double max = static_cast<double>(topmostPos) + static_cast<double>(keyboardHeight);
        double result = invLerp(topmostPos, max, position);
        result = result * keyboardHeightDivisions;
        return result;
    }

    double percentageToHeightIndex(double percentage) const
    {
        double result = std::lerp(0, keyboardHeightDivisions, percentage / 100.);
        return result;
    }

    double getKeyboardWidth(CorsairLedPositions* ledPositions)
    {
        const auto maxLed = std::max_element(ledPositions->pLedPosition, ledPositions->pLedPosition + ledPositions->numberOfLed,
                                             [](const CorsairLedPosition& clp1, const CorsairLedPosition& clp2) {
                                                 return clp1.left < clp2.left;
                                             });
        return maxLed->left + maxLed->width - leftmostPos;
    }

    double getKeyboardHeight(CorsairLedPositions* ledPositions)
    {
        const auto maxLed = std::max_element(ledPositions->pLedPosition, ledPositions->pLedPosition + ledPositions->numberOfLed,
                                             [](const CorsairLedPosition& clp1, const CorsairLedPosition& clp2) {
                                                 return clp1.top < clp2.top;
                                             });
        return maxLed->top + maxLed->height - topmostPos;
    }

    static double getLeftPosition(CorsairLedPositions* ledPositions) 
    {
        const auto minLed = std::min_element(ledPositions->pLedPosition, ledPositions->pLedPosition + ledPositions->numberOfLed,
                                             [](const CorsairLedPosition& clp1, const CorsairLedPosition& clp2) {
                                                 return clp1.left < clp2.left;
                                             });
        return minLed->left;
    }

    static double getTopPosition(CorsairLedPositions* ledPositions) 
    {
        const auto minLed = std::min_element(ledPositions->pLedPosition, ledPositions->pLedPosition + ledPositions->numberOfLed,
                                             [](const CorsairLedPosition& clp1, const CorsairLedPosition& clp2) {
                                                 return clp1.top < clp2.top;
                                             });
        return minLed->top;
    }


    void addValue(T& value) 
    {
        buffer.addVal(value);
        update();
    }

protected:
    void update() 
    {
        // get the last N samples
        auto newData = buffer.getDataTimeSorted(keyboardWidthDivisions);
        unsigned int i = 0;
        for (auto& divGroup : keys) {
            unsigned int newRVal;
            // find the CPU load at each sample
            unsigned int highestIdx = static_cast<unsigned int>(percentageToHeightIndex(newData.at(i)));
            for (unsigned int j = 0; j < keyboardHeightDivisions; j++) {
                if (j <= highestIdx) {
                    newRVal = 255;
                }
                else {
                    newRVal = 0;
                }
                for (auto& key : divGroup.at(j)) {
                    key.r = newRVal;
                }
                CorsairSetLedsColorsBufferByDeviceIndex(deviceIdx, static_cast<int>(divGroup.at(j).size()), divGroup.at(j).data());
                CorsairSetLedsColorsFlushBuffer();
            }
            i++;
        }
    }

private:
    constexpr static unsigned int keyboardWidthDivisions = WIDTH_DIVISIONS;
    constexpr static unsigned int keyboardHeightDivisions = HEIGHT_DIVISIONS;
    RingBuffer<T, keyboardWidthDivisions> buffer;
    std::array<std::array<std::vector<CorsairLedColor>, keyboardHeightDivisions>, keyboardWidthDivisions> keys;


    const unsigned int deviceIdx;

    const unsigned int leftmostPos;
    const unsigned int topmostPos;
    const unsigned int keyboardWidth;
    const unsigned int keyboardHeight;
};

