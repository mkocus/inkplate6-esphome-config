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

void initExtended(esphome::mcp23017::MCP23017* extender1, esphome::mcp23017::MCP23017* extender2) {
    // Set all pins of seconds I/O expander to outputs, low.
    // For some reason, it draw more current in deep sleep when pins are set as
    // inputs...
    for (int i = 0; i < 15; i++) {
        extender2->pin_mode(i, gpio::FLAG_OUTPUT);
        extender2->digital_write(i, false);
    }

    // For same reason, unused pins of first I/O expander have to be also set as
    // outputs, low.
    extender1->pin_mode(13, gpio::FLAG_OUTPUT);
    extender1->pin_mode(14, gpio::FLAG_OUTPUT);
    extender1->pin_mode(15, gpio::FLAG_OUTPUT);
    extender1->digital_write(13, false);
    extender1->digital_write(14, false);
    extender1->digital_write(15, false); 
}