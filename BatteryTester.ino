#include <RunningAverage.h>
#include <LiquidCrystal.h>

//configure load
const double Resistor = 2.7; //ohm

//configure pins
LiquidCrystal lcd(11, 12, 5, 4, 3, 2);// initialize the library with the numbers of the interface pins
const int BATPin = 0;    // select the input pin for the BAT probe
const int FETPin = 1;    //select the input pin for the FET probe
const int FETCtrlPin = 8;      // select the pin for the MOSFET
const int LEDPin = 13;     //LED pin
const int BuzzPin = 6;     //Piezo Buzzer pin
const int LCDcontrast = 10;     //Piezo Buzzer pin

//configure battery properties
const int LiMinThreshold = 2500; // Lithium Minimal Voltage for load removal
const int LiTypical = 3600; //Typical LiIon Voltage
const int LiMaxThreshold = 4300; // Lithium Max Voltage for load removal
const int NimhMinThreshold = 900; // NMH Minimal Voltage for load removal
const int NimhTypical = 1200; //Typical NMH Voltage
const int NimhMaxThreshold = 1600; // NMH Max Voltage for load removal

//dont touch this
unsigned int SelectedMinThreshold = 5000;
unsigned int TypicalVoltage = 0;
double BATVoltage;
double FETVoltage;
int CurrentCurrent;
double TotalCurrent = 0;
double TotalWh = 0;
double TotalWhCalc;
double PrevTotalCurrent = -1;
double TotalTime = 0;
unsigned long PrevMillis;
unsigned long MillisPassed = 0;
double Vcc;
boolean detected = false;
boolean doneMsg = false;
boolean done = false;
boolean switchLCD = false;
byte batSymbol = 5;
RunningAverage RMVcc(20);
RunningAverage RMVFET(10);
RunningAverage RMVBAT(5);

//often used strings
String mV = "mV";
String mAh = "mAh";

