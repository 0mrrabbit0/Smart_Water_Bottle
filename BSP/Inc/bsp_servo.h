/**
 * @file    bsp_servo.h
 * @brief   SG90 servo motor driver (TIM3 PWM)
 */

#ifndef __BSP_SERVO_H__
#define __BSP_SERVO_H__

#include <stdint.h>

void BSP_Servo_Init(void);
void BSP_Servo_SetAngle(uint8_t angle);
void BSP_Servo_Lock(void);
void BSP_Servo_Unlock(void);

#endif /* __BSP_SERVO_H__ */
