// SPDX-License-Identifier: GPL-2.0
/* Copyright Sunplus Technology Co., Ltd.
 *       All rights reserved.
 *
 * ov9281 Image Sensor Driver
 *
 */

#include <linux/clk.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/pm_runtime.h>
#include <linux/regulator/consumer.h>
#include <linux/sysfs.h>
#include <media/media-entity.h>
#include <media/v4l2-async.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-subdev.h>
#include <linux/pinctrl/consumer.h>
#include "ov9281.h"


static const struct ov9281_mode_info ov9281_mode_data[OV9281_NUM_MODES] = {
	{OV9281_MODE_WXGA_1280_800, SUBSAMPLING,
	 1280, 1280, 800, 800},
};

static const int ov9281_framerates[] = {
	[OV9281_30_FPS] = 30,
};

// GREY, RAW10
static const struct regval ov9281_grey_1280x800_regs[] = {
	{0x0103, 0x01}, {0x0302, 0x32}, {0x030d, 0x50}, {0x030e, 0x02},
	{0x3001, 0x00}, {0x3004, 0x00}, {0x3005, 0x00}, {0x3006, 0x04},
	{0x3011, 0x0a}, {0x3013, 0x18}, {0x301c, 0xf0}, {0x3022, 0x01},
	{0x3030, 0x10}, {0x3039, 0x32}, {0x303a, 0x00}, {0x3500, 0x00},
	{0x3501, 0x2a}, {0x3502, 0x90}, {0x3503, 0x08}, {0x3505, 0x8c},
	{0x3507, 0x03}, {0x3508, 0x00}, {0x3509, 0x10}, {0x3610, 0x80},
	{0x3611, 0xa0}, {0x3620, 0x6e}, {0x3632, 0x56}, {0x3633, 0x78},
	{0x3662, 0x05}, {0x3666, 0x00}, {0x366f, 0x5a}, {0x3680, 0x84},
	{0x3712, 0x80}, {0x372d, 0x22}, {0x3731, 0x80}, {0x3732, 0x30},
	{0x3778, 0x00}, {0x377d, 0x22}, {0x3788, 0x02}, {0x3789, 0xa4},
	{0x378a, 0x00}, {0x378b, 0x4a}, {0x3799, 0x20}, {0x3800, 0x00},
	{0x3801, 0x00}, {0x3802, 0x00}, {0x3803, 0x00}, {0x3804, 0x05},
	{0x3805, 0x0f}, {0x3806, 0x03}, {0x3807, 0x2f}, {0x3808, 0x05},
	{0x3809, 0x00}, {0x380a, 0x03}, {0x380b, 0x20}, {0x380c, 0x02},
	{0x380d, 0xd8}, {0x380e, 0x03}, {0x380f, 0x8e}, {0x3810, 0x00},
	{0x3811, 0x08}, {0x3812, 0x00}, {0x3813, 0x08}, {0x3814, 0x11},
	{0x3815, 0x11}, {0x3820, 0x04}, {0x3821, 0x04}, {0x382c, 0x05},
	{0x382d, 0xb0}, {0x389d, 0x00}, {0x3881, 0x42}, {0x3882, 0x01},
	{0x3883, 0x00}, {0x3885, 0x02}, {0x38a8, 0x02}, {0x38a9, 0x80},
	{0x38b1, 0x00}, {0x38b3, 0x02}, {0x38c4, 0x00}, {0x38c5, 0xc0},
	{0x38c6, 0x04}, {0x38c7, 0x80}, {0x3920, 0xff}, {0x4003, 0x40},
	{0x4008, 0x04}, {0x4009, 0x0b}, {0x400c, 0x00}, {0x400d, 0x07},
	{0x4010, 0x40}, {0x4043, 0x40}, {0x4307, 0x30}, {0x4317, 0x00},
	{0x4501, 0x00}, {0x4507, 0x00}, {0x4509, 0x00}, {0x450a, 0x08},
	{0x4601, 0x04}, {0x470f, 0x00}, {0x4f07, 0x00}, {0x4800, 0x00},
	{0x5000, 0x9f}, {0x5001, 0x00}, {0x5e00, 0x00}, {0x5d00, 0x07},
	{0x5d01, 0x00}, {0x4f00, 0x04}, {0x4f10, 0x00}, {0x4f11, 0x98},
	{0x4f12, 0x0f}, {0x4f13, 0xc4}, {0x0100, 0x01},
	{0x3501, 0x38},
	{0x3502, 0x20},
	{0x5000, 0x87},

	// from 120fps to 60fps
	//{0x380e, 0x07}, {0x380f, 0x1c},

	// from 120fps to 30fps
	//{0x380e, 0x0e}, {0x380f, 0x38},

	// from raw10 to raw8
	//{0x0100, 0x00}, {0x3662, 0x07}, {0x4837, 0x14},  {0x4601, 0x4f},
	//{0x0100, 0x01},

	{REG_NULL, 0x00}
};


