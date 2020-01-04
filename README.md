# The gif_pixel

![gif_pixel_picture](https://github.com/akerlund/gif_pixel/blob/master/gif_pixel.jpg)

The gif_pixel has an STM32F4 Discovery board inside.

A Python script extracts the RGB data from a .gif (16x16)px and
serializes the data and sends to the MCU.

The MCU in turn buffers the data and forwards it to SPI with DMA.

SPI controls the LED drivers which in turn controls 2 rows at the time,
thus it is connected like a matrix.

MOSFETs are used to select which two rows are connected to the LED drivers.
It all goes really fast so it looks like all are on at the same time.
