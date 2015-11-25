# Copyright (c) 2015, the Fletch project authors. Please see the AUTHORS file
# for details. All rights reserved. Use of this source code is governed by a
# BSD-style license that can be found in the LICENSE.md file.

{
  'variables': {
    'stm32_cube_f7_hal': '<(stm32_cube_f7)/Drivers/STM32F7xx_HAL_Driver/',
    'stm32_cube_f7_hal_src': '<(stm32_cube_f7_hal)/Src/',
  },
  'include_dirs': [
    '<(stm32_cube_f7_hal)/Inc/',
  ],
  'sources': [
    # All files from the HAL source directory.
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_adc.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_adc_ex.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_can.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_cec.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_cortex.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_crc.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_crc_ex.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_cryp.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_cryp_ex.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_dac.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_dac_ex.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_dcmi.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_dcmi_ex.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_dma2d.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_dma.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_dma_ex.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_eth.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_flash.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_flash_ex.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_gpio.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_hash.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_hash_ex.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_hcd.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_i2c.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_i2c_ex.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_i2s.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_irda.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_iwdg.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_lptim.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_ltdc.c',
    # Don't include the template file.
    #'<(stm32_cube_f7_hal_src)/stm32f7xx_hal_msp_template.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_nand.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_nor.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_pcd.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_pcd_ex.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_pwr.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_pwr_ex.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_qspi.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_rcc.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_rcc_ex.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_rng.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_rtc.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_rtc_ex.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_sai.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_sai_ex.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_sd.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_sdram.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_smartcard.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_smartcard_ex.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_spdifrx.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_spi.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_sram.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_tim.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_tim_ex.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_uart.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_usart.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_hal_wwdg.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_ll_fmc.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_ll_sdmmc.c',
    '<(stm32_cube_f7_hal_src)/stm32f7xx_ll_usb.c',
  ],
}