void setup() {
	Serial.begin(9600);// start serial port to send data during run to the PC
	pinMode(FETCtrlPin, OUTPUT);
	pinMode(LEDPin, OUTPUT);
	pinMode(LCDcontrast, OUTPUT);
	pinMode(BuzzPin, OUTPUT);
	analogWrite(LCDcontrast, 100); //lcd contrst
	lcd.begin(16, 2);// set up the LCD's number of rows and columns:
	createChars();
	Serial.println(F("Battery Power Tester [ON]"));
	Serial.print(F("Detecting Battery Type: "));
	tone(BuzzPin, 30, 10000);

	//do cycle for detecting battery type based on battery voltage
	do {
		lcd.setCursor(0, 0);
		if (!switchLCD) {
			lcd.write((uint8_t)5);
			lcd.print(F(" Detecting Type"));
			lcd.setCursor(0, 1);
			lcd.write((uint8_t)0);
			lcd.print(F(" Insert Battery"));
			switchLCD = true;
		}
		else {
			lcd.setCursor(0, 0);
			lcd.write((uint8_t)0);
			lcd.print(F(" Detecting Type"));
			lcd.setCursor(0, 1);//
			lcd.write((uint8_t)5);
			lcd.print(F(" Insert Battery"));
			switchLCD = false;
		}
		//activate circuit
		digitalWrite(FETCtrlPin, HIGH);
		digitalWrite(LEDPin, HIGH);
		getSensorValues(); //BATVoltage, FETVoltage, CurrentCurrent
		//deactivate circuit
		digitalWrite(FETCtrlPin, LOW);
		digitalWrite(LEDPin, LOW);

		//detecting battery type
		lcd.setCursor(2, 1);
		if (BATVoltage > 4500){
			lcd.print(F("       High-V!"));
			Serial.print(F("Warning high-V!  "));
			detected = true;
			done = true;
		}
		else if (BATVoltage > LiMinThreshold){
			lcd.print(F("        Li-Ion"));
			Serial.print(F("Li-Ion  "));
			SelectedMinThreshold = LiMinThreshold;
			TypicalVoltage = LiTypical;
			detected = true;
		}
		else if (BATVoltage > NimhMinThreshold){
			lcd.print(F("       NiMH/Cd"));
			Serial.print(F("NiMH/Cd  "));
			SelectedMinThreshold = NimhMinThreshold;
			TypicalVoltage = NimhTypical;
			detected = true;
		}
		else if (BATVoltage < NimhMinThreshold && BATVoltage > 100){
			lcd.print(F("       Unknown"));
			Serial.print(F("Unknown battery  "));
			detected = true;
			done = true;
		}
		delay(500);
	}
	while (!detected);

	//print battery voltage to lcd
	lcd.setCursor(2, 1);
	lcd.print(int(BATVoltage));
	lcd.print(mV);
	Serial.print(BATVoltage);
	Serial.println(mV);
	//beep 2 times
	tone(BuzzPin, 30, 10000);
	delay(250);
	tone(BuzzPin, 30, 10000);
	delay(1750); //let the battery type msg apear on lcd for a while

	//print headers and stuff
	if (!done){
		Serial.print(F("Discharging to: "));
		Serial.print(SelectedMinThreshold);
		Serial.print(F("[mV], with "));
		Serial.print(Resistor);
		Serial.println(F("[Ohm] load. Please wait... "));
		Serial.println("");
		Serial.print(F("Time[s]"));
		Serial.print("\t");
		Serial.print(F("Fet[mV]"));
		Serial.print("\t");
		Serial.print(F("Vcc[mV]"));
		Serial.print("\t");
		Serial.print(F("Cur[mA]"));
		Serial.print("\t");
		Serial.print(F("Bat[mV]"));
		Serial.print("\t");
		Serial.print(F("Ch[mAh]"));
		Serial.println("");
		//print discharging msg from -> to
		lcd.setCursor(0, 0);
		lcd.write((uint8_t)5);
		lcd.print(F(" Discharging to"));
		lcd.setCursor(0, 1);
		lcd.write((uint8_t)0);
		lcd.setCursor(8, 1);
		lcd.print("        ");
		lcd.setCursor(14-countDigits(int(SelectedMinThreshold)), 1);
		lcd.print(SelectedMinThreshold);
		lcd.print(mV);
		lcd.setCursor(8, 1);
		lcd.print(F("->"));
		delay(2000); //let the lcd msg with discharging target appear for a while
		clearLCD();
	}
	PrevMillis = millis();
}

