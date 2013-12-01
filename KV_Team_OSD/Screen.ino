
char *ItoaPadded(int val, char *str, uint8_t bytes, uint8_t decimalpos)  {
  uint8_t neg = 0;
  if(val < 0) {
    neg = 1;
    val = -val;
  }

  str[bytes] = 0;
  for(;;) {
    if(bytes == decimalpos) {
      str[--bytes] = DECIMAL;
      decimalpos = 0;
    }
    str[--bytes] = '0' + (val % 10);
    val = val / 10;
    if(bytes == 0 || (decimalpos == 0 && val == 0))
      break;
  }

  if(neg && bytes > 0)
    str[--bytes] = '-';

  while(bytes != 0)
    str[--bytes] = ' ';
  return str;
}

char *FormatGPSCoord(int32_t val, char *str, uint8_t p, char pos, char neg) {
  if(val < 0) {
    pos = neg;
    val = -val;
  }

  uint8_t bytes = p+8;

  str[bytes] = 0;
  str[--bytes] = pos;
  for(;;) {
    if(bytes == p) {
      str[--bytes] = DECIMAL;
      continue;
    }
    str[--bytes] = '0' + (val % 10);
    val = val / 10;
    if(bytes < 3 && val == 0)
       break;
   }

   while(bytes != 0)
     str[--bytes] = ' ';

   return str;
}

// Take time in Seconds and format it as 'MM:SS'
// Alternately Take time in Minutes and format it as 'HH:MM'
// If hhmmss is 1, display as HH:MM:SS
char *formatTime(uint16_t val, char *str, uint8_t hhmmss) {
  int8_t bytes = 5;
  if(hhmmss)
    bytes = 8;
  str[bytes] = 0;
  do {
    str[--bytes] = '0' + (val % 10);
    val = val / 10;
    str[--bytes] = '0' + (val % 6);
    val = val / 6;
    str[--bytes] = ':';
  } while(hhmmss-- != 0);
  do {
    str[--bytes] = '0' + (val % 10);
    val = val / 10;
  } while(val != 0 && bytes != 0);

  while(bytes != 0)
     str[--bytes] = ' ';

  return str;
}

uint8_t FindNull(void)
{
  uint8_t xx;
  for(xx=0;screenBuffer[xx]!=0;xx++)
    ;
  return xx;
}

// Unit conversions
/*uint8_t CMsToKMh(uint16_t speed)       // cm/sec to km/h or to mph
{
  return GPS_speed *
           (Settings[S_UNITSYSTEM] ?
               (0.036*0.62137) :       // From MWii cm/sec to mph
               0.036);                 // From MWii cm/sec to Km/h           //   NOT DELETE
}

int16_t TempConverter(int16_t temp) { // deg-C to deg-C or deg-F
  return Settings[S_UNITSYSTEM] ?
           temp * 1.8 + 32 :       // convert to Fahrenheit
           temp;
}

void displayTemperature(void)        // WILL WORK ONLY WITH V1.2                 //   NOT DELETE
{
  itoa(TempConverter(temperature), screenBuffer, 10);
  uint8_t xx = FindNull();
  screenBuffer[xx++] = temperatureUnitAdd[Settings[S_UNITSYSTEM]];
  screenBuffer[xx] = 0;
  MAX7456_WriteString(screenBuffer,getPosition(temperaturePosition));
}*/

void displayTemperature(void)        // WILL WORK ONLY WITH V1.2
{
  int xxx;
  if (Settings[S_UNITSYSTEM])
    xxx = temperature*1.8+32;       //Fahrenheit conversion for imperial system.
  else
    xxx = temperature;

  if(xxx > temperMAX)
    temperMAX = xxx;

  ItoaPadded(xxx,screenBuffer,3,0);
  //uint8_t xx = FindNull();   // find the NULL
  //screenBuffer[xx++]=temperatureUnitAdd[Settings[S_UNITSYSTEM]];
  //screenBuffer[xx]=0;  // Restore the NULL
  
  screenBuffer[3]=temperatureUnitAdd[Settings[S_UNITSYSTEM]];
  

  
  MAX7456_WriteString(screenBuffer,getPosition(temperaturePosition));
}

