#include "sai_adapter_interface.h"

// static sai_api_service_t sai_api_service;
static S_O_Handle sai_adapter;
static sai_api_t api_id = SAI_API_UNSPECIFIED;
// switch_device_t device = 0;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

static const char *module[] = {
    "UNSPECIFIED",
    "SWITCH",
    "ROUTER_INTERFACE",
    "OTN_ATTENUATOR",
    "OTN_OA",
    "OTN_OCM",
    "OTN_OSC",
    "OTN_OTDR"
};

sai_status_t
sai_api_initialize(uint64_t flags, const sai_service_method_table_t *services)
{
    if (services && services->profile_get_value) {
        sai_adapter_set_profile_getter(services->profile_get_value);
    }
    sai_adapter = create_sai_adapter();
    return SAI_STATUS_SUCCESS;
}

sai_status_t
sai_api_query(sai_api_t sai_api_id, void **api_method_table)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    if (!api_method_table) {
        status = SAI_STATUS_INVALID_PARAMETER;
        return status;
    }

    status = sai_adapter_api_query(sai_adapter, sai_api_id, api_method_table);
    return status;
}

sai_status_t
sai_api_uninitialize(void)
{ 
    free_sai_adapter(sai_adapter);
    return SAI_STATUS_SUCCESS;
}

sai_status_t
sai_log_set(sai_api_t sai_api_id, sai_log_level_t log_level)
{ 
    return SAI_STATUS_NOT_IMPLEMENTED;
}

sai_object_type_t
sai_object_type_query(sai_object_id_t sai_object_id)
{
    return sai_adapter_object_type_query(sai_adapter, sai_object_id);
}

sai_object_id_t
sai_switch_id_query(sai_object_id_t sai_object_id)
{
    return 0; //TODO add switch id
}

sai_status_t
sai_dbg_generate_dump(const char *dump_file_name)
{
    return SAI_STATUS_NOT_IMPLEMENTED;
}

sai_status_t
sai_query_api_version(sai_api_version_t *version)
{
    *version = SAI_API_VERSION;
    return SAI_STATUS_SUCCESS;
}

sai_status_t
sai_object_type_get_availability(
        sai_object_id_t switch_id,
        sai_object_type_t object_type,
        uint32_t attr_count,
        const sai_attribute_t *attr_list,
        uint64_t *count)
{
    return SAI_STATUS_NOT_IMPLEMENTED;
}

sai_status_t
sai_query_attribute_capability(
        sai_object_id_t switch_id,
        sai_object_type_t object_type,
        sai_attr_id_t attr_id,
        sai_attr_capability_t *attr_capability)
{
    return sai_adapter_query_attribute_capability(sai_adapter, switch_id, object_type, attr_id, attr_capability);
}

sai_status_t
sai_query_attribute_enum_values_capability(
        sai_object_id_t switch_id,
        sai_object_type_t object_type,
        sai_attr_id_t attr_id,
        sai_s32_list_t *enum_values_capability)
{
    return SAI_STATUS_NOT_IMPLEMENTED;
}

sai_status_t
sai_query_stats_capability(
        sai_object_id_t switch_id,
        sai_object_type_t object_type,
        sai_stat_capability_list_t *stats_capability)
{
    return SAI_STATUS_NOT_IMPLEMENTED;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
