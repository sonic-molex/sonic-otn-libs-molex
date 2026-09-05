#include "ApiHelper.h"
#include "Format.h"
#include "Convert.h"
#include "virtual_platform.h"


template<typename T>
T
__GetMapValue(const string &name, std::map<string, T> &dicts, int32_t def)
{
    T tar;

    if (dicts.find(name) == dicts.end()) {
        tar = def;

        /* Save the initial target speed to cache. */
        dicts[name] = def;
    } else {
        tar = dicts[name];
    }

    return tar;
}


int32_t
ApiHelper::GetDevType(const string &name)
{
    for(mapsi::const_iterator it = m_devs.begin(); it != m_devs.end(); ++it) {
        if (it->first == name) {
            return it->second;
        }
    }

    return DEV_TYPE_NONE;
}

int32_t
ApiHelper::RecordDevIndex(const string &name)
{
    int32_t index = 0;
    if (m_idxs.find(name) == m_idxs.end()) {
        m_idxs[name] = index;
        return index;
    }
    index = m_idxs[name];
    m_idxs[name] = ++index;
    return index;
}

void
ApiHelper::GetThermalNames(void)
{
    m_thermals.clear();

    /* Chassis */
    size_t index = 0;
/*     for (; index < CRegisterFile::ms_stBoardRegInfo.staTempInfo.size(); index++) {
        switch (index)
        {
        case 0:
            m_thermals[index] = "System Exhaust";
            break;
        case 1:
            m_thermals[index] = "System Board";
            break;
        case 2:
            m_thermals[index] = "System Intake";
            break;
        default:
            m_thermals[index] = util::Format("Board Chip{0}", index - 3);
            break;
        }
    } */

    /* CPU */
    char *line = NULL;
    size_t len = 0;
    ssize_t read = 0;
    FILE *fp = popen(DEF_CPU_CMD_TEMP, "r");
    if (NULL == fp) {
        return;
    }
    while ((read = getline(&line, &len, fp)) != -1) {
        /* Get the name ended by character : */
        *strchr(line, ':') = 0;
        string strName("CPU ");
        strName += line;
        m_thermals[index++] = strName;
    }

    if (line) {
        free(line);
    }
    pclose(fp);
}

string
ApiHelper::GetChassisName(void)
{
    if (auto* chassis = ensure_virtual_device<virtual_chassis_device>(mgr, "Chassis"))
        return chassis->get_chassis_name();

    return DEF_COMPONENT_NAME_NA;
}

int32_t
ApiHelper::GetChassisSlot(void)
{
    if (auto* chassis = ensure_virtual_device<virtual_chassis_device>(mgr, "Chassis"))
        return chassis->get_slot();

    return 0;
}

bool
ApiHelper::IsPsuFan(const string &name)
{
    return name.find(DEF_PSU_NAME) != string::npos;
}

int32_t
ApiHelper::GetDevIndex(const string &name)
{
    /* Chassis */
    if (name == DEF_MODULE_NAME_SUPERVISOR) {
        return GetChassisSlot();
    }

    /* Thermal */
    for (size_t i = 0; i < m_thermals.size(); i++) {
        if (name == m_thermals[i]) {
            return i;
        }
    }

    int32_t iIndex1 = 0, iIndex2 = 0;
    int32_t count = sscanf(name.c_str(), "%*[A-Za-z^-]%d%*[A-Za-z^-]%d", &iIndex1, &iIndex2);

    return 2 == count ? iIndex2 : iIndex1;
}

int32_t
ApiHelper::GetModuleType(const string &name)
{
    if (name == DEF_MODULE_NAME_SUPERVISOR) {
        return MODULE_TYPE::MODULE_TYPE_SUPERVISOR;
    }

    if (name.find(DEF_MODULE_NAME_LINECARD) != string::npos) {
        return MODULE_TYPE::MODULE_TYPE_LINECARD;
    }

    return MODULE_TYPE::MODULE_TYPE_FABRIC;
}

pairii
ApiHelper::GetModuleInfo(const string &name)
{
    int32_t iDevType = GetDevType(name);
    if (DEV_TYPE_NONE == iDevType) {
        return pairii(iDevType, GetDevIndex(name));
    }
    return pairii(iDevType, 0);
}