void displayMode(void)
{
  // Put sensor symbold (was displaySensors)
  screenBuffer[0] = (MwSensorPresent&ACCELEROMETER) ? SYM_ACC : ' ';
  screenBuffer[1] = (MwSensorPresent&BAROMETER) ? SYM_BAR : ' ';
  screenBuffer[2] = (MwSensorPresent&MAGNETOMETER) ? SYM_MAG : ' ';
  screenBuffer[3] = (MwSensorPresent&GPSSENSOR) ? SYM_GPS : ' ';

  if(MwSensorActive&mode_stable)
  {
    screenBuffer[4]=SYM_STABLE;
    screenBuffer[5]=SYM_STABLE1;
  }
  else
  {
    screenBuffer[4]=SYM_ACRO;
    screenBuffer[5]=SYM_ACRO1;
  }
  screenBuffer[6]=' ';
  if(MwSensorActive&mode_gpshome)
    screenBuffer[7]=SYM_G_HOME;
  else if(MwSensorActive&mode_gpshold)
    screenBuffer[7]=SYM_HOLD;
  else if(GPS_fix)
    screenBuffer[7]=SYM_3DFIX;
  else
    screenBuffer[7]=' ';

  screenBuffer[8]=0;
  MAX7456_WriteString(screenBuffer,getPosition(sensorPosition));

  // Put ON indicator under sensor symbol
  screenBuffer[0] = (MwSensorActive&mode_stable) ? SYM_CHECK : ' ';
  screenBuffer[1] = (MwSensorActive&mode_baro) ? SYM_CHECK : ' ';
  screenBuffer[2] = (MwSensorActive&mode_mag) ? SYM_CHECK : ' ';
  screenBuffer[3] = (MwSensorActive&(mode_gpshome|mode_gpshold)) ? SYM_CHECK : ' ';
  screenBuffer[4] = 0;
  MAX7456_WriteString(screenBuffer,getPosition(sensorPosition)+LINE);

/*
  if(MwSensorActive & mode_llights)
    screenBuffer[0] = 0x04;
  else
    screenBuffer[0] = ' ';

  if(MwSensorActive & mode_osd_switch)
    screenBuffer[1] = 0x05;
  else
    screenBuffer[1] = ' ';
  screenBuffer[2]=0;
  MAX7456_WriteString(screenBuffer,getPosition(sensorPosition)+2*LINE);
*/
}

void displayArmed(void)
{
  if(!armed)
    MAX7456_WriteString_P(disarmed_text, getPosition(motorArmedPosition));
  else if(Blink10hz && flyTime < 8)
    MAX7456_WriteString_P(armed_text, getPosition(motorArmedPosition));
}

void displayCallsign(void)
{
  //if(armed){
  uint16_t position = getPosition(callSignPosition);
    if(Settings[S_DISPLAY_CS]){
      for(int X=0; X<10; X++) {
          screenBuffer[X] = char(Settings[S_CS0 + X]);
      }   
       screenBuffer[10] = 0;
       MAX7456_WriteString(screenBuffer, getPosition(callSignPosition)); 
    }
  //}
}

void displayHorizon(int rollAngle, int pitchAngle)
{
  if(!fieldIsVisible(horizonPosition))
    return;

  uint16_t position = getPosition(horizonPosition);

  if(pitchAngle>200) pitchAngle=200;
  if(pitchAngle<-250) pitchAngle=-250;
  if(rollAngle>400) rollAngle=400;
  if(rollAngle<-400) rollAngle=-400;

  for(int X=0; X<=8; X++) {
    int Y = (rollAngle * (4-X)) / 64;
    Y -= pitchAngle / 8;
    Y += 41;
    if(Y >= 0 && Y <= 81) {
      uint16_t pos = position + LINE*(Y/9) + 2 - 2*LINE + X;
      screen[pos] = SYM_AH_BAR9_0+(Y%9);
      if(Y>=9 && (Y%9) == 0)
        screen[pos-LINE] = SYM_AH_BAR9_9;
    }
  }

  //if(Settings[S_DISPLAY_HORIZON_BR]){
    //Draw center screen
    //screen[position+2*LINE+6-1] = SYM_AH_CENTER_LINE;
    //screen[position+2*LINE+6+1] = SYM_AH_CENTER_LINE;
    screen[position+2*LINE+6] =   SYM_AH_CENTER;
  //}
  //if (Settings[S_WITHDECORATION]){
    // Draw AH sides
    //screen[position+2*LINE+1] =   SYM_AH_LEFT;
    //screen[position+2*LINE+11] =  SYM_AH_RIGHT;
    screen[position+0*LINE] =     SYM_AH_DECORATION_LEFT;
    screen[position+1*LINE] =     SYM_AH_DECORATION_LEFT;
    screen[position+2*LINE] =     SYM_AH_DEC_CENTER_LEFT;
    screen[position+3*LINE] =     SYM_AH_DECORATION_LEFT;
    screen[position+4*LINE] =     SYM_AH_DECORATION_LEFT;
    screen[position+0*LINE+12] =  SYM_AH_DECORATION_RIGHT;
    screen[position+1*LINE+12] =  SYM_AH_DECORATION_RIGHT;
    screen[position+2*LINE+12] =  SYM_AH_DEC_CENTER_RIGHT;
    screen[position+3*LINE+12] =  SYM_AH_DECORATION_RIGHT;
    screen[position+4*LINE+12] = SYM_AH_DECORATION_RIGHT;
  //}
}

