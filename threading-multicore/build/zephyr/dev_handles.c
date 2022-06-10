#include <zephyr/device.h>
#include <zephyr/toolchain.h>

/* 1 : /uart@10000000:
 * Direct Dependencies:
 *   - (/)
 *   - (/soc/interrupt-controller@c000000)
 */
const device_handle_t __aligned(2) __attribute__((__section__(".__device_handles_pass2")))
__devicehdl_DT_N_S_uart_10000000[] = { DEVICE_HANDLE_SEP, DEVICE_HANDLE_SEP, DEVICE_HANDLE_ENDS };
