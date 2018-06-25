#ifndef ULTRALIGHT_H
#define ULTRALIGHT_H

uint8_t Ultra_Read(uint8_t page,uint8_t *buff);
uint8_t Ultra_Write(uint8_t page,uint8_t *buff);
uint8_t Ultra_PwdAuth(uint8_t pwd[4], uint8_t pack[2]);

#endif

