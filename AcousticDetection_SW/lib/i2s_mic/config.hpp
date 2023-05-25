#ifndef I2S_MIC_CONFIG_HPP
#define I2S_MIC_CONFIG_HPP

// I2S pinout
#define I2S_MIC_BCLK_PIN GPIO_NUM_19   // BCLK
#define I2S_MIC_LRCL_PIN GPIO_NUM_18   // WS
#define I2S_MIC_DOUT_PIN GPIO_NUM_21   // it's DIN for ESP32 and DOUT for SPH0645

// I2S settings (for DMA)
#define I2S_SAMPLE_RATE 44100  // 44.1 kHz
#define I2S_DMA_BUF_COUNT 14   // see: https://www.atomic14.com/2021/04/20/esp32-i2s-dma-buf-len-buf-count.html
#define I2S_DMA_BUF_LEN 1024


#endif