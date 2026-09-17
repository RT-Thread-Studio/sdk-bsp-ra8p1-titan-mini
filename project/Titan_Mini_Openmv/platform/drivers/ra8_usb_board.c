/* SPDX-License-Identifier: Apache-2.0 */
#include <rtdevice.h>
#include "tusb.h"
#include "hal_data.h"

int tusb_board_init(void)
{
    /* Titan Mini schematic V1.0, MCU_PORTS p3 / USB_DEV p5:
     * P413 drives U3 TS3USB221 (data) and U2 CH443K (VBUS).
     * LOW selects USBFS; HIGH selects USBHS. Configure the latch and
     * direction together before either controller enables its pull-up.
     */
    uint32_t config = IOPORT_CFG_PORT_DIRECTION_OUTPUT |
        (BOARD_DEVICE_RHPORT_NUM == 1 ? IOPORT_CFG_PORT_OUTPUT_HIGH :
                                       IOPORT_CFG_PORT_OUTPUT_LOW);
    return R_IOPORT_PinCfg(&g_ioport_ctrl, BSP_IO_PORT_04_PIN_13, config) ==
           FSP_SUCCESS ? RT_EOK : -RT_EIO;
}