static const struct regval ov9281_start_settings[] = {
	{0x0100, 0x01},
	{REG_NULL, 0x00}
};

static const struct regval ov9281_stop_settings[] = {
	{0x0100, 0x00},
	{REG_NULL, 0x00}
};

struct ov9281_pixfmt {
	u32 code;
	u32 colorspace;
};

static const struct ov9281_pixfmt ov9281_formats[] = {
	{ MEDIA_BUS_FMT_Y10_1X10, V4L2_COLORSPACE_SRGB, },
};

/* Read registers up to 4 at a time */
static int ov9281_read_reg(struct i2c_client *client, u16 reg, unsigned int len, u32 *val)
{
	struct i2c_msg msgs[2];
	u8 *data_be_p;
	__be32 data_be = 0;
	__be16 reg_addr_be = cpu_to_be16(reg);
	int ret;

	if ((len > 4) || !len)
		return -EINVAL;

	data_be_p = (u8 *)&data_be;

	/* Write register address */
	msgs[0].addr = client->addr;
	msgs[0].flags = 0;
	msgs[0].len = 2;
	msgs[0].buf = (u8 *)&reg_addr_be;

	/* Read data from register */
	msgs[1].addr = client->addr;
	msgs[1].flags = I2C_M_RD;
	msgs[1].len = len;
	msgs[1].buf = &data_be_p[4 - len];

	ret = i2c_transfer(client->adapter, msgs, ARRAY_SIZE(msgs));

	if (ret != ARRAY_SIZE(msgs))
		return -EIO;

	*val = be32_to_cpu(data_be);
	return 0;
}

/* Write registers up to 4 at a time */
static int ov9281_write_reg(struct i2c_client *client, u16 reg, u32 len, u32 val)
{
	u32 buf_i, val_i;
	u8 buf[6];
	u8 *val_p;
	__be32 val_be;

	if (len > 4)
		return -EINVAL;

	buf[0] = reg >> 8;
	buf[1] = reg & 0xff;

	val_be = cpu_to_be32(val);
	val_p = (u8 *)&val_be;
	buf_i = 2;
	val_i = 4 - len;

	while (val_i < 4)
		buf[buf_i++] = val_p[val_i++];

	if (i2c_master_send(client, buf, len + 2) != len + 2)
		return -EIO;

	return 0;
}

static int ov9281_write_array(struct i2c_client *client, const struct regval *regs)
{
	u32 i;
	int ret = 0;
	//int val;

	FUNC_DEBUG();

	for (i = 0; ((ret == 0) && (regs[i].addr != REG_NULL)); i++) {
		ret = ov9281_write_reg(client, regs[i].addr,
					OV9281_REG_VALUE_08BIT,
					regs[i].val);
		//ov9281_read_reg(client, regs[i].addr, OV9281_REG_VALUE_08BIT, &val);
		//		  printk("i=%4d:, reg=%4x, val=%4x, rb_val=%4x\n",
		//		  i, regs[i].addr, regs[i].val, val);
	}

	return ret;
}

