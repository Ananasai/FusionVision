
/* Flag operations */
#define SET_FLAG(variable, flag)    ((variable) |= (1U << (flag)))
#define CLEAR_FLAG(variable, flag)  ((variable) &= ~(1U << (flag)))
#define READ_FLAG(variable, flag)   (((variable) >> (flag)) & 1U)

#define LCD_WIDTH 480
#define LCD_HEIGTH 320

#define TERMO_RAW_WIDTH 160
#define TERMO_RAW_HEIGTH (120 - 4) /* TODO: figure out why 58 and 59 packets are bad */
