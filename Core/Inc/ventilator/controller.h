/**
 * controller.h:
 *
 * Handles the interface with the controller. These functions control communication and conversion for controller-read
 * values.
 *
 * @author mstarch
 */
#include <ventilator/panel_public.h>

/**
 * Performs an update to/from the controller. Runs the actual communication over the SPI interface.
 * return\: HAL_OK on success otherwise error.
 */
HAL_StatusTypeDef do_controller_cycle(void);

// Internal functions

/**
 * Convert controller's pascal measurements to CMH20
 */
int32_t pascal_to_cmh2O(int32_t pascal);
/**
 * Convert panel's CMH20 measurements to pascal
 */
int32_t cmh2O_to_pascal(int32_t pascal);
/**
 * Convert breaths-per-minute to ms period. Works in reverse too.
 */
int32_t bpm_to_ms_period(int32_t bpm);
/**
 * Prepares an outgoing panel packed by filling-in the basic values to send out to the controller. Converts from panel
 * unit space to controller unit space before transmission.
 * panel_packet_t* packet:  panel packet to fill
 * NumericalValues* values: numerical state to read to fill panel packet
 * PowerState power_state: current power state only ventilates when on
 * uint8_t plateau_count: take a plateau measurement (counts remaining for transmission)
 * uint8_t halt_vent: force-halt ventilation fo some error detected
 */
void prepare_panel_packet(panel_packet_t* packet, NumericalValues* values, PowerState power_state, uint8_t plateau_count, uint8_t halt_vent);
/**
 * Process the incoming controller packet. Converts to the panel's unit space.
 * controller_packet_t* packet: packet to process
 * NumericalValues* values: numerical state to fill
 */
void process_control_packet(controller_packet_t* packet, NumericalValues* values);