static int ov9281_set_mode(struct ov9281 *ov9281)
{
	const struct ov9281_mode_info *mode = ov9281->current_mode;
	const struct ov9281_mode_info *orig_mode = ov9281->last_mode;
	enum ov9281_downsize_mode dn_mode, orig_dn_mode;
	unsigned long rate;
	int ret = 0;

	FUNC_DEBUG();

	dn_mode = mode->dn_mode;
	orig_dn_mode = orig_mode->dn_mode;

	/*
	 * All the formats we support have 16 bits per pixel, seems to require
	 * the same rate than YUV, so we can just use 16 bpp all the time.
	 */
	rate = mode->vtot * mode->htot * 16;
	rate *= ov9281_framerates[ov9281->current_fr];


	//if ((dn_mode == SUBSAMPLING && orig_dn_mode == SCALING) ||
	//    (dn_mode == SCALING && orig_dn_mode == SUBSAMPLING)) {
	//	// change between subsampling and scaling
	//	// go through exposure calculation
	//	ret = ov9281_set_mode_exposure_calc(ov9281, mode);
	//} else {
	//	// change inside subsampling or scaling
	//	// download firmware directly
	//	ret = ov9281_set_mode_direct(ov9281, mode);
	//}

	ov9281->pending_mode_change = false;
	ov9281->last_mode = mode;

	return ret;
}

static int ov9281_set_framefmt(struct ov9281 *ov9281,
			       struct v4l2_mbus_framefmt *format)
{
	int ret = 0;
	const struct regval *reg_list;

	FUNC_DEBUG();
	DBG_INFO("%s, format->code: 0x%04x\n", __func__, format->code);

	switch (format->code) {
	case MEDIA_BUS_FMT_Y10_1X10:
		/* GREY, RAW10 */
		reg_list = ov9281_grey_1280x800_regs;
		break;

	default:
		return -EINVAL;
	}

	ret = ov9281_write_array(ov9281->i2c_client, reg_list);

	return ret;
}

static int ov9281_set_stream_mipi(struct ov9281 *ov9281, bool on)
{
	int ret;
	const struct regval *reg_list;

	FUNC_DEBUG();
	DBG_INFO("%s, on: %d\n", __func__, on);

	if (on)
		reg_list = ov9281_start_settings;
	else
		reg_list = ov9281_stop_settings;

	ret = ov9281_write_array(ov9281->i2c_client, reg_list);
	if (ret)
		return ret;

	return 0;
}

static int ov9281_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct ov9281 *ov9281 = to_ov9281(sd);
	int ret = 0;

	FUNC_DEBUG();
	DBG_INFO("%s, streaming: %d, pending_mode_change: %d, pending_fmt_change: %d\n",
		 __func__, ov9281->streaming, ov9281->pending_mode_change,
		 ov9281->pending_fmt_change);

	mutex_lock(&ov9281->lock);

	if (ov9281->streaming == !enable) {
		if (enable && ov9281->pending_mode_change) {
			ret = ov9281_set_mode(ov9281);
			if (ret)
				goto out;
		}

		if (enable && ov9281->pending_fmt_change) {
			ret = ov9281_set_framefmt(ov9281, &ov9281->fmt);
			if (ret)
				goto out;
			ov9281->pending_fmt_change = false;
		}

		//if (ov9281->ep.bus_type == V4L2_MBUS_CSI2_DPHY)
			ret = ov9281_set_stream_mipi(ov9281, enable);
		//else
		//	ret = ov9281_set_stream_dvp(ov9281, enable);

		if (!ret)
			ov9281->streaming = enable;
	}
out:
	if (ret)
		DBG_ERR("Start streaming failed while write sensor registers!\n");

	mutex_unlock(&ov9281->lock);
	return ret;
}

static struct v4l2_subdev_video_ops ov9281_subdev_video_ops = {
	.s_stream       = ov9281_s_stream,
};

static const struct ov9281_mode_info *
ov9281_find_mode(struct ov9281 *ov9281, enum ov9281_frame_rate fr,
		 int width, int height, bool nearest)
{
	const struct ov9281_mode_info *mode;

	mode = v4l2_find_nearest_size(ov9281_mode_data,
				      ARRAY_SIZE(ov9281_mode_data),
				      hact, vact,
				      width, height);

	DBG_INFO("%s, mode: %p, width: %d, height: %d, nearest: %d\n",
		 __func__, mode, width, height, nearest);

	if (!mode || (!nearest && (mode->hact != width || mode->vact != height)))
		return NULL;

	/* Only 640x480 can operate at 30fps (for now) */
	if (fr == OV9281_30_FPS && !(mode->hact == 1280 && mode->vact == 800))
		return NULL;

	return mode;
}

