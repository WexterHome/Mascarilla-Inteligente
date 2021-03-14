int getSoundLength() {
  int thisMax = 0;
  int thisSignal = 0;
  for (int i = 0; i < 200; i++) {    
    thisSignal = analogRead(ADC_PIN); 
    if (thisSignal > thisMax) thisMax = thisSignal;
  }
  
  static float filMax = thisMax;
  filMax += (thisMax - filMax) * 0.95;
  static float steady = thisMax;

  if (filMax - steady < 30) steady += (filMax - steady) * 0.005;
  if (steady > filMax) steady = filMax;

  if (filMax - steady > VOL_THR) return (constrain(map(filMax - steady - VOL_THR, 0, VOL_MAX, 0, 7), 0, 7));
  else return 0;
}

void analyzeAudio() {
  for (int i = 0 ; i < FHT_N ; i++) {
    int sample = analogRead(FREQ_PIN);
    fht_input[i] = sample; 
    delayMicroseconds(500);
  }
  fht_window();
  fht_reorder();
  fht_run(); 
  fht_mag_log();
}