double
ApiHelper::GetThermalTemp(const string &name)
{
    double temp = 0.0;
    //int32_t index = GetDevIndex(name);

    /* Chassis */
/*     int32_t iDevNum = (int32_t)CRegisterFile::ms_stBoardRegInfo.staTempInfo.size();
    if (index < iDevNum) {
        CBoardStatusData data = {0};
        if (OPLK_OK == BOARD_GetStatusData(&data)) {
            temp = TempConvert::IntToFloat(data.aiTemp[index]);
        }
        return temp;
    } */

    /* CPU */
    const char *title = name.data() + strlen(DEF_CPU_NAME) + 1;
    char *line = NULL;
    size_t len = 0;
    ssize_t read = 0;
    FILE *fp = popen(DEF_CPU_CMD_TEMP, "r");
    if (NULL == fp) {
        return temp;
    }
    while ((read = getline(&line, &len, fp)) != -1) {
        if (strstr(line, title)) {
            char *start = line + strlen(title) + 1;
            char *end = strchr(start, 'C');
            if (end) {
                *end = 0;
                temp = (int32_t)(atof(start) * DEF_CPU_TEMP_WEIGHT);
            } else 
            {
                temp = 0;
            }
            break;
        }
    }

    if (line) {
        free(line);
    }
    pclose(fp);
    return TempConvert::IntToFloat(temp);
}

const json &
ApiHelper::GetDevicesList(void)
{
    if (!m_json.empty()) {
        return m_json;
    }


    m_json.clear();
    //m_json["chassis"] = jc;

    return m_json;
}

string
ApiHelper::GetLedState(int32_t index)
{
    string state(DEF_COMPONENT_NAME_NA);
    static int32_t sim_index;
    state = LedConvert::StateToColor(sim_index);
    sim_index = (sim_index + 1) % 7;
    return state;
}

bool
ApiHelper::SetLedState(int32_t index, const string &state)
{
    //return OPLK_OK == BOARD_SetLedState(index, LedConvert::ColorToState(state));
    return false;
}

/* thrift API implements */
void
ApiHelper::PsuGetModel(std::string& _return, const std::string& name)
{
    if (auto* psu = ensure_virtual_device<virtual_psu_device>(mgr, name))
        _return =  psu->get_model();
    else
        _return = DEF_COMPONENT_NAME_NA;
  
}

void
ApiHelper::PsuGetSerial(std::string& _return, const std::string& name)
{
    if (auto* psu = ensure_virtual_device<virtual_psu_device>(mgr, name))
        _return =  psu->get_serial();
    else
        _return = DEF_COMPONENT_NAME_NA;

}

void
ApiHelper::PsuGetRevision(std::string& _return, const std::string& name)
{  
    if (auto* psu = ensure_virtual_device<virtual_psu_device>(mgr, name))
        _return =  psu->get_revision();
    else
        _return = DEF_COMPONENT_NAME_NA;
    
    return;
}

bool
ApiHelper::PsuGetPresence(const std::string& name)
{
    if (auto* psu = ensure_virtual_device<virtual_psu_device>(mgr, name))
        return psu->is_present();
    
    return false;
    
}

bool
ApiHelper::PsuGetStatus(const std::string& name)
{
    if (auto* psu = ensure_virtual_device<virtual_psu_device>(mgr, name))
        return psu->get_status();
    
    return false;
}

bool
ApiHelper::PsuIsReplaceable(const std::string& name)
{
    return true;
}

double
ApiHelper::PsuGetVoltage(const std::string& name)
{
    if (auto* psu = ensure_virtual_device<virtual_psu_device>(mgr, name))
        return psu->is_present() ? psu->get_voltage() : 0.0;
    
    return 0.0;
}

double
ApiHelper::PsuGetCurrent(const std::string& name)
{
    if (auto* psu = ensure_virtual_device<virtual_psu_device>(mgr, name))
        return psu->is_present() ? psu->get_current() : 0.0;
    
    return 0.0;
}


double
ApiHelper::PsuGetPower(const std::string& name)
{ 
    if (auto* psu = ensure_virtual_device<virtual_psu_device>(mgr, name))
        return psu->is_present() ? psu->get_power() : 0.0;
    
    return 0.0;

}

double
ApiHelper::PsuGetTemp(const std::string& name)
{   
    if (auto* psu = ensure_virtual_device<virtual_psu_device>(mgr, name))
        return psu->is_present() ? psu->get_temp() : 0.0;
    
    return 0.0;

}