void displayVoltage(void)
{
  if (Settings[S_VIDVOLTAGE_VBAT]){
    vidvoltage=MwVBat;
  }
  if (Settings[S_MAINVOLTAGE_VBAT]){
    voltage=MwVBat;
  }
  ItoaPadded(voltage, screenBuffer, 4, 3);
  screenBuffer[4] = SYM_VOLT;
  screenBuffer[5] = 0;
  MAX7456_WriteString(screenBuffer,getPosition(voltagePosition));



  if (Settings[S_SHOWBATLEVELEVOLUTION]){
    // For battery evolution display
    int BATTEV1 =Settings[S_BATCELLS] * 35;
    int BATTEV2 =Settings[S_BATCELLS] * 36;
    int BATTEV3 =Settings[S_BATCELLS] * 37;
    int BATTEV4 =Settings[S_BATCELLS] * 38;
    int BATTEV5 =Settings[S_BATCELLS] * 40;
    int BATTEV6 = Settings[S_BATCELLS] * 41;

    if (voltage < BATTEV1) screenBuffer[0]=SYM_BATT_EMPTY;
    else if (voltage < BATTEV2) screenBuffer[0]=SYM_BATT_1;
    else if (voltage < BATTEV3) screenBuffer[0]=SYM_BATT_2;
    else if (voltage < BATTEV4) screenBuffer[0]=SYM_BATT_3;
    else if (voltage < BATTEV5) screenBuffer[0]=SYM_BATT_4;
    else if (voltage < BATTEV6) screenBuffer[0]=SYM_BATT_5;
    else screenBuffer[0]=SYM_BATT_FULL;                              // Max charge icon
  }
  else {
    screenBuffer[0]=SYM_MAIN_BATT;
  }
  screenBuffer[1]=0;
  MAX7456_WriteString(screenBuffer,getPosition(voltagePosition)-1);

  if (Settings[S_VIDVOLTAGE]){
    ItoaPadded(vidvoltage, screenBuffer, 4, 3);
    screenBuffer[4]=SYM_VOLT;
    screenBuffer[5]=0;
    MAX7456_WriteString(screenBuffer,getPosition(vidvoltagePosition));
    screenBuffer[0]=SYM_VID_BAT;
    screenBuffer[1]=0;
    MAX7456_WriteString(screenBuffer,getPosition(vidvoltagePosition)-1);
  }
}

void displayCurrentThrottle(void)
{

  if (MwRcData[THROTTLESTICK] > HighT) HighT = MwRcData[THROTTLESTICK] -5;
  if (MwRcData[THROTTLESTICK] < LowT) LowT = MwRcData[THROTTLESTICK];      // Calibrate high and low throttle settings  --defaults set in GlobalVariables.h 1100-1900
  
  //screenBuffer[0]=0;
  //screenBuffer[1]=0;
  //MAX7456_WriteString(screenBuffer,getPosition(CurrentThrottlePosition));
  
  
  if(!armed) {
    //screenBuffer[0]=' ';
    //screenBuffer[1]=' ';
    screenBuffer[0]='-';
    screenBuffer[1]='-';
    screenBuffer[2]=0;
    MAX7456_WriteString(screenBuffer,getPosition(CurrentThrottlePosition)+2);
  }
  else
  {
    
    int CurThrottle = map(MwRcData[THROTTLESTICK],LowT,HighT,0,99);
    ItoaPadded(CurThrottle,screenBuffer,2,0);
    screenBuffer[2]='%';
    screenBuffer[3]=0;
    
    MAX7456_WriteString(screenBuffer,getPosition(CurrentThrottlePosition));    
 
    screenBuffer[2]=0;
    
    
    if(CurThrottle > 90)      screenBuffer[0] = SYM_THR_POINTER_TOP;
    else if(CurThrottle > 80) screenBuffer[0] = SYM_THR_POINTER;
    else if(CurThrottle > 70) screenBuffer[0] = SYM_THR_POINTER_BOTTOM;
    else screenBuffer[0] = ' ';
    
    screenBuffer[1]=SYM_THR_SCALE;

    MAX7456_WriteString(screenBuffer,getPosition(ThrottleGraphPosition));
    
    if (CurThrottle > 70) screenBuffer[0] = ' ';
    else if(CurThrottle > 60) screenBuffer[0] = SYM_THR_POINTER_TOP;
    else if(CurThrottle > 50) screenBuffer[0] = SYM_THR_POINTER;
    else if(CurThrottle > 40) screenBuffer[0] = SYM_THR_POINTER_BOTTOM;
    
    screenBuffer[1]=SYM_THR_SCALE;
    
    MAX7456_WriteString(screenBuffer,getPosition(ThrottleGraphPosition) + LINE);
    
    if (CurThrottle > 40) screenBuffer[0] = ' ';
    else if(CurThrottle > 30) screenBuffer[0] = SYM_THR_POINTER_TOP;
    else if(CurThrottle > 20) screenBuffer[0] = SYM_THR_POINTER;
    else screenBuffer[0] = SYM_THR_POINTER_BOTTOM;
    
    screenBuffer[1]=SYM_THR_SCALE;
    
    MAX7456_WriteString(screenBuffer,getPosition(ThrottleGraphPosition) + LINE + LINE);
    
  }
}

void displayTime(void)
{ 
  if(flyTime < 3600) {
    screenBuffer[0] = SYM_FLY_M;
    formatTime(flyTime, screenBuffer+1, 0);
  }
  else {
    screenBuffer[0] = SYM_FLY_H;
    formatTime(flyTime/60, screenBuffer+1, 0);
  }
  MAX7456_WriteString(screenBuffer,getPosition(flyTimePosition));

  if (armed) return ;
  uint16_t position = getPosition(onTimePosition);
  if(onTime < 3600) {
    screenBuffer[0] = SYM_ON_M;
    formatTime(onTime, screenBuffer+1, 0);
  }
  else {
    screenBuffer[0] = SYM_ON_H;
    formatTime(onTime/60, screenBuffer+1, 0);
  }
  MAX7456_WriteString(screenBuffer,getPosition(onTimePosition));
}

