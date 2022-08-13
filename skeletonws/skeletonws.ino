#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>


// MH-Z19B CO_2 sensor defines
#define PWM_PIN 5
long ppm_pwm;

// DHT11 temperature/humidity sensor defines 
#define DHT_PIN 2
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);
float dht_tmp, dht_hum;

// SSD1306 128x32 OLED display defines
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);


// Define exactly 1 of the following 4 CO_2 display formats:
// FORMAT_CO2_PREFIX - leading 'CO_2' 
// FORMAT_CO2_POSTFIX - trailing 'CO_2'
// FORMAT_CO2_SIMPLE - trailing 'PPM' in smallest font
// FORMAT_CO2_DIAG - trailing 'PPM' in a bit bigger font and diagonally
#define FORMAT_CO2_POSTFIX

#if defined(FORMAT_CO2_PREFIX) + \
    defined(FORMAT_CO2_POSTFIX) + \
    defined(FORMAT_CO2_SIMPLE) + \
    defined(FORMAT_CO2_DIAG) != 1
#error "Bad CO_2 display format"
#endif


// Define exactly 1 of the following 2 DHT display formats:
// FORMAT_DHT_PREFIX - leading 'T', 'H' 
// FORMAT_DHT_POSTFIX - trailing '^C', '%'
#define FORMAT_DHT_POSTFIX

#if defined(FORMAT_DHT_PREFIX) + \
    defined(FORMAT_DHT_POSTFIX) != 1
#error "Bad DHT display format"
#endif

 
// displaying CO_2 
void displayCO2 () {
  display.clearDisplay();
  display.setCursor(0, 0);

  // displaying prefix texting
#ifdef FORMAT_CO2_PREFIX    
  display.setTextSize(2);
  display.print("CO");
  display.setTextSize(1);
  display.setCursor(24, 10);
  display.print("2");
  display.setCursor(0, 24);
  display.print("(PPM)");
  // x == 32 - lefts space just for 4 big chars
  display.setCursor(32, 0); 
#endif

  // displaying amount (right aligned)  
  display.setTextSize(4);
  // right align
  if (ppm_pwm < 1000) 
    display.print(" ");
  if (ppm_pwm < 100) 
    display.print(" ");
  if (ppm_pwm < 10) 
    display.print(" ");
  display.print(String(ppm_pwm));

  // displaying postfix texting
#ifdef FORMAT_CO2_SIMPLE
  display.setTextSize(1);
  display.print("PPM");
#endif

#ifdef FORMAT_CO2_DIAG
  display.setTextSize(2);
  display.print("P");
  display.setCursor(106, 8);
  display.print("P");
  display.setCursor(116, 16);
  display.print("M");
#endif
  
#ifdef FORMAT_CO2_POSTFIX
  display.setTextSize(2);
  display.print("CO");
  display.setTextSize(1);
  display.setCursor(120, 10);
  display.print("2");
  display.setCursor(96, 24);
  display.print("(PPM)");
#endif

  display.display();
}


// displaying temperature/humidity
void displayDHT () {
  String tmp, hum;
  tmp = isnan(dht_tmp)? "--": String(round(dht_tmp)); 
  hum = isnan(dht_hum)? "--": String(round(dht_hum)); 
  
  display.clearDisplay();
  display.setCursor(0, 0);

#ifdef FORMAT_DHT_PREFIX
  // displaying temperature
  display.setTextSize(2);
  display.print("T");
  display.setCursor(14, 0);
  display.setTextSize(4);
  display.print(tmp);
  
  // displaying humidity
  display.setCursor(66, 0);
  display.setTextSize(2);
  display.print("H");
  display.setCursor(80, 0);
  display.setTextSize(4);
  display.print(hum);
#endif

#ifdef FORMAT_DHT_POSTFIX
  // displaying temperature
  display.setTextSize(4);
  display.print(tmp);
  display.setTextSize(1);
  display.print("O");
  display.setTextSize(2);
  display.print("C");
  
  // displaying humidity
  display.setCursor(68, 0);
  display.setTextSize(4);
  display.print(hum);
  display.setTextSize(2);
  display.print("%");
#endif

  display.display();
}


// reading CO_2 level to global varianle 'ppm_pwm'
void readCO2PWM() {
  // 1004 - PWM period in ms, 1000 ms for actual data and 
  //                          2+2 ms for high/low guard levels
  // 5000 - CO_2 range in ppm
  unsigned long th, tl;  // PWM high/low pulses durations
  ppm_pwm = 0;
  do {
    th = pulseIn(PWM_PIN, HIGH, 1004000) / 1000;
    tl = 1004 - th;
    ppm_pwm = 5000 * (th - 2)/(th + tl - 4);
  } while (th == 0);
  Serial.print("CO_2 PPM (PWM): ");
  Serial.println(ppm_pwm); 
}


// reading DHT data to global variables dht_tmp, dht_hum
void readDHT() {
  dht_tmp = dht.readTemperature();
  dht_hum = dht.readHumidity();
  if (isnan(dht_hum) || isnan(dht_tmp)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  Serial.println("DHT.TMP: " + String(dht_tmp));
  Serial.println("DHT.HUM: " + String(dht_hum));
}


void setup() {
  Serial.begin(9600);
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextColor(WHITE);
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(4);
  display.print("Hello");
  display.display(); 

  pinMode(PWM_PIN, INPUT);
  dht.begin();
}


void loop() {
  readCO2PWM();
  displayCO2();
  delay(5000);

  readDHT();
  displayDHT();
  delay(5000);
}
