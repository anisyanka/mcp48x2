#include "mcp48x2.h"
#include <stdint.h>
#include <stddef.h>

#define REF_VOLTAGE_MV 2048

#if defined(DAC_TYPE_MCP4822)
 #define MAX_OUT_VALUE (4096UI)
#elif defined(DAC_TYPE_MCP4812)
 #define MAX_OUT_VALUE (1024UI)
#elif defined(DAC_TYPE_MCP4802)
 #define MAX_OUT_VALUE (256UI)
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
	dev->mode_gain_ch_a = MCP48X2_GAIN_2X;
	dev->mode_gain_ch_b = MCP48X2_GAIN_2X;

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

	dev->ch_a_val = 0;
	dev->ch_b_val = 0;
	dev->ll = ll;

	if (ch == MCP48X2_DAC_CH_A)
	{
		dev->mode_ch_a = mode;
		dev->mode_gain_ch_a = gain;
		dev->mode_ch_b = 0;
		dev->mode_gain_ch_b = 0;
	}
	else if (ch == MCP48X2_DAC_CH_B)
	{
		dev->mode_ch_a = 0;
		dev->mode_gain_ch_a = 0;
		dev->mode_ch_b = mode;
		dev->mode_gain_ch_b = gain;
	}

	return MCP48X2_OK;
}
