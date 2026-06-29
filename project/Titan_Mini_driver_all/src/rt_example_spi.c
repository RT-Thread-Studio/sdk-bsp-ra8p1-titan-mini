#include <rtthread.h>
#include <rtdevice.h>
#include "rt_example.h"
#include "drv_spi.h"

#define TEXT_NUMBER_SIZE    1024
#define SPI_BUS_NAME        "spi1"
#define SPI_NAME            "spi10"

void spi_loop_test(void)
{
    static uint8_t sendbuf[TEXT_NUMBER_SIZE] = {0};
    static uint8_t readbuf[TEXT_NUMBER_SIZE] = {0};
    struct rt_spi_device *spi_dev = RT_NULL;
    struct rt_spi_configuration cfg;

    for (int i = 0; i < (int) sizeof(sendbuf); i++)
    {
        sendbuf[i] = (uint8_t) i;
    }

    rt_hw_spi_device_attach(SPI_BUS_NAME, SPI_NAME, RT_NULL);

    cfg.data_width = 8;
    cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB | RT_SPI_NO_CS;
    cfg.max_hz = 1 * 1000 * 1000;

    spi_dev = (struct rt_spi_device *) rt_device_find(SPI_NAME);
    if (spi_dev == RT_NULL)
    {
        rt_kprintf("spi sample run failed! can't find %s device!\n", SPI_NAME);
        return;
    }

    rt_spi_configure(spi_dev, &cfg);

    rt_kprintf("%s send:\n", SPI_NAME);
    for (int i = 0; i < (int) sizeof(sendbuf); i++)
    {
        rt_kprintf("%02x ", sendbuf[i]);
    }

    rt_spi_transfer(spi_dev, sendbuf, readbuf, sizeof(sendbuf));
    rt_kprintf("\n\n%s rcv:\n", SPI_NAME);

    for (int i = 0; i < (int) sizeof(readbuf); i++)
    {
        if (readbuf[i] != sendbuf[i])
        {
            rt_kprintf("SPI test fail!!!\n");
            break;
        }

        rt_kprintf("%02x ", readbuf[i]);
    }

    rt_kprintf("\n\nSPI test end\n");
}
MSH_CMD_EXPORT(spi_loop_test, test spi1);
