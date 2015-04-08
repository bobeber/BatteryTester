void createChars(){

  byte customChar0[8] = {
    0b01110,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111
  };

  byte customChar1[8] = {
    0b01110,
    0b11111,
    0b10001,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111
  };

  byte customChar2[8] = {
    0b01110,
    0b11111,
    0b10001,
    0b10001,
    0b11111,
    0b11111,
    0b11111,
    0b11111
  };

  byte customChar3[8] = {
    0b01110,
    0b11111,
    0b10001,
    0b10001,
    0b10001,
    0b11111,
    0b11111,
    0b11111
  };

  byte customChar4[8] = {
    0b01110,
    0b11111,
    0b10001,
    0b10001,
    0b10001,
    0b10001,
    0b11111,
    0b11111
  };

  byte customChar5[8] = {
    0b01110,
    0b11111,
    0b10001,
    0b10001,
    0b10001,
    0b10001,
    0b10001,
    0b11111
  };

  byte npn[8] = {
    0b10001,
    0b10010,
    0b10100,
    0b11000,
    0b11000,
    0b10101,
    0b10011,
    0b10111
  };

  byte smile[8] = {
    0b00000,
    0b11011,
    0b11011,
    0b00000,
    0b00100,
    0b10101,
    0b10001,
    0b01110
  };

  lcd.createChar(0, customChar5); //empty batt icon
  lcd.createChar(1, customChar4);
  lcd.createChar(2, customChar3);
  lcd.createChar(3, customChar2);
  lcd.createChar(4, customChar1);
  lcd.createChar(5, customChar0); //full batt icon
  lcd.createChar(6, npn);
  lcd.createChar(7, smile);   
}