void displayAmperage(void)
{
  // Real Ampere is ampere / 10
  ItoaPadded(amperage, screenBuffer, 4, 3);     // 99.9 ampere max!
  screenBuffer[4] = SYM_AMP;
  screenBuffer[5] = 0;
  MAX7456_WriteString(screenBuffer,getPosition(amperagePosition));
}

void displaypMeterSum(void)
{
  if (Settings[S_ENABLEADC]){
    pMeterSum = amperagesum;
  }
  screenBuffer[0]=SYM_MAH;
  int xx = pMeterSum / EST_PMSum;
  itoa(xx,screenBuffer+1,10);
  MAX7456_WriteString(screenBuffer,getPosition(pMeterSumPosition));
}

void displayRSSI(void)
{
  screenBuffer[0] = SYM_RSSI;
  // Calcul et affichage du Rssi
  itoa(rssi,screenBuffer+1,10);
  uint8_t xx = FindNull();
  screenBuffer[xx++] = '%';
  screenBuffer[xx] = 0;
  MAX7456_WriteString(screenBuffer,getPosition(rssiPosition));
}

void displayHeading(void)
{
  int16_t heading = MwHeading;
  if (Settings[S_HEADING360]) {
    if(heading < 0)
      heading += 360;
    ItoaPadded(heading,screenBuffer,3,0);
    screenBuffer[3]=SYM_DEGREES;
    screenBuffer[4]=0;
  }
  else {
    ItoaPadded(heading,screenBuffer,4,0);
    screenBuffer[4]=SYM_DEGREES;
    screenBuffer[5]=0;
  }
  MAX7456_WriteString(screenBuffer,getPosition(MwHeadingPosition));
}

void displayHeadingGraph(void)
{
  int xx;
  xx = MwHeading * 4;
  xx = xx + 720 + 45;
  xx = xx / 90;

  uint16_t pos = getPosition(MwHeadingGraphPosition);
  memcpy_P(screen+pos, headGraph+xx, 10);
}

void displayIntro(void)
{

  MAX7456_WriteString_P(message0, KVTeamVersionPosition);

  if (Settings[S_VIDEOSIGNALTYPE])
    MAX7456_WriteString_P(message2, KVTeamVersionPosition+30);
  else
    MAX7456_WriteString_P(message1, KVTeamVersionPosition+30);

    

  MAX7456_WriteString_P(MultiWiiLogoL1Add, KVTeamVersionPosition+120);
  MAX7456_WriteString_P(MultiWiiLogoL2Add, KVTeamVersionPosition+120+LINE);
  MAX7456_WriteString_P(MultiWiiLogoL3Add, KVTeamVersionPosition+120+LINE+LINE);

  MAX7456_WriteString_P(message5, KVTeamVersionPosition+120+LINE+LINE+LINE);
  MAX7456_WriteString(itoa(MwVersion,screenBuffer,10),KVTeamVersionPosition+131+LINE+LINE+LINE);

  MAX7456_WriteString_P(message6, KVTeamVersionPosition+120+LINE+LINE+LINE+LINE+LINE);
  MAX7456_WriteString_P(message7, KVTeamVersionPosition+125+LINE+LINE+LINE+LINE+LINE+LINE);
  MAX7456_WriteString_P(message8, KVTeamVersionPosition+125+LINE+LINE+LINE+LINE+LINE+LINE+LINE);
  
  MAX7456_WriteString_P(message59, KVTeamVersionPosition+120+LINE+LINE+LINE+LINE+LINE+LINE+LINE+LINE);
   if(Settings[S_DISPLAY_CS]){
      for(int X=0; X<10; X++) {
          screenBuffer[X] = char(Settings[S_CS0 + X]);
      }
   if (Blink2hz)
   MAX7456_WriteString(screenBuffer, KVTeamVersionPosition+130+LINE+LINE+LINE+LINE+LINE+LINE+LINE+LINE);;     // Call Sign on the beggining of the transmission (blink 2hz)  
   }
}

void displayFontScreen(void) {
  MAX7456_WriteString_P(PSTR("UPLOADING FONT"), 35);
  MAX7456_WriteString(itoa(nextCharToRequest, screenBuffer, 10), 51);

  for(uint16_t i = 0; i < 256; i++)
    screen[90+i] = i;
}

void displayGPSPosition(void)
{
  if(!GPS_fix)
    return;

  if(Settings[S_COORDINATES]){
    if(fieldIsVisible(MwGPSLatPosition)) {
      screenBuffer[0] = SYM_LAT;
      FormatGPSCoord(GPS_latitude,screenBuffer+1,3,'N','S');
      MAX7456_WriteString(screenBuffer,getPosition(MwGPSLatPosition));
    }

    if(fieldIsVisible(MwGPSLatPosition)) {
      screenBuffer[0] = SYM_LON;
      FormatGPSCoord(GPS_longitude,screenBuffer+1,4,'E','W');
      MAX7456_WriteString(screenBuffer,getPosition(MwGPSLonPosition));
    }
  }

  //screenBuffer[0] = MwGPSAltPositionAdd[Settings[S_UNITSYSTEM]];
  
  screenBuffer[0] = ' ';
  
  uint16_t xx;
  if(Settings[S_UNITSYSTEM])
    xx = GPS_altitude * 3.2808; // Mt to Feet
  else
    xx = GPS_altitude;          // Mt
  itoa(xx,screenBuffer+1,10);
  MAX7456_WriteString(screenBuffer,getPosition(MwGPSAltPosition));
}