double
ApiHelper::PsuGetInputVoltage(const std::string& name)
{
    if (auto* psu = ensure_virtual_device<virtual_psu_device>(mgr, name))
        return psu->is_present() ? psu->get_voltage() : 0.0;
    
    return 0.0;
}

double
ApiHelper::PsuGetInputCurrent(const std::string& name)
{
    if (auto* psu = ensure_virtual_device<virtual_psu_device>(mgr, name))
        return psu->is_present() ? psu->get_current() : 0.0;
    
    return 0.0;
}

void
ApiHelper::PsuGetLedState(std::string& _return, const std::string& name)
{    
    if (auto* psu = ensure_virtual_device<virtual_psu_device>(mgr, name))
        _return =  LedConvert::StateToColor(psu->get_led_state());
    else
        _return = DEF_COMPONENT_NAME_NA;   
}

bool
ApiHelper::PsuSetLedState(const std::string& name, const std::string& state)
{
    /* Not support yet. */
    return false;
}

void
ApiHelper::FanGetModel(std::string& _return, const std::string& name)
{
    if (auto* fan = ensure_virtual_device<virtual_fan_device>(mgr, name))
        _return =  fan->get_model();
    else
        _return = DEF_COMPONENT_NAME_NA;
}

void
ApiHelper::FanGetSerial(std::string& _return, const std::string& name)
{
    if (auto* fan = ensure_virtual_device<virtual_fan_device>(mgr, name))
        _return =  fan->get_serial();
    else
        _return = DEF_COMPONENT_NAME_NA;
}

void
ApiHelper::FanGetRevision(std::string& _return, const std::string& name)
{
    if (auto* fan = ensure_virtual_device<virtual_fan_device>(mgr, name))
        _return =  fan->get_revision();
    else
        _return = DEF_COMPONENT_NAME_NA;
}

bool
ApiHelper::FanGetPresence(const std::string& name)
{
    if (IsPsuFan(name)) {
        return PsuGetPresence(name);
    }
   
    if (auto* fan = ensure_virtual_device<virtual_fan_device>(mgr, name))
        return fan->is_present();

    return false;
}

bool
ApiHelper::FanGetStatus(const std::string& name)
{
    if (IsPsuFan(name)) {
        return PsuGetStatus(name);
    }

    if (auto* fan = ensure_virtual_device<virtual_fan_device>(mgr, name))
        return fan->is_present();

    return false;
}

bool
ApiHelper::FanIsReplaceable(const std::string& name)
{
    return IsPsuFan(name) ? false : true;
}

void
ApiHelper::FanGetDirection(std::string& _return, const std::string& name)
{
    FAN_DIRECTION state = IsPsuFan(name) ? FAN_DIRECTION::FAN_DIRECTION_INTAKE : FAN_DIRECTION::FAN_DIRECTION_EXHAUST;
    _return = FanConvert::StateToDirect(state);
}

int32_t
ApiHelper::FanGetSpeed(const std::string& name)
{
    if (IsPsuFan(name)) {
        return DEF_FAN_SPEED_UP;
    }

    if (auto* fan = ensure_virtual_device<virtual_fan_device>(mgr, name))
        return fan->is_present() ? fan->get_actual_speed() : 0;

    return 0;
}

int32_t
ApiHelper::FanGetTargetSpeed(const std::string& name)
{
    if (IsPsuFan(name)) {
        return DEF_FAN_SPEED_UP;
    }

    if (auto* fan = ensure_virtual_device<virtual_fan_device>(mgr, name))
        return fan->is_present() ? fan->get_actual_speed() : 0;

    return 0;
}

int32_t
ApiHelper::FanGetSpeedTolerance(const std::string& name)
{
    return DEF_FAN_SPEED_TOLERANCE;
}

bool
ApiHelper::FanSetSpeed(const std::string& name, const int32_t speed)
{
    if (IsPsuFan(name)) {
        return false;
    }
   
    if (auto* fan = ensure_virtual_device<virtual_fan_device>(mgr, name))
    {
        fan->set_speed(speed);
        /* Update target speed cache. */
        m_fanTarSpd[name] = speed;
        return true;
    }

    return false;
}

