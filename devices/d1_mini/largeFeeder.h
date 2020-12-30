#include "esphome.h"
#include "AccelStepper.h"

#define STEP_PIN 5
#define DIRECTION_PIN 4
#define ENABLE_PIN 12

class LargeFeederSwitch : public Component, public Switch
{
    public:
        AccelStepper *stepper;

        bool isMoving = false;

        void setup() override {
            ESP_LOGD("feeder", "In Custom Setup");

            stepper = new AccelStepper(AccelStepper::DRIVER, STEP_PIN, DIRECTION_PIN);

            ESP_LOGD("feeder", "Created Object");

            stepper->setMaxSpeed(1000);
            stepper->setAcceleration(1000);
            stepper->setPinsInverted(false, false, true);
            stepper->setEnablePin(ENABLE_PIN);
            stepper->disableOutputs();

            ESP_LOGD("feeder", "Done!");
        }

        void write_state(bool state) override
        {
            if (!state)
            {
                return;
            }
            ESP_LOGD("custom", "Rotating Feeding Motor");

            stepper->move(1600);
            publish_state(state);
            isMoving = true;
            stepper->enableOutputs();

        }

        void loop() override {
            
            if(isMoving) {
                stepper->run();
            }

            if(isMoving && stepper->distanceToGo() == 0) {
                isMoving = false;
                publish_state(false);
                stepper->disableOutputs();
            }
        }
};
