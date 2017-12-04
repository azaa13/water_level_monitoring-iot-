#include<SoftwareSerial.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 7, 6, 5, 4);
SoftwareSerial gsm(2,3);
int water;
int waterPin = 0;
float temp;
int tempPin = 1;
void setup() {
  // put your setup code here, to run once:

Serial.begin(9600);
gsm.begin(9600);
Serial.println("Initializing modem...");
delay(500);
gsm.println("AT");
delay(500);

printSerialData();
checkAndSend();

lcd.begin(16, 2);
lcd.setCursor(0,0);
lcd.print("Temperature:");
lcd.print(temp);
lcd.setCursor(0,1);
lcd.print("Water Level:");
lcd.print(water);

connectionStart();
}

void printSerialData()
{
 while(gsm.available()!=0)
 Serial.write(gsm.read());
}

void connectionStart()
{//DEACTIVATE GPRS PDP CONTEXT
 gsm.println("AT+CIPSHUT");
 printSerialData();
 delay(1000);
 //START UP MULTI IP CONNECTION
 gsm.println("AT+CIPMUX=0");
 printSerialData();
 delay(3000);
 gsm.println("AT+CGATT=1");
 printSerialData();
 delay(3000);
 //Serial.println("Modem connected successfully...");
 gsm.println("AT+CSTT=\"www\",\"\",\"\"");//setting the APN,2nd parameter empty works for all networks 
 printSerialData();
 delay(2000);
 //BRING UP WIRELESS CONNECTION WITH GPRS
 gsm.println("AT+CIICR");
 printSerialData();
 delay(3000);
 gsm.println("AT+CIFSR"); //init the HTTP request, GET LOCAL IP ADDRESS
 printSerialData();
 delay(6000); 
 gsm.println("AT+CIPSTART=\"TCP\",\"www.thingspeak.com\",\"80\"");
 printSerialData();
 delay(7000);
 gsm.println("AT+CIPSEND");
 printSerialData();
 delay(5000);
 Serial.println("Modem ready to send value to the server...");
 delay(1000);
 sendValue();
}

void sendValue() 
{
  // put your main code here, to run repeatedly:
 tempMeasure(); 
 waterMeasure();

 //need to connect thingspeak api 
 String str = "GET https://api.thingspeak.com/"; 
 str+="update?api_key=";
 //provide GET request
 str+="FO1M3RCDRXMB97O5&";
 str+="field1=" + String(water); 
 str+="&field2=" + String(temp) + " HTTP/1.1\r\n";
 String str1 = "Host:";
 str1+="www.thingspeak.com\r\n";
 String str2 = "Connection close\r\n\r\n\r\n";

 gsm.println(str);
 printSerialData();
 delay(5000);
 gsm.println(str1);
 printSerialData();
 delay(5000);
 gsm.println(str2);
 printSerialData();
 delay(5000);
 printSerialData();
 delay(5000);
 gsm.println((char)26);
 delay(5000);
 gsm.println();
 delay(5000);
 printSerialData();
 delay(5000);
 gsm.println("AT+CIPCLOSE\r\n");
 delay(5000);
 printSerialData();
 delay(5000);
 gsm.println("AT+CIPSHUT\r\n");
 delay(5000);
 printSerialData();
 delay(5000);
 gsm.println("Successfully sent value!"); 
 
}

void loop()
{
  { 
    float voltage = analogRead(tempPin);
    temp = ((voltage/1024.0)*3300)/10;
    water = analogRead(waterPin); 
    lcd.setCursor(0,0);
    lcd.print("Temperature:");
    lcd.print(temp);
    lcd.setCursor(0,1);
    lcd.print("Water Level:");
    lcd.print(water);
    delay(1000);
  }
  
  
  unsigned long interval = 30000; //time we need to wait
  unsigned long previousMillis = 0;
  
  unsigned long currentMillis = millis(); //grab current time 
  if ((unsigned long)(currentMillis - previousMillis) >= interval)
    { 
      checkAndSend();
      connectionStart();
      sendValue();
      previousMillis = millis();
      
    }

}

void tempMeasure()
{
 float voltage = analogRead(tempPin);
 temp = ((voltage/1024.0)*3300)/10;
 Serial.print("Temperature value:");
 Serial.print(temp);
 Serial.print("\n");
 
}

void waterMeasure()
{
 water = analogRead(waterPin); 
 Serial.print("Water level value:");
 Serial.print(water);
 Serial.print("\n");
}

void checkAndSend()
{
 waterMeasure();
 tempMeasure();
 if ((water > 100) && (temp > 30))
 {
    Serial.println("Warning!!!");
    sendSMS();
    delay(1000);
    
 }
 else
 {
    Serial.println("Safe");
  }

}

void sendSMS()
{  gsm.println("AT+CMGF=1");
    delay(500);
    printSerialData();
    gsm.println("AT+CMGS=\"+9199870XXXXX\""); // provide phone Num: which will be received Warning SMS
    delay(2000);
    printSerialData();
    gsm.print("Warning! Water level value:");
    gsm.print(water);
    gsm.print("\n");
    gsm.print("Temperature value:");
    gsm.print(temp);
    gsm.write(0x1A);
    delay(1000);
    printSerialData();
    delay(500);
    gsm.write(0x0D);
    delay(500);
    gsm.write(0x0A);
    gsm.println((char)26);// ASCII code of CTRL+Z
    delay(2000);
    printSerialData();
    
}


