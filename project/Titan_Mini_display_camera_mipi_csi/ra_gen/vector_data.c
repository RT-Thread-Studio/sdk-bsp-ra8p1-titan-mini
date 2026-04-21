/* generated vector source file - do not edit */
        #include "bsp_api.h"
        /* Do not build these data structures if no interrupts are currently allocated because IAR will have build errors. */
        #if VECTOR_DATA_IRQ_COUNT > 0
        BSP_DONT_REMOVE const fsp_vector_t g_vector_table[BSP_ICU_VECTOR_NUM_ENTRIES] BSP_PLACE_IN_SECTION(BSP_SECTION_APPLICATION_VECTORS) =
        {
                        [0] = sci_b_uart_rxi_isr, /* SCI1 RXI (Receive data full) */
            [1] = sci_b_uart_txi_isr, /* SCI1 TXI (Transmit data empty) */
            [2] = sci_b_uart_tei_isr, /* SCI1 TEI (Transmit end) */
            [3] = sci_b_uart_eri_isr, /* SCI1 ERI (Receive error) */
            [4] = glcdc_line_detect_isr, /* GLCDC LINE DETECT (Specified line) */
            [5] = drw_int_isr, /* DRW INT (DRW interrupt) */
            [6] = vin_status_isr, /* VIN IRQ (Interrupt Request) */
            [7] = vin_error_isr, /* VIN ERR (Interrupt Request for SYNC Error) */
            [8] = mipi_csi_rx_isr, /* MIPICSI RX (Receive interrupt) */
            [9] = mipi_csi_dl_isr, /* MIPICSI DL (Data Lane interrupt) */
            [10] = mipi_csi_vc_isr, /* MIPICSI VC (Virtual Channel interrupt) */
            [11] = mipi_csi_pm_isr, /* MIPICSI PM (Power Management interrupt) */
            [12] = mipi_csi_gst_isr, /* MIPICSI GST (Generic Short Packet interrupt) */
            [13] = gpt_counter_overflow_isr, /* GPT0 COUNTER OVERFLOW (Overflow) */
            [14] = iic_master_rxi_isr, /* IIC0 RXI (Receive data full) */
            [15] = iic_master_txi_isr, /* IIC0 TXI (Transmit data empty) */
            [16] = iic_master_tei_isr, /* IIC0 TEI (Transmit end) */
            [17] = iic_master_eri_isr, /* IIC0 ERI (Transfer error) */
            [18] = sci_b_uart_rxi_isr, /* SCI2 RXI (Receive data full) */
            [19] = sci_b_uart_txi_isr, /* SCI2 TXI (Transmit data empty) */
            [20] = sci_b_uart_tei_isr, /* SCI2 TEI (Transmit end) */
            [21] = sci_b_uart_eri_isr, /* SCI2 ERI (Receive error) */
        };
        #if BSP_FEATURE_ICU_HAS_IELSR
        const bsp_interrupt_event_t g_interrupt_event_link_select[BSP_ICU_VECTOR_NUM_ENTRIES] =
        {
            [0] = BSP_PRV_VECT_ENUM(EVENT_SCI1_RXI,GROUP0), /* SCI1 RXI (Receive data full) */
            [1] = BSP_PRV_VECT_ENUM(EVENT_SCI1_TXI,GROUP1), /* SCI1 TXI (Transmit data empty) */
            [2] = BSP_PRV_VECT_ENUM(EVENT_SCI1_TEI,GROUP2), /* SCI1 TEI (Transmit end) */
            [3] = BSP_PRV_VECT_ENUM(EVENT_SCI1_ERI,GROUP3), /* SCI1 ERI (Receive error) */
            [4] = BSP_PRV_VECT_ENUM(EVENT_GLCDC_LINE_DETECT,GROUP4), /* GLCDC LINE DETECT (Specified line) */
            [5] = BSP_PRV_VECT_ENUM(EVENT_DRW_INT,GROUP5), /* DRW INT (DRW interrupt) */
            [6] = BSP_PRV_VECT_ENUM(EVENT_VIN_IRQ,GROUP6), /* VIN IRQ (Interrupt Request) */
            [7] = BSP_PRV_VECT_ENUM(EVENT_VIN_ERR,GROUP7), /* VIN ERR (Interrupt Request for SYNC Error) */
            [8] = BSP_PRV_VECT_ENUM(EVENT_MIPICSI_RX,GROUP0), /* MIPICSI RX (Receive interrupt) */
            [9] = BSP_PRV_VECT_ENUM(EVENT_MIPICSI_DL,GROUP1), /* MIPICSI DL (Data Lane interrupt) */
            [10] = BSP_PRV_VECT_ENUM(EVENT_MIPICSI_VC,GROUP2), /* MIPICSI VC (Virtual Channel interrupt) */
            [11] = BSP_PRV_VECT_ENUM(EVENT_MIPICSI_PM,GROUP3), /* MIPICSI PM (Power Management interrupt) */
            [12] = BSP_PRV_VECT_ENUM(EVENT_MIPICSI_GST,GROUP4), /* MIPICSI GST (Generic Short Packet interrupt) */
            [13] = BSP_PRV_VECT_ENUM(EVENT_GPT0_COUNTER_OVERFLOW,GROUP5), /* GPT0 COUNTER OVERFLOW (Overflow) */
            [14] = BSP_PRV_VECT_ENUM(EVENT_IIC0_RXI,GROUP6), /* IIC0 RXI (Receive data full) */
            [15] = BSP_PRV_VECT_ENUM(EVENT_IIC0_TXI,GROUP7), /* IIC0 TXI (Transmit data empty) */
            [16] = BSP_PRV_VECT_ENUM(EVENT_IIC0_TEI,GROUP0), /* IIC0 TEI (Transmit end) */
            [17] = BSP_PRV_VECT_ENUM(EVENT_IIC0_ERI,GROUP1), /* IIC0 ERI (Transfer error) */
            [18] = BSP_PRV_VECT_ENUM(EVENT_SCI2_RXI,GROUP2), /* SCI2 RXI (Receive data full) */
            [19] = BSP_PRV_VECT_ENUM(EVENT_SCI2_TXI,GROUP3), /* SCI2 TXI (Transmit data empty) */
            [20] = BSP_PRV_VECT_ENUM(EVENT_SCI2_TEI,GROUP4), /* SCI2 TEI (Transmit end) */
            [21] = BSP_PRV_VECT_ENUM(EVENT_SCI2_ERI,GROUP5), /* SCI2 ERI (Receive error) */
        };
        #endif
        #endif