static int ov9281_enum_mbus_code(struct v4l2_subdev *sd,
				 struct v4l2_subdev_pad_config *cfg,
				 struct v4l2_subdev_mbus_code_enum *code)
{
	FUNC_DEBUG();

	if (code->pad != 0)
		return -EINVAL;
	if (code->index >= ARRAY_SIZE(ov9281_formats))
		return -EINVAL;

	code->code = ov9281_formats[code->index].code;

	DBG_INFO("%s, index: %d, code: 0x%04x\n",
		 __func__, code->index, code->code);

	return 0;
}

static int ov9281_enum_frame_size(struct v4l2_subdev *sd,
				  struct v4l2_subdev_pad_config *cfg,
				  struct v4l2_subdev_frame_size_enum *fse)
{
	FUNC_DEBUG();

	if (fse->pad != 0)
		return -EINVAL;
	if (fse->index >= OV9281_NUM_MODES)
		return -EINVAL;

	fse->min_width = ov9281_mode_data[fse->index].hact;
	fse->max_width = fse->min_width;
	fse->min_height = ov9281_mode_data[fse->index].vact;
	fse->max_height = fse->min_height;

	DBG_INFO("%s, index: %d, min_w: %d, max_w: %d, min_h: %d, max_h: %d\n",
		 __func__, fse->index,
		 fse->min_width, fse->max_width,
		 fse->min_height, fse->max_height);

	return 0;
}

static int ov9281_try_frame_interval(struct ov9281 *ov9281,
				     struct v4l2_fract *fi,
				     u32 width, u32 height)
{
	const struct ov9281_mode_info *mode;
	enum ov9281_frame_rate rate = OV9281_30_FPS;
	int minfps, maxfps, best_fps, fps;
	int i;

	minfps = ov9281_framerates[OV9281_30_FPS];
	maxfps = ov9281_framerates[OV9281_30_FPS];

	if (fi->numerator == 0) {
		fi->denominator = maxfps;
		fi->numerator = 1;
		rate = OV9281_30_FPS;
		goto find_mode;
	}

	fps = clamp_val(DIV_ROUND_CLOSEST(fi->denominator, fi->numerator),
			minfps, maxfps);

	DBG_INFO("%s, fps: %d, numerator: %d, denominator = %d\n",
		 __func__, fps, fi->numerator, fi->denominator);

	best_fps = minfps;
	for (i = 0; i < ARRAY_SIZE(ov9281_framerates); i++) {
		int curr_fps = ov9281_framerates[i];

		if (abs(curr_fps - fps) < abs(best_fps - fps)) {
			best_fps = curr_fps;
			rate = i;
		}
	}

	fi->numerator = 1;
	fi->denominator = best_fps;

find_mode:
	mode = ov9281_find_mode(ov9281, rate, width, height, false);
	return mode ? rate : -EINVAL;
}

static int ov9281_enum_frame_interval(struct v4l2_subdev *sd,
				      struct v4l2_subdev_pad_config *cfg,
				      struct v4l2_subdev_frame_interval_enum *fie)
{
	struct ov9281 *ov9281 = to_ov9281(sd);
	struct v4l2_fract tpf;
	int ret;

	FUNC_DEBUG();

	if (fie->pad != 0)
		return -EINVAL;
	if (fie->index >= OV9281_NUM_FRAMERATES)
		return -EINVAL;

	tpf.numerator = 1;
	tpf.denominator = ov9281_framerates[fie->index];

	ret = ov9281_try_frame_interval(ov9281, &tpf, fie->width, fie->height);
	if (ret < 0)
		return -EINVAL;

	DBG_INFO("%s, index: %d, numerator: %d, denominator = %d\n",
		 __func__, fie->index, tpf.numerator, tpf.denominator);

	fie->interval = tpf;
	return 0;
}

static int ov9281_get_fmt(struct v4l2_subdev *sd,
			  struct v4l2_subdev_pad_config *cfg,
			  struct v4l2_subdev_format *format)
{
	struct ov9281 *ov9281 = to_ov9281(sd);
	struct v4l2_mbus_framefmt *fmt;

	if (format->pad != 0)
		return -EINVAL;

