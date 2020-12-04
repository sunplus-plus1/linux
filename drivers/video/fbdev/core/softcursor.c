/*
 * linux/drivers/video/console/softcursor.c
 *
 * Generic software cursor for frame buffer devices
 *
 *  Created 14 Nov 2002 by James Simmons
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License.  See the file COPYING in the main directory of this
 * archive for more details.
 */

#include <linux/module.h>
#include <linux/string.h>
#include <linux/fb.h>
#include <linux/slab.h>

#include <asm/io.h>

#include "fbcon.h"

int soft_cursor(struct fb_info *info, struct fb_cursor *cursor)
{
	struct fbcon_ops *ops = info->fbcon_par;
	unsigned int scan_align = info->pixmap.scan_align - 1;
	unsigned int buf_align = info->pixmap.buf_align - 1;
	unsigned int i, size, dsize, s_pitch, d_pitch;
	struct fb_image *image;
	u8 *src, *dst;

	if (info->state != FBINFO_STATE_RUNNING)
		return 0;

printk(KERN_INFO "%s():%d\n", __FUNCTION__, __LINE__);
	s_pitch = (cursor->image.width + 7) >> 3;
	dsize = s_pitch * cursor->image.height;

printk(KERN_INFO "%s():%d\n", __FUNCTION__, __LINE__);
	if (dsize + sizeof(struct fb_image) != ops->cursor_size) {
printk(KERN_INFO "%s():%d\n", __FUNCTION__, __LINE__);
		kfree(ops->cursor_src);
		ops->cursor_size = dsize + sizeof(struct fb_image);

printk(KERN_INFO "%s():%d\n", __FUNCTION__, __LINE__);
		ops->cursor_src = kmalloc(ops->cursor_size, GFP_ATOMIC);
printk(KERN_INFO "%s():%d\n", __FUNCTION__, __LINE__);
		if (!ops->cursor_src) {
printk(KERN_INFO "%s():%d\n", __FUNCTION__, __LINE__);
			ops->cursor_size = 0;
			return -ENOMEM;
		}
	}

printk(KERN_INFO "%s():%d\n", __FUNCTION__, __LINE__);
	src = ops->cursor_src + sizeof(struct fb_image);
	image = (struct fb_image *)ops->cursor_src;
	*image = cursor->image;
	d_pitch = (s_pitch + scan_align) & ~scan_align;

printk(KERN_INFO "%s():%d\n", __FUNCTION__, __LINE__);
	size = d_pitch * image->height + buf_align;
	size &= ~buf_align;
	dst = fb_get_buffer_offset(info, &info->pixmap, size);

printk(KERN_INFO "%s():%d\n", __FUNCTION__, __LINE__);
	if (cursor->enable) {
		switch (cursor->rop) {
		case ROP_XOR:
			for (i = 0; i < dsize; i++)
				src[i] = image->data[i] ^ cursor->mask[i];
			break;
		case ROP_COPY:
		default:
			for (i = 0; i < dsize; i++)
				src[i] = image->data[i] & cursor->mask[i];
			break;
		}
	} else
		memcpy(src, image->data, dsize);

printk(KERN_INFO "%s():%d\n", __FUNCTION__, __LINE__);
	fb_pad_aligned_buffer(dst, d_pitch, src, s_pitch, image->height);
printk(KERN_INFO "%s():%d\n", __FUNCTION__, __LINE__);
	image->data = dst;
	info->fbops->fb_imageblit(info, image);
printk(KERN_INFO "%s():%d\n", __FUNCTION__, __LINE__);
	return 0;
}

EXPORT_SYMBOL(soft_cursor);
