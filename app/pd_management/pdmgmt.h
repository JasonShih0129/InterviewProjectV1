#ifndef __PD_MGMT_H__
#define __PD_MGMT_H__

// PD Device Inform
#define PD_I2C_ADDR             0x48

// PD Register
#define PD_REG_4CC_CMD          0x08
#define PD_REG_B2B_MOS_CONFIG   0x27
#define PD_REG_ROLE_CONFIG      0x28
#define PD_REG_INT_READ         0x14
#define PD_REG_INT_CLEAR        0x18

// PD interrupt pin behavior
#define PD_INT_ASSERT           0x00
#define PD_INT_DE_ASSERT        0x01
#define PD_EV_CONTRACT_READY    int_reg_buf[12] & 0x10

// PD task list
#define ENABLE_B2B_MOS          0x01


// Varriable
extern uint8_t Task_Code;

// PD function list
void PD_B2B_MOS_Configuration(void);
void PD_Power_Role_Configuration(void);
void PD_Write_Data(const uint8_t *data, size_t len,const uint8_t reg); 
void pd_thread(void *p1, void *p2, void *p3);
void PD_Task_Service(void);
void PD_Initial(void);
void PD_Interrupt_Service(void);
#endif /* __PD_MGMT_H__ */