void displayNumberOfSat(void)
{
  screenBuffer[0] = SYM_SAT_L;
  screenBuffer[1] = SYM_SAT_R;
  itoa(GPS_numSat,screenBuffer+2,10);
  MAX7456_WriteString(screenBuffer,getPosition(GPS_numSatPosition));
}


void displayGPS_speed(void)
{

  if(!GPS_fix) return;
  if(!armed) GPS_speed=0;

  int xx;
  if(!Settings[S_UNITSYSTEM])
    xx = GPS_speed * 0.036;           // From MWii cm/sec to Km/h
  else
    xx = GPS_speed * 0.02236932;      // (0.036*0.62137)  From MWii cm/sec to mph

  if(xx > speedMAX)
    speedMAX = xx;
    
  //screenBuffer[0]=speedUnitAdd[Settings[S_UNITSYSTEM]];
  screenBuffer[0]=' ';
  itoa(xx,screenBuffer+1,10);
    
  MAX7456_WriteString(screenBuffer,getPosition(speedPosition));
}


/*void displayGPS_speed(void)
{
  if(!GPS_fix)
    return;                                               //  DO NOT DELETE

  screenBuffer[0] = speedUnitAdd[Settings[S_UNITSYSTEM]];
  itoa(CMsToKMh(GPS_speed), screenBuffer+1, 10);
  MAX7456_WriteString(screenBuffer,getPosition(speedPosition));
}*/

void displayAltitude(void)
{  
  int16_t altitude;
  if(Settings[S_UNITSYSTEM])
    altitude = MwAltitude*0.032808;    // cm to feet
  else
    altitude = MwAltitude/100;         // cm to m

  if(armed && allSec>5 && altitude > altitudeMAX)
    altitudeMAX = altitude;
    
  //Limit the displayed altitude to 999 regardless of units
  //Note that actual altitudeMax is still saved
    
  if (altitude > 999) altitude = 999;
  if (altitude < -999) altitude = -999;

  //screenBuffer[0]=MwAltitudeAdd[Settings[S_UNITSYSTEM]];
  //screenBuffer[0]=MwAltitudeAdd;//
  
  screenBuffer[0]=' ';
  
  itoa(altitude,screenBuffer+1,10);
  MAX7456_WriteString(screenBuffer,getPosition(MwAltitudePosition));
}

void displayClimbRate(void)
{

  //if(MwVario > 70)       screenBuffer[0] = SYM_POS_CLIMB3;
  //else if(MwVario > 50)  screenBuffer[0] = SYM_POS_CLIMB2;
  //else if(MwVario > 30)  screenBuffer[0] = SYM_POS_CLIMB1;
  //else if(MwVario > 20)  screenBuffer[0] = SYM_POS_CLIMB;
  //else if(MwVario < -70) screenBuffer[0] = SYM_NEG_CLIMB3;
  //else if(MwVario < -50) screenBuffer[0] = SYM_NEG_CLIMB2;
  //else if(MwVario < -30) screenBuffer[0] = SYM_NEG_CLIMB1;
  //else if(MwVario < -20) screenBuffer[0] = SYM_NEG_CLIMB;
  //else screenBuffer[0] = SYM_ZERO_CLIMB;


 if(MwVario > 70)        {climbLineOffset = -2; screenBuffer[0] = SYM_CLIMB_TOP;}
  else if(MwVario > 60)  {climbLineOffset = -2; screenBuffer[0] = SYM_CLIMB;}
  else if(MwVario > 50)  {climbLineOffset = -2; screenBuffer[0] = SYM_CLIMB_BOTTOM;}
  else if(MwVario > 40)  {climbLineOffset = -1; screenBuffer[0] = SYM_CLIMB_TOP;}
  else if(MwVario > 30)  {climbLineOffset = -1; screenBuffer[0] = SYM_CLIMB;}
  else if(MwVario > 20)  {climbLineOffset = -1; screenBuffer[0] = SYM_CLIMB_BOTTOM;}
  else if(MwVario > 10)  {climbLineOffset = -0; screenBuffer[0] = SYM_CLIMB_TOP;}
  else if(MwVario < -70) {climbLineOffset = 2; screenBuffer[0] = SYM_CLIMB_BOTTOM;}
  else if(MwVario < -60) {climbLineOffset = 2; screenBuffer[0] = SYM_CLIMB;}
  else if(MwVario < -50) {climbLineOffset = 2; screenBuffer[0] = SYM_CLIMB_TOP;}
  else if(MwVario < -40) {climbLineOffset = 1; screenBuffer[0] = SYM_CLIMB_BOTTOM;}
  else if(MwVario < -30) {climbLineOffset = 1; screenBuffer[0] = SYM_CLIMB;}
  else if(MwVario < -20) {climbLineOffset = 1; screenBuffer[0] = SYM_CLIMB_TOP;}
  else if(MwVario < -10) {climbLineOffset = 0; screenBuffer[0] = SYM_CLIMB_BOTTOM;}
  else {climbLineOffset = 0; screenBuffer[0] = SYM_CLIMB;}
    
  //screenBuffer[0] = SYM_POS_CLIMB;
  
  //MAX7456_WriteString(screenBuffer,getPosition(MwClimbRatePosition) + climbLineOffset * LINE);
  
  //screenBuffer[1] = MwClimbRateAdd[Settings[S_UNITSYSTEM]];
  //screenBuffer[1] = MwClimbRateAdd;//

  //Rate of Climb is usually in 100's of Feet per Min (Which is also close to
  //the vertical speed in knots), or Meters per Second
  //http://en.wikipedia.org/wiki/Variometer  (First Paragraph)

  int16_t vario;
  if(Settings[S_UNITSYSTEM])
    //vario = MwVario * 0.032808;       // cm/sec ----> ft/sec
    vario = MwVario * .0196848;       // cm/sec ----> 100s of ft/min
  else
    vario = MwVario / 100;            // cm/sec ----> m/sec
    //vario = MwVario / 1.66667;            // cm/sec ----> m/min
  itoa(vario, screenBuffer+1, 10);

  MAX7456_WriteString(screenBuffer,getPosition(MwClimbRatePosition) + climbLineOffset * LINE);
}

