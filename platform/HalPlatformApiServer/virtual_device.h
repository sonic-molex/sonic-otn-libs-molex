#pragma once
#include <unordered_map>
#include <string>




class virtual_device_base {
public:
    virtual_device_base(std::string name) : dev_name(std::move(name)) {}
    virtual ~virtual_device_base() = default;

    const std::string& name() const { return dev_name; }
    virtual void update_data(size_t index) {}

    enum class Led_Status {
        OFF,
        RED,
        GREEN,
        AMBER,
        RED_BLINK = 5,
        GREEN_BLINK,
        AMBER_BLINK
    };

    enum class Slot_State
    {
        OFFLINE = 0,
        ONLINE,
        MISMATCH
    };
    
protected:
    std::string dev_name;
};


class virtual_chassis_device : public virtual_device_base {
public:
    explicit virtual_chassis_device(const std::string& name)
        : virtual_device_base(name),
          list("null"),
          chassis_name("ot-kvm-chassis"),
          model("KVM-1234"),
          serial("123456789"),
          revision("1.0"),
          base_mac("00:1A:2B:3C:4D:5E"),
          slot(0) {}

    virtual ~virtual_chassis_device() = default;

    // --- Getters ---
    const std::string& get_list() const { return list; }
    const std::string& get_chassis_name() const { return chassis_name; }
    const std::string& get_model() const { return model; }
    const std::string& get_serial() const { return serial; }
    const std::string& get_revision() const { return revision; }
    const std::string& get_base_mac() const { return base_mac; }
    int get_slot() const { return slot; }

    // --- Setters (optional, for simulation updates) ---
    void set_list(const std::string& l) { list = l; }
    void set_chassis_name(const std::string& n) { chassis_name = n; }
    void set_model(const std::string& m) { model = m; }
    void set_serial(const std::string& s) { serial = s; }
    void set_revision(const std::string& r) { revision = r; }
    void set_base_mac(const std::string& mac) { base_mac = mac; }
    void set_slot(int s) { slot = s; }

private:
    std::string list;
    std::string chassis_name;
    std::string model;
    std::string serial;
    std::string revision;
    std::string base_mac;
    int slot;
};

class virtual_component_device : public virtual_device_base {
public:
    explicit virtual_component_device(const std::string& name)
        : virtual_device_base(name), revision("1.0") {}
    virtual ~virtual_component_device() = default;

    const std::string& get_revision() const { return revision; }
    void set_revision(const std::string& r) { revision = r; }

private:
    std::string revision;
};

class virtual_fan_device : public virtual_device_base {
public:
    explicit virtual_fan_device(const std::string& name)
        : virtual_device_base(name), 
        speed_rpm(8000), 
        led_state(static_cast<int>(Led_Status::RED)), 
        present(true), 
        replaceable(true),
        model("Fan Model"),
        serial("123456789"),
        revision("1.0") {}

    virtual ~virtual_fan_device() = default;

    enum class FAN_DIRECTION
    {
        INTAKE = 0,
        EXHAUST
    };

    void set_speed(int rpm) { speed_rpm = rpm; }
    int get_speed() const { return speed_rpm; }
    int get_actual_speed() const { return actual_speed_percentage; }
    const std::string& get_model() const { return model; }
    const std::string& get_serial() const { return serial; }
    const std::string& get_revision() const { return revision; }

    void set_model(const std::string& m) { model = m; }
    const int get_led_state() const { return led_state; }
    void set_led_state(int state) { led_state = state; }

    bool is_present() const { return present; }
    bool is_replaceable() const { return replaceable; }
    void update_data(size_t fan_num) {
        speed_rpm = speed_rpm + fan_num;
        model = model + std::to_string(fan_num);
        serial = serial + std::to_string(fan_num);
        led_state = fan_num % 2 == 0 ? static_cast<int>(Led_Status::GREEN) : static_cast<int>(Led_Status::RED);
        actual_speed_percentage = 100 - (5 * fan_num);
        actual_speed_percentage = std::max(0, std::min(100, actual_speed_percentage));
    }

private:
    int speed_rpm;
    int actual_speed_percentage;
    int led_state;
    bool present;
    bool replaceable;
    std::string model;
    std::string serial;
    std::string revision;
};



class virtual_psu_device : public virtual_device_base {
public:
    explicit virtual_psu_device(const std::string& name)
        : virtual_device_base(name),
          voltage(12.0), 
          current(1.5), 
          power(18.0),
          temperature(40.0), 
          present(true), 
          status(true), 
          model("PSU Model"), 
          serial("123456789"), 
          revision("1.0"),
          led_state(static_cast<int>(Led_Status::GREEN)) {}

    virtual ~virtual_psu_device() = default;

    double get_voltage() const { return voltage; }
    double get_current() const { return current; }
    double get_power() const { return power; }
    double get_temp() const { return temperature; }
    const std::string& get_model() const { return model; }
    const std::string& get_serial() const { return serial; }
    const std::string& get_revision() const { return revision; }
    bool is_present() const { return present; }
    bool get_status() const { return status; }

    const int get_led_state() const { return led_state; }
    void set_led_state(int state) { led_state = state; }
    void set_model(const std::string& s) { model = s; }
    
    void update_data(size_t psu_num) {
        voltage = voltage + psu_num * 0.04;
        current = current + psu_num * 0.1;
        power = power + psu_num * 0.5;
        temperature = temperature + psu_num * 0.2;
        model = model + std::to_string(psu_num);
        serial = serial + std::to_string(psu_num);
        led_state = static_cast<int>(Led_Status::GREEN);
    }

private:
    double voltage;
    double current;
    double power;
    double temperature;
    bool present;
    bool status;
    std::string model;
    std::string serial;
    std::string revision;
    int led_state;
};