	mutex_lock(&ov9281->lock);

#ifdef CONFIG_VIDEO_V4L2_SUBDEV_API
	if (format->which == V4L2_SUBDEV_FORMAT_TRY)
		fmt = v4l2_subdev_get_try_format(&ov9281->subdev, cfg, format->pad);
	else
		fmt = &ov9281->fmt;
#else
	fmt = &ov9281->fmt;
#endif

	format->format = *fmt;

	mutex_unlock(&ov9281->lock);

	return 0;
}

static int ov9281_try_fmt_internal(struct v4l2_subdev *sd,
				   struct v4l2_mbus_framefmt *fmt,
				   enum ov9281_frame_rate fr,
				   const struct ov9281_mode_info **new_mode)
{
	struct ov9281 *ov9281 = to_ov9281(sd);
	const struct ov9281_mode_info *mode;
	int i;

	FUNC_DEBUG();

	mode = ov9281_find_mode(ov9281, fr, fmt->width, fmt->height, true);
	if (!mode)
		return -EINVAL;
	fmt->width = mode->hact;
	fmt->height = mode->vact;

	if (new_mode)
		*new_mode = mode;

	for (i = 0; i < ARRAY_SIZE(ov9281_formats); i++)
		if (ov9281_formats[i].code == fmt->code)
			break;
	if (i >= ARRAY_SIZE(ov9281_formats))
		i = 0;

	fmt->code = ov9281_formats[i].code;
	fmt->colorspace = ov9281_formats[i].colorspace;
	fmt->ycbcr_enc = V4L2_MAP_YCBCR_ENC_DEFAULT(fmt->colorspace);
	fmt->quantization = V4L2_QUANTIZATION_FULL_RANGE;
	fmt->xfer_func = V4L2_MAP_XFER_FUNC_DEFAULT(fmt->colorspace);

	DBG_INFO("%s, code: 0x%04x, width: %d, height: %d\n",
		 __func__, fmt->code, fmt->width, fmt->height);

	return 0;
}

static int ov9281_set_fmt(struct v4l2_subdev *sd,
			  struct v4l2_subdev_pad_config *cfg,
			  struct v4l2_subdev_format *format)
{
	struct ov9281 *ov9281 = to_ov9281(sd);
	const struct ov9281_mode_info *new_mode;
	struct v4l2_mbus_framefmt orig_fmt = ov9281->fmt;
	struct v4l2_mbus_framefmt *mbus_fmt = &format->format;
	struct v4l2_mbus_framefmt *fmt;
	int ret;

	FUNC_DEBUG();
	DBG_INFO("%s, mbus_fmt code: 0x%04x, %dx%d\n",
		 __func__, mbus_fmt->code, mbus_fmt->width, mbus_fmt->height);

	if (format->pad != 0)
		return -EINVAL;

	mutex_lock(&ov9281->lock);

	if (ov9281->streaming) {
		ret = -EBUSY;
		goto out;
	}

	ret = ov9281_try_fmt_internal(sd, mbus_fmt,
				      ov9281->current_fr, &new_mode);
	if (ret)
		goto out;

	DBG_INFO("%s, mbus_fmt->code: 0x%04x, ov9281->fmt.code: 0x%04x\n",
		 __func__, mbus_fmt->code, ov9281->fmt.code);

#ifdef CONFIG_VIDEO_V4L2_SUBDEV_API
	if (format->which == V4L2_SUBDEV_FORMAT_TRY)
		fmt = v4l2_subdev_get_try_format(sd, cfg, 0);
	else
		fmt = &ov9281->fmt;
#else
	fmt = &ov9281->fmt;
#endif

	*fmt = *mbus_fmt;

	if (new_mode != ov9281->current_mode) {
		ov9281->current_mode = new_mode;
		ov9281->pending_mode_change = true;
	}
	if ((mbus_fmt->code != ov9281->fmt.code) || (orig_fmt.code != ov9281->fmt.code))
		ov9281->pending_fmt_change = true;

	DBG_INFO("%s, code: 0x%04x, pending_mode_change: %d, pending_fmt_change: %d\n",
		 __func__, ov9281->fmt.code, ov9281->pending_mode_change,
		 ov9281->pending_fmt_change);

out:
	mutex_unlock(&ov9281->lock);
	return ret;
}