void loop() {
	if (BATVoltage > SelectedMinThreshold && !done) {
		//activate circuit (in loop() the circuit remains activated)
		digitalWrite(FETCtrlPin, HIGH);
		digitalWrite(LEDPin, HIGH);
		getSensorValues(); //BATVoltage, FETVoltage, CurrentCurrent
		//add up measured values and calculate total charge in mAh and Wh
		TotalCurrent = TotalCurrent+double(MillisPassed/1000)*(BATVoltage-FETVoltage)/Resistor/3600; //time window * voltage drop over resistor / resistance
		TotalWh = TotalWh+double(MillisPassed/1000)*(((BATVoltage-FETVoltage)/Resistor)/1000)*((BATVoltage-FETVoltage)/1000)/3600; // time window * current
		delay(1000);
		//calculate time window
		MillisPassed = millis()-PrevMillis;
		PrevMillis = millis();
		TotalTime = TotalTime+double(MillisPassed/1000);

		//animate battery icon discharging and print values to lcd
		lcd.setCursor(0, 0);
		if (batSymbol > 1 ) {
			lcd.write((uint8_t)batSymbol);
			batSymbol = batSymbol - 1;
		}
		else {
			lcd.write((uint8_t)batSymbol);
			batSymbol = 4;
		}
		lcd.setCursor(2, 0);
		lcd.print("              ");
		lcd.setCursor(2, 0);
		lcd.print(int(BATVoltage));
		lcd.print(mV);
		lcd.setCursor(13-countDigits(int(TotalCurrent)), 0); //right align
		lcd.print(int(TotalCurrent));
		lcd.print(mAh);
		lcd.setCursor(0, 1);
		lcd.write((uint8_t)6);
		lcd.setCursor(2, 1);
		lcd.print("              ");
		lcd.setCursor(2, 1);
		lcd.print(int(FETVoltage));
		lcd.print(mV);
		lcd.setCursor(14-countDigits(int(CurrentCurrent)), 1); //right align
		lcd.print(CurrentCurrent);
		lcd.print("mA");

		//print measured values to serial only if there is change in mAh Charge
		if (int(TotalCurrent) != int(PrevTotalCurrent)){
			Serial.print(int(TotalTime));
			Serial.print("\t");
			Serial.print(int(FETVoltage));
			Serial.print("\t");
			Serial.print(int(Vcc)); //this needs to be stable as much as possible
			Serial.print("\t");
			Serial.print(CurrentCurrent);
			Serial.print("\t");
			Serial.print(int(BATVoltage));
			Serial.print("\t");
			Serial.print(int(TotalCurrent));
			Serial.println("");
			PrevTotalCurrent = TotalCurrent;
		}
	}
	else //battery is discharged, test done
	{
		done = true;
		//deactivate circuit
		digitalWrite(FETCtrlPin, LOW);
		digitalWrite(LEDPin, LOW);
		getSensorValues(); //BATVoltage, FETVoltage, CurrentCurrent - get values without load
		//calculate test duration
		byte h = byte(TotalTime/60/60);
		byte m = int(TotalTime)%(60*60)/60;
		byte s = int(TotalTime)%(60*60)%60;

		//lets print some summary info of completed measurement
		if (!doneMsg) {
			clearLCD();
			Serial.println("");
			Serial.print(F("Battery Capacity: "));
			Serial.print(int(TotalCurrent));
			Serial.print(mAh);
			Serial.print(" ");
			Serial.print(TotalWh);
			Serial.print(F("[Wh]"));
			Serial.print(F(" (calculated "));
			Serial.print(TotalCurrent*TypicalVoltage/1000000);
			Serial.println(F("[Wh])"));
			Serial.print(F("Test Duration: "));
			Serial.print(h);
			Serial.print(F("[h] "));
			Serial.print(m);
			Serial.print(F("[m] "));
			Serial.print(s);
			Serial.println(F("[s]"));
			Serial.println(F("Battery Power Tester [DONE]"));
			//beep 3 times
			tone(BuzzPin, 30, 10000);
			delay(250);
			tone(BuzzPin, 30, 10000);
			delay(250);
			tone(BuzzPin, 30, 10000);
			doneMsg = true;
		}
		//cycle between total time and done msg
		if (!switchLCD) {
			lcd.setCursor(0, 1);
			lcd.print("        ");
			lcd.setCursor(0, 1);
			lcd.write((uint8_t)7);
			lcd.print(F(" Done"));
			switchLCD = true;
		}
		else {
			lcd.setCursor(0, 1);
			lcd.print("        ");
			lcd.setCursor(0, 1);
			lcd.print(h);
			lcd.print(F("h"));
			lcd.print(m);
			lcd.print(F("m"));
			lcd.print(s);
			lcd.print(F("s"));
			switchLCD = false;
		}
		//print current bat voltage and total charge
		lcd.setCursor(0, 0);
		lcd.write((uint8_t)0);
		lcd.setCursor(2, 0);
		if (BATVoltage > 850) { //when you remove battery
			lcd.print(int(BATVoltage));
			lcd.print(mV);
		}
		else {
			lcd.print(F("NaN     "));
		};
		lcd.setCursor(13-countDigits(int(TotalCurrent)), 0); //right align
		lcd.print(int(TotalCurrent));
		lcd.print(mAh);
		//set proper cursor for Wh value to right align
		if (TotalWh < 10) {
			lcd.setCursor(10, 1);
		}
		else {
			lcd.setCursor(14-countDigits(int((TotalWh*1000))), 1);
		}
		lcd.print(TotalWh);
		lcd.print(F("Wh"));

		//disco
		for (int i=0; i<5 ; i++){
			digitalWrite(LEDPin, HIGH);
			delay(50);
			digitalWrite(LEDPin, LOW);
			delay(50);
		}
		delay(2000);
	}
}
