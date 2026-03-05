#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define entrada A0
#define entrada_2 A1
#define pantalla 33
#define boton 34

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

float voltaje_medido [200]; 
float corriente_medida [200];
//float array_voltaje [200]; 
//float array_corriente [200];

float voltaje_rms = 0; 
float corriente_rms = 0;
float suma_voltaje = 0;
float suma_corriente = 0;
float promedio_voltaje = 0;
float promedio_corriente = 0;
float potencia_activa = 0;
float potencia_aparente = 0;
float potencia_reactiva = 0;
float maximo_v = 0;
float maximo_i = 0;
float muestras = 200;
float factor = 0;
float ola1 = 0;
float ola2 = 4;
float voltaje_rms_red = 0;
float corriente_rms_red = 0;

int pagina = 1;


void setup() {
  Serial.begin(9600);
  Serial1.begin(115200);
  delay(500);
  pmc_enable_periph_clk(ID_ADC);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  pinMode(entrada, INPUT);
  pinMode(entrada_2, INPUT);
  pinMode(pantalla, INPUT_PULLUP);
  pinMode(boton, INPUT_PULLUP);
  analogReadResolution(12);
}

void loop() {
  if (digitalRead(boton) == 0){
    delay(100);
    leer_medidas();
  }
  
  if (digitalRead(pantalla) == LOW){
    delay(100);
    if (pagina == 1){
      mostrar_datos_1();
    }else {
      mostrar_datos_2();
    }
    delay(100);
    pagina ++;

  }
}


void leer_medidas(){
    float array_voltaje [200] ;
    float array_corriente [200] ;
  for(int i = 0; i < muestras; i++){
    voltaje_medido[i] = analogRead(entrada);
    corriente_medida[i] = analogRead(entrada_2);
    delayMicroseconds(71);
  }
   maximo_v = 0;
   maximo_i = 0;
  potencia_aparente = 0;
  potencia_reactiva = 0;
  potencia_activa = 0;
  for (int i = 0; i <muestras; i++){
    voltaje_medido[i] = (3.3/4095)*voltaje_medido[i];
    corriente_medida[i] = ((3.3/4095)*corriente_medida[i]);
    suma_voltaje += voltaje_medido[i];  
    suma_corriente += corriente_medida[i];
  }

  promedio_voltaje = suma_voltaje/ muestras;
  promedio_corriente = suma_corriente/ muestras;

  for(int i = 0; i < muestras; i++){
    
    float iac = (corriente_medida[i]- promedio_corriente)/0.82;  //0.82 resistencia shunt 
    float vac = (170/1.6)*(voltaje_medido[i] - promedio_voltaje);
     array_voltaje[i] = vac;
     array_corriente[i] = iac;
    potencia_activa += (iac*vac);

    voltaje_rms += (vac*vac);
    corriente_rms += (iac*iac);

    if (vac > maximo_v){
      maximo_v = vac;
    }
    if (iac > maximo_i){
      maximo_i = iac;
    }
  }
  
  corriente_rms = sqrt(corriente_rms/muestras);
  voltaje_rms = sqrt(voltaje_rms/muestras);
  voltaje_rms_red = voltaje_rms;
  corriente_rms_red = corriente_rms;
  potencia_activa = (potencia_activa/muestras);

  potencia_aparente = (voltaje_rms*corriente_rms);
  potencia_reactiva = (sqrt(potencia_aparente*potencia_aparente - potencia_activa*potencia_activa));
  factor = potencia_activa/potencia_aparente;

  Serial1.write((byte*)array_voltaje, sizeof(array_voltaje));
  Serial1.write((byte*)array_corriente, sizeof(array_corriente));
  
  voltaje_rms = 0;
  corriente_rms = 0;
  suma_voltaje = 0;
  promedio_voltaje = 0;
  return;
}

void mostrar_datos_1() { 
  if (pagina > 0) pagina = 1;
  display.clearDisplay();
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0); 
  display.print("Vrms:"); display.print(voltaje_rms_red); display.println("v");
  display.setCursor(0, 9);
  display.print("Vp:"); display.print(maximo_v); display.println("v");
  display.setCursor(0, 19);
  display.print("Irms:"); display.print(corriente_rms_red); display.println("A");
  display.setCursor(64, 19);
  display.print("Ip:"); display.print(maximo_i); display.println("A");
  display.display();
}
void mostrar_datos_2(){
  if (pagina > 1) pagina = 0;
  display.clearDisplay();
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);
  display.print("S:"); display.print(potencia_aparente); display.println("VA");
  display.setCursor(0, 10);
  display.print("P:"); display.print(potencia_activa); display.println("W");
  display.setCursor(64, 0);
  display.print("Q:"); display.print(potencia_reactiva); display.println("VAr");
  display.setCursor(64, 10);
  display.print("fp:"); display.println(factor);
  display.display();
}