/* generated vector source file - do not edit */
        #include "bsp_api.h"
        /* Do not build these data structures if no interrupts are currently allocated because IAR will have build errors. */
        #if VECTOR_DATA_IRQ_COUNT > 0
        BSP_DONT_REMOVE const fsp_vector_t g_vector_table[BSP_ICU_VECTOR_NUM_ENTRIES] BSP_PLACE_IN_SECTION(BSP_SECTION_APPLICATION_VECTORS) =
        {
                        [0] = sci_b_uart_rxi_isr, /* SCI2 RXI (Receive data full) */
            [1] = sci_b_uart_txi_isr, /* SCI2 TXI (Transmit data empty) */
            [2] = sci_b_uart_tei_isr, /* SCI2 TEI (Transmit end) */
            [3] = sci_b_uart_eri_isr, /* SCI2 ERI (Receive error) */
            [4] = ssi_txi_isr, /* SSI0 TXI (Transmit data empty) */
            [5] = ssi_int_isr, /* SSI0 INT (Error interrupt) */
            [6] = sdhimmc_accs_isr, /* SDHIMMC0 ACCS (Card access) */
            [7] = sdhimmc_card_isr, /* SDHIMMC0 CARD (Card detect) */
            [8] = dmac_int_isr, /* DMAC0 INT (DMAC0 transfer end) */
            [9] = sci_b_uart_rxi_isr, /* SCI1 RXI (Receive data full) */
            [10] = sci_b_uart_txi_isr, /* SCI1 TXI (Transmit data empty) */
            [11] = sci_b_uart_tei_isr, /* SCI1 TEI (Transmit end) */
            [12] = sci_b_uart_eri_isr, /* SCI1 ERI (Receive error) */
            [13] = pdm_sdet_isr, /* PDM SDET (Sound detection interrupt) */
            [14] = pdm_dat_isr, /* PDM DAT2 (Data reception interrupt channel 2) */
            [15] = pdm_err_isr, /* PDM ERR2 (Error detection interrupt channel 2) */
        };
        #if BSP_FEATURE_ICU_HAS_IELSR
        const bsp_interrupt_event_t g_interrupt_event_link_select[BSP_ICU_VECTOR_NUM_ENTRIES] =
        {
            [0] = BSP_PRV_VECT_ENUM(EVENT_SCI2_RXI,GROUP0), /* SCI2 RXI (Receive data full) */
            [1] = BSP_PRV_VECT_ENUM(EVENT_SCI2_TXI,GROUP1), /* SCI2 TXI (Transmit data empty) */
            [2] = BSP_PRV_VECT_ENUM(EVENT_SCI2_TEI,GROUP2), /* SCI2 TEI (Transmit end) */
            [3] = BSP_PRV_VECT_ENUM(EVENT_SCI2_ERI,GROUP3), /* SCI2 ERI (Receive error) */
            [4] = BSP_PRV_VECT_ENUM(EVENT_SSI0_TXI,GROUP4), /* SSI0 TXI (Transmit data empty) */
            [5] = BSP_PRV_VECT_ENUM(EVENT_SSI0_INT,GROUP5), /* SSI0 INT (Error interrupt) */
            [6] = BSP_PRV_VECT_ENUM(EVENT_SDHIMMC0_ACCS,GROUP6), /* SDHIMMC0 ACCS (Card access) */
            [7] = BSP_PRV_VECT_ENUM(EVENT_SDHIMMC0_CARD,GROUP7), /* SDHIMMC0 CARD (Card detect) */
            [8] = BSP_PRV_VECT_ENUM(EVENT_DMAC0_INT,GROUP0), /* DMAC0 INT (DMAC0 transfer end) */
            [9] = BSP_PRV_VECT_ENUM(EVENT_SCI1_RXI,GROUP1), /* SCI1 RXI (Receive data full) */
            [10] = BSP_PRV_VECT_ENUM(EVENT_SCI1_TXI,GROUP2), /* SCI1 TXI (Transmit data empty) */
            [11] = BSP_PRV_VECT_ENUM(EVENT_SCI1_TEI,GROUP3), /* SCI1 TEI (Transmit end) */
            [12] = BSP_PRV_VECT_ENUM(EVENT_SCI1_ERI,GROUP4), /* SCI1 ERI (Receive error) */
            [13] = BSP_PRV_VECT_ENUM(EVENT_PDM_SDET,GROUP5), /* PDM SDET (Sound detection interrupt) */
            [14] = BSP_PRV_VECT_ENUM(EVENT_PDM_DAT2,GROUP6), /* PDM DAT2 (Data reception interrupt channel 2) */
            [15] = BSP_PRV_VECT_ENUM(EVENT_PDM_ERR2,GROUP7), /* PDM ERR2 (Error detection interrupt channel 2) */
        };
        #endif
        #endif