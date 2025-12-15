

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Configuração do LCD 20x4
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Pinos dos botões e saídas
const int botaoManual = 2;
const int botaoPurga = 3;
const int botaoEsteira = 4;
const int valvula = 5;

// Variáveis de controle
bool modoManual = false;
bool esteiraPressionada = false;
unsigned long inicioContagem = 0;
unsigned long tempoAtual = 0;
unsigned long tempoEsteira = 120000; // Tempo em milissegundos (2 minutos)
const unsigned long tempoValvula = 10000; // Tempo da válvula (10 segundos)
bool contagemFinalizada = false;

void setup() {
  pinMode(botaoManual, INPUT_PULLUP);
  pinMode(botaoPurga, INPUT_PULLUP);
  pinMode(botaoEsteira, INPUT_PULLUP);
  pinMode(valvula, OUTPUT);

  digitalWrite(valvula, LOW);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Sistema Inicializado");
  delay(2000);
}

void loop() {
  modoManual = !digitalRead(botaoManual);

  if (modoManual) {
    lcd.setCursor(0, 0);
    lcd.print("Modo: Manual     ");

    if (!digitalRead(botaoPurga)) {
      digitalWrite(valvula, HIGH);
      lcd.setCursor(0, 1);
      lcd.print("Valvula: Ligada   ");
    } else {
      digitalWrite(valvula, LOW);
      lcd.setCursor(0, 1);
      lcd.print("Valvula: Desligada");
    }
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Modo: Automatico ");

    if (!digitalRead(botaoEsteira)) {
      if (!esteiraPressionada) {
        esteiraPressionada = true;
        inicioContagem = millis();
      } else {
        tempoAtual = millis() - inicioContagem;
      }

      if (tempoAtual >= tempoEsteira) {
        contagemFinalizada = true;
        digitalWrite(valvula, HIGH);
        lcd.setCursor(0, 1);
        lcd.print("Valvula: Ligada   ");
        delay(tempoValvula);
        digitalWrite(valvula, LOW);
        inicioContagem = millis(); // Reinicia a contagem automaticamente
        contagemFinalizada = false;
      }
    } else {
      if (esteiraPressionada) {
        inicioContagem = millis() - tempoAtual; // Pausa a contagem
      }
      esteiraPressionada = false;
    }

    lcd.setCursor(0, 1);
    if (contagemFinalizada) {
      lcd.print("Contagem Completa");
    } else {
      lcd.print("Tempo: ");
      lcd.print((tempoEsteira - tempoAtual) / 1000);
      lcd.print(" s    ");
    }
  }

  delay(200); // Atualização do display
}
