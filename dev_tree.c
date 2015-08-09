#include <linux/slab.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/debugfs.h>
#include <linux/mmc/card.h>
#include <linux/mmc/host.h>

struct device_iterator {
	struct device *dev;
	struct device *dev_card;
	struct device *dev_host;
	char *buf;
	int consumed;
	int depth;
};

static int dev_tree_rec(struct device *dev, void *data)
{
	struct device_iterator *di = (struct device_iterator *)data;
	char *extra, *start = di->buf + di->consumed;
	int i, delta, ret;

	if (!dev->p)
		return 0;

	if (dev == di->dev_card)
		extra = " [card]";
	else if (dev == di->dev_host)
		extra = " [host]";
	else
		extra = "";

	for (i = 0; i < 2; i++) {
		int indent;

		for (indent = 0; indent < di->depth; indent++) {
			*(di->buf + di->consumed) = indent ? 0 - indent : ' ';
			di->consumed++;
			di->consumed += snprintf(di->buf + di->consumed,
				PAGE_SIZE - di->consumed, "   ");
		}
		di->consumed += snprintf(di->buf + di->consumed,
			PAGE_SIZE - di->consumed, i ? "+-- %s%s\n" : "|\n",
			dev_name(dev), extra);
	}

	di->depth++;
	ret = device_for_each_child(dev, data, dev_tree_rec);
	di->depth--;

	/* consolidate indentation of di->dpeth up to this device */
	extra = strchr(di->buf, (char)(0 - di->depth));
	if (extra) {
		while (extra < start) {
			if (*extra == (char)(0 - di->depth))
				*extra = '|';
			extra++;
		}
	}

	/* remove possible dangling '|' caused by chilren */
	delta = (di->buf + di->consumed) - start;
	for (i = 0; i < delta; i++) {
		if (*(start + i) == (char)(0 - (di->depth + 1)))
			*(start + i) = ' ';
	}
	return ret;
}

static int mmc_dev_tree_open(struct inode *inode, struct file *filp)
{
	struct mmc_card *card = inode->i_private;

	filp->private_data = card;
	return 0;
}

static ssize_t mmc_dev_tree_read(struct file *filp, char __user *ubuf,
	size_t cnt, loff_t *ppos)
{
	struct mmc_card *card = filp->private_data;
	struct mmc_host *host = card->host;
	struct device *dev = &card->dev;
	struct device_iterator di = {0};
	char *buf;
	int len;
	ssize_t ret;

	if (!access_ok(VERIFY_READ, ubuf, cnt))
		return cnt;

	buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	di.dev_card = dev;
	di.dev_host = &host->class_dev;
	di.buf = buf;
	di.consumed = 0;
	di.depth = 0;

	while (dev->parent)
		dev = dev->parent; /* rewind */

	di.consumed += snprintf(di.buf + di.consumed, PAGE_SIZE - di.consumed,
		".\n");

	dev_tree_rec(dev, &di);
	len = strnlen(buf, cnt);
	ret = simple_read_from_buffer(ubuf, len, ppos, buf, len);
	kfree(buf);

	return ret;
}

static const struct file_operations mmc_dbg_dev_tree_fops = {
	.open		= mmc_dev_tree_open,
	.read		= mmc_dev_tree_read,
};

bool mmc_dev_tree_debugfs(struct mmc_card *card, struct dentry *root)
{
	if (!mmc_card_mmc(card))
		return true;

	return debugfs_create_file("dev_tree", S_IRUSR, root, card,
		&mmc_dbg_dev_tree_fops) ? true : false;
}

