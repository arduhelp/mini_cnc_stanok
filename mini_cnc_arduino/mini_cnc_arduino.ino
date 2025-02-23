const int motorXForward = 9;
const int motorXBackward = 10;
const int motorYForward = 11;
const int motorYBackward = 12;
const int joystickXPin = A1;
const int joystickYPin = A2;
const int stopButtonPin = 8;
const int sensorPinX = 7; // Датчик на піні X7
const int sensorPinY = 6; // Датчик на піні Y6

bool globalStop = false;
int currentX = 0;
int currentY = 0;
bool joystickMode = true; // Режим джойстика за замовчуванням
const int maxTableSize = 170; // Максимальний розмір столу

// Масив з G-кодами
String gCodeArray[] = {
  "G1 X170 Y41",
  "G1 X170 Y105",
  "G1 X137 Y105",
  "G1 X126 Y105",
  "G1 X126 Y93",
  "G1 X82 Y93",
  "G1 X82 Y36",
  "G1 X126 Y36",
  "G1 X126 Y92",
  "G1 X82 Y92",
  "G1 X68 Y92",
  "G1 X68 Y37",
  "G1 X68 Y108",
  "G1 X30 Y108",
};

const int gCodeCount = sizeof(gCodeArray) / sizeof(gCodeArray[0]);
int gCodeIndex = 0;

void stopMotors() {
  digitalWrite(motorXForward, LOW);
  digitalWrite(motorXBackward, LOW);
  digitalWrite(motorYForward, LOW);
  digitalWrite(motorYBackward, LOW);
}

void setup() {
  Serial.begin(9600);
  pinMode(motorXForward, OUTPUT);
  pinMode(motorXBackward, OUTPUT);
  pinMode(motorYForward, OUTPUT);
  pinMode(motorYBackward, OUTPUT);
  pinMode(stopButtonPin, INPUT_PULLUP);
  pinMode(joystickXPin, INPUT);
  pinMode(joystickYPin, INPUT);
  pinMode(sensorPinX, INPUT);
  pinMode(sensorPinY, INPUT);
  
}

void moveMotors(int targetX, int targetY) {
  bool moving = false;
  // Рух по осі X
  if (currentX < targetX) {
    digitalWrite(motorXForward, HIGH);
    digitalWrite(motorXBackward, LOW);
    moving = true;
  } else if (currentX > targetX) {
    digitalWrite(motorXForward, LOW);
    digitalWrite(motorXBackward, HIGH);
    moving = true;
  }

  // Рух по осі Y
  if (currentY < targetY) {
    digitalWrite(motorYForward, HIGH);
    digitalWrite(motorYBackward, LOW);
    moving = true;
  } else if (currentY > targetY) {
    digitalWrite(motorYForward, LOW);
    digitalWrite(motorYBackward, HIGH);
    moving = true;
  }

  // Очікування, поки рух відбувається
  while (moving) {
    delay(10); // Затримка, щоб не перевантажувати процесор

  Serial.print("X:"); Serial.print(currentX);
  Serial.print(" Y:"); Serial.print(currentY);
  Serial.print(" toX: "); Serial.print(targetX);
        Serial.print(" toY: "); Serial.print(targetY);
        Serial.print(" SensorX:"); Serial.print(digitalRead(sensorPinX)); // Додано для датчика X
        Serial.print(" SensorY:"); Serial.print(digitalRead(sensorPinY)); // Додано для датчика Y
        Serial.println();

    // Оновлюємо положення (потрібно реалізувати фізичне зчитування координат)
    // Наприклад, зчитування з енкодерів або датчиків

    // Для демонстрації просто умовно оновимо координати
    if (currentX < targetX) if(digitalRead(sensorPinX)==1) currentX++;
    if (currentX > targetX) if(digitalRead(sensorPinX)==1) currentX--;

    if (currentY < targetY) if(digitalRead(sensorPinY)==1) currentY++;
    if (currentY > targetY) if(digitalRead(sensorPinY)==1) currentY--;

    // Якщо досягли цільових координат, зупиняємо мотори
    if (currentX == targetX && currentY == targetY) {
      stopMotors();
      moving = false; // Завершити рух
      Serial.print("Reached target X: "); Serial.print(currentX);
      Serial.print(" Y: "); Serial.println(currentY);
    }
  }
}

