#ifndef __PD_MGMT_H__
#define __PD_MGMT_H__

#define PD_I2C_ADDR             0x48
#define PD_REG_B2B_MOS_CONFIG   0x27
#define PD_REG_ROLE_CONFIG      0x28
#define PD_REG_INT_READ         0x14
#define PD_REG_INT_RELEASE      0x18
#define PD_INT_ASSERT           0x00

void PD_B2B_MOS_Configuration(void);
void PD_Power_Role_Configuration(void);
void PD_Write_Data(const uint8_t *data, size_t len,const uint8_t reg); 
void pd_thread(void *p1, void *p2, void *p3);
void PD_Initial(void);
void PD_Interrupt_Service(void);
#endif /* __PD_MGMT_H__ */
