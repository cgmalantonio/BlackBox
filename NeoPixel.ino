
//-SD-Initialize  7
//-SD-Create File 6
//-Log-Write File 5
//-GPS-Fix        4
//N
//S
//E
//W
//-Status
void NeoStart(){
  for (int z = 0; z < 8; z++){
r[z]=0;
g[z]=0;
b[z]=0; 
}
NeoDisplay();
}


//SD Initialization
void NeoSDInitErr(){
r[7]=255; g[7]=0; b[7]=0; NeoDisplay();

}
void NeoSDInitOK(){
r[7]=0; g[7]=0; b[7]=255; NeoDisplay();
}

//SD Create Log
void NeoSDLogErr(){
r[6]=255; g[6]=0; b[6]=0; NeoDisplay();

}
void NeoSDLogOK(){
r[6]=0; g[6]=0; b[6]=255; NeoDisplay();
}

//Log Write File
void NeoLogErr(){
r[5]=255; g[5]=0; b[5]=0; NeoDisplay();

}
void NeoLogOK(){
r[5]=0; g[5]=0; b[5]=255; NeoDisplay();
}

//GPS Fix
void NeoGPSFixErr(){
r[4]=255; g[4]=0; b[4]=0; NeoDisplay();

}
void NeoGPSFixOK(){
r[4]=0; g[4]=0; b[4]=255; NeoDisplay();
}



void NeoN(){
r[3]=0; g[3]=0; b[3]=255; 
r[2]=0; g[2]=0; b[2]=0;
r[1]=0; g[1]=0; b[1]=0;
r[0]=0; g[0]=0; b[0]=0;
NeoDisplay();
}

void NeoE(){
r[3]=0; g[3]=0; b[3]=0; 
r[2]=0; g[2]=0; b[2]=255;
r[1]=0; g[1]=0; b[1]=0;
r[0]=0; g[0]=0; b[0]=0; 
NeoDisplay();
}

void NeoS(){
r[3]=0; g[3]=0; b[3]=0; 
r[2]=0; g[2]=0; b[2]=0;
r[1]=0; g[1]=0; b[1]=255;
r[0]=0; g[0]=0; b[0]=0; 
NeoDisplay();
}

void NeoW(){
r[3]=0; g[3]=0; b[3]=0; 
r[2]=0; g[2]=0; b[2]=0;
r[1]=0; g[1]=0; b[1]=0;
r[0]=0; g[0]=0; b[0]=255; 
NeoDisplay();
}



void NeoDisplay(){
for (int i = 0; i < 8; i++){
strip.setPixelColor(i, r[i], g[i], b[i]); 
}
  strip.show(); //output to the neopixel
  delay(2); //for safety 
}
