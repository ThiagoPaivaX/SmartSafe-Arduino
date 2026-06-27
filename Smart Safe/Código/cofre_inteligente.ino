// ============================================================
//  COFRE INTELIGENTE — TCC
//  Versão: Memória + Ultrassônico | Três Servos | Conquista
//  Componentes: LCD I2C 16x2, Servo tampa A (13),
//               Servo tampa B (12), Buzzer, 3 LEDs, 3 Botões,
//               HC-SR04 (10/11)
// ============================================================

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// ============================================================
//  VARIÁVEIS DE TEMPO — altere aqui para ajustar o jogo
// ============================================================
const int   TEMPO_RESPOSTA_MS    = 2000;  // tempo para apertar cada botão (ms)
const int   TEMPO_LED_ACESO_MS   =  50;  // duração do LED na sequência (ms)
const int   TEMPO_LED_APAGADO_MS =  100;  // intervalo entre LEDs na sequência (ms)
const int   TEMPO_SEGURAR_MS     = 8000;  // tempo para segurar a mão no ultrassônico (ms)
const float DIST_MIN_CM          = 15.0;  // distância mínima aceita pelo sensor (cm)
const float DIST_MAX_CM          = 18.0;  // distância máxima aceita pelo sensor (cm)

// ============================================================
//  CONFIGURAÇÃO DA TAMPA A (pino 13) — MG90S
//  MG90S suporta até 180°, aproveite mais ângulo se precisar
// ============================================================
const bool SERVO_TAMPA_A_INVERTIDO = false;
const int  TAMPA_A_FECHADO         = 0;
const int  TAMPA_A_ABERTO          = 80;

// ============================================================
//  CONFIGURAÇÃO DA TAMPA B (pino 12) — MG90S
//  MG90S suporta até 180°, aproveite mais ângulo se precisar
// ============================================================
const bool SERVO_TAMPA_B_INVERTIDO = true;
const int  TAMPA_B_FECHADO         = 90;
const int  TAMPA_B_ABERTO          = 155;

// ============================================================
//  PINOS
// ============================================================
const int ledPins[]   = {2, 3, 4};
const int btnPins[]   = {5, 6, 7};
const int buzzer      = 8;
const int trigPin     = 10;
const int echoPin     = 11;
const int servoTampaB = 12;
const int servoTampaA = 13;

// ============================================================
//  OBJETOS GLOBAIS
// ============================================================
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo meuServoTampaA;
Servo meuServoTampaB;

int faseAtual = 0;

// ============================================================
//  HELPERS — converte ângulo considerando inversão do servo
// ============================================================
int anguloTampaA(int angulo) { return SERVO_TAMPA_A_INVERTIDO ? 180 - angulo : angulo; }
int anguloTampaB(int angulo) { return SERVO_TAMPA_B_INVERTIDO ? 180 - angulo : angulo; }

// ============================================================
//  SETUP
// ============================================================
void setup() {
  for (int i = 0; i < 3; i++) {
    pinMode(ledPins[i], OUTPUT);
    pinMode(btnPins[i], INPUT_PULLUP);
  }
  pinMode(buzzer,  OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  meuServoTampaA.attach(servoTampaA);
  meuServoTampaA.write(anguloTampaA(TAMPA_A_FECHADO));

  meuServoTampaB.attach(servoTampaB);
  meuServoTampaB.write(anguloTampaB(TAMPA_B_FECHADO));

  Wire.begin();
  lcd.init();
  lcd.backlight();

  randomSeed(analogRead(A0));

  telaInicial();
}

// ============================================================
//  LOOP PRINCIPAL
// ============================================================
void loop() {
  if      (faseAtual == 0) aguardarInicio();
  else if (faseAtual == 1) executarFaseMemoria();
  else if (faseAtual == 2) executarFaseDistancia();
}

// ============================================================
//  TELA INICIAL
// ============================================================
void telaInicial() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("COFRE INTELIG.  ");
  lcd.setCursor(0, 1);
  lcd.print("Aperte p/ inicio");
}

// ============================================================
//  AGUARDAR INÍCIO
// ============================================================
void aguardarInicio() {
  for (int b = 0; b < 3; b++) {
    if (digitalRead(btnPins[b]) == LOW) {
      digitalWrite(ledPins[b], HIGH);
      tone(buzzer, 1000, 150);
      delay(300);
      noTone(buzzer);
      digitalWrite(ledPins[b], LOW);
      while (digitalRead(btnPins[b]) == LOW);
      delay(100);
      faseAtual = 1;
      break;
    }
  }
}