void displayDistanceToHome(void)
{
  if(!GPS_fix)
    return;

  int16_t dist;
  if(Settings[S_UNITSYSTEM])
    dist = GPS_distanceToHome * 3.2808;           // mt to feet
  else
    dist = GPS_distanceToHome;                    // Mt

  if(dist > distanceMAX)
    distanceMAX = dist;

  screenBuffer[0] = GPS_distanceToHomeAdd[Settings[S_UNITSYSTEM]];
  //screenBuffer[0] = GPS_distanceToHomeAdd;//

  itoa(dist, screenBuffer+1, 10);
  MAX7456_WriteString(screenBuffer,getPosition(GPS_distanceToHomePosition));
}

void displayAngleToHome(void)
{
  if(!GPS_fix)
    return;
  if(GPS_distanceToHome <= 2 && Blink2hz)
    return;

  ItoaPadded(GPS_directionToHome,screenBuffer,3,0);
  screenBuffer[3] = SYM_DEGREES;
  screenBuffer[4] = 0;
  MAX7456_WriteString(screenBuffer,getPosition(GPS_angleToHomePosition));
}

void displayDirectionToHome(void)
{
  if(!GPS_fix)
    return;
  if(GPS_distanceToHome <= 2 && Blink2hz)
    return;

  int16_t d = MwHeading + 22 + 180 + 360 - GPS_directionToHome;  // Revision: 8370bd56cef9 
  d *= 4;
  d += 45;
  d = (d/90)%16;

  screenBuffer[0] = SYM_ARROW_SOUTH + d;
  screenBuffer[1]=0;
  MAX7456_WriteString(screenBuffer,getPosition(GPS_directionToHomePosition));
}

void displayCursor(void)
{
  int cursorpos;

  if(ROW==10){
    if(COL==3) cursorpos=SAVEP+16-1;    // page
    if(COL==1) cursorpos=SAVEP-1;       // exit
    if(COL==2) cursorpos=SAVEP+6-1;     // save/exit
  }
  if(ROW<10){
    if(configPage==1){
      if (ROW==9) ROW=7;
      if (ROW==8) ROW=10;
      if(COL==1) cursorpos=(ROW+2)*30+10;
      if(COL==2) cursorpos=(ROW+2)*30+10+6;
      if(COL==3) cursorpos=(ROW+2)*30+10+6+6;
      }
    if(configPage==2){
      COL=3;
      if (ROW==7) ROW=5;
      if (ROW==6) ROW=10;
      if (ROW==9) ROW=5;
      cursorpos=(ROW+2)*30+10+6+6;
      }
    if(configPage==3){
      COL=3;
      if (ROW==1) ROW=2;
      if (ROW==9) ROW=7;
      if (ROW==8) ROW=10;
      cursorpos=(ROW+2)*30+10+6+6;
     
      }
    if(configPage==4){
      COL=3;
      if (ROW==2) ROW=3;
      if (ROW==9) ROW=7;
      if (ROW==8) ROW=10;
      if ((ROW==6)||(ROW==7)) cursorpos=(ROW+2)*30+10+6+6-2;  // Narrow/Imperial strings longer
      else cursorpos=(ROW+2)*30+10+6+6;
      }
    if(configPage==5){
      COL=3;
      if (ROW==9) ROW=7;
      if (ROW==8) ROW=10;
      cursorpos=(ROW+2)*30+10+6+6;
      }
    if(configPage==6){
      ROW=10;
      }
  }
  if(Blink10hz)
    screen[cursorpos] = SYM_CURSOR;
}