static struct v4l2_subdev_pad_ops ov9281_subdev_pad_ops = {
	.enum_mbus_code      = ov9281_enum_mbus_code,
	.get_fmt             = ov9281_get_fmt,
	.set_fmt             = ov9281_set_fmt,
	.enum_frame_size     = ov9281_enum_frame_size,
	.enum_frame_interval = ov9281_enum_frame_interval,
};

static struct v4l2_subdev_ops ov9281_subdev_ops = {
	.video          = &ov9281_subdev_video_ops,
	.pad            = &ov9281_subdev_pad_ops,
};

static int ov9281_check_sensor_id(struct ov9281 *ov9281, struct i2c_client *client)
{
	u32 val = 0;
	int ret;

	ret = ov9281_read_reg(client, OV9281_REG_CHIP_ID,
			      OV9281_REG_VALUE_16BIT, &val);
	if ((ret != 0) || (val != CHIP_ID)) {
		DBG_ERR("Unexpected sensor (id = 0x%04x, ret = %d)!\n", val, ret);
		return -1;
	}
	DBG_INFO("Check sensor id success (id = 0x%04x).\n", val);

	return 0;
}

static int ov9281_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct device *dev = &client->dev;
	struct ov9281 *ov9281;
	struct v4l2_subdev *sd;
	int ret;

	FUNC_DEBUG();

	ov9281 = devm_kzalloc(dev, sizeof(*ov9281), GFP_KERNEL);
	if (!ov9281) {
		DBG_ERR("Failed to allocate memory for \'ov9281\'!\n");
		return -ENOMEM;
	}

	ov9281->i2c_client = client;

	mutex_init(&ov9281->lock);

	sd = &ov9281->subdev;
	v4l2_i2c_subdev_init(sd, client, &ov9281_subdev_ops);
	DBG_INFO("Initialized V4L2 I2C subdevice.\n");

	ret = ov9281_check_sensor_id(ov9281, client);
	if (ret)
		return ret;

#ifdef CONFIG_VIDEO_V4L2_SUBDEV_API
	sd->internal_ops = &ov9281_internal_ops;
	sd->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
#endif

	// Let IMX290 go to standby mode
	ov9281_write_reg(ov9281->i2c_client, OV9281_REG_CTRL_MODE,
			 OV9281_REG_VALUE_08BIT, OV9281_MODE_SW_STANDBY);

	ret = v4l2_async_register_subdev(sd);
	if (ret) {
		DBG_ERR("V4L2 async register subdevice failed.\n");
		goto err_clean_entity;
	}
	DBG_INFO("Registered V4L2 sub-device successfully.\n");

	return 0;

err_clean_entity:
#if defined(CONFIG_MEDIA_CONTROLLER)
	media_entity_cleanup(&sd->entity);
#endif

	return ret;
}

static int ov9281_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct ov9281 *ov9281 = to_ov9281(sd);

	FUNC_DEBUG();

	v4l2_async_unregister_subdev(sd);
#if defined(CONFIG_MEDIA_CONTROLLER)
	media_entity_cleanup(&sd->entity);
#endif
	v4l2_ctrl_handler_free(&ov9281->ctrl_handler);
	mutex_destroy(&ov9281->lock);

	return 0;
}

//#if IS_ENABLED(CONFIG_OF)
//static const struct of_device_id ov9281_of_match[] = {
//	{ .compatible = "sunplus,ov9281" },
//	{},
//};
//MODULE_DEVICE_TABLE(of, ov9281_of_match);
//#endif

static const struct i2c_device_id ov9281_match_id[] = {
	{ "ov9281", 0 },
	{ },
};
MODULE_DEVICE_TABLE(i2c, ov9281_match_id);

static struct i2c_driver ov9281_i2c_driver = {
	.probe          = ov9281_probe,
	.remove         = ov9281_remove,
	.id_table       = ov9281_match_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "ov9281",
		//.of_match_table = of_match_ptr(ov9281_of_match),
	},
};

module_i2c_driver(ov9281_i2c_driver);

MODULE_DESCRIPTION("Sunplus ov9281 sensor driver");
MODULE_LICENSE("GPL v2");
