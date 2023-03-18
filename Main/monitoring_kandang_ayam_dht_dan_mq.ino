

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <FirebaseESP32.h>
#include <WiFi.h>
#include <MQUnifiedsensor.h>


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define BUZZER 33

// Isikan sesuai pada Firebase
#define FIREBASE_HOST "https://monitoring-kandang-ayam-trm-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define FIREBASE_AUTH "DGlialEDWKE14oYOdMMKEXqCCINoFrftz7Qb84h5"

//Definitions sensor mq137
#define placa "esp-32"
#define Voltage_Resolution 3.3
#define pin 32 //Analog input 0 of your arduino
#define type "MQ-137" //MQ137
#define ADC_Bit_Resolution 10 // For arduino UNO/MEGA/NANO
#define RatioMQ137CleanAir 3.6//RS / R0 = 3.6 ppm  
//#define calibration_button 13 //Pin to calibrate your sensor
double NH3 = (0);

//Declare Sensor mq137
MQUnifiedsensor MQ137(placa, Voltage_Resolution, ADC_Bit_Resolution, pin, type);


// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

//Nama Wifi
#define WIFI_SSID "AndroidAP4E55"
#define WIFI_PASSWORD "12345678"

// mendeklarasikan objek data dari FirebaseESP8266
FirebaseData firebaseData;

#define DHTPIN 4     // Digital pin connected to the DHT sensor

// Uncomment the type of sensor in use:
//#define DHTTYPE    DHT11     // DHT 11
#define DHTTYPE    DHT22     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);

  pinMode (BUZZER, OUTPUT);

  dht.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  delay(2000);
  display.clearDisplay();
  display.setTextColor(WHITE);

  /*Konfigurasi MQ137*/
  MQ137.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ137.setA(102.2); MQ137.setB(-2.473); // Configure the equation to to calculate NH4 concentration

  /*
    Exponential regression:
    GAS      | a      | b
    CO       | 605.18 | -3.937
    Alcohol  | 77.255 | -3.18
    CO2      | 110.47 | -2.862
    Toluen  | 44.947 | -3.445
    NH4      | 102.2  | -2.473
    Aceton  | 34.668 | -3.369
  */

  /*****************************  MQ Init ********************************************/
  //Remarks: Configure the pin of arduino as input.
  /************************************************************************************/
  MQ137.init();
  Serial.print("Calibrating please wait.");
  float calcR0 = 0;
  for (int i = 1; i <= 10; i ++)
  {
    MQ137.update(); // Update data, the arduino will read the voltage from the analog pin
    calcR0 += MQ137.calibrate(RatioMQ137CleanAir);
    Serial.print(".");
  }
  MQ137.setR0(calcR0 / 10);
  Serial.println("  done!.");

  /*****************************  MQ CAlibration ********************************************/
  MQ137.serialDebug(true);

  // Koneksi ke Wifi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.print("Connected with IP: ");
  display.setCursor(0, 0);                // Koordinat awal tulisan (x,y) dimulai dari atas-kiri
  display.println("Connected with IP: ");
  Serial.println(WiFi.localIP());
  display.setCursor(0, 10);                // Koordinat awal tulisan (x,y) dimulai dari atas-kiri
  display.println("Connected with IP: ");
  Serial.println();
  //Menampilkan Tulisan RobotikIndonesia

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);


}

