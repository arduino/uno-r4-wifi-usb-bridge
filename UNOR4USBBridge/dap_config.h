// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2017-2022, Alex Taradov <alex@taradov.com>. All rights reserved.

#ifndef _DAP_CONFIG_H_
#define _DAP_CONFIG_H_

/*- Includes ----------------------------------------------------------------*/
#define CONFIG_BRIDGE_GPIO_BOOT     9
#define CONFIG_BRIDGE_GPIO_RST      4

/*- Definitions -------------------------------------------------------------*/
#define DAP_CONFIG_DEFAULT_PORT        DAP_PORT_SWD
#define DAP_CONFIG_DEFAULT_CLOCK       1000000 // Hz

#define DAP_CONFIG_PACKET_SIZE         64
#define DAP_CONFIG_PACKET_COUNT        2

#define DAP_CONFIG_JTAG_DEV_COUNT      8

//#define DAP_CONFIG_RESET_TARGET_FN     target_specific_reset_function
//#define DAP_CONFIG_VENDOR_FN           vendor_command_handler_function

// Attribute to use for performance-critical functions
#define DAP_CONFIG_PERFORMANCE_ATTR    //__attribute__((section(".ramfunc")))

// A value at which dap_clock_test() produces 1 kHz output on the SWCLK pin
#define DAP_CONFIG_DELAY_CONSTANT      7700

// A threshold for switching to fast clock (no added delays)
// This is the frequency produced by dap_clock_test(1) on the SWCLK pin
#define DAP_CONFIG_FAST_CLOCK          2400000 // Hz

#define DAP_CONFIG_VENDOR_STR          "Arduino"
#define DAP_CONFIG_PRODUCT_STR         "Santiago CMSIS-DAP Adapter"
#define DAP_CONFIG_SER_NUM_STR         "123456789101112"
#define DAP_CONFIG_CMSIS_DAP_VER_STR   "2.0.0"

/*- Prototypes --------------------------------------------------------------*/
extern char usb_serial_number[16];

/*- Implementations ---------------------------------------------------------*/

#include "driver/gpio.h"

#define CONFIG_BRIDGE_GPIO_SWDIO      8
#define CONFIG_BRIDGE_GPIO_SWCLK      7

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWCLK_TCK_write(int value)
{
    gpio_set_level(CONFIG_BRIDGE_GPIO_SWCLK, value);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWDIO_TMS_write(int value)
{
    gpio_set_level(CONFIG_BRIDGE_GPIO_SWDIO, value);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_TDI_write(int value)
{
#ifdef DAP_CONFIG_ENABLE_JTAG
  HAL_GPIO_TDI_write(value);
#else
  (void)value;
#endif
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_TDO_write(int value)
{
#ifdef DAP_CONFIG_ENABLE_JTAG
  HAL_GPIO_TDO_write(value);
#else
  (void)value;
#endif
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_nTRST_write(int value)
{
  (void)value;
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_nRESET_write(int value)
{
    gpio_set_level(CONFIG_BRIDGE_GPIO_RST, value);
}

//-----------------------------------------------------------------------------
static inline int DAP_CONFIG_SWCLK_TCK_read(void)
{
    return gpio_get_level(CONFIG_BRIDGE_GPIO_SWCLK);
}

//-----------------------------------------------------------------------------
static inline int DAP_CONFIG_SWDIO_TMS_read(void)
{
    return gpio_get_level(CONFIG_BRIDGE_GPIO_SWDIO);
}

//-----------------------------------------------------------------------------
static inline int DAP_CONFIG_TDO_read(void)
{
#ifdef DAP_CONFIG_ENABLE_JTAG
  return HAL_GPIO_TDO_read();
#else
  return 0;
#endif
}

//-----------------------------------------------------------------------------
static inline int DAP_CONFIG_TDI_read(void)
{
#ifdef DAP_CONFIG_ENABLE_JTAG
  return HAL_GPIO_TDI_read();
#else
  return 0;
#endif
}

//-----------------------------------------------------------------------------
static inline int DAP_CONFIG_nTRST_read(void)
{
  return 0;
}

//-----------------------------------------------------------------------------
static inline int DAP_CONFIG_nRESET_read(void)
{
    return gpio_get_level(CONFIG_BRIDGE_GPIO_RST);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWCLK_TCK_set(void)
{
  gpio_set_level(CONFIG_BRIDGE_GPIO_SWCLK, true);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWCLK_TCK_clr(void)
{
  gpio_set_level(CONFIG_BRIDGE_GPIO_SWCLK, false);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWDIO_TMS_in(void)
{
    gpio_set_direction(CONFIG_BRIDGE_GPIO_SWDIO, GPIO_MODE_INPUT);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWDIO_TMS_out(void)
{
    gpio_set_direction(CONFIG_BRIDGE_GPIO_SWDIO, GPIO_MODE_OUTPUT);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SETUP(void)
{
    gpio_config_t io_conf = {};
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << CONFIG_BRIDGE_GPIO_SWDIO) | (1ULL << CONFIG_BRIDGE_GPIO_SWCLK);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
#ifdef DAP_CONFIG_ENABLE_JTAG
  HAL_GPIO_TDO_in();
  HAL_GPIO_TDI_in();
#endif
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_DISCONNECT(void)
{
  gpio_set_direction(CONFIG_BRIDGE_GPIO_SWCLK, GPIO_MODE_INPUT);
  gpio_set_direction(CONFIG_BRIDGE_GPIO_SWDIO, GPIO_MODE_INPUT);
  gpio_set_direction(CONFIG_BRIDGE_GPIO_RST, GPIO_MODE_INPUT);
#ifdef DAP_CONFIG_ENABLE_JTAG
  HAL_GPIO_TDO_in();
  HAL_GPIO_TDI_in();
#endif
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_CONNECT_SWD(void)
{
  gpio_set_direction(CONFIG_BRIDGE_GPIO_SWDIO, GPIO_MODE_OUTPUT);
  gpio_set_level(CONFIG_BRIDGE_GPIO_SWDIO, true);

  gpio_set_direction(CONFIG_BRIDGE_GPIO_SWCLK, GPIO_MODE_OUTPUT);
  gpio_set_level(CONFIG_BRIDGE_GPIO_SWCLK, true);

  gpio_set_direction(CONFIG_BRIDGE_GPIO_RST, GPIO_MODE_OUTPUT);
  gpio_set_level(CONFIG_BRIDGE_GPIO_RST, true);

#ifdef DAP_CONFIG_ENABLE_JTAG
  HAL_GPIO_TDO_in();
  HAL_GPIO_TDI_in();
#endif
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_CONNECT_JTAG(void)
{
#ifdef DAP_CONFIG_ENABLE_JTAG
  HAL_GPIO_TDO_in();

  HAL_GPIO_TDI_out();
  HAL_GPIO_TDI_set();
#endif
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_LED(int index, int state)
{
  (void)index;
  (void)state;
}

//-----------------------------------------------------------------------------
__attribute__((always_inline))
static inline void DAP_CONFIG_DELAY(uint32_t cycles)
{
  volatile int i = cycles;
  while (i-- > 0) {
    __asm__ __volatile__ ("nop");
  }
}

#endif // _DAP_CONFIG_H_