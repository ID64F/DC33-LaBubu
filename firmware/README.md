This code is optimized for ultra-low power consumption with a CR2032 battery. Here are the key power-saving features:
Power Optimizations:

Uses SLEEP_MODE_PWR_DOWN (lowest power mode ~0.1µA)
Disables unused peripherals (ADC, Timer1)
Uses watchdog timer for timing instead of active delays
I2C runs at 100kHz to reduce power consumption
LEDs flash briefly (500ms) to minimize current draw

Key Features:

Controls 16 LEDs (8 on each MCP23017 port)
Each LED flashes for exactly 500ms
Cycles through all LEDs sequentially
Optional 10-second sleep between full cycles

Power Consumption Estimates:

Sleep mode: ~0.1µA (ATtiny85) + ~1µA (MCP23017)
Active mode: ~1mA (ATtiny85) + ~1mA (MCP23017) + LED current
With typical LEDs (~10mA each), total active current ~22mA per LED

Battery Life:
With a 220mAh CR2032, assuming 500ms on per LED every ~20 seconds, you should get several months of operation.
Programming Notes:

Use Arduino IDE with ATtiny85 board support
You'll need the Wire library (should work with TinyWireM as well)
Program using ISP programmer or Arduino as ISP

The wiring diagram is included in the comments. Make sure to add pullup resistors for I2C and current limiting resistors for LEDs!
