#ifndef __PD_MGMT_H__
#define __PD_MGMT_H__

// Charger Device Inform
#define CHG_I2C_ADDR             0x12

// Charger Register
#define CHG_REG_CURRENT         0x14
#define CHG_REG_VOLTAGE         0x15
#define CHG_REG_CP_POINT        0x3F
#define CHG_REG_AC_PROCHOT      0x48
#define CHG_REG_DC_PROCHOT      0x49

// Charger Power Source definication
extern const struct device *gpio_dev_dc;
extern const struct device *gpio_dev_ac;
#define GPIO_DC_DETECT_CONTROLLER "GPIO_1"
#define GPIO_AC_DETECT_CONTROLLER "GPIO_2"

// PD function list
void Charger_Initial(void);
void Charge_Control(void);
void CHG_Write_Data(const uint8_t *data, size_t len,const uint8_t reg); 
void chg_thread(void *p1, void *p2, void *p3);
void Charger_Initial_Current(void);
void Charger_Initial_Voltage(void);
void Charger_Initial_CurrentLimit(void);
void Charger_Initial_AC_Prochot(void);
void Charger_Initial_DC_Prochot(void);
void Charger_Initial_PowerSources(void);
#endif /* __CHG_MGMT_H__ */
