# The gif_pixel

![gif_pixel_picture](https://github.com/akerlund/rgb_driver_3ch/blob/master/soldered_an_ready.jpg)

STM32F4 Discovery board inside
A python script extract the RGB data from a .gif (16x16)px and
serializes the data and sends to the MCU.

The MCU in turn buffers the data and forwards it to SPI with DMA.