void
ApiHelper::FanGetLedState(std::string& _return, const std::string& name)
{
    if (IsPsuFan(name)) {
        PsuGetLedState(_return, name);
        return;
    }

    if (auto* fan = ensure_virtual_device<virtual_fan_device>(mgr, name))
        _return =  LedConvert::StateToColor(fan->get_led_state());
    else
        _return = DEF_COMPONENT_NAME_NA;
}

bool
ApiHelper::FanSetLedState(const std::string& name, const std::string& state)
{
    if (IsPsuFan(name)) {
        return PsuSetLedState(name, state);
    }

    if (auto* fan = ensure_virtual_device<virtual_fan_device>(mgr, name))
    {
        fan->set_led_state( LedConvert::ColorToState(state));
        return true;
    }

    return false;
}

int32_t
ApiHelper::FanGetPositionInParent(const std::string& name)
{
    if (IsPsuFan(name)) {
        return 0;
    }
    return GetDevIndex(name);
}

bool ApiHelper::FanDrawerGetPresence(const std::string& name)
{
    if (auto* fan_drawer = ensure_virtual_device<virtual_fan_drawer_device>(mgr, name))
    {
        return fan_drawer->is_present();
    }

    return false;

}

bool ApiHelper::FanDrawerGetStatus(const std::string& name)
{
    if (auto* fan_drawer = ensure_virtual_device<virtual_fan_drawer_device>(mgr, name))
    {
        return fan_drawer->is_present();
    }
    return false;
}

bool ApiHelper::FanDrawerIsReplaceable(const std::string& name)
{
    return false;
}

void ApiHelper::FanDrawerGetLedState(std::string& _return, const std::string& name)
{
    if (auto* fan_drawer = ensure_virtual_device<virtual_fan_drawer_device>(mgr, name))
        _return =  LedConvert::StateToColor(fan_drawer->get_led_state());
    else
        _return = DEF_COMPONENT_NAME_NA;
}

bool ApiHelper::FanDrawerSetLedState(const std::string& name, const std::string& state)
{
    if (auto* fan_drawer = ensure_virtual_device<virtual_fan_device>(mgr, name))
    {
        fan_drawer->set_led_state( LedConvert::ColorToState(state));
        return true;
    }
    return false;
}

void ApiHelper::LedGetState(std::string& _return, const std::string& name)
{
    _return = DEF_COMPONENT_NAME_NA;
}

bool ApiHelper::LedSetState(const std::string& name, const std::string& state)
{
    return false;
}

double
ApiHelper::ThermalGetTemp(const std::string& name)
{
    double temp = GetThermalTemp(name);

    /* Record the temperature. */
    if (m_thermalMinRecord.find(name) == m_thermalMinRecord.end() || m_thermalMinRecord[name] > temp) {
        m_thermalMinRecord[name] = temp;
    }
    if (m_thermalMaxRecord.find(name) == m_thermalMaxRecord.end() || m_thermalMaxRecord[name] < temp) {
        m_thermalMaxRecord[name] = temp;
    }

    if (auto* thermal = ensure_virtual_device<virtual_thermal_device>(mgr, name))
        return thermal->get_temp();

    return 0;
}

double
ApiHelper::ThermalGetHighThreshold(const std::string& name)
{
    if (auto* thermal = ensure_virtual_device<virtual_thermal_device>(mgr, name))
        return thermal->get_high_threshold();

    return 0;
}

double
ApiHelper::ThermalGetLowThreshold(const std::string& name)
{
    if (auto* thermal = ensure_virtual_device<virtual_thermal_device>(mgr, name))
        return thermal->get_low_threshold();

    return 0;
}


bool
ApiHelper::ThermalSetHighThreshold(const std::string& name, const double value)
{
    m_thermalHighThres[name] = value; 
    if (auto* thermal = ensure_virtual_device<virtual_thermal_device>(mgr, name))
    {
        thermal->set_high_threshold(value);
        return true;
    }
    return false;
}

bool
ApiHelper::ThermalSetLowThreshold(const std::string& name, const double value)
{
    m_thermalLowThres[name] = value;

    if (auto* thermal = ensure_virtual_device<virtual_thermal_device>(mgr, name))
    {
        thermal->set_low_threshold(value);
        return true;
    }
    return false;
}

double
ApiHelper::ThermalGetHighCriticalThreshold(const std::string& name)
{
    if (auto* thermal = ensure_virtual_device<virtual_thermal_device>(mgr, name))
        return thermal->get_high_critical();

    return 0;

}

