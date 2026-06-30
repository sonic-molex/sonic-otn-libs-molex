#pragma once

namespace hal_listener {

/* Start the HAL-event listener thread (idempotent). It binds the HAL-event
 * socket and, for each received event, reports the mapped alarm through the SAI
 * alarm-notification path (sai_adapter::report_hal_alarm). Safe to call after
 * the switch is initialized. */
void start_listener();

}  // namespace hal_listener
