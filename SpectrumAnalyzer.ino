#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include "AiEsp32RotaryEncoder.h"
#include "si5351.h"
#include "Wire.h"
#include "esp32/ulp.h"
#include "driver/adc.h"     






//------------------------------- TFT Display Init ------------------------------//
TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h
void setupDisplay(void)
{
  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_WHITE);
}

//------------------------------- Encoder Init ------------------------------//
#define ROTARY_ENCODER_A_PIN 34
#define ROTARY_ENCODER_B_PIN 35
#define ROTARY_ENCODER_BUTTON_PIN 32
#define ROTARY_ENCODER_STEPS 4
#define ROTARY_ENCODER_VCC_PIN -1
AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);
long lastEncoderReading = 0;

void IRAM_ATTR readEncoderISR()
{
	rotaryEncoder.readEncoder_ISR();
}

void setupEncoder(void)
{
  	//we must initialize rotary encoder
	rotaryEncoder.begin();
	rotaryEncoder.setup(readEncoderISR);
	rotaryEncoder.setAcceleration(250); //or set the value - larger number = more accelearation; 0 or 1 means disabled acceleration
}

//------------------------------- Si5351 Init ------------------------------//
Si5351 si5351(0x60);
uint64_t start_frequency = 6750000;
uint64_t stop_frequency =  7250000;
uint64_t step_frequency = (stop_frequency-start_frequency)/160;
uint64_t clk_1_frequency = start_frequency; // 7MHz

//uint64_t clk_1_frequency = 10000000; // 7MHz


void setupSi5351()
{
// Start serial and initialize the Si5351
  bool i2c_found = si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
  if(!i2c_found)
  {
    Serial.println("Device not found on I2C bus!");
  }
  else
    Serial.println("Device found on I2C bus!");  

  si5351.set_correction(146999, SI5351_PLL_INPUT_XO);

  si5351.drive_strength(SI5351_CLK0,SI5351_DRIVE_8MA);
  si5351.set_freq(clk_1_frequency*100, SI5351_CLK0);

  si5351.update_status();
}

void updateTFT()
{

  tft.setTextColor(TFT_YELLOW,TFT_BLUE );  
  String freq = String(clk_1_frequency);
  tft.drawString(freq,60,30,2);

}

void setup()
{
   Serial.begin(115200);
   while(!Serial);

  setupDisplay();
  setupEncoder();
  setupSi5351();
 
  
  tft.setTextColor(TFT_BLUE,TFT_WHITE );  
  tft.drawString(" www.RADIOBUILDER.org",5,5,2);
  tft.drawString(" OSC-1 ",5,30,2);

  updateTFT();

  adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_2_5);
   adc1_config_width(ADC_WIDTH_BIT_12);
   adc1_ulp_enable();


   tft.fillScreen(TFT_WHITE);




}



// Main
void loop()
{ 
 	// if (rotaryEncoder.encoderChanged())
	// {
	// 	Serial.print("Value: ");
	// 	Serial.println(rotaryEncoder.readEncoder());

  //   long currentReading = rotaryEncoder.readEncoder();
  //   long change =  currentReading -lastEncoderReading;

  //   lastEncoderReading = currentReading;

  //   clk_1_frequency += change*100;  
  //   updateTFT();
  //   si5351.set_freq(clk_1_frequency*100, SI5351_CLK0);   
	// }

    static int x_count = 0;
    
    si5351.set_freq(clk_1_frequency*100, SI5351_CLK0);   

    clk_1_frequency += step_frequency;
    if(clk_1_frequency > stop_frequency){
      clk_1_frequency = start_frequency;
     // tft.fillScreen(TFT_WHITE);
      x_count = 0;
    }

    
    //Serial.println(clk_1_frequency);

    delay(1);

    //int potValue = analogRead(36);
    int value = adc1_get_raw(ADC1_CHANNEL_0);

//        Serial.print(000); // To freeze the lower limit
// Serial.print(" ");
// Serial.print(4000); // To freeze the upper limit
// Serial.print(" ");

//     Serial.println(value);

    for(int i =0;i<128;i++)
    {
      tft.drawPixel(x_count,i , tft.color565(50,50,50));
    }
    int y_value = 150 - ((value-1000)/10 );
    tft.drawPixel(x_count,y_value , tft.color565(255,255,0));
    tft.drawPixel(x_count,y_value-1 , tft.color565(255,255,0));

    x_count++;

}