double
ApiHelper::ThermalGetLowCriticalThreshold(const std::string& name)
{
    if (auto* thermal = ensure_virtual_device<virtual_thermal_device>(mgr, name))
        return thermal->get_low_critical();

    return 0;
}

bool
ApiHelper::ThermalSetHighCriticalThreshold(const std::string& name, const double value)
{
    m_thermalHighCriticalThres[name] = value;

    if (auto* thermal = ensure_virtual_device<virtual_thermal_device>(mgr, name))
    {
        thermal->set_high_critical(value);
        return true;
    }
    return false;
}

bool
ApiHelper::ThermalSetLowCriticalThreshold(const std::string& name, const double value)
{
    m_thermalLowCriticalThres[name] = value;
    if (auto* thermal = ensure_virtual_device<virtual_thermal_device>(mgr, name))
    {
        thermal->set_low_critical(value);
        return true;
    }
    return false;
}

double
ApiHelper::ThermalGetMinimumRecorded(const std::string& name)
{
    if (m_thermalMinRecord.find(name) == m_thermalMinRecord.end()) {
        return DEF_THERMAL_NONE;
    }
    return m_thermalMinRecord[name];
}

double
ApiHelper::ThermalGetMaximumRecorded(const std::string& name)
{
    if (m_thermalMaxRecord.find(name) == m_thermalMaxRecord.end()) {
        return DEF_THERMAL_NONE;
    }
    return m_thermalMaxRecord[name];
}

bool
ApiHelper::WatchDogArm(const int32_t index, const int32_t seconds)
{

    if (auto* watchdog = ensure_virtual_device<virtual_watchdog_device>(mgr, "Watchdog" + std::to_string(index)))
    {
        watchdog->arm(seconds);
        m_wdArmed[index] = true;
        return true;
    }
    return false;
}

bool
ApiHelper::WatchDogDisrm(const int32_t index)
{
    if (auto* watchdog = ensure_virtual_device<virtual_watchdog_device>(mgr, "Watchdog" + std::to_string(index)))
    {
        watchdog->disarm();
        m_wdArmed[index] = false;
        return true;
    }
    return false;
}

bool
ApiHelper::WatchDogIsArmed(const int32_t index)
{
    return m_wdArmed.find(index) == m_wdArmed.end() ? true : m_wdArmed[index];
}

void
ApiHelper::ChassisGetName(std::string& _return)
{
    _return = GetChassisName();
}

void
ApiHelper::ChassisGetModel(std::string& _return)
{
    if (auto* chassis = ensure_virtual_device<virtual_chassis_device>(mgr, "Chassis"))
        _return =  chassis->get_model();
    else
        _return = DEF_COMPONENT_NAME_NA;

}

void
ApiHelper::ChassisGetSerial(std::string& _return)
{
    if (auto* chassis = ensure_virtual_device<virtual_chassis_device>(mgr, "Chassis"))
        _return =  chassis->get_serial();
    else
        _return = DEF_COMPONENT_NAME_NA;
}

void
ApiHelper::ChassisGetRevision(std::string& _return)
{
    if (auto* chassis = ensure_virtual_device<virtual_chassis_device>(mgr, "Chassis"))
        _return =  chassis->get_revision();
    else
        _return = DEF_COMPONENT_NAME_NA;
}

void
ApiHelper::ChassisGetBaseMac(std::string& _return)
{   
    if (auto* chassis = ensure_virtual_device<virtual_chassis_device>(mgr, "Chassis"))
        _return =  chassis->get_base_mac();
    else
        _return = DEF_COMPONENT_NAME_NA;
}

int32_t
ApiHelper::ChassisGetSlot()
{
    return GetChassisSlot();
}

void
ApiHelper::ChassisGetLedState(std::string& _return)
{
    _return = GetLedState(DEF_CHASSIS_LED_INDEX);
}

bool
ApiHelper::ChassisSetLedState(const std::string& state)
{
    return SetLedState(DEF_CHASSIS_LED_INDEX, state);
}

void
ApiHelper::ChassisGetList(std::string& _return)
{
    const json &js = GetDevicesList();
    _return = js.dump();
}

bool
ApiHelper::ChassisIsModular()
{
    return true;
}