void processGCode(String command) {
  // Приклад обробки команди G-коду
  char commandType = command.charAt(0);
  int targetX = 0;
  int targetY = 0;

  // Витягнення координат з команди
  if (commandType == 'G') {
    int spaceIndex = command.indexOf(' ');
    if (spaceIndex != -1) {
      String coords = command.substring(spaceIndex + 1);
      sscanf(coords.c_str(), "X%d Y%d", &targetX, &targetY);

      // Перевірка на перевищення максимальної координати
      if (targetX > maxTableSize || targetY > maxTableSize) {
        Serial.println("Error: E max coordin is 170"); // Помилка, якщо координати перевищують межі
      } else {
        // Оновлення координат
        Serial.print("Moving to X: "); Serial.print(targetX);
        Serial.print(" Y: "); Serial.println(targetY);
        moveMotors(targetX, targetY); // Виконати рух до цільових координат
      }
    }
  }
}

void executeNextGCode() {
  if (gCodeIndex < gCodeCount) {
    String gCode = gCodeArray[gCodeIndex];
    processGCode(gCode);
    gCodeIndex++;
  } else {
    Serial.println("All G-codes executed.");
  }
}

void dbg() {
  Serial.print("X:"); Serial.print(currentX);
  Serial.print(" Y:"); Serial.print(currentY);
  Serial.print(" JoyX:"); Serial.print(analogRead(joystickXPin));
  Serial.print(" JoyY:"); Serial.print(analogRead(joystickYPin));
  Serial.print(" Stop:"); Serial.print(digitalRead(stopButtonPin) == LOW ? "1" : "0");
  Serial.print(" SensorX:"); Serial.print(digitalRead(sensorPinX)); // Додано для датчика X
  Serial.print(" SensorY:"); Serial.print(digitalRead(sensorPinY)); // Додано для датчика Y
  Serial.println();
}

void loop() {
  if (digitalRead(stopButtonPin) == LOW) {
    globalStop = true;
    stopMotors();
    Serial.println("Emergency STOP!");
  }

  if (!globalStop) {
    if (Serial.available()) {
      String input = Serial.readStringUntil('\n');
      input.trim();
      
      if (input == "1") {
        joystickMode = true; // Перейти в режим джойстика
        Serial.println("Switched to Joystick Mode.");
      } else if (input == "2") {
        joystickMode = false; // Перейти в режим авто G-коду
        Serial.println("Switched to Auto G-Code Mode.");
      } else if (!joystickMode) {
        processGCode(input); // Обробка G-коду
      }
    }

    if (joystickMode) {
      int joyX = analogRead(joystickXPin);
      int joyY = analogRead(joystickYPin);
      
      // Обробка координат на основі джойстика і датчика
      if (digitalRead(sensorPinX) == HIGH) { // Якщо датчик сигналізує про мікнення
        if (joyX < 400) { // Джойстик назад
          currentX -= 1; // Зменшити координату X на 1
        } else if (joyX > 600) { // Джойстик вперед
          currentX += 1; // Збільшити координату X на 1
        }
      }
      // Обробка координат на основі джойстика і датчика
      if (digitalRead(sensorPinY) == HIGH) { // Якщо датчик сигналізує про мікнення
        if (joyY < 400) { // Джойстик назад
          currentY -= 1; // Зменшити координату Y на 1
        } else if (joyY > 600) { // Джойстик вперед
          currentY += 1; // Збільшити координату Y на 1
        }
      }

      // Рух моторів на основі положення джойстика
      if (joyX < 400) {
        digitalWrite(motorXForward, LOW);
        digitalWrite(motorXBackward, HIGH);
      } else if (joyX > 600) {
        digitalWrite(motorXForward, HIGH);
        digitalWrite(motorXBackward, LOW);
      } else {
        digitalWrite(motorXForward, LOW);
        digitalWrite(motorXBackward, LOW);
      }

      if (joyY < 400) {
        digitalWrite(motorYForward, LOW);
        digitalWrite(motorYBackward, HIGH);
      } else if (joyY > 600) {
        digitalWrite(motorYForward, HIGH);
        digitalWrite(motorYBackward, LOW);
      } else {
        digitalWrite(motorYForward, LOW);
        digitalWrite(motorYBackward, LOW);
      }
    }

    // Виконання наступної команди G-коду в режимі авто
    if (!joystickMode) {
      executeNextGCode();
    }

    // Виклик функції для дебагу
    dbg();
  }
}