class virtual_thermal_device : public virtual_device_base {
public:
    explicit virtual_thermal_device(const std::string& name)
        : virtual_device_base(name),
          temperature(35.0), 
          high_threshold(70.0), 
          low_threshold(10.0),
          high_critical(90.0), 
          low_critical(5.0) {}
    virtual ~virtual_thermal_device() = default;

    double get_temp() const { return temperature; }
    double get_high_threshold() const { return high_threshold; }
    double get_low_threshold() const { return low_threshold; }
    double get_high_critical() const { return high_critical; }
    double get_low_critical() const { return low_critical; }
    void set_temp(double value) { temperature = value; }
    void set_high_threshold(double value) { high_threshold = value; }
    void set_low_threshold(double value) { low_threshold = value; }
    void set_high_critical(double value) { high_critical = value; }
    void set_low_critical(double value) { low_critical = value; }
    
    void update_data(size_t thermal_num) {
        temperature = temperature + thermal_num;
        high_threshold = high_threshold + thermal_num * 0.5;
        low_threshold = low_threshold + thermal_num * 0.2;
        high_critical = high_critical + thermal_num * 0.8;
        low_critical = low_critical + thermal_num * 0.1;
    }

private:
    double temperature;
    double high_threshold;
    double low_threshold;
    double high_critical;
    double low_critical;
};

class virtual_led_device : public virtual_device_base {
public:
    explicit virtual_led_device(const std::string& name)
        : virtual_device_base(name), state("off") {}
    virtual ~virtual_led_device() = default;

    const std::string& get_state() const { return state; }
    void set_state(const std::string& s) { state = s; }
    


private:
    std::string state;
};


class virtual_module_device : public virtual_device_base {
public:
    explicit virtual_module_device(const std::string& name)
        : virtual_device_base(name),
          temp(42.0), 
          led_state(static_cast<int>(Led_Status::GREEN)), 
          present(true),
          model("OTN Module"),
          serial("123456789"),
          revision("1.0"),
          base_mac("00:1A:2B:3C:4D:5E"),
          slot(0),
          state(Slot_State::ONLINE),
          type(Module_Type::MODULE_TYPE_FABRIC) {
        const auto slot_pos = name.find_last_not_of("0123456789");
        if (slot_pos + 1 < name.size())
            slot = std::stoi(name.substr(slot_pos + 1));

        if (name.compare(0, 10, "SUPERVISOR") == 0)
            type = Module_Type::MODULE_TYPE_SUPERVISOR;
        else if (name.compare(0, 9, "LINE-CARD") == 0)
            type = Module_Type::MODULE_TYPE_LINECARD;
    }
    virtual ~virtual_module_device() = default;

    enum class Module_Type
    {
        MODULE_TYPE_SUPERVISOR = 0,
        MODULE_TYPE_LINECARD,
        MODULE_TYPE_FABRIC,

        MODULE_TYPE_NONE
    };

    // --- Getters ---
    const std::string& get_model() const { return model; }
    const std::string& get_serial() const { return serial; }
    const std::string& get_revision() const { return revision; }
    const std::string& get_base_mac() const { return base_mac; }
    int get_slot() const { return slot; }
    double get_temp() const { return temp; }
    int get_led_state() const { return led_state; }
    bool is_present() const { return present; }
    Slot_State get_state() const { return state; }
    Module_Type get_type() const { return type; }

    // --- Setters (optional, for simulation updates) ---
    void set_model(const std::string& m) { model = m; }
    void set_serial(const std::string& s) { serial = s; }
    void set_revision(const std::string& r) { revision = r; }
    void set_base_mac(const std::string& mac) { base_mac = mac; }
    void set_slot(int s) { slot = s; }
    void set_temp(double t) { temp = t; }
    void set_led_state(int ls) { led_state = ls; }
    void set_present(bool p) { present = p; }
    void set_state(Slot_State st) { state = st; }
    void set_type(Module_Type t) { type = t; }

    void update_data(size_t module_num) {
        temp = temp + module_num;
        led_state = module_num % 2 == 0 ? static_cast<int>(Led_Status::GREEN) : static_cast<int>(Led_Status::RED);
        model = model + std::to_string(module_num);
        serial = serial + std::to_string(module_num);
        // just to change last digit of base_mac
        base_mac[base_mac.length() - 1] = std::to_string(module_num)[0];
    }

private:
    double temp;
    int led_state;
    bool present;
    std::string model;
    std::string serial;
    std::string revision;
    std::string base_mac;
    int slot;
    Slot_State state;
    Module_Type type;
};

class virtual_fan_drawer_device : public virtual_device_base {
public:
    explicit virtual_fan_drawer_device(const std::string& name)
        : virtual_device_base(name), led_state(static_cast<int>(Led_Status::GREEN)), present(true) {}
    virtual ~virtual_fan_drawer_device() = default;

    int get_led_state() const { return led_state; }
    void set_led_state(int s) { led_state = s; }

    bool is_present() const { return present; }

private:
    int led_state;
    bool present;
};

class virtual_watchdog_device : public virtual_device_base {
public:
    explicit virtual_watchdog_device(const std::string& name)
        : virtual_device_base(name), armed(false), timeout_s(60) {}
    virtual ~virtual_watchdog_device() = default;

    bool arm(int seconds) { armed = true; timeout_s = seconds; return true; }
    bool disarm() { armed = false; return true; }
    bool is_armed() const { return armed; }

private:
    bool armed;
    int timeout_s;
};
