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
#include "chg_mgmt.h"

LOG_MODULE_DECLARE(periph, CONFIG_PERIPHERAL_LOG_LEVEL);

// Define the global I2C mutex
K_MUTEX_DEFINE(i2c_mutex);

void CHG_Write_Data(const uint8_t *data, size_t len, const uint8_t reg)
{
    const struct device *i2c_chg;
    uint8_t i2c_write_buffer[20];
    int i,ret;

    i2c_chg = device_get_binding("I2C_pd");

    if (i2c_chg == NULL)
    {
        printk("Failed to get pd device\n");
        return;
    }

    i2c_write_buffer[0] = reg;
    memcpy(&i2c_write_buffer[1], data, len);

    // Acquire the I2C mutex lock to ensure exclusive access to I2C bus.
    k_mutex_lock(&i2c_mutex, K_FOREVER);

    ret = i2c_hub_write(0, i2c_write_buffer, len + 1, CHG_I2C_ADDR);

    // Release the I2C mutex lock after the I2C operation is done.
    k_mutex_unlock(&i2c_mutex);

    if (ret != 0)
    {
        printk("I2C write failed\n");
    }
    else
    {
        printk("I2C write success\n");
        for (i = 0; i < 20; i++)
        {
            i2c_write_buffer[i] = 0;
        }
    }
}

void Charger_Initial_Current(void)
{
    const uint8_t current_data[] = {0x33,0x13};
    size_t current_data_len = sizeof(current_data);

    CHG_Write_Data(current_data, current_data_len, CHG_REG_CURRENT);
}

void Charger_Initial_Voltage(void)
{
    const uint8_t voltage_data[] = {0x10,0x20};
    size_t voltage_data_len = sizeof(voltage_data);

    CHG_Write_Data(voltage_data, voltage_data_len, CHG_REG_VOLTAGE);
}


void Charger_Initial_CurrentLimit(void)
{
    const uint8_t CurrentLimit_data[] = {0x05,0xDC};
    size_t CurrentLimit_data_len = sizeof(CurrentLimit_data);

    CHG_Write_Data(CurrentLimit_data, CurrentLimit_data_len, CHG_REG_CP_POINT);
}

void Charger_Initial_AC_Prochot(void)
{
    const uint8_t ac_prochot_data[] = {0x35,0x01};
    size_t ac_prochot_data_len = sizeof(ac_prochot_data);

    CHG_Write_Data(ac_prochot_data, ac_prochot_data_len, CHG_REG_AC_PROCHOT);
}

void Charger_Initial_DC_Prochot(void)
{
    const uint8_t dc_prochot_data[] = {0x35,0x01};
    size_t dc_prochot_data_len = sizeof(dc_prochot_data);

    CHG_Write_Data(dc_prochot_data, dc_prochot_data_len, CHG_REG_DC_PROCHOT);
}
void Charger_Initial_PowerSources(void)
{
    gpio_dev_dc = device_get_binding(GPIO_DC_DETECT_CONTROLLER);
    gpio_dev_ac = device_get_binding(GPIO_AC_DETECT_CONTROLLER);
}

void Charger_Initial(void)
{
    // initial charger reference power source
    Charger_Initial_PowerSources();

    // initial item as below
    Charger_Initial_Current();
    Charger_Initial_Voltage();
    Charger_Initial_CurrentLimit();
    Charger_Initial_AC_Prochot();
    Charger_Initial_DC_Prochot();
}

void Charge_Control(void)
{
    bool ac_detected = gpio_pin_get(gpio_dev_dc, GPIO_AC_DETECT);
    bool dc_detected = gpio_pin_get(gpio_dev_dc, GPIO_DC_DETECT);

    if ( ac_detected && dc_detected )
    {
        const uint8_t ac_dc_charging_data[] = {0x33,0x13};
        size_t ac_dc_charging_data_len = sizeof(ac_dc_charging_data);
        CHG_Write_Data(ac_dc_charging_data, ac_dc_charging_data_len, CHG_REG_CURRENT);
    }
    else
    {
        const uint8_t dc_stop_charging_data[] = {0x00,0x00};
        size_t dc_stop_charging_data_len = sizeof(dc_stop_charging_data);
        CHG_Write_Data(dc_stop_charging_data, dc_stop_charging_data_len, CHG_REG_CURRENT);
    }
}

void chg_thread(void *p1, void *p2, void *p3)
{
	uint32_t period = *(uint32_t *)p1;

    Charger_Initial();

    while (true) 
	{
        Charge_Control();
        k_msleep(period);
	}
}
