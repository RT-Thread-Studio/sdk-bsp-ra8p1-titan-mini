/* generated vector header file - do not edit */
        #ifndef VECTOR_DATA_H
        #define VECTOR_DATA_H
        #ifdef __cplusplus
        extern "C" {
        #endif
                /* Number of interrupts allocated */
        #ifndef VECTOR_DATA_IRQ_COUNT
        #define VECTOR_DATA_IRQ_COUNT    (22)
        #endif
        /* ISR prototypes */
        void sci_b_uart_rxi_isr(void);
        void sci_b_uart_txi_isr(void);
        void sci_b_uart_tei_isr(void);
        void sci_b_uart_eri_isr(void);
        void glcdc_line_detect_isr(void);
        void drw_int_isr(void);
        void vin_status_isr(void);
        void vin_error_isr(void);
        void mipi_csi_rx_isr(void);
        void mipi_csi_dl_isr(void);
        void mipi_csi_vc_isr(void);
        void mipi_csi_pm_isr(void);
        void mipi_csi_gst_isr(void);
        void gpt_counter_overflow_isr(void);
        void iic_master_rxi_isr(void);
        void iic_master_txi_isr(void);
        void iic_master_tei_isr(void);
        void iic_master_eri_isr(void);

        /* Vector table allocations */
        #define VECTOR_NUMBER_SCI1_RXI ((IRQn_Type) 0) /* SCI1 RXI (Receive data full) */
        #define SCI1_RXI_IRQn          ((IRQn_Type) 0) /* SCI1 RXI (Receive data full) */
        #define VECTOR_NUMBER_SCI1_TXI ((IRQn_Type) 1) /* SCI1 TXI (Transmit data empty) */
        #define SCI1_TXI_IRQn          ((IRQn_Type) 1) /* SCI1 TXI (Transmit data empty) */
        #define VECTOR_NUMBER_SCI1_TEI ((IRQn_Type) 2) /* SCI1 TEI (Transmit end) */
        #define SCI1_TEI_IRQn          ((IRQn_Type) 2) /* SCI1 TEI (Transmit end) */
        #define VECTOR_NUMBER_SCI1_ERI ((IRQn_Type) 3) /* SCI1 ERI (Receive error) */
        #define SCI1_ERI_IRQn          ((IRQn_Type) 3) /* SCI1 ERI (Receive error) */
        #define VECTOR_NUMBER_GLCDC_LINE_DETECT ((IRQn_Type) 4) /* GLCDC LINE DETECT (Specified line) */
        #define GLCDC_LINE_DETECT_IRQn          ((IRQn_Type) 4) /* GLCDC LINE DETECT (Specified line) */
        #define VECTOR_NUMBER_DRW_INT ((IRQn_Type) 5) /* DRW INT (DRW interrupt) */
        #define DRW_INT_IRQn          ((IRQn_Type) 5) /* DRW INT (DRW interrupt) */
        #define VECTOR_NUMBER_VIN_IRQ ((IRQn_Type) 6) /* VIN IRQ (Interrupt Request) */
        #define VIN_IRQ_IRQn          ((IRQn_Type) 6) /* VIN IRQ (Interrupt Request) */
        #define VECTOR_NUMBER_VIN_ERR ((IRQn_Type) 7) /* VIN ERR (Interrupt Request for SYNC Error) */
        #define VIN_ERR_IRQn          ((IRQn_Type) 7) /* VIN ERR (Interrupt Request for SYNC Error) */
        #define VECTOR_NUMBER_MIPICSI_RX ((IRQn_Type) 8) /* MIPICSI RX (Receive interrupt) */
        #define MIPICSI_RX_IRQn          ((IRQn_Type) 8) /* MIPICSI RX (Receive interrupt) */
        #define VECTOR_NUMBER_MIPICSI_DL ((IRQn_Type) 9) /* MIPICSI DL (Data Lane interrupt) */
        #define MIPICSI_DL_IRQn          ((IRQn_Type) 9) /* MIPICSI DL (Data Lane interrupt) */
        #define VECTOR_NUMBER_MIPICSI_VC ((IRQn_Type) 10) /* MIPICSI VC (Virtual Channel interrupt) */
        #define MIPICSI_VC_IRQn          ((IRQn_Type) 10) /* MIPICSI VC (Virtual Channel interrupt) */
        #define VECTOR_NUMBER_MIPICSI_PM ((IRQn_Type) 11) /* MIPICSI PM (Power Management interrupt) */
        #define MIPICSI_PM_IRQn          ((IRQn_Type) 11) /* MIPICSI PM (Power Management interrupt) */
        #define VECTOR_NUMBER_MIPICSI_GST ((IRQn_Type) 12) /* MIPICSI GST (Generic Short Packet interrupt) */
        #define MIPICSI_GST_IRQn          ((IRQn_Type) 12) /* MIPICSI GST (Generic Short Packet interrupt) */
        #define VECTOR_NUMBER_GPT0_COUNTER_OVERFLOW ((IRQn_Type) 13) /* GPT0 COUNTER OVERFLOW (Overflow) */
        #define GPT0_COUNTER_OVERFLOW_IRQn          ((IRQn_Type) 13) /* GPT0 COUNTER OVERFLOW (Overflow) */
        #define VECTOR_NUMBER_IIC0_RXI ((IRQn_Type) 14) /* IIC0 RXI (Receive data full) */
        #define IIC0_RXI_IRQn          ((IRQn_Type) 14) /* IIC0 RXI (Receive data full) */
        #define VECTOR_NUMBER_IIC0_TXI ((IRQn_Type) 15) /* IIC0 TXI (Transmit data empty) */
        #define IIC0_TXI_IRQn          ((IRQn_Type) 15) /* IIC0 TXI (Transmit data empty) */
        #define VECTOR_NUMBER_IIC0_TEI ((IRQn_Type) 16) /* IIC0 TEI (Transmit end) */
        #define IIC0_TEI_IRQn          ((IRQn_Type) 16) /* IIC0 TEI (Transmit end) */
        #define VECTOR_NUMBER_IIC0_ERI ((IRQn_Type) 17) /* IIC0 ERI (Transfer error) */
        #define IIC0_ERI_IRQn          ((IRQn_Type) 17) /* IIC0 ERI (Transfer error) */
        #define VECTOR_NUMBER_SCI2_RXI ((IRQn_Type) 18) /* SCI2 RXI (Receive data full) */
        #define SCI2_RXI_IRQn          ((IRQn_Type) 18) /* SCI2 RXI (Receive data full) */
        #define VECTOR_NUMBER_SCI2_TXI ((IRQn_Type) 19) /* SCI2 TXI (Transmit data empty) */
        #define SCI2_TXI_IRQn          ((IRQn_Type) 19) /* SCI2 TXI (Transmit data empty) */
        #define VECTOR_NUMBER_SCI2_TEI ((IRQn_Type) 20) /* SCI2 TEI (Transmit end) */
        #define SCI2_TEI_IRQn          ((IRQn_Type) 20) /* SCI2 TEI (Transmit end) */
        #define VECTOR_NUMBER_SCI2_ERI ((IRQn_Type) 21) /* SCI2 ERI (Receive error) */
        #define SCI2_ERI_IRQn          ((IRQn_Type) 21) /* SCI2 ERI (Receive error) */
        /* The number of entries required for the ICU vector table. */
        #define BSP_ICU_VECTOR_NUM_ENTRIES (22)

        #ifdef __cplusplus
        }
        #endif
        #endif /* VECTOR_DATA_H */