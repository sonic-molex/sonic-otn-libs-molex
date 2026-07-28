#pragma once

#include "saiextensions.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void *S_O_Handle;

S_O_Handle create_sai_adapter();

void free_sai_adapter(S_O_Handle);

sai_status_t sai_adapter_api_query(S_O_Handle, sai_api_t, void **);

sai_object_type_t sai_adapter_object_type_query(S_O_Handle, sai_object_id_t);

/* Hand the SAI profile get_value callback (from sai_api_initialize's service
 * method table) to the adapter, so init-time config files referenced in
 * sai.profile (e.g. SAI_OTN_THRESHOLD_FILE) can be read. */
void sai_adapter_set_profile_getter(sai_profile_get_value_fn fn);

sai_status_t sai_adapter_query_attribute_capability(
        S_O_Handle,
        sai_object_id_t switch_id,
        sai_object_type_t object_type,
        sai_attr_id_t attr_id,
        sai_attr_capability_t *attr_capability);

#ifdef __cplusplus
}
#endif

