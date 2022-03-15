#define X_RES 1024
#define Y_RES 758
#define PADDING 10
#define TAG "n7.inkplate"

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

bool shouldDraw() {
    float currentTemp = id(temp_outside).state;
    float previousTemp = id(global_temp_outside);
    bool freshStart = id(global_fresh_start);

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
    id(global_fresh_start) = false;
    ESP_LOGD(TAG, "Value changed. Previous: %.2f, new: %.2f", previousTemp, currentTemp);
    return true;
}

void drawWeatherIcon(esphome::display::DisplayBuffer& canvas, esphome::display::Font* font, int x, int y) {
    if (id(fc_weather).state == "clear-night") {
        canvas.printf(x, y, font, COLOR_OFF, TextAlign::TOP_LEFT, "\U000F0594");
    }
    else if (id(fc_weather).state == "cloudy") {
        canvas.printf(x, y, font, COLOR_OFF, TextAlign::TOP_LEFT, "\U000F0590");
    }
    else if (id(fc_weather).state == "partlycloudy") {
        canvas.printf(x, y, font, COLOR_OFF, TextAlign::TOP_LEFT, "\U000F0595");
    }
    else if (id(fc_weather).state == "fog") {
        canvas.printf(x, y, font, COLOR_OFF, TextAlign::TOP_LEFT, "\U000F0591");
    }
    else if (id(fc_weather).state == "hail") {
        canvas.printf(x, y, font, COLOR_OFF, TextAlign::TOP_LEFT, "\U000F0592");
    }
    else if (id(fc_weather).state == "lightning") {
        canvas.printf(x, y, font, COLOR_OFF, TextAlign::TOP_LEFT, "\U000F0593");
    }
    else if (id(fc_weather).state == "lightning-rainy") {
        canvas.printf(x, y, font, COLOR_OFF, TextAlign::TOP_LEFT, "\U000F067E");
    }
    else if (id(fc_weather).state == "pouring") {
        canvas.printf(x, y, font, COLOR_OFF, TextAlign::TOP_LEFT, "\U000F0596");
    }
    else if (id(fc_weather).state == "rainy") {
        canvas.printf(x, y, font, COLOR_OFF, TextAlign::TOP_LEFT, "\U000F0597");
    }
    else if (id(fc_weather).state == "snowy") {
        canvas.printf(x, y, font, COLOR_OFF, TextAlign::TOP_LEFT, "\U000F0F36");
    }
    else if (id(fc_weather).state == "snowy-rainy") {
        canvas.printf(x, y, font, COLOR_OFF, TextAlign::TOP_LEFT, "\U000F067F");
    }
    else if (id(fc_weather).state == "sunny") {
        canvas.printf(x, y, font, COLOR_OFF, TextAlign::TOP_LEFT, "\U000F0599");
    }
    else if (id(fc_weather).state == "windy") {
        canvas.printf(x, y, font, COLOR_OFF, TextAlign::TOP_LEFT, "\U000F059D");
    }
    else if (id(fc_weather).state == "windy-variant") {
        canvas.printf(x, y, font, COLOR_OFF, TextAlign::TOP_LEFT, "\U000F059E");
    }
    else if (id(fc_weather).state == "exceptional") {
        canvas.printf(x, y, font, COLOR_OFF, TextAlign::TOP_LEFT, "\U000F0F38");
    }
}

void drawFooter(esphome::display::DisplayBuffer& canvas, esphome::display::Font* font) {
    int tmp = (1-(4.1-id(batt_volt).state)/(4.1-3.3))*100;
    int perc = 100;
    if (tmp < 100) perc = tmp;

    canvas.strftime(PADDING, Y_RES-PADDING/2, font, COLOR_OFF, TextAlign::BASELINE_LEFT, "Aktualizacja: %H:%M", id(rtc_time).now());
    canvas.printf(X_RES-PADDING, Y_RES-PADDING/2, font, COLOR_OFF, TextAlign::BASELINE_RIGHT, "%.2fV/%d%%", id(batt_volt).state, perc);
}