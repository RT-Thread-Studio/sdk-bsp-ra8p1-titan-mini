#ifndef RT_EXAMPLE_H
#define RT_EXAMPLE_H

void adc_sample(void);
int canfd_test(void);
void eth_sample(void);
void flash_sample(void);
void flash_speed_test(void);
void i2c_sample(void);
void key_irq_sample(void);
void rtc_sample(void);
void alarm_sample(void);
void sdcard_sample(void);
void sdram_speed_test(void);
void spi_loop_test(void);
int wdt_sample(void);
rt_bool_t udp_raw_rx_fastpath_try_consume(const void *frame, rt_uint32_t frame_len);

#endif /* RT_EXAMPLE_H */