void loop() {
  delay(5000);

  //read temperature and humidity
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  // Memeriksa apakah sensor berhasil mambaca suhu dan kelembaban
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
  }


  MQ137.update(); // Update data, the arduino will read the voltage from the analog pin
  NH3 = MQ137.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

  // Menampilkan NH3 kepada serial monitor
  Serial.print("NH3   : ");
  Serial.print (NH3);
  Serial.println("   ppm");

  if ( NH3 >= 0 && NH3 <= 30 ) {
    analogWrite (BUZZER, 0);
    Serial.print("BUZZER   : off ");
    if (Firebase.setFloat(firebaseData, "/Hasil_Pembacaan/buzzer", 0)) {
      Serial.println("Buzzer terkirim");
    } else {
      Serial.println("Buzzer tidak terkirim");
      Serial.println("Karena: " + firebaseData.errorReason());
    }
  }

  if ( NH3 >= 31 && NH3 <= 60 ) {
    analogWrite (BUZZER, 255);
    Serial.print("BUZZER   : on ");
    if (Firebase.setFloat(firebaseData, "/Hasil_Pembacaan/buzzer", 1)) {
      Serial.println("Buzzer terkirim");
    } else {
      Serial.println("Buzzer tidak terkirim");
      Serial.println("Karena: " + firebaseData.errorReason());
    }
  }

  if ( NH3 >= 61 && NH3 <= 100 ) {
    analogWrite (BUZZER, 255);
    Serial.print("BUZZER   : on ");
    if (Firebase.setFloat(firebaseData, "/Hasil_Pembacaan/buzzer", 2)) {
      Serial.println("Buzzer terkirim");
    } else {
      Serial.println("Buzzer tidak terkirim");
      Serial.println("Karena: " + firebaseData.errorReason());
    }
  }

  if ( NH3 >= 101 && NH3 <= 200 ) {
    analogWrite (BUZZER, 255);
    Serial.print("BUZZER   : on ");
    if (Firebase.setFloat(firebaseData, "/Hasil_Pembacaan/buzzer", 2)) {
      Serial.println("Buzzer terkirim");
    } else {
      Serial.println("Buzzer tidak terkirim");
      Serial.println("Karena: " + firebaseData.errorReason());
    }
  }

  // Menampilkan suhu dan kelembaban kepada serial monitor
  Serial.print("Suhu: ");
  Serial.print(t);
  Serial.println(" *C");
  Serial.print("Kelembaban: ");
  Serial.print(h);
  Serial.println(" %");
  Serial.println();

  // clear display
  display.clearDisplay();

  //  // display temperature
  //  display.setTextSize(1);
  //  display.setCursor(0, 0);
  //  display.print("Temperature: ");
  //  display.setTextSize(2);
  //  display.setCursor(0, 10);
  //  display.print(t);
  //  display.print(" ");
  //  display.setTextSize(2);
  //  display.cp437(true);
  //  display.write(167);
  //  display.setTextSize(2);
  //  display.print("C");
  //
  //  // display humidity
  //  display.setTextSize(1);
  //  display.setCursor(0, 35);
  //  display.print("Humidity: ");
  //  display.setTextSize(2);
  //  display.setCursor(0, 45);
  //  display.print(h);
  //  display.print(" %");

  // display temperature
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Temperature: ");
  display.setTextSize(1.5);
  display.setCursor(0, 10);
  display.print(t);
  display.print(" ");
  display.setTextSize(1);
  display.cp437(true);
  display.write(167);
  display.setTextSize(1);
  display.print("C");

  // display humidity
  display.setTextSize(1);
  display.setCursor(0, 22);
  display.print("Humidity: ");
  display.setTextSize(1.5);
  display.setCursor(0, 32);
  display.print(h);
  display.print(" %");

  // display NH3
  display.setTextSize(1);
  display.setCursor(0, 44);
  display.print("NH3 : ");
  display.setTextSize(1.5);
  display.setCursor(0, 54);
  display.print(NH3);
  display.print(" ppm");

  display.display();

  // Memberikan status suhu dan kelembaban kepada firebase
  if (Firebase.setFloat(firebaseData, "/Hasil_Pembacaan/suhu", t)) {
    Serial.println("Suhu terkirim");
  } else {
    Serial.println("Suhu tidak terkirim");
    Serial.println("Karena: " + firebaseData.errorReason());
  }

  if (Firebase.setFloat(firebaseData, "/Hasil_Pembacaan/kelembapan", h)) {
    Serial.println("Kelembaban terkirim");
    Serial.println();
  } else {
    Serial.println("Kelembaban tidak terkirim");
    Serial.println("Karena: " + firebaseData.errorReason());
  }

  if (Firebase.setFloat(firebaseData, "/Hasil_Pembacaan/NH3", NH3)) {
    Serial.println("NH3 terkirim");
    Serial.println();
  } else {
    Serial.println("NH3 tidak terkirim");
    Serial.println("Karena: " + firebaseData.errorReason());
  }
}
