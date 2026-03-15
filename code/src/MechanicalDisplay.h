#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

class MechanicalDisplay
{
private:
  Adafruit_PWMServoDriver PWM;

  int _btnUpPin;
  int _btnDownPin;

  int currentDigit = 0;
  const int frequency = 50;
  const int delayForBegin = 10;
  bool lastBtnUpState = LOW;
  bool lastBtnDownState = LOW;
  bool autoModeDown = false;
  bool autoModeUp = false;
  unsigned long lastBtnUpTime = 0;
  unsigned long lastBtnDownTime = 0;
  const unsigned long debounceDelay = 150;
  const unsigned long autoStepDelay = 1500;
  const unsigned long btnHoldThreshold = 2000;
  static const int numServos = 7;
  static const int numDigits = 10;


  unsigned long btnUpPressedTime = 0;   // коли почали тримати кнопку +
  unsigned long btnDownPressedTime = 0; // коли почали тримати кнопку -
  unsigned long lastAutoStepTime = 0;   // час останнього кроку авто режимі

  int servo_off[numServos] = {150, 150, 150, 150, 150, 150, 150};
  int servo_on[numServos] = {345, 350, 352, 335, 334, 375, 349};

  /*
  окремі індивідуальні налашування для кожного серво, враховуючи похибки
  */

  const byte digits[numDigits][numServos] = 
  {
      // масив для відображення від 0 до 9
      {1, 1, 1, 1, 1, 1, 0}, // 0
      {0, 1, 1, 0, 0, 0, 0}, // 1
      {1, 1, 0, 1, 1, 0, 1}, // 2
      {1, 1, 1, 1, 0, 0, 1}, // 3
      {0, 1, 1, 0, 0, 1, 1}, // 4
      {1, 0, 1, 1, 0, 1, 1}, // 5
      {1, 0, 1, 1, 1, 1, 1}, // 6
      {1, 1, 1, 0, 0, 0, 0}, // 7
      {1, 1, 1, 1, 1, 1, 1}, // 8
      {1, 1, 1, 1, 0, 1, 1}  // 9
  };

  enum DisplayMode
  {
    MODE_NORMAL,   // для ручного перемикання
    MODE_AUTO_UP,  // авто додавання коли затиснули клавішу +
    MODE_AUTO_DOWN // віднімання коли затиснули клавішу -
  };
  DisplayMode currentMode = MODE_NORMAL; // поточний мод

  void showDigit(int num) // метод для відображень
  {
    for (int i = 0; i < numServos; i++)
    {
      if (digits[num][i] == 1)
      {
        PWM.setPWM(i, 0, servo_on[i]);
      }
      else
      {
        PWM.setPWM(i, 0, servo_off[i]);
      }
    }
  }

  void handleButtons() // метод для обробки натискань кнопок, для визначення чи просто натискаємо чи утримуємо
  {
    // читаємо стани кнопок один раз на початку методу
    bool btnUpState = !digitalRead(_btnUpPin);
    bool btnDownState = !digitalRead(_btnDownPin);

    // для КНОПКИ +
    // Якщо кнопку зараз натиснули запам'ятовуємо час
    if (btnUpState == HIGH && lastBtnUpState == LOW)
    {
      btnUpPressedTime = millis();
      autoModeUp = false; // скидаємо прапорець авто мода якшо раніше був
    }

    // перевірка чи кнопку тримають 2 сек
    if (btnUpState == HIGH && lastBtnUpState == HIGH && !autoModeUp)
    {
      if (millis() - btnUpPressedTime >= btnHoldThreshold)
      {
        currentMode = MODE_AUTO_UP; // change mode
        currentDigit = 0;       // cкидаємо на 0 з поточного
        showDigit(currentDigit);
        lastAutoStepTime = millis();          // Запускаємо таймер для авто-кроків
        autoModeUp = true; // піднімаємо прапорець
      }
    }

    // перевірка на звичайни клік +
    if (btnUpState == HIGH && lastBtnUpState == LOW && (millis() - lastBtnUpTime > debounceDelay))
    {
      currentDigit = (currentDigit + 1) % numDigits;
      showDigit(currentDigit);
      lastBtnUpTime = millis();
    }


    // для КНОПКИ -

    // запаим'ятовуємо час натискання
    if (btnDownState == HIGH && lastBtnDownState == LOW)
    {
      btnDownPressedTime = millis();
      autoModeDown = false; // скидаємо прапорець авто мода якщо раніше був але вже для -
    }

    // знову перевірка на 2 сек
    if (btnDownState == HIGH && lastBtnDownState == HIGH && !autoModeDown)
    {
      if (millis() - btnDownPressedTime >= btnHoldThreshold)
      {
        currentMode = MODE_AUTO_DOWN; // change mode
        currentDigit = 9;             // починаємо з 9
        showDigit(currentDigit);
        lastAutoStepTime = millis();
        autoModeDown = true; // те саме що і для +
      }
    }

    // Звичайний клік -
    if (btnDownState == HIGH && lastBtnDownState == LOW && (millis() - lastBtnDownTime > debounceDelay))
    {
      currentDigit = (currentDigit + 9) % numDigits;
      showDigit(currentDigit);
      lastBtnDownTime = millis();
    }

    // зберігаємо стани для некст кола
    lastBtnUpState = btnUpState;
    lastBtnDownState = btnDownState;
  }

  void handleAutoMode() // метод для обробки авто режиму
  {
    // якщо ми в режимі авто додавання
    if (currentMode == MODE_AUTO_UP)
    {
      if (millis() - lastAutoStepTime >= autoStepDelay)
      { // прошло 1.5 сек
        currentDigit++;
        showDigit(currentDigit);
        lastAutoStepTime = millis();

        // Якщо дійшли до 9 виходимо
        if (currentDigit == 9)
        {
          currentMode = MODE_NORMAL;
        }
      }
    }

    if (currentMode == MODE_AUTO_DOWN)
    {
      if (millis() - lastAutoStepTime >= autoStepDelay)
      { 
        currentDigit--;
        showDigit(currentDigit);
        lastAutoStepTime = millis();

        // Якщо дійшли до 0 виходимо
        if (currentDigit == 0)
        {
          currentMode = MODE_NORMAL;
        }
      }
    }
  }

public:
  MechanicalDisplay(int btnUpPin, int btnDownPin) // конструктор для пінів
  {
    _btnUpPin = btnUpPin;
    _btnDownPin = btnDownPin;
    PWM = Adafruit_PWMServoDriver(); // драйвер серво для дефолтного адреса 0x40
  }

  void begin()
  {
    pinMode(_btnUpPin, INPUT);
    pinMode(_btnDownPin, INPUT);
    PWM.begin();
    PWM.setPWMFreq(frequency);
    delay(delayForBegin);
    showDigit(currentDigit);
  }

  void updateDigit() // метод для оновлення станів
  {
    if (currentMode == MODE_NORMAL)
    {
      handleButtons();
    }
    else
    {
      handleAutoMode();
    }
  }
};