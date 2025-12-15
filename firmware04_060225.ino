
-

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Configuração do display 20x4
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Definição dos pinos de entrada e saída
const int chaveModo         = 5;    // Chave seletora (Manual/Automático)
const int esteiraLigada     = 6;    // Sinal da esteira (LOW = Ligada)
const int botaoAssoalho     = 3;    // Botão "Assoalho"
const int botaoLanca        = 4;    // Botão "Lança"
const int saidaLubrificar1  = A0;   // Saída "Lubrificar1"
const int saidaLubrificar2  = 7;    // Saída "Lubrificar2"
const int pressostato       = A1;   // Sinal do pressostato (LOW = pressão OK)
const int saidaOsciladora   = 8;    // Nova saída: piscará em caso de falta de pressão

// Variáveis de tempo (em milissegundos)
unsigned long tempoLubrificar1 = 3000;  // Tempo de ativação de Lubrificar1 (3 s)
unsigned long tempoLubrificar2 = 3000;  // Tempo de ativação de Lubrificar2 (3 s)
unsigned long intervaloAlvo    = 30000; // Intervalo total para ciclo automático (10 s)
unsigned long intervaloAtual   = 0;     // Contador do intervalo

// Variáveis de controle
unsigned long ultimoTempo = 0;
bool esteiraAtiva = false;

// Variáveis para a saída osciladora (D8)
unsigned long lastToggleTime = 0;
bool stateOsciladora = LOW;

void setup() {
  // Inicializa o display
  lcd.init();
  lcd.backlight();

  // Configura os pinos de entrada com resistor de pull-up para evitar flutuações
  pinMode(chaveModo,       INPUT_PULLUP);
  pinMode(esteiraLigada,   INPUT_PULLUP);
  pinMode(botaoAssoalho,   INPUT_PULLUP);
  pinMode(botaoLanca,      INPUT_PULLUP);
  pinMode(pressostato,     INPUT_PULLUP);
  
  // Configura os pinos de saída
  pinMode(saidaLubrificar1, OUTPUT);
  pinMode(saidaLubrificar2, OUTPUT);
  pinMode(saidaOsciladora, OUTPUT);
  
  // Garante que as saídas iniciem desligadas
  digitalWrite(saidaLubrificar1, LOW);
  digitalWrite(saidaLubrificar2, LOW);
  digitalWrite(saidaOsciladora, LOW);

  //INICIALIZAÇÃO DE ENTRADA
  lcd.setCursor(0,0);
  lcd.print("  Eletroneves   ");
  lcd.setCursor(0,1);
  lcd.print("  Mecatronica   ");
  delay(5000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("    Central     ");
  lcd.setCursor(0,1);
  lcd.print("  Lubrificacao  ");
  delay(5000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Eng. Felix_2025 ");
  lcd.setCursor(0,1);
  lcd.print("**v1.0  Rev2.1**");
  delay(5000);
  lcd.clear();
  
  // Mensagem inicial no display
  lcd.setCursor(0, 0);
  lcd.print("Inicializado... ");
  delay(2000);
}

void loop() {
  // Verifica a posição da chave: (LOW) = automático, (HIGH) = manual
  bool modoAutomatico = !digitalRead(chaveModo);

  if (modoAutomatico) {
    executarModoAutomatico();
  } else {
    executarModoManual();
  }
  
  // Saída osciladora (D8):
  // Se o pressostato indicar falta de pressão (ou seja, não estiver em LOW),
  // D8 pisca a cada 500 ms.
  if (digitalRead(pressostato) != LOW) {
    if (millis() - lastToggleTime >= 500) {
      lastToggleTime = millis();
      stateOsciladora = !stateOsciladora;
      digitalWrite(saidaOsciladora, stateOsciladora);
    }
  } else {
    digitalWrite(saidaOsciladora, LOW);
  }
  
  // Pequeno delay para evitar leituras muito rápidas
  delay(800);
}

void executarModoAutomatico() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Modo: Automatico");

  // Verifica o estado do pressostato:
  // Se o pressostato não estiver em LOW (ou seja, pressão inadequada),
  // exibe mensagem e interrompe a execução deste modo.
  if (digitalRead(pressostato) != LOW) {
    lcd.setCursor(0, 1);
    lcd.print("Sem Pressao     ");
    return;
  }
  
  // Verifica o estado da esteira (LOW = ligada)
  esteiraAtiva = !digitalRead(esteiraLigada);
  
  if (esteiraAtiva) {
    // Incrementa o contador a cada 1 segundo
    if (millis() - ultimoTempo >= 1000) {
      intervaloAtual += 1000;
      ultimoTempo = millis();
    }
    
    // Exibe o tempo decorrido no display (linha 2)
    lcd.setCursor(0, 1);
    lcd.print("Intervalo: ");
    lcd.print(intervaloAtual / 1000);
    lcd.print("s   ");
    
    // Se o intervalo atingir o tempo alvo, realiza o ciclo de lubrificação
    if (intervaloAtual >= intervaloAlvo) {
      realizarCicloLubrificacao();
      intervaloAtual = 0;  // Reinicia a contagem após o ciclo
    }
  } else {
    lcd.setCursor(0, 1);
    lcd.print("Esteira: OFF    ");
    ultimoTempo = millis(); // Reinicia a contagem para evitar pulos
  }
}

void realizarCicloLubrificacao() {
  // Aciona Lubrificar1 por tempo definido
  digitalWrite(saidaLubrificar1, HIGH);
  delay(tempoLubrificar1);
  digitalWrite(saidaLubrificar1, LOW);
  
  // Aciona Lubrificar2 por tempo definido
  digitalWrite(saidaLubrificar2, HIGH);
  delay(tempoLubrificar2);
  digitalWrite(saidaLubrificar2, LOW);
}

void executarModoManual() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Modo: Manual    ");
  
  // Verifica o sinal do pressostato:
  // Se não estiver em LOW, exibe mensagem e não permite acionar as saídas.
  if (digitalRead(pressostato) != LOW) {
    lcd.setCursor(0, 1);
    lcd.print("Sem Pressao      ");
    digitalWrite(saidaLubrificar1, LOW);
    digitalWrite(saidaLubrificar2, LOW);
    return;
  }
  
  // Leitura dos botões (como INPUT_PULLUP, pressionado = LOW)
  bool assoalhoPressionado = !digitalRead(botaoAssoalho);
  bool lancaPressionado    = !digitalRead(botaoLanca);
  
  // Intertravamento: somente um botão aciona a saída por vez
  if (assoalhoPressionado && !lancaPressionado) {
    digitalWrite(saidaLubrificar1, HIGH);
    digitalWrite(saidaLubrificar2, LOW);
    lcd.setCursor(0, 1);
    lcd.print("Assoalho:Lubr...");
  } 
  else if (lancaPressionado && !assoalhoPressionado) {
    digitalWrite(saidaLubrificar2, HIGH);
    digitalWrite(saidaLubrificar1, LOW);
    lcd.setCursor(0, 1);
    lcd.print("Lanca: Lubr...  ");
  } 
  else {
    // Caso nenhum botão ou ambos estejam pressionados, nenhuma saída é acionada
    digitalWrite(saidaLubrificar1, LOW);
    digitalWrite(saidaLubrificar2, LOW);
    lcd.setCursor(0, 1);
    lcd.print("AG. Acionamento ");
  }
}