// ============================================================
//  FASE 1 — MEMÓRIA
// ============================================================
void executarFaseMemoria() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("FASE 1: MEMORIA ");
  lcd.setCursor(0, 1);
  lcd.print("Memorize!       ");
  delay(1500);

  const int tamanhos[] = {2, 3, 4};

  for (int nivel = 0; nivel < 3; nivel++) {
    int tam = tamanhos[nivel];
    int sequencia[4];

    int anterior = -1;
    for (int i = 0; i < tam; i++) {
      int novo;
      do { novo = random(0, 3); } while (novo == anterior);
      sequencia[i] = novo;
      anterior = novo;
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Nivel ");
    lcd.print(nivel + 1);
    lcd.print(" - ");
    lcd.print(tam);
    lcd.print(" passos ");
    lcd.setCursor(0, 1);
    lcd.print("Observe...      ");
    delay(1200);

    for (int i = 0; i < tam; i++) {
      digitalWrite(ledPins[sequencia[i]], HIGH);
      tone(buzzer, 800 + sequencia[i] * 200, TEMPO_LED_ACESO_MS);
      delay(TEMPO_LED_ACESO_MS);
      noTone(buzzer);
      digitalWrite(ledPins[sequencia[i]], LOW);
      delay(TEMPO_LED_APAGADO_MS);
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sua vez!        ");
    lcd.setCursor(0, 1);
    lcd.print("0/");
    lcd.print(tam);
    lcd.print("              ");

    for (int i = 0; i < tam; i++) {
      int pressionado = -1;
      unsigned long inicio = millis();

      while (millis() - inicio < TEMPO_RESPOSTA_MS) {
        for (int b = 0; b < 3; b++) {
          if (digitalRead(btnPins[b]) == LOW) {
            pressionado = b;
            digitalWrite(ledPins[b], HIGH);
            tone(buzzer, 800 + b * 200, 150);
            delay(250);
            noTone(buzzer);
            digitalWrite(ledPins[b], LOW);
            while (digitalRead(btnPins[b]) == LOW);
            delay(100);
            break;
          }
        }
        if (pressionado != -1) break;
      }

      if (pressionado == -1 || pressionado != sequencia[i]) {
        gameOver();
        return;
      }

      lcd.setCursor(0, 1);
      lcd.print(i + 1);
      lcd.print("/");
      lcd.print(tam);
      lcd.print(" OK     ");
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Correto!        ");
    for (int c = 0; c < 2; c++) {
      for (int l = 0; l < 3; l++) digitalWrite(ledPins[l], HIGH);
      tone(buzzer, 1200, 150);
      delay(200);
      noTone(buzzer);
      for (int l = 0; l < 3; l++) digitalWrite(ledPins[l], LOW);
      delay(150);
    }
    delay(600);
  }

  faseAtual = 2;
}

// ============================================================
//  FASE 2 — SENSOR ULTRASSÔNICO
// ============================================================
void executarFaseDistancia() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("FASE 2: DIST.   ");
  lcd.setCursor(0, 1);
  lcd.print("Coloque a mao!  ");
  delay(2000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Aproxime a mao  ");
  lcd.setCursor(0, 1);
  lcd.print("entre 15-18 cm  ");

  float distRef = 999.0;
  while (distRef > 50.0) {
    distRef = lerDistancia();
    delay(100);
  }

  delay(300);
  float dist = lerDistancia();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Dist: ");
  if ((int)dist < 10) lcd.print(" ");
  lcd.print((int)dist);
  lcd.print(" cm       ");

  if (dist < DIST_MIN_CM || dist > DIST_MAX_CM) {
    lcd.setCursor(0, 1);
    lcd.print("Fora! Use 15-18 ");
    tone(buzzer, 300, 600);
    delay(700);
    noTone(buzzer);
    delay(800);
    gameOver();
    return;
  }

  lcd.setCursor(0, 1);
  lcd.print("Segure!         ");

  unsigned long inicio = millis();
  bool soltou = false;

  while (millis() - inicio < TEMPO_SEGURAR_MS) {
    float leitura = lerDistancia();

    if (leitura > 50.0 || abs(leitura - dist) > 3.0) {
      soltou = true;
      break;
    }

    int restante = (TEMPO_SEGURAR_MS / 1000) - (int)((millis() - inicio) / 1000);
    lcd.setCursor(10, 1);
    lcd.print(restante);
    lcd.print("s  ");

    delay(100);
  }

  if (soltou) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Moveu a mao!    ");
    lcd.setCursor(0, 1);
    lcd.print("GAME OVER       ");
    tone(buzzer, 200, 800);
    delay(900);
    noTone(buzzer);
    delay(600);
    gameOver();
    return;
  }

  abrirCofre();
}

// ============================================================
//  LER DISTÂNCIA — HC-SR04
// ============================================================
float lerDistancia() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long tempo = pulseIn(echoPin, HIGH, 15000);
  if (tempo == 0) return 999.0;
  return tempo * 0.034 / 2.0;
}

// ============================================================
//  GAME OVER
// ============================================================
void gameOver() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ERROU!          ");
  lcd.setCursor(0, 1);
  lcd.print("Aperte p/ jogar ");
  tone(buzzer, 200, 800);
  delay(1000);
  noTone(buzzer);
  delay(1000);
  faseAtual = 0;
}