void displayConfigScreen(void)
{
  MAX7456_WriteString_P(configMsg0, SAVEP);    //EXIT
  if(!previousarmedstatus) {
    MAX7456_WriteString_P(configMsg1, SAVEP+6);  //SaveExit
    MAX7456_WriteString_P(configMsg2, SAVEP+16); //<Page>
  }

  if(configPage==1)
  {
    MAX7456_WriteString_P(configMsg3, 38);
    MAX7456_WriteString_P(configMsg4, ROLLT);
    MAX7456_WriteString(itoa(P8[0],screenBuffer,10),ROLLP);
    MAX7456_WriteString(itoa(I8[0],screenBuffer,10),ROLLI);
    MAX7456_WriteString(itoa(D8[0],screenBuffer,10),ROLLD);

    MAX7456_WriteString_P(configMsg5, PITCHT);
    MAX7456_WriteString(itoa(P8[1],screenBuffer,10), PITCHP);
    MAX7456_WriteString(itoa(I8[1],screenBuffer,10), PITCHI);
    MAX7456_WriteString(itoa(D8[1],screenBuffer,10), PITCHD);

    MAX7456_WriteString_P(configMsg6, YAWT);
    MAX7456_WriteString(itoa(P8[2],screenBuffer,10),YAWP);
    MAX7456_WriteString(itoa(I8[2],screenBuffer,10),YAWI);
    MAX7456_WriteString(itoa(D8[2],screenBuffer,10),YAWD);

    MAX7456_WriteString_P(configMsg7, ALTT);
    MAX7456_WriteString(itoa(P8[3],screenBuffer,10),ALTP);
    MAX7456_WriteString(itoa(I8[3],screenBuffer,10),ALTI);
    MAX7456_WriteString(itoa(D8[3],screenBuffer,10),ALTD);

    MAX7456_WriteString_P(configMsg8, VELT);
    MAX7456_WriteString(itoa(P8[4],screenBuffer,10),VELP);
    MAX7456_WriteString(itoa(I8[4],screenBuffer,10),VELI);
    MAX7456_WriteString(itoa(D8[4],screenBuffer,10),VELD);

    MAX7456_WriteString_P(configMsg9, LEVT);
    MAX7456_WriteString(itoa(P8[7],screenBuffer,10),LEVP);
    MAX7456_WriteString(itoa(I8[7],screenBuffer,10),LEVI);
    MAX7456_WriteString(itoa(D8[7],screenBuffer,10),LEVD);

    MAX7456_WriteString_P(configMsg10, MAGT);
    MAX7456_WriteString(itoa(P8[8],screenBuffer,10),MAGP);

    MAX7456_WriteString("P",71);
    MAX7456_WriteString("I",77);
    MAX7456_WriteString("D",83);
  }

  if(configPage==2)
  {
    MAX7456_WriteString_P(configMsg11, 38);
    MAX7456_WriteString_P(configMsg12, ROLLT);
    MAX7456_WriteString(itoa(rcRate8,screenBuffer,10),ROLLD);
    MAX7456_WriteString_P(configMsg13, PITCHT);
    MAX7456_WriteString(itoa(rcExpo8,screenBuffer,10),PITCHD);
    MAX7456_WriteString_P(configMsg14, YAWT);
    MAX7456_WriteString(itoa(rollPitchRate,screenBuffer,10),YAWD);
    MAX7456_WriteString_P(configMsg15, ALTT);
    MAX7456_WriteString(itoa(yawRate,screenBuffer,10),ALTD);
    MAX7456_WriteString_P(configMsg16, VELT);
    MAX7456_WriteString(itoa(dynThrPID,screenBuffer,10),VELD);
    MAX7456_WriteString_P(configMsg17, LEVT);
    MAX7456_WriteString(itoa(cycleTime,screenBuffer,10),LEVD);
    MAX7456_WriteString_P(configMsg18, MAGT);
    MAX7456_WriteString(itoa(I2CError,screenBuffer,10),MAGD);
  }

  if(configPage==3)
  {
    MAX7456_WriteString_P(configMsg19, 35);
    MAX7456_WriteString_P(configMsg23, PITCHT);
    if(Settings[S_DISPLAYVOLTAGE]){
      MAX7456_WriteString_P(configMsg21, PITCHD);
    }
    else {
      MAX7456_WriteString_P(configMsg22, PITCHD);
    }
    MAX7456_WriteString_P(configMsg24, YAWT);
    MAX7456_WriteString(itoa(Settings[S_VOLTAGEMIN],screenBuffer,10),YAWD);
    MAX7456_WriteString_P(configMsg25, ALTT);

    if(Settings[S_DISPLAYTEMPERATURE] ){
      MAX7456_WriteString_P(configMsg21, ALTD);
    }
    else {
      MAX7456_WriteString_P(configMsg22, ALTD);
    }
    MAX7456_WriteString_P(configMsg26, VELT);
    MAX7456_WriteString(itoa(Settings[S_TEMPERATUREMAX],screenBuffer,10),VELD);
    MAX7456_WriteString_P(configMsg27, LEVT);

    if(Settings[S_DISPLAYGPS]){
      MAX7456_WriteString_P(configMsg21, LEVD);
     }
     else {
      MAX7456_WriteString_P(configMsg22, LEVD);
    }
    MAX7456_WriteString_P(configMsg28, MAGT);
    if(Settings[S_COORDINATES]){
      MAX7456_WriteString_P(configMsg21, MAGD);
     }
     else {
      MAX7456_WriteString_P(configMsg22, MAGD);
    }
  }

  if(configPage==4)
  {
    MAX7456_WriteString_P(configMsg31, 39);

    MAX7456_WriteString_P(configMsg32, ROLLT);
    MAX7456_WriteString(itoa(rssiADC,screenBuffer,10),ROLLD);

    MAX7456_WriteString_P(configMsg33, PITCHT);
    MAX7456_WriteString(itoa(rssi,screenBuffer,10),PITCHD);

    MAX7456_WriteString_P(configMsg34, YAWT);
    if(rssiTimer>0) MAX7456_WriteString(itoa(rssiTimer,screenBuffer,10),YAWD-5);
    MAX7456_WriteString(itoa(Settings[S_RSSIMIN],screenBuffer,10),YAWD);

    MAX7456_WriteString_P(configMsg35, ALTT);
    MAX7456_WriteString(itoa(Settings[S_RSSIMAX],screenBuffer,10),ALTD);

    MAX7456_WriteString_P(configMsg36, VELT);
    if(Settings[S_DISPLAYRSSI]){
      MAX7456_WriteString_P(configMsg21, VELD);
    }
    else{
      MAX7456_WriteString_P(configMsg22, VELD);
    }

    MAX7456_WriteString_P(configMsg37, LEVT);
    if(Settings[S_UNITSYSTEM]==METRIC){
      MAX7456_WriteString_P(configMsg38, LEVD-2);
    }
    else {
      MAX7456_WriteString_P(configMsg39, LEVD-2);
    }

    MAX7456_WriteString_P(configMsg40, MAGT);
    if(Settings[S_VIDEOSIGNALTYPE]){
      MAX7456_WriteString_P(configMsg42, MAGD);
    }
    else {
      MAX7456_WriteString_P(configMsg41, MAGD);
    }
  }

  if(configPage==5)
  {
    MAX7456_WriteString_P(configMsg43, 37);

    MAX7456_WriteString_P(configMsg44, ROLLT);
    if(accCalibrationTimer>0)
      MAX7456_WriteString(itoa(accCalibrationTimer,screenBuffer,10),ROLLD);
    else
      MAX7456_WriteString("-",ROLLD);

    MAX7456_WriteString_P(configMsg45, PITCHT);
    MAX7456_WriteString(itoa(MwAccSmooth[0],screenBuffer,10),PITCHD);

    MAX7456_WriteString_P(configMsg46, YAWT);
    MAX7456_WriteString(itoa(MwAccSmooth[1],screenBuffer,10),YAWD);

    MAX7456_WriteString_P(configMsg47, ALTT);
    MAX7456_WriteString(itoa(MwAccSmooth[2],screenBuffer,10),ALTD);

    MAX7456_WriteString_P(configMsg48, VELT);
    if(magCalibrationTimer>0)
      MAX7456_WriteString(itoa(magCalibrationTimer,screenBuffer,10),VELD);
    else
      MAX7456_WriteString("-",VELD);

    MAX7456_WriteString_P(configMsg49, LEVT);
    MAX7456_WriteString(itoa(MwHeading,screenBuffer,10),LEVD);

    MAX7456_WriteString_P(configMsg50, MAGT);
    if(eepromWriteTimer>0)
      MAX7456_WriteString(itoa(eepromWriteTimer,screenBuffer,10),MAGD);
    else
      MAX7456_WriteString("-",MAGD);
  }

  if(configPage==6)
  {
    int xx;
    MAX7456_WriteString_P(configMsg51, 38);

    MAX7456_WriteString_P(configMsg52, ROLLT);
    MAX7456_WriteString(itoa(trip,screenBuffer,10),ROLLD);

    MAX7456_WriteString_P(configMsg53, PITCHT);
    MAX7456_WriteString(itoa(distanceMAX,screenBuffer,10),PITCHD);

    MAX7456_WriteString_P(configMsg54, YAWT);
    MAX7456_WriteString(itoa(altitudeMAX,screenBuffer,10),YAWD);

    MAX7456_WriteString_P(configMsg55, ALTT);
    MAX7456_WriteString(itoa(speedMAX,screenBuffer,10),ALTD);
    //MAX7456_WriteString(itoa(CMsToKMh(speedMAX), screenBuffer, 10), ALTD);     DO NOT DELETE

    MAX7456_WriteString_P(configMsg56, VELT);

    formatTime(flyingTime, screenBuffer, 1);
    MAX7456_WriteString(screenBuffer,VELD-7);

    MAX7456_WriteString_P(configMsg57, LEVT);
    xx= pMeterSum / EST_PMSum;
    MAX7456_WriteString(itoa(xx,screenBuffer,10),LEVD);

    MAX7456_WriteString_P(configMsg58, MAGT);
    MAX7456_WriteString(itoa(temperMAX,screenBuffer,10),MAGD);
    //MAX7456_WriteString(itoa(TempConverter(temperMAX), screenBuffer, 10), MAGD);      DO NOT DELETE
  }
  displayCursor();
}
