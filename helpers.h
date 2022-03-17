#define X_RES 1024
#define Y_RES 758
#define PADDING 10
#define TAG "n7.inkplate"

#include "esphome/core/hal.h"

void drawRectangle(esphome::display::DisplayBuffer& canvas,
                   int x1,
                   int y1,
                   int width,
                   int height,
                   int thickness) {
    canvas.filled_rectangle(x1, y1, width, thickness, COLOR_OFF);
    canvas.filled_rectangle(x1, y1, thickness, height, COLOR_OFF);
    canvas.filled_rectangle(x1+width, y1, thickness, height, COLOR_OFF);
    canvas.filled_rectangle(x1, y1+height, width+thickness, thickness, COLOR_OFF);
}

float readBattery(esphome::gpio::GPIOSwitch* batteryReadMosfet, esphome::adc::ADCSensor* adcSensor) {
    GPIOPin* pin = batteryReadMosfet->get_pin();
    pin->pin_mode(gpio::FLAG_INPUT);
    int state = pin->digital_read();
    pin->pin_mode(gpio::FLAG_OUTPUT);
    if (state) {
        pin->digital_write(false);
    } else {
        pin->digital_write(true);
    }
    delay(1);
    adcSensor->set_output_raw(true);
    int adc = adcSensor->sample();
    if (state) {
        pin->pin_mode(gpio::FLAG_INPUT);
    } else {
        pin->digital_write(false);
    }
    return double(adc) / 4095 * 1.1 * 3.548133892 * 2;
}

bool shouldDrawHome() {
    float currentTemp = id(sensor_temp_outside).state;
    float previousTemp = id(global_temp_outside);
    bool freshStart = id(global_should_redraw);

    // wait for sensors to be available
    if (isnan(currentTemp)){
      ESP_LOGD(TAG, "HA API not yet ready.");
      id(inkplate_display).set_skip_update(true);
      return false;
    }

    // only refresh if something has changed or this is fresh start
    if (currentTemp == previousTemp && !freshStart) {
      id(inkplate_display).set_skip_update(true);
      return false;
    }
    id(global_temp_outside) = currentTemp;
    id(global_should_redraw) = false;
    ESP_LOGD(TAG, "Value changed. Previous: %.2f, new: %.2f", previousTemp, currentTemp);
    return true;
}

bool shouldDrawSensor() {
    bool freshStart = id(global_should_redraw);
    if (!freshStart) {
      id(inkplate_display).set_skip_update(true);
      return false;
    }
    id(global_should_redraw) = false;
    return true;
}

void drawWeather(esphome::display::DisplayBuffer& canvas, esphome::display::Font* fontIcon, esphome::display::Font* fontTemp,
                     esphome::text_sensor::TextSensor* weather_sensor,  esphome::sensor::Sensor* temp_sensor,
                     int x, int y) {
    auto state = weather_sensor->state;
    auto icon = "";
    
    if (state == "clear-night") {
        icon = "\U000F0594";
    }
    else if (state == "cloudy") {
        icon = "\U000F0590";
    }
    else if (state == "partlycloudy") {
        icon = "\U000F0595";
    }
    else if (state == "fog") {
        icon = "\U000F0591";
    }
    else if (state == "hail") {
        icon = "\U000F0592";
    }
    else if (state == "lightning") {
        icon = "\U000F0593";
    }
    else if (state == "lightning-rainy") {
        icon = "\U000F067E";
    }
    else if (state == "pouring") {
        icon = "\U000F0596";
    }
    else if (state == "rainy") {
        icon = "\U000F0597";
    }
    else if (state == "snowy") {
        icon = "\U000F0F36";
    }
    else if (state == "snowy-rainy") {
        icon = "\U000F067F";
    }
    else if (state == "sunny") {
        icon = "\U000F0599";
    }
    else if (state == "windy") {
        icon = "\U000F059D";
    }
    else if (state == "windy-variant") {
        icon = "\U000F059E";
    }
    else if (state == "exceptional") {
        icon = "\U000F0F38";
    }
    canvas.printf(x, y, fontIcon, COLOR_OFF, TextAlign::TOP_LEFT, icon);
    canvas.printf(x + 300, y + 100, fontTemp, COLOR_OFF, TextAlign::TOP_LEFT, "%.2f °C", temp_sensor->state);
}

void drawFlower(esphome::display::DisplayBuffer& canvas, esphome::display::Font* fontFlower,
                    esphome::display::Font* fontText, esphome::sensor::Sensor* flower_sensor, int x, int y) {
    auto value = flower_sensor->state;
    canvas.printf(x, y, fontFlower, COLOR_OFF, TextAlign::TOP_LEFT, "\U000F024A");
    canvas.printf(x + 20, y + 100, fontText, COLOR_OFF, TextAlign::TOP_LEFT, "%d%%", (int)value);
}

void drawFooter(esphome::display::DisplayBuffer& canvas, esphome::display::Font* font, 
                esphome::time::RealTimeClock* time, esphome::sensor::Sensor* sensor_battery_voltage) {
    auto voltage = sensor_battery_voltage->state;
    int tmp = (1-(4.1-voltage)/(4.1-3.3))*100;
    int perc = 100;
    if (tmp < 100) perc = tmp;

    canvas.strftime(PADDING, Y_RES-PADDING/2, font, COLOR_OFF, TextAlign::BASELINE_LEFT, "Aktualizacja: %H:%M", time->now());
    canvas.printf(X_RES-PADDING, Y_RES-PADDING/2, font, COLOR_OFF, TextAlign::BASELINE_RIGHT, "%.2fV/%d%%", voltage, perc);
}