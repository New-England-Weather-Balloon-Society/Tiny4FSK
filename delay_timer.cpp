#include "delay_timer.h"

volatile bool tc3Flag = false;

void TC3_Handler() {
  // Check for the compare match interrupt
  if (TC3->COUNT16.INTFLAG.bit.MC0) {
    // Clear the interrupt flag
    TC3->COUNT16.INTFLAG.reg = TC_INTFLAG_MC0;
    tc3Flag = true;
  }
}

void setupTC3(uint16_t delay_ms) {
  // Enable the TC3 module
  PM->APBCMASK.reg |= PM_APBCMASK_TC3;

  // Enable the XOSC32K clock
  SYSCTRL->XOSC32K.reg = SYSCTRL_XOSC32K_ENABLE | SYSCTRL_XOSC32K_XTALEN | SYSCTRL_XOSC32K_EN32K;
  while (SYSCTRL->PCLKSR.bit.XOSC32KRDY == 0)
    ;

  // Configure GCLK1 to use XOSC32K as source
  GCLK->GENDIV.reg = GCLK_GENDIV_ID(1) | GCLK_GENDIV_DIV(1);
  while (GCLK->STATUS.bit.SYNCBUSY)
    ;

  GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(1) | GCLK_GENCTRL_SRC_XOSC32K | GCLK_GENCTRL_GENEN;
  while (GCLK->STATUS.bit.SYNCBUSY)
    ;

  // Connect GCLK1 to TC3
  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_TCC2_TC3 | GCLK_CLKCTRL_GEN_GCLK1 | GCLK_CLKCTRL_CLKEN;
  while (GCLK->STATUS.bit.SYNCBUSY)
    ;

  // Reset the TC3 module
  TC3->COUNT16.CTRLA.reg = TC_CTRLA_SWRST;
  while (TC3->COUNT16.STATUS.bit.SYNCBUSY)
    ;
  while (TC3->COUNT16.CTRLA.bit.SWRST)
    ;

  // Set TC3 to 16-bit mode
  TC3->COUNT16.CTRLA.reg = TC_CTRLA_MODE_COUNT16;

  // Set prescaler to 1, TC3 frequency will be 32.768kHz
  TC3->COUNT16.CTRLA.reg |= TC_CTRLA_PRESCALER_DIV1;
  // Calculate compare value for the desired delay
  // For example, for 10 ms: (32768 / 1000) * 10 = 327
  uint16_t compareValue = 301;

  // Set compare value
  TC3->COUNT16.CC[0].reg = compareValue;
  while (TC3->COUNT16.STATUS.bit.SYNCBUSY)
    ;

  // Enable the compare match interrupt
  TC3->COUNT16.INTENSET.reg = TC_INTENSET_MC0;

  // Enable the TC3 interrupt in NVIC
  NVIC_EnableIRQ(TC3_IRQn);

  // Enable TC3
  TC3->COUNT16.CTRLA.reg |= TC_CTRLA_ENABLE;
  while (TC3->COUNT16.STATUS.bit.SYNCBUSY)
    ;
}

void delayWithTC3(uint16_t delay_ms) {
  tc3Flag = false;  // Reset the flag
  setupTC3(delay_ms);
  while (!tc3Flag)
    ;  // Wait for the flag to be set by the ISR
}
