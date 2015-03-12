void FilterAD()
{
  // read from AD and subtract the offset
  xRaw=compass.a.x-xOffset;
  yRaw=compass.a.y-yOffset;
  zRaw=compass.a.z-zOffset;
  
  xFiltered= xFilteredOld + alphaAccel * (xRaw - xFilteredOld);
  yFiltered= yFilteredOld + alphaAccel * (yRaw - yFilteredOld);
  zFiltered= zFilteredOld + alphaAccel * (zRaw - zFilteredOld);
  
  xFilteredOld = xFiltered;
  yFilteredOld = yFiltered;
  zFilteredOld = zFiltered;
}


void AD2Degree()
{ 
  // 3.3 = Vref; 1023 = 10Bit AD; 0.8 = Sensivity Accelerometer
  // (compare datasheet of your accelerometer)
  // the *Accel must be between -1 and 1; you may have to
  // to add/subtract +1 depending on the orientation of the accelerometer
  // (like me on the zAccel)
  // they are not necessary, but are useful for debugging
  xAccel=xFiltered *3.3 / (1023.0*0.8);       
  yAccel=yFiltered *3.3 / (1023.0*0.8);       
  zAccel=zFiltered *3.3 / (1023.0*0.8)+1.0;
  //angled are radian, for degree (* 180/3.14159)
  Roll   = atan2(  yAccel ,  sqrt(sq(xAccel)+sq(zAccel)));  
  Pitch  = atan2(  xAccel ,   sqrt(sq(yAccel)+sq(zAccel)));
   
}


 void getAccelOffset()
{ //you can make approx 60 iterations because we use an unsigned int 
  //otherwise you get an overflow. But 60 iterations should be fine
   for (int i=1; i <= 60; i++){        
     xOffset += compass.a.x;     
     yOffset += compass.a.y;
     zOffset += compass.a.z;
     } 
   xOffset /=60;   
   yOffset /=60;
   zOffset /=60;
   
   Serial.print("xOffset: ");
   Serial.print(xOffset);
   Serial.print("   yOffset: ");
   Serial.print(yOffset);
   Serial.print("   zOffset: ");
   Serial.println(zOffset);
}

