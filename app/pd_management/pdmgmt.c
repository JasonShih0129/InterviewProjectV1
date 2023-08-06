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
    uint8_t i2c_write_buffer[20];
    int i;

    i2c_pd = device_get_binding("I2C_pd");

    if (i2c_pd == NULL) 
    {
        printk("Failed to get pd device\n");
        return;
    }

    i2c_write_buffer[0] = reg;
    memcpy(&i2c_write_buffer[1], data, len);

    if (i2c_write(i2c_pd, i2c_write_buffer, len + 1, PD_I2C_ADDR) != 0) 
    {
        printk("I2C write failed\n");
    } 
    else 
    {
        printk("I2C write success\n");
        for(i = 0 ; i < 20 ; i++)
        {
            i2c_write_buffer[i] = 0;
        }
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
    // initial item as below
    PD_B2B_MOS_Configuration();
    PD_Power_Role_Configuration();
}

void PD_Interrupt_Service(void)
{
    const struct device *pd_int;

    uint8_t int_reg_buf[14];
    uint8_t int_reg_clr[14] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    size_t int_reg_clr_len = sizeof(int_reg_clr);
    
    pd_int = device_get_binding("pd_int");
    if (!pd_int) 
	{
        printk("Failed to get pd_int device\n");
        return;
    }

    if (!gpio_pin_get(pd_int, PD_INT_ASSERT)) 
    {
        Task_Code = 0x00;
        const struct device *i2c_pd = device_get_binding("I2C_pd");
        if (!i2c_pd) 
        {
            printk("Failed to get I2C device\n");
            return;
        }


        // Read PD event in PD reg[0x14]
        if (i2c_read(i2c_pd, &int_reg_buf, 1, PD_I2C_ADDR, PD_REG_INT_READ) != 0) 
		{
            printk("I2C read failed\n");
        } 
		else 
		{
            printk("Interrupt Register Value: 0x%x\n", int_reg_buf);

            // Get PD event and trigger PD task.
            if( PD_EV_CONTRACT_READY )
            {
                Task_Code = 0x01;
            }
            else
            {
                Task_Code = 0x00;
            }
        }
        
        // Clear all PD event in PD reg[0x18], that cause PD INT pin de-assert.
        PD_Write_Data(int_reg_clr, int_reg_clr_len, PD_REG_INT_CLEAR);
    }
    else
    {
        Task_Code = 0x00;
    }
}

void PD_Task_Service(void)
{
    switch(Task_Code)
    {
        case ENABLE_B2B_MOS:
            const uint8_t pd_b2bmos_open[] = {0x53, 0x52, 0x44, 0x59}; // PD 4CC CMD "SRDY"
            size_t pd_b2bmos_open_len = sizeof(pd_b2bmos_open);

            PD_Write_Data(pd_b2bmos_open, pd_b2bmos_open_len, PD_REG_4CC_CMD);
            break;
        default:
            break;
    }
}

void pd_thread(void *p1, void *p2, void *p3)
{
	uint32_t period = *(uint32_t *)p1;
    static uint8_t Task_Code = 0;

    PD_Initial();
	while (true) 
	{
		PD_Interrupt_Service();
    	PD_Task_Service();
        k_msleep(period);
	}
}