// ============================================================
//  SOM DE CONQUISTA
//  Fase 1 (4s): bipes graves e lentos → agudos e rápidos
//  Clímax:      rajada rápida no tom máximo
//  Fase 2 (1s): bipe constante e agudo
// ============================================================
void tocarConquista() {

  unsigned long accDur = 4000;
  unsigned long inicio = millis();

  while (millis() - inicio < accDur) {
    float t    = (float)(millis() - inicio) / accDur;
    int   freq = 400 + (int)(1200.0 * t);
    int   intv = 300 + (int)(-240.0 * t);
    int   dur  = (int)(intv * 0.55);

    tone(buzzer, freq, dur);
    delay(dur);
    noTone(buzzer);
    delay(intv - dur);
  }

  int climax[] = {1600, 1800, 2000, 1800, 2000};
  for (int i = 0; i < 5; i++) {
    tone(buzzer, climax[i], 100);
    delay(120);
    noTone(buzzer);
  }

  unsigned long stDur = 1000;
  unsigned long stIni = millis();

  while (millis() - stIni < stDur) {
    tone(buzzer, 1568, 55);
    delay(70);
    noTone(buzzer);
  }
  noTone(buzzer);
}

// ============================================================
//  MOVER SERVOS COM SUAVIDADE
//  Os dois servos da tampa ativam ao mesmo tempo
// ============================================================
void moverTampa(int de_a, int ate_a, int de_b, int ate_b) {
  int passoA = (de_a < ate_a) ? 5 : -5;
  int passoB = (de_b < ate_b) ? 5 : -5;
  int posA   = de_a;
  int posB   = de_b;

  while (posA != ate_a || posB != ate_b) {
    if (posA != ate_a) { meuServoTampaA.write(anguloTampaA(posA)); posA += passoA; }
    if (posB != ate_b) { meuServoTampaB.write(anguloTampaB(posB)); posB += passoB; }
    delay(50);
  }

  meuServoTampaA.write(anguloTampaA(ate_a));
  meuServoTampaB.write(anguloTampaB(ate_b));
}

// ============================================================
//  AGUARDAR QUALQUER BOTÃO
// ============================================================
void aguardarBotao() {
  bool pressionado = false;
  while (!pressionado) {
    for (int b = 0; b < 3; b++) {
      if (digitalRead(btnPins[b]) == LOW) {
        pressionado = true;
        delay(50);
        while (digitalRead(btnPins[b]) == LOW);
        break;
      }
    }
  }
}

// ============================================================
void abrirCofre() {

  // Mensagem de acesso
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ACESSO CONCEDIDO");
  lcd.setCursor(0, 1);
  lcd.print("CONQUISTA!      ");

  // Pisca LEDs durante a conquista
  for (int l = 0; l < 3; l++) digitalWrite(ledPins[l], HIGH);
  tocarConquista();
  for (int l = 0; l < 3; l++) digitalWrite(ledPins[l], LOW);

  delay(500);

  // Abre os dois servos da tampa ao mesmo tempo
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Abrindo tampa...");
  lcd.setCursor(0, 1);
  lcd.print("                ");

  moverTampa(
    TAMPA_A_FECHADO,
    TAMPA_A_ABERTO,
    TAMPA_B_FECHADO,
    TAMPA_B_ABERTO
  );

  tone(buzzer, 1200, 200);
  delay(300);
  noTone(buzzer);
  delay(800);

  // Aguarda botão para fechar a tampa
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Cofre aberto!   ");
  lcd.setCursor(0, 1);
  lcd.print("Botao: fechar   ");

  aguardarBotao();

  // Fecha os dois servos da tampa
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Fechando tampa..");
  lcd.setCursor(0, 1);
  lcd.print("                ");

  moverTampa(
    TAMPA_A_ABERTO,
    TAMPA_A_FECHADO,
    TAMPA_B_ABERTO,
    TAMPA_B_FECHADO
  );

  tone(buzzer, 800, 150);
  delay(200);
  noTone(buzzer);
  delay(500);

  // Mensagem final
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Cofre FECHADO!  ");
  lcd.setCursor(0, 1);
  lcd.print("Ate a proxima!  ");

  tone(buzzer, 600, 150);
  delay(200);
  noTone(buzzer);
  tone(buzzer, 400, 150);
  delay(300);
  noTone(buzzer);

  delay(2000);

  // Pisca LEDs sinalizando reinício
  for (int c = 0; c < 3; c++) {
    for (int l = 0; l < 3; l++) digitalWrite(ledPins[l], HIGH);
    delay(200);

    for (int l = 0; l < 3; l++) digitalWrite(ledPins[l], LOW);
    delay(200);
  }

  telaInicial();
  faseAtual = 0;
}