class LedNotification{
    private:
      int led_pin;
      bool blinking = false;
    public:
      int ledState = LOW;
      LedNotification(short led_pin)
      : led_pin(led_pin)
      {
        pinMode(led_pin, OUTPUT);
      }

      void startBlinking(){
        blinking = true;
        while (blinking)
        {
          ledState = !ledState;
          sleep(1000);
          digitalWrite(led_pin, ledState);
        }
      }

      void stopBlinking(){
        blinking = false;
      }
};