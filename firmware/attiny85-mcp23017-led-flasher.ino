#include <Wire.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>

// MCP23017 I2C address (A0, A1, A2 = LOW)
#define MCP23017_ADDR 0x20

// MCP23017 Register addresses
#define MCP23017_IODIRA   0x00  // I/O Direction Register A
#define MCP23017_IODIRB   0x01  // I/O Direction Register B
#define MCP23017_GPIOA    0x12  // General Purpose I/O Port Register A
#define MCP23017_GPIOB    0x13  // General Purpose I/O Port Register B

// ATtiny85 pins for I2C (using TinyWireM library pins)
// SDA = Pin 0 (PB0)
// SCL = Pin 2 (PB2)

volatile bool wakeUp = false;

void setup() {
  // Disable unused peripherals to save power
  power_adc_disable();
  power_timer1_disable();
  
  // Initialize I2C
  Wire.begin();
  Wire.setClock(100000); // 100kHz for lower power consumption
  
  // Initialize MCP23017
  initMCP23017();
  
  // Setup watchdog timer for 1 second intervals
  setupWatchdog();
  
  // Set sleep mode
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
}

void loop() {
  // Flash each LED for 500ms
  for (uint8_t port = 0; port < 2; port++) {
    for (uint8_t pin = 0; pin < 8; pin++) {
      // Turn on current LED
      setLED(port, pin, true);
      
      // Wait 500ms using watchdog timer (sleep)
      sleepFor500ms();
      
      // Turn off current LED
      setLED(port, pin, false);
    }
  }
  
  // After cycling through all LEDs, sleep for longer period
  // to conserve battery (optional - remove if continuous cycling desired)
  for (uint8_t i = 0; i < 10; i++) { // Sleep for ~10 seconds
    sleepFor1Second();
  }
}

void initMCP23017() {
  // Set all pins as outputs (0 = output, 1 = input)
  writeRegister(MCP23017_IODIRA, 0x00); // Port A all outputs
  writeRegister(MCP23017_IODIRB, 0x00); // Port B all outputs
  
  // Turn off all LEDs initially
  writeRegister(MCP23017_GPIOA, 0x00);
  writeRegister(MCP23017_GPIOB, 0x00);
}

void writeRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(MCP23017_ADDR);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
  
  // Small delay to ensure I2C transaction completes
  delayMicroseconds(100);
}

uint8_t readRegister(uint8_t reg) {
  Wire.beginTransmission(MCP23017_ADDR);
  Wire.write(reg);
  Wire.endTransmission();
  
  Wire.requestFrom(MCP23017_ADDR, 1);
  return Wire.read();
}

void setLED(uint8_t port, uint8_t pin, bool state) {
  uint8_t reg = (port == 0) ? MCP23017_GPIOA : MCP23017_GPIOB;
  uint8_t currentValue = readRegister(reg);
  
  if (state) {
    currentValue |= (1 << pin);   // Set bit
  } else {
    currentValue &= ~(1 << pin);  // Clear bit
  }
  
  writeRegister(reg, currentValue);
}

void setupWatchdog() {
  // Clear the reset flag
  MCUSR &= ~(1 << WDRF);
  
  // Start timed sequence
  WDTCR |= (1 << WDCE) | (1 << WDE);
  
  // Set new watchdog timeout prescaler value to 1 second
  WDTCR = (1 << WDIE) | (1 << WDP2) | (1 << WDP1); // 1 second
}

void sleepFor1Second() {
  wakeUp = false;
  sleep_enable();
  sleep_cpu();
  sleep_disable();
}

void sleepFor500ms() {
  // For 500ms, we'll use two 250ms sleeps or approximate with delay
  // Since watchdog minimum is ~15ms, we'll use a more efficient approach
  
  // Temporarily set watchdog to shorter interval
  WDTCR |= (1 << WDCE) | (1 << WDE);
  WDTCR = (1 << WDIE) | (1 << WDP2); // ~250ms
  
  // Sleep twice for ~500ms
  sleepFor1Second();
  sleepFor1Second();
  
  // Restore 1 second watchdog
  setupWatchdog();
}

// Watchdog interrupt service routine
ISR(WDT_vect) {
  wakeUp = true;
}

/*
 * WIRING:
 * 
 * ATtiny85:
 * Pin 1 (RESET) -> 10kΩ pullup to VCC
 * Pin 4 (GND)   -> Ground
 * Pin 8 (VCC)   -> 3.3V from CR2032
 * Pin 5 (PB0)   -> SDA (MCP23017 pin 13)
 * Pin 7 (PB2)   -> SCL (MCP23017 pin 12)
 * 
 * MCP23017:
 * Pin 9 (VDD)   -> 3.3V
 * Pin 10 (VSS)  -> Ground
 * Pin 12 (SCL)  -> ATtiny85 Pin 7 (PB2) + 4.7kΩ pullup to VCC
 * Pin 13 (SDA)  -> ATtiny85 Pin 5 (PB0) + 4.7kΩ pullup to VCC
 * Pin 15 (A0)   -> Ground (sets I2C address to 0x20)
 * Pin 16 (A1)   -> Ground
 * Pin 17 (A2)   -> Ground
 * Pin 18 (RESET)-> 10kΩ pullup to VCC
 * 
 * Pins 21-28 (GPA0-GPA7) -> LEDs with appropriate current limiting resistors
 * Pins 1-8 (GPB0-GPB7)   -> LEDs with appropriate current limiting resistors
 * 
 * For 3.3V operation with typical LEDs, use ~150-220Ω current limiting resistors
 * 
 * POWER OPTIMIZATION NOTES:
 * - Uses power-down sleep mode (lowest power consumption)
 * - Disables unused peripherals (ADC, Timer1)
 * - Uses watchdog timer for timing instead of active delays
 * - I2C at 100kHz for lower power consumption
 * - LEDs are only on for 500ms each, minimizing current draw
 * - Optional longer sleep periods between LED cycles
 */
