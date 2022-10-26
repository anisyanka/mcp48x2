#include "mcp48x2.h"
#include <stdint.h>
#include <stddef.h>

#define REF_VOLTAGE_MV 2048

#if defined(DAC_TYPE_MCP4822)
 #define MAX_OUT_VALUE (4095)
#elif defined(DAC_TYPE_MCP4812)
 #define MAX_OUT_VALUE (1023)
#elif defined(DAC_TYPE_MCP4802)
 #define MAX_OUT_VALUE (255)
#else
 #error "Please, define DAC type"
#endif

#define CHANNEL_BIT_POS 15
#define GAIN_BIT_POS 13
#define SHDN_BIT_POS 12

static mcp48x2_ret_t write_packet(mcp48x2_ll_t *ll, uint16_t data)
{
	mcp48x2_ret_t ret = MCP48X2_FAIL;

	ll->nss_reset();
	ret = ll->spi_write_packet(data);
	ll->nss_set();

	if (ret != MCP48X2_OK)
	{
		return MCP48X2_FAIL;
	}

	return MCP48X2_OK;
}

static void toggle_ldac(mcp48x2_ll_t *ll)
{
	ll->ldac_reset();
	ll->delay_us(100);
	ll->ldac_set();
}

/* Returns 0 in good case */
static int check_args_for_nulls(mcp48x2_device_t *dev, mcp48x2_ll_t *ll)
{
	if (!dev || !ll || !ll->delay_us ||
		!ll->ldac_reset || !ll->ldac_set ||
		!ll->nss_set || !ll->nss_reset ||
		!ll->spi_write_packet)
	{
		return 1;
	}

	return 0;
}

mcp48x2_ret_t mcp48x2_default_init(mcp48x2_device_t *dev, mcp48x2_ll_t *ll)
{
	uint16_t data = 0;

	if (check_args_for_nulls(dev, ll))
	{
		return MCP48X2_FAIL;
	}

	/* channel A */
	data |= ((uint16_t)MCP48X2_CH_ACTIVE << SHDN_BIT_POS);
	if (write_packet(ll, data) == MCP48X2_FAIL)
	{
		return MCP48X2_FAIL;
	}

	/* channel B */
	data |= ((uint16_t)MCP48X2_DAC_CH_B << CHANNEL_BIT_POS) | \
			((uint16_t)MCP48X2_CH_ACTIVE << SHDN_BIT_POS);
	if (write_packet(ll, data) == MCP48X2_FAIL)
	{
		return MCP48X2_FAIL;
	}

	toggle_ldac(ll);

	dev->ch_a_val = 0;
	dev->ch_b_val = 0;
	dev->ll = ll;
	dev->mode_ch_a = MCP48X2_CH_ACTIVE;
	dev->mode_ch_b = MCP48X2_CH_ACTIVE;
	dev->gain_ch_a = MCP48X2_GAIN_2X;
	dev->gain_ch_b = MCP48X2_GAIN_2X;

	return MCP48X2_OK;
}

mcp48x2_ret_t mcp48x2_init_channel(mcp48x2_device_t *dev, mcp48x2_ll_t *ll,
                                   mcp48x2_ch_t ch, mcp48x2_ch_mode_t mode,
                                   mcp48x2_gain_t gain)
{
	uint16_t data = 0;

	if (check_args_for_nulls(dev, ll))
	{
		return MCP48X2_FAIL;
	}

	data |= (((uint16_t)ch << CHANNEL_BIT_POS) | \
			 ((uint16_t)gain << GAIN_BIT_POS) | \
			 ((uint16_t)mode << SHDN_BIT_POS));
	if (write_packet(ll, data) == MCP48X2_FAIL)
	{
		return MCP48X2_FAIL;
	}

	toggle_ldac(ll);

	dev->ll = ll;

	if (ch == MCP48X2_DAC_CH_A)
	{
		dev->ch_a_val = 0;
		dev->mode_ch_a = mode;
		dev->gain_ch_a = gain;
	}
	else if (ch == MCP48X2_DAC_CH_B)
	{
		dev->ch_b_val = 0;
		dev->mode_ch_b = mode;
		dev->gain_ch_b = gain;
	}

	return MCP48X2_OK;
}

