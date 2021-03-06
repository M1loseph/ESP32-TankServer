#include "arm_controller.hpp"
#include "debug.hpp"

#if ARM_DEBUG

#define LOG_ARM(message) LOG(message)
#define LOG_ARM_NL(message) LOG_NL(message)
#define LOG_ARM_F(...) LOG_F(__VA_ARGS__)

#else

#define LOG_ARM(message)
#define LOG_ARM_NL(message)
#define LOG_ARM_F(...)

#endif // ARM_DEBUG

namespace json_parser
{
    arm_controller::arm_controller() : templated_controller("arm", JSON_ARRAY_SIZE(SERVOS) + JSON_OBJECT_SIZE(SERVOS * 4) + JSON_OBJECT_SIZE(2))
    {
    }

    arm_controller::servo_data *arm_controller::get_servo_by_name(const char *servo_name)
    {
        if (servo_name)
        {
            for (uint8_t i = 0; i < SERVOS; i++)
            {
                if (!strcmp(arm[i].NAME, servo_name))
                {
                    return arm + i;
                }
            }
        }
        return nullptr;
    }

    void arm_controller::send_angle(uint8_t index)
    {
        if (index < SERVOS)
        {
            LOG_ARM_F("[%s] sending angle %d to servo %s\n", _name, arm[index].current_angle, arm[index].NAME);

            uint16_t pulse = static_cast<uint16_t>(map(arm[index].current_angle, 0, 180, PULSE_MS_MIN, PULSE_MS_MAX));
            constexpr double pulse_length = 1000000.0 / (PULSES_FREQUENCY * 4096);
            pulse /= pulse_length;
            _pwm.setPWM(arm[index].extern_module_pin, 0, pulse);
        }
    }

    arm_controller::servo_data *arm_controller::get_servo_ptr(const JsonObject *json)
    {
        if (json)
        {
            if (json->containsKey(NAME_KEY))
            {
                const char *servo_name = (*json)[NAME_KEY];
                auto servo = get_servo_by_name(servo_name);
                if (!servo)
                {
                    LOG_ARM_F("[%s] wrong servo name\n", _name)
                }
                return servo;
            }
            else
            {
                LOG_ARM_F("[%s] no servo name field\n", _name)
            }
        }
        else
        {
            LOG_ARM_F("[%s] JSON object was null\n", _name)
        }
        return nullptr;
    }

    bool arm_controller::initialize()
    {
        if(!Wire.begin())   
            return false;
        _pwm.begin();
        _pwm.setPWMFreq(PULSES_FREQUENCY); 
        for (uint8_t i = 0; i < SERVOS; i++)
            send_angle(i);

        bool if_added = true;

        if_added &= add_event(SERVO_MINUS, &arm_controller::servo_minus);
        if_added &= add_event(SERVO_PLUS, &arm_controller::servo_plus);
        if_added &= add_event(SERVO_STOP, &arm_controller::servo_stop);
        if_added &= add_event(SERVO_ANGLE, &arm_controller::servo_angle);
        
        return if_added;
    }

    bool arm_controller::servo_minus(const JsonObject *json)
    {
        servo_data *servo = get_servo_ptr(json);
        if (servo)
        {
            servo->destination_angle = servo->MIN_ANGLE;
            LOG_ARM_F("[%s] servo %s moving to %d\n", _name, servo->NAME, servo->destination_angle)
            return true;
        }
        return false;
    }

    bool arm_controller::servo_plus(const JsonObject *json)
    {
        servo_data *servo = get_servo_ptr(json);
        if (servo)
        {
            servo->destination_angle = servo->MAX_ANGLE;
            LOG_ARM_F("[%s] servo %s moving to %d\n", _name, servo->NAME, servo->destination_angle)
            return true;
        }
        return false;
    }

    bool arm_controller::servo_stop(const JsonObject *json)
    {
        servo_data *servo = get_servo_ptr(json);
        if (servo)
        {
            servo->destination_angle = servo->current_angle;
            LOG_ARM_F("[%s] servo %s stopping at angle %d\n", _name, servo->NAME, servo->current_angle)
            return true;
        }
        return false;
    }

    bool arm_controller::servo_angle(const JsonObject *json)
    {
        servo_data *servo = get_servo_ptr(json);
        if (servo)
        {
            if (json->containsKey(ANGLE_KEY))
            {
                uint8_t new_angle = (*json)[ANGLE_KEY];
                if (new_angle >= servo->MIN_ANGLE && new_angle <= servo->MAX_ANGLE)
                {
                    servo->destination_angle = new_angle;
                    LOG_ARM_F("[%s] servo %s moving to %d\n", _name, servo->NAME, servo->destination_angle)
                    return true;
                }
                else
                {
                    LOG_ARM_F("[%s] angle %d out of range for servo %s", _name, new_angle, servo->NAME)
                }
            }
            else
            {
                LOG_ARM_F("[%s] no angle field\n", _name)
            }
        }
        return false;
    }

    void arm_controller::update()
    {
        static unsigned long timer = millis();
        if (millis() - timer > SERVO_TIMEOUT)
        {
            for (uint8_t i = 0; i < SERVOS; i++)
            {
                auto &servo = arm[i];
                bool send_changes = false;
                if (servo.destination_angle > servo.current_angle && servo.current_angle < servo.MAX_ANGLE)
                {
                    servo.current_angle++;
                    send_changes = true;
                }
                else if (servo.destination_angle < servo.current_angle && servo.current_angle > servo.MIN_ANGLE)
                {
                    servo.current_angle--;
                    send_changes = true;
                }
                if (send_changes)
                    send_angle(i);
            }
            timer = millis();
        }
    }


    DynamicJsonDocument arm_controller::retrive_data()
    {
        DynamicJsonDocument json(_json_size);
        json[NAME_FIELD] = _name;
        JsonArray data = json.createNestedArray(DATA_FIELD);

        for(uint8_t i = 0; i < SERVOS; i++)
        {
            JsonObject servo = data.createNestedObject();
            servo["servo"] = arm[i].NAME;
            servo["min"] = arm[i].MIN_ANGLE;
            servo["max"] = arm[i].MAX_ANGLE;
            servo["angle"] = arm[i].current_angle;
        }
        return json;
    }
} // namespace json_parser