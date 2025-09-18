#include <Arduino.h>
#include "Renderer.hpp"
#include <vector>
#include <inttypes.h>
#include <stdio.h>
#include "Pins.h"
#include "WifiHandler.hpp"
#include "FormattedString.hpp"
#include "TelemetryHandler.hpp"
#include "Color.hpp"
#include <GpsProvider.hpp>

#ifndef RENDERER_H
#define RENDERER_H

class Icon{
public:
    using Img = std::pair<std::vector<uint8_t>, Color>;

private:
    TFT_eSprite *sprite;
    std::vector<Img> images;
    uint8_t width, height, scale;
    std::function<size_t()> getSpriteIndex;
    size_t current_sprite_index = 0;

    void redraw(){
        Img img = images[current_sprite_index];

        for (uint8_t i = 0; i < height; i++){
            for (uint8_t j = 0; j < width; j++){
                if (img.first[i] & (0x80 >> j)){
                    sprite->fillRect(j * scale, i * scale, scale, scale, static_cast<uint16_t>(img.second));
                }
                else{
                    sprite->fillRect(j * scale, i * scale, scale, scale, static_cast<uint16_t>(Color::Black));
                }
            }
        }
    }

public:
    Icon(const std::vector<Img> images, std::function<size_t()> getSpriteIndex, uint8_t width = 8, uint8_t scale = 2) : images(images), width(width), height(images[0].first.size()), scale(scale), getSpriteIndex(getSpriteIndex){
        sprite = new TFT_eSprite(&Renderer::tft);
        sprite->createSprite(width * scale, height * scale);
        redraw();
    }

    Icon(const Img image, uint8_t width, uint8_t scale) : images({image}), width(width), height(image.first.size()), scale(scale), getSpriteIndex([]()
                                                                                                                                                  { return 0; }){
        sprite = new TFT_eSprite(&Renderer::tft);
        sprite->createSprite(width * scale, height * scale);
        redraw();
    }

    TFT_eSprite *getSprite(){
        size_t new_index = getSpriteIndex();
        if (new_index != current_sprite_index){
            if (new_index >= 0 && new_index < images.size()){
                current_sprite_index = new_index;
            }
            else{
                current_sprite_index = 0;
            }
            redraw();
        }

        return sprite;
    }
};

class Panel
{
private:
    FormattedString text;
    FormattedString past_text;
    std::function<FormattedString()> getText;
    TFT_eSprite *sprite;
    Icon *icon;
    uint8_t x, y, width, height;

public:
    Panel(uint8_t x, uint8_t y, uint8_t width, uint8_t height, std::function<FormattedString()> getText, Icon *icon = nullptr)
        : text(""), past_text(""), x(x), y(y), width(width), height(height), getText(getText), sprite(new TFT_eSprite(&Renderer::tft)){
        sprite->createSprite(width, height);
        this->icon = icon;
    }

    bool checkChangesAndUpdate(){
        text = getText();
        if (text != past_text){
            past_text = text;
            updateSprite();

            return true;
        }

        return false;
    }

    void updateSprite(){
        sprite->fillSprite(0x528a);
        sprite->setTextColor(TFT_WHITE);
        sprite->setTextFont(1);
        sprite->setTextSize(2);

        int8_t dy = (height - sprite->fontHeight() + 1) / 2;

        sprite->setCursor(5, dy);

        if (icon != nullptr){
            TFT_eSprite *icon_sprite = icon->getSprite();

            int8_t dy_icon = (height - icon_sprite->height() + 1) / 2;
            icon_sprite->pushToSprite(sprite, 5, dy_icon, static_cast<uint16_t>(Color::Black));

            // Serial.println("Icon size: " + String(icon_sprite->width()) + "x" + String(icon_sprite->height()));
            // Serial.println("Icon position: (5, " + String(dy_icon) + ")");
            sprite->setCursor(10 + icon_sprite->width(), dy);
        }

        for (auto it = text.begin(); it != text.end(); ++it){
            sprite->setTextColor(static_cast<uint16_t>(it->second));
            sprite->print(it->first.c_str());
        }

        sprite->pushSprite(x, y);
    }
};

std::vector<Panel> panels;

