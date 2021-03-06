//do the measurements
void getSensorValues() {
	int offset = 4; //this is just small correction for my particular arduino
	//if we know what kind off battery is in (we know in loop() already), use average voltages, else use current voltages (during setup())
	if (detected && !done) { //get smooth voltages

		RMVcc.addValue(readVcc());
		Vcc = RMVcc.getAverage();
		RMVBAT.addValue(analogRead(BATPin)*Vcc);
		RMVFET.addValue(analogRead(FETPin)*Vcc);
		BATVoltage = RMVBAT.getAverage()/1023+offset;
		FETVoltage = RMVFET.getAverage()/1023+offset;
	}
	else { //get instant voltages
		BATVoltage = analogRead(BATPin)*readVcc()/1023+offset;
		FETVoltage = analogRead(FETPin)*readVcc()/1023+offset;
	}
	CurrentCurrent = BATVoltage/Resistor; //get current current in the circuit
	//Serial.print(RMVFET.getCount());Serial.print("\t");Serial.println(RMVBAT.getCount()); //debug
}


//measure power supply voltage for better accuracy
long readVcc() {
	// Read 1.1V reference against AVcc
	// set the reference to Vcc and the measurement to the internal 1.1V reference
	#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
	ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
	#elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
	ADMUX = _BV(MUX5) | _BV(MUX0);
	#elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
	ADMUX = _BV(MUX3) | _BV(MUX2);
	#else
	ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
	#endif
	delay(2); // Wait for Vref to settle
	ADCSRA |= _BV(ADSC); // Start conversion
	while (bit_is_set(ADCSRA,ADSC)); // measuring
	uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH
	uint8_t high = ADCH; // unlocks both
	long result = (high<<8) | low;
	//result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
	result = 1122500L / result;
	return result; // Vcc in millivolts
}

//counts digits in a number
byte countDigits(int num){
	byte count=0;
	while(num){
		num=num/10;
		count++;
	}
	return count;
}

//clear lcd
void clearLCD(){
	lcd.setCursor(0, 0);
	lcd.print("                        ");
	lcd.setCursor(0, 1);
	lcd.print("                        ");
}