mcp48x2_ret_t mcp48x2_set_channel_value(mcp48x2_device_t *dev,
                                        mcp48x2_ch_t ch, uint16_t val)

{
	uint16_t data = val;
	mcp48x2_gain_t gain = 0;
	mcp48x2_ch_mode_t mode = 0;

	if (!dev || val > MAX_OUT_VALUE)
	{
		return MCP48X2_FAIL;
	}

	gain = (ch == MCP48X2_DAC_CH_A) ? (dev->gain_ch_a) : (dev->gain_ch_b);
	mode = (ch == MCP48X2_DAC_CH_A) ? (dev->mode_ch_a) : (dev->mode_ch_b);

	data |= (((uint16_t)ch << CHANNEL_BIT_POS) | \
			 ((uint16_t)gain << GAIN_BIT_POS) | \
			 ((uint16_t)mode << SHDN_BIT_POS));
	if (write_packet(dev->ll, data) == MCP48X2_FAIL)
	{
		return MCP48X2_FAIL;
	}

	toggle_ldac(dev->ll);

	if (ch == MCP48X2_DAC_CH_A)
	{
		dev->ch_a_val = val;
	}
	else if (ch == MCP48X2_DAC_CH_B)
	{
		dev->ch_b_val = val;
	}

	return MCP48X2_OK;
}

mcp48x2_ret_t mcp48x2_set_channel_mode(mcp48x2_device_t *dev,
                                       mcp48x2_ch_t ch,
                                       mcp48x2_ch_mode_t mode)
{
	if (!dev)
	{
		return MCP48X2_FAIL;
	}

	uint16_t data = (ch == MCP48X2_DAC_CH_A) ? (dev->ch_a_val) : (dev->ch_b_val);
	mcp48x2_gain_t gain = (ch == MCP48X2_DAC_CH_A) ? (dev->gain_ch_a) : (dev->gain_ch_b);

	data |= (((uint16_t)ch << CHANNEL_BIT_POS) | \
			 ((uint16_t)gain << GAIN_BIT_POS) | \
			 ((uint16_t)mode << SHDN_BIT_POS));
	if (write_packet(dev->ll, data) == MCP48X2_FAIL)
	{
		return MCP48X2_FAIL;
	}

	toggle_ldac(dev->ll);

	if (ch == MCP48X2_DAC_CH_A)
	{
		dev->mode_ch_a = mode;
	}
	else if (ch == MCP48X2_DAC_CH_B)
	{
		dev->mode_ch_b = mode;
	}

	return MCP48X2_OK;
}

mcp48x2_ret_t mcp48x2_set_channel_gain(mcp48x2_device_t *dev,
                                       mcp48x2_ch_t ch,
                                       mcp48x2_gain_t gain)
{
	if (!dev)
	{
		return MCP48X2_FAIL;
	}

	return MCP48X2_OK;
}

mcp48x2_ret_t mcp48x2_set_channel_values_sync(mcp48x2_device_t *dev,
                                              uint16_t val_ch_a,
                                              uint16_t val_ch_b)
{
	if (!dev)
	{
		return MCP48X2_FAIL;
	}

	// uint16_t data_a = val_ch_a;
	// uint16_t data_b = val_ch_b;

	// mcp48x2_gain_t gain_a = 0;
	// mcp48x2_gain_t gain_b = 0;

	// mcp48x2_ch_mode_t mode_a = 0;
	// mcp48x2_ch_mode_t mode_b = 0;

	// /* channel A */
	// data |= ((uint16_t)MCP48X2_CH_ACTIVE << SHDN_BIT_POS);
	// if (write_packet(ll, data) == MCP48X2_FAIL)
	// {
	// 	return MCP48X2_FAIL;
	// }

	// /* channel B */
	// data |= ((uint16_t)MCP48X2_DAC_CH_B << CHANNEL_BIT_POS) | //*&\*/
	// 		((uint16_t)MCP48X2_CH_ACTIVE << SHDN_BIT_POS);
	// if (write_packet(ll, data) == MCP48X2_FAIL)
	// {
	// 	return MCP48X2_FAIL;
	// }

	// toggle_ldac(ll);

	return MCP48X2_OK;
}