void
ApiHelper::ModuleGetModel(std::string& _return, const std::string& name)
{

    if (auto* module = ensure_virtual_device<virtual_module_device>(mgr, name))
        _return =  module->get_model();
    else
        _return = DEF_COMPONENT_NAME_NA;
}

void
ApiHelper::ModuleGetSerial(std::string& _return, const std::string& name)
{
    if (auto* module = ensure_virtual_device<virtual_module_device>(mgr, name))
        _return =  module->get_serial();
    else
        _return = DEF_COMPONENT_NAME_NA;
}

void
ApiHelper::ModuleGetRevision(std::string& _return, const std::string& name)
{
    if (auto* module = ensure_virtual_device<virtual_module_device>(mgr, name))
        _return =  module->get_revision();
    else
        _return = DEF_COMPONENT_NAME_NA;
}

void
ApiHelper::ModuleGetBaseMac(std::string& _return, const std::string& name)
{  
    if (auto* module = ensure_virtual_device<virtual_module_device>(mgr, name))
        _return =  module->get_base_mac();
    else
        _return = DEF_COMPONENT_NAME_NA;

}

int32_t
ApiHelper::ModuleGetSlot(const std::string& name)
{  
    if (auto* module = ensure_virtual_device<virtual_module_device>(mgr, name))
        return module->get_slot();
    else
        return 0;
}

void
ApiHelper::ModuleGetType(std::string& _return, const std::string& name)
{
    if (auto* module = ensure_virtual_device<virtual_module_device>(mgr, name))
        _return = ModuleConvert::TypeToStr(static_cast<int>(module->get_type()));
    else
        _return = DEF_COMPONENT_NAME_NA;
}

void
ApiHelper::ModuleGetStatus(std::string& _return, const std::string& name)
{
    if (auto* module = ensure_virtual_device<virtual_module_device>(mgr, name))
        _return = ModuleConvert::StateToStr(static_cast<int>(module->get_state()));
    else
        _return = DEF_COMPONENT_NAME_NA;
}

double
ApiHelper::ModuleGetTemp(const std::string& name)
{

    if (auto* module = ensure_virtual_device<virtual_module_device>(mgr, name))
        return module->get_temp();
    else
        return 0;    
}

void
ApiHelper::ModuleGetLedState(std::string& _return, const std::string& name)
{
    int32_t index = GetDevIndex(name);

    if (index == GetChassisSlot()) {
        ChassisGetLedState(_return);
        return;
    }

    if (auto* module = ensure_virtual_device<virtual_module_device>(mgr, name))
        _return = LedConvert::StateToColor(static_cast<int>(module->get_led_state()));
    else
        _return = DEF_COMPONENT_NAME_NA;
}

bool
ApiHelper::ModuleSetLedState(const std::string& name, const std::string& state)
{
    if (auto* module = ensure_virtual_device<virtual_module_device>(mgr, name))
    {
        module->set_led_state(LedConvert::ColorToState(state));
        return true;
    }
    else 
    {
        return false;
    }


}

void
ApiHelper::ComponentGetFwVer(std::string& _return, const std::string& name)
{
    /* BIOS */
    if (name == DEF_COMPONENT_NAME_BIOS) {
        _return = DEF_COMPONENT_VER_BIOS;
        return;
    }
    /* ONIE */
    if (name == DEF_COMPONENT_NAME_ONIE) {
        _return = DEF_COMPONENT_VER_ONIE;
        return;
    }
    /* Chassis FPGA */
    //CMfg mfg = {0};
    if (name == DEF_COMPONENT_NAME_FPGA) {
       // _return = (OPLK_OK == BOARD_GetSccMfg(&mfg) && strlen(mfg.acFpga1Ver) > 0) ? mfg.acFpga1Ver : DEF_COMPONENT_NAME_NA;
        return;
    }
    /* Chassis CPLD */
    if (name == DEF_COMPONENT_NAME_CPLD) {
        //_return = (OPLK_OK == BOARD_GetSccMfg(&mfg) && strlen(mfg.acCpld1Ver) > 0) ? mfg.acCpld1Ver : DEF_COMPONENT_NAME_NA;
        return;
    }

    
    /* module or component */
    if (auto* component = ensure_virtual_device<virtual_component_device>(mgr, name))
        _return = component->get_revision();
    else
        _return = DEF_COMPONENT_NAME_NA;

  
}
