
/* Flag operations */
#define SET_FLAG(variable, flag)    ((variable) |= (1U << (flag)))
#define CLEAR_FLAG(variable, flag)  ((variable) &= ~(1U << (flag)))
#define READ_FLAG(variable, flag)   (((variable) >> (flag)) & 1U)

#define LCD_WIDTH 480
#define LCD_HEIGTH 320
