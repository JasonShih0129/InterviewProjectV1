#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <soc.h>
#include <zephyr/logging/log.h>
#include "gpio_ec.h"
#include "periphmgmt.h"
#include "pwrbtnmgmt.h"
#include "board_config.h"
#include "acpi_region.h"
#include "smchost.h"
#include <drivers/i2c.h>
#include "pd_mgmt.h"

LOG_MODULE_DECLARE(periph, CONFIG_PERIPHERAL_LOG_LEVEL);


void PD_Write_Data(const uint8_t *data, size_t len,const uint8_t reg)
{
    const struct device *i2c_pd;
    uint8_t buffer[10];

    i2c_pd = device_get_binding("I2C_pd");

    if (i2c_pd == NULL) {
        printk("Failed to get pd device\n");
        return;
    }

    buffer[0] = PD_Role_Config_Reg;
    memcpy(&buffer[1], data, len);

    if (i2c_write(i2c_pd, buffer, len + 1, PD_I2C_ADDR) != 0) {
        printk("I2C write failed\n");
    } else {
        printk("I2C write success\n");
    }
}

void PD_B2B_MOS_Configuration(void)
{
    const uint8_t b2b_config_data[] = {0x05, 0x04, 0x03, 0x02, 0x01};
    size_t b2b_config_len = sizeof(b2b_config_data);

    PD_Write_Data(b2b_config_data, b2b_config_len, PD_REG_B2B_MOS_CONFIG);
}

void PD_Power_Role_Configuration(void)
{
    const uint8_t pd_role_data[] = {0x05, 0xDC};
    size_t pd_role_len = sizeof(pd_role_data);

    PD_Write_Data(pd_role_data, pd_role_len, PD_REG_ROLE_CONFIG);
}

void PD_Initial(void)
{
    LOG_INF("%s", __func__);
    PD_B2B_MOS_Configuration();
    PD_Power_Role_Configuration();
}

void pd_interrupt_handle(void)
{
    const struct device *pd_int;
    pd_int = device_get_binding("pd_int");
    if (!pd_int) 
	{
        printk("Failed to get pd_int device\n");
        return;
    }

    if (!gpio_pin_get(pd_int, PD_INT_Assert)) 
    {
        uint8_t int_reg_val;
        const struct device *i2c_pd = device_get_binding("I2C_pd");
        if (!i2c_pd) 
		{
            printk("Failed to get I2C device\n");
            return;
        }

        if (i2c_read(i2c_pd, &int_reg_val, 1, PD_I2C_ADDR, PD_INT_REG) != 0) 
		{
            printk("I2C read failed\n");
        } 
		else 
		{
            printk("Interrupt Register Value: 0x%x\n", int_reg_val);
            // 在這裡可以根據讀取到的中斷寄存器值進行相應處理
        }
    }
}

void pd_thread(void *p1, void *p2, void *p3)
{
	uint32_t period = *(uint32_t *)p1;
	PD_Initial();
	PD_Interrupt_Service();

	while (true) 
	{
		k_msleep(period);
	}
}
