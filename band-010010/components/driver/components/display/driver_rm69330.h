#ifndef _DRIVER_RM69330_H
#define _DRIVER_RM69330_H

#include <stdint.h>

void rm69330_init(void);

void rm69330_display(uint16_t x_s, uint16_t x_e, uint16_t y_s, uint16_t y_e, uint16_t *data);

#endif  // _DRIVER_RM69330_H

