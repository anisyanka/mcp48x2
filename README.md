#  MCP48x2 DAC SPI library

## Quick start
**1. Add submodule to your project**
```bash
git submodule add git@github.com:anisyanka/mcp48x2.git /third/party/folder
```
**2. Add defenition for a DAC type and sources**
```make
# C defines
C_DEFS += -DDAC_TYPE_MCP4822
C_SOURCES += /third/party/folder/mcp48x2.c
C_INCLUDES += /third/party/folder
```
For now `DAC_TYPE_MCP4822` (12 bit DAC), `DAC_TYPE_MCP4812` (10 bit DAC) and `DAC_TYPE_MCP4802` (8 bit DAC) are only supported.

**3. Call default initialization function**
```C
mcp48x2_ret_t mcp48x2_default_init(mcp48x2_device_t *dev, mcp48x2_ll_t *ll);
```
Default setup is:
 * both channels are in active mode
 * gain factor equals 2 for both channels
 * initial output voltages are set to zero for both channels.

You can use other init function from the header file if the default setup doesn't fit you. 

`dev` - describes one instance of your DACs on your board. The board can contain plenty of DACs. This param helps you to control each instance separetally.

`ll` - describes low level functions to allow the library to contorl hardware. In this case to control GPIOs and SPI. It looks like this:
```C
typedef struct
{
	/* Manage NSS pin */
	void (*nss_set)(void);
	void (*nss_reset)(void);

	/* Manage LDAC pin */
	void (*ldac_set)(void);
	void (*ldac_reset)(void);
	void (*delay_us)(uint32_t us);

	/* Transmit 16 bits with SPI */
	mcp48x2_ret_t (*spi_write_packet)(uint16_t data);
} mcp48x2_ll_t;
```
You have to implement all these function by yourself and pass it to the library.
The lib is able to work on different hardwares with the help of this.

**4. Enjoy using the DAC**

Sey output voltage with the help of:
```
mcp48x2_ret_t mcp48x2_set_channel_value(mcp48x2_device_t *dev,
                                        mcp48x2_ch_t ch, uint16_t val);

```
Remember that:

`Vout = (2048 * val * G) / 2^(DAC resolution)`
 * 	2048 - internal reference voltage
 *	G - gain factor
 * 	val = [0 - 4095] for MCP4922
 *	val = [0 - 1023] for MCP4912
 *	val = [0 - 255]  for MCP4902