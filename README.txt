--------------- Hardware connection ---------------
OLED		BBB
------------------------
VCC 	: 	3V3
SCl 	: 	I2C1_SCL
SDA 	: 	I2C1_SDA
GND	:	GND

- How to use:
$ insmod i2c_omap.ko
$ ls /dev/i2c-1
$ insmod i2c_client.ko


---------------- I2C Bus Driver   -----------------
1. Edit i2c1 node:
                         i2c1: i2c@0 {
                                 compatible = "ti,demo-i2c";
                                 #address-cells = <1>;
                                 #size-cells = <0>;
                                 reg = <0x0 0x1000>;
                                 interrupts = <71>;
                                 status = "disabled";
                         };

------ How to Program I2C Bus Driver: ------
- Module Configuration Before Enabling the Module: 
1. Program the prescaler to obtain an approximately 12-MHz I2C module clock (I2C_PSC = x; this value
is to be calculated and is dependent on the System clock frequency).
2. Program the I2C clock to obtain 100 Kbps or 400 Kbps (SCLL = x and SCLH = x; these values are to
be calculated and are dependent on the System clock frequency).
3. Configure its own address (I2C_OA = x) - only in case of I2C operating mode (F/S mode).
4. Take the I2C module out of reset (I2C_CON:I2C_EN = 1).

- Initialization Procedure:
1. Configure the I2C mode register (I2C_CON) bits.
2. Enable interrupt masks (I2C_IRQENABLE_SET), if using interrupt for transmit/receive data.
3. Enable the DMA (I2C_BUF and I2C_DMA/RX/TX/ENABLE_SET) and program the DMA controller) -
only in case of I2C operating mode (F/S mode), if using DMA for transmit/receive data.

- Configure Slave Address and DATA Counter Registers:
In master mode, configure the slave address (I2C_SA = x) and the number of byte associated with the
transfer (I2C_CNT = x).

- Initiate a Transfer:
Poll the bus busy (BB) bit in the I2C status register (I2C_IRQSTATUS_RAW). If it is cleared to 0 (bus not
busy), configure START/STOP (I2C_CON: STT / I2C_CON: STP condition to initiate a transfer) - only in
case of I2C operating mode (F/S mode).

- Receive Data:
Poll the receive data ready interrupt flag bit (RRDY) in the I2C status register (I2C_IRQSTATUS_RAW),
use the RRDY interrupt (I2C_IRQENABLE_SET.RRDY_IE set) or use the DMA RX (I2C_BUF.RDMA_EN
set together with I2C_DMARXENABLE_SET) to read the received data in the data receive register
(I2C_DATA). Use draining feature (I2C_IRQSTATUS_RAW.RDR enabled by
I2C_IRQENABLE_SET.RDR_IE)) if the transfer length is not equal with FIFO threshold.

- Transmit Data:
Poll the transmit data ready interrupt flag bit (XRDY) in the I2C status register (I2C_IRQSTATUS_RAW),
use the XRDY interrupt (I2C_IRQENABLE_SET.XRDY_IE set) or use the DMA TX (I2C_BUF.XDMA_EN
set together with I2C_DMATXENABLE_SET) to write data into the data transmit register (I2C_DATA). Use
draining feature (I2C_IRQSTATUS_RAW.XDR enabled by I2C_IRQENABLE_SET.XDR_IE)) if the transfer
length is not equal with FIFO threshold.


---------------- I2C Client Driver -----------------
1. Add the pin ctrl for i2c1 node:

       i2c1_pins: pinmux_i2c1_pins {
               pinctrl-single,pins = <
                       AM33XX_PADCONF(AM335X_PIN_SPI0_D1, PIN_INPUT_PULLUP, MUX_MODE2)
                       AM33XX_PADCONF(AM335X_PIN_SPI0_CS0, PIN_INPUT_PULLUP, MUX_MODE2)     
               >;
       };

2. Enable I2C bus node and add the i2c client device node:

&i2c1 {
       pinctrl-names = "default";
       pinctrl-0 = <&i2c1_pins>;
       status = "okay";
       clock-frequency = <100000>;
       ssd1306: ssd1306@3c {
               compatible = "solomon,ssd1306";
               reg = <0x3C>;
               #address-cells = <1>;
               #size-cells = <1>;
       };
};


