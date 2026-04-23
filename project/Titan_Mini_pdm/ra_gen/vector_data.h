/* generated vector header file - do not edit */
        #ifndef VECTOR_DATA_H
        #define VECTOR_DATA_H
        #ifdef __cplusplus
        extern "C" {
        #endif
                /* Number of interrupts allocated */
        #ifndef VECTOR_DATA_IRQ_COUNT
        #define VECTOR_DATA_IRQ_COUNT    (16)
        #endif
        /* ISR prototypes */
        void sci_b_uart_rxi_isr(void);
        void sci_b_uart_txi_isr(void);
        void sci_b_uart_tei_isr(void);
        void sci_b_uart_eri_isr(void);
        void ssi_txi_isr(void);
        void ssi_int_isr(void);
        void sdhimmc_accs_isr(void);
        void sdhimmc_card_isr(void);
        void dmac_int_isr(void);
        void pdm_sdet_isr(void);
        void pdm_dat_isr(void);
        void pdm_err_isr(void);

        /* Vector table allocations */
        #define VECTOR_NUMBER_SCI2_RXI ((IRQn_Type) 0) /* SCI2 RXI (Receive data full) */
        #define SCI2_RXI_IRQn          ((IRQn_Type) 0) /* SCI2 RXI (Receive data full) */
        #define VECTOR_NUMBER_SCI2_TXI ((IRQn_Type) 1) /* SCI2 TXI (Transmit data empty) */
        #define SCI2_TXI_IRQn          ((IRQn_Type) 1) /* SCI2 TXI (Transmit data empty) */
        #define VECTOR_NUMBER_SCI2_TEI ((IRQn_Type) 2) /* SCI2 TEI (Transmit end) */
        #define SCI2_TEI_IRQn          ((IRQn_Type) 2) /* SCI2 TEI (Transmit end) */
        #define VECTOR_NUMBER_SCI2_ERI ((IRQn_Type) 3) /* SCI2 ERI (Receive error) */
        #define SCI2_ERI_IRQn          ((IRQn_Type) 3) /* SCI2 ERI (Receive error) */
        #define VECTOR_NUMBER_SSI0_TXI ((IRQn_Type) 4) /* SSI0 TXI (Transmit data empty) */
        #define SSI0_TXI_IRQn          ((IRQn_Type) 4) /* SSI0 TXI (Transmit data empty) */
        #define VECTOR_NUMBER_SSI0_INT ((IRQn_Type) 5) /* SSI0 INT (Error interrupt) */
        #define SSI0_INT_IRQn          ((IRQn_Type) 5) /* SSI0 INT (Error interrupt) */
        #define VECTOR_NUMBER_SDHIMMC0_ACCS ((IRQn_Type) 6) /* SDHIMMC0 ACCS (Card access) */
        #define SDHIMMC0_ACCS_IRQn          ((IRQn_Type) 6) /* SDHIMMC0 ACCS (Card access) */
        #define VECTOR_NUMBER_SDHIMMC0_CARD ((IRQn_Type) 7) /* SDHIMMC0 CARD (Card detect) */
        #define SDHIMMC0_CARD_IRQn          ((IRQn_Type) 7) /* SDHIMMC0 CARD (Card detect) */
        #define VECTOR_NUMBER_DMAC0_INT ((IRQn_Type) 8) /* DMAC0 INT (DMAC0 transfer end) */
        #define DMAC0_INT_IRQn          ((IRQn_Type) 8) /* DMAC0 INT (DMAC0 transfer end) */
        #define VECTOR_NUMBER_SCI1_RXI ((IRQn_Type) 9) /* SCI1 RXI (Receive data full) */
        #define SCI1_RXI_IRQn          ((IRQn_Type) 9) /* SCI1 RXI (Receive data full) */
        #define VECTOR_NUMBER_SCI1_TXI ((IRQn_Type) 10) /* SCI1 TXI (Transmit data empty) */
        #define SCI1_TXI_IRQn          ((IRQn_Type) 10) /* SCI1 TXI (Transmit data empty) */
        #define VECTOR_NUMBER_SCI1_TEI ((IRQn_Type) 11) /* SCI1 TEI (Transmit end) */
        #define SCI1_TEI_IRQn          ((IRQn_Type) 11) /* SCI1 TEI (Transmit end) */
        #define VECTOR_NUMBER_SCI1_ERI ((IRQn_Type) 12) /* SCI1 ERI (Receive error) */
        #define SCI1_ERI_IRQn          ((IRQn_Type) 12) /* SCI1 ERI (Receive error) */
        #define VECTOR_NUMBER_PDM_SDET ((IRQn_Type) 13) /* PDM SDET (Sound detection interrupt) */
        #define PDM_SDET_IRQn          ((IRQn_Type) 13) /* PDM SDET (Sound detection interrupt) */
        #define VECTOR_NUMBER_PDM_DAT2 ((IRQn_Type) 14) /* PDM DAT2 (Data reception interrupt channel 2) */
        #define PDM_DAT2_IRQn          ((IRQn_Type) 14) /* PDM DAT2 (Data reception interrupt channel 2) */
        #define VECTOR_NUMBER_PDM_ERR2 ((IRQn_Type) 15) /* PDM ERR2 (Error detection interrupt channel 2) */
        #define PDM_ERR2_IRQn          ((IRQn_Type) 15) /* PDM ERR2 (Error detection interrupt channel 2) */
        /* The number of entries required for the ICU vector table. */
        #define BSP_ICU_VECTOR_NUM_ENTRIES (16)

        #ifdef __cplusplus
        }
        #endif
        #endif /* VECTOR_DATA_H */