float getSmoothBatteryVoltage(){
    static uint32_t t0 = 0;
    uint32_t t1 = millis();
    uint32_t dt = t1 - t0;

    static float voltage_avg = -1;
    static float output_volts = 0;

    // reset count every second
    if (dt > 1000){
        t0 = t1;
        output_volts = max(voltage_avg, 0.0f);
        voltage_avg = -1.0f;
    }

    uint16_t v = analogRead(ADC_PIN);
    //                        0-4095 -> 0-1        ?  voltage  correction factor
    float battery_voltage = ((float)v / 4095.0f) * 2.0f * 3.3f * (1100.0f / 1000.0f);

    // should reset current cycle
    if (voltage_avg == -1){
        voltage_avg = battery_voltage;
    }
    else{
        // average readings to reduce noise
        voltage_avg = voltage_avg * 0.7 + battery_voltage * 0.3;
    }

    return output_volts;
}

uint8_t getBatteryPercent(){
    return max(min((int)((getSmoothBatteryVoltage() - 2.7) / (4.25 - 2.7) * 100), 99), 0);
}

float getMillisAsFloat(){
    return static_cast<float>(millis());
}

Icon *batt_icon = new Icon(std::vector<Icon::Img>(
    {Icon::Img({
        0b00110000,
        0b00110000,
        0b11001100,
        0b10000100,
        0b11111100,
        0b11111100,
        0b11111100,
        0b11111100
    }, Color::Green),

    Icon::Img({
        0b00110000,
        0b00110000,
        0b11001100,
        0b10000100,
        0b10000100,
        0b10000100,
        0b11111100,
        0b11111100
    },Color::Yellow),

    Icon::Img({
        0b00110000,
        0b00110000,
        0b11001100,
        0b10000100,
        0b10000100,
        0b10000100,
        0b10000100,
        0b11111100
    },Color::Red)}),
    *[](){
        // return 0;
        uint8_t p = getBatteryPercent();
        if(p > 70) return 0;
        if(p > 20) return 1;
        return 2;
    },
    6, 2
);

Icon *gps_icon = new Icon(Icon::Img({
    0b00111000,
    0b01101100,
    0b01000100,
    0b00101000,
    0b00111000,
    0b00010000,
    0b00010000,
    0b11111110
}, Color::White), 7, 2);

Icon *satellite_icon = new Icon(Icon::Img({
    0b00110000,
    0b00110110,
    0b00001110,
    0b00011100,
    0b11011011,
    0b00100011,
    0b00010000,
    0b00010000
}, Color::White), 8, 2);

namespace Renderer{
    TFT_eSPI tft = TFT_eSPI(135, 240);

    void init(){
        tft.init();
        tft.setRotation(3);
        tft.fillScreen(TFT_BLACK);
    }

    void initPanels(){
        panels = std::vector<Panel>();

        panels.push_back(Panel(0, 0, 65, 20,
            *[](){
                return FormattedString("%2d%%", getBatteryPercent());
            },
            batt_icon
        ));

        panels.push_back(Panel((65 + 240-55)/2-95/2, 0, 95, 20,
            *[](){
                return FormattedString("%01d:%02d:%02d",
                (int)(millis() / 1000 / 3600), (int)((millis() / 1000) / 60 % 60), (int)(millis() / 1000 % 60));
            }));

        panels.push_back(Panel(240-55, 0, 55, 20,
            *[](){
                return FormattedString("%02d", GpsProvider::getNumSatellites());
            },
            satellite_icon
            ));

        panels.push_back(Panel(0, 30, 240, 20,
            *[](){
                return FormattedString("Network: %s",
                WifiHandler::connection_status ? (WifiHandler::connection_status == WIFI_CONNECTED ? "$g{Connected}" : " $y{Waiting}") : "$r{Disabled}");
            }));

        panels.push_back(Panel(0, 60, 240, 20,
            *[](){
                return FormattedString("Logging: %s",
                (TelemetryHandler::is_logging ? "$g{On}" : "$r{Off}"));
            }));

        panels.push_back(Panel(0, 90, 70, 20,
            *[](){
                return FormattedString("GPS");
            },
            gps_icon
        ));
        
        

        for (auto &panel : panels){
            panel.updateSprite();
        }
    }

    void updateDisplay(){
        for (auto &panel : panels){
            panel.checkChangesAndUpdate();
        }
    }
}

#endif // RENDER_H