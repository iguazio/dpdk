/*-
 *   BSD LICENSE
 *
 *   Copyright 2017 6WIND S.A.
 *   Copyright 2017 Mellanox.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of 6WIND S.A. nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <string.h>

#include <rte_devargs.h>
#include <rte_malloc.h>
#include <rte_kvargs.h>

#include "failsafe_private.h"

#define DEVARGS_MAXLEN 4096

/* Callback used when a new device is found in devargs */
typedef int (parse_cb)(struct rte_eth_dev *dev, const char *params,
		uint8_t head);

int mac_from_arg = 0;

const char *pmd_failsafe_init_parameters[] = {
	PMD_FAILSAFE_MAC_KVARG,
	NULL,
};

/*
 * input: text.
 * output: 0: if text[0] != '(',
 *         0: if there are no corresponding ')'
 *         n: distance to corresponding ')' otherwise
 */
static size_t
closing_paren(const char *text)
{
	int nb_open = 0;
	size_t i = 0;

	while (text[i] != '\0') {
		if (text[i] == '(')
			nb_open++;
		if (text[i] == ')')
			nb_open--;
		if (nb_open == 0)
			return i;
		i++;
	}
	return 0;
}

static int
fs_parse_device(struct sub_device *sdev, char *args)
{
	struct rte_devargs *d;
	int ret;

	d = &sdev->devargs;
	DEBUG("%s", args);
	ret = rte_eal_devargs_parse(args, d);
	if (ret) {
		DEBUG("devargs parsing failed with code %d", ret);
		return ret;
	}
	sdev->bus = d->bus;
	sdev->state = DEV_PARSED;
	return 0;
}

static int
fs_parse_device_param(struct rte_eth_dev *dev, const char *param,
		uint8_t head)
{
	struct fs_priv *priv;
	struct sub_device *sdev;
	char *args = NULL;
	size_t a, b;
	int ret;

	priv = PRIV(dev);
	a = 0;
	b = 0;
	ret = 0;
	while  (param[b] != '(' &&
		param[b] != '\0')
		b++;
	a = b;
	b += closing_paren(&param[b]);
	if (a == b) {
		ERROR("Dangling parenthesis");
		return -EINVAL;
	}
	a += 1;
	args = strndup(&param[a], b - a);
	if (args == NULL) {
		ERROR("Not enough memory for parameter parsing");
		return -ENOMEM;
	}
	sdev = &priv->subs[head];
	if (strncmp(param, "dev", 3) == 0) {
		ret = fs_parse_device(sdev, args);
		if (ret)
			goto free_args;
	} else {
		ERROR("Unrecognized device type: %.*s", (int)b, param);
		return -EINVAL;
	}
free_args:
	free(args);
	return ret;
}

static int
fs_parse_sub_devices(parse_cb *cb,
		struct rte_eth_dev *dev, const char *params)
{
	size_t a, b;
	uint8_t head;
	int ret;

	a = 0;
	head = 0;
	ret = 0;
	while (params[a] != '\0') {
		b = a;
		while (params[b] != '(' &&
		       params[b] != ',' &&
		       params[b] != '\0')
			b++;
		if (b == a) {
			ERROR("Invalid parameter");
			return -EINVAL;
		}
		if (params[b] == ',') {
			a = b + 1;
			continue;
		}
		if (params[b] == '(') {
			size_t start = b;

			b += closing_paren(&params[b]);
			if (b == start) {
				ERROR("Dangling parenthesis");
				return -EINVAL;
			}
			ret = (*cb)(dev, &params[a], head);
			if (ret)
				return ret;
			head += 1;
			b += 1;
			if (params[b] == '\0')
				return 0;
		}
		a = b + 1;
	}
	return 0;
}

static int
fs_remove_sub_devices_definition(char params[DEVARGS_MAXLEN])
{
	char buffer[DEVARGS_MAXLEN] = {0};
	size_t a, b;
	int i;

	a = 0;
	i = 0;
	while (params[a] != '\0') {
		b = a;
		while (params[b] != '(' &&
		       params[b] != ',' &&
		       params[b] != '\0')
			b++;
		if (b == a) {
			ERROR("Invalid parameter");
			return -EINVAL;
		}
		if (params[b] == ',' || params[b] == '\0')
			i += snprintf(&buffer[i], b - a + 1, "%s", &params[a]);
		if (params[b] == '(') {
			size_t start = b;
			b += closing_paren(&params[b]);
			if (b == start)
				return -EINVAL;
			b += 1;
			if (params[b] == '\0')
				goto out;
		}
		a = b + 1;
	}
out:
	snprintf(params, DEVARGS_MAXLEN, "%s", buffer);
	return 0;
}

static int
fs_get_mac_addr_arg(const char *key __rte_unused,
		const char *value, void *out)
{
	struct ether_addr *ea = out;
	int ret;

	if ((value == NULL) || (out == NULL))
		return -EINVAL;
	ret = sscanf(value, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
		&ea->addr_bytes[0], &ea->addr_bytes[1],
		&ea->addr_bytes[2], &ea->addr_bytes[3],
		&ea->addr_bytes[4], &ea->addr_bytes[5]);
	return ret != ETHER_ADDR_LEN;
}

int
failsafe_args_parse(struct rte_eth_dev *dev, const char *params)
{
	struct fs_priv *priv;
	char mut_params[DEVARGS_MAXLEN] = "";
	struct rte_kvargs *kvlist = NULL;
	unsigned int arg_count;
	size_t n;
	int ret;

	priv = PRIV(dev);
	ret = 0;
	priv->subs_tx = FAILSAFE_MAX_ETHPORTS;
	/* default parameters */
	n = snprintf(mut_params, sizeof(mut_params), "%s", params);
	if (n >= sizeof(mut_params)) {
		ERROR("Parameter string too long (>=%zu)",
				sizeof(mut_params));
		return -ENOMEM;
	}
	ret = fs_parse_sub_devices(fs_parse_device_param,
				   dev, params);
	if (ret < 0)
		return ret;
	ret = fs_remove_sub_devices_definition(mut_params);
	if (ret < 0)
		return ret;
	if (strnlen(mut_params, sizeof(mut_params)) > 0) {
		kvlist = rte_kvargs_parse(mut_params,
				pmd_failsafe_init_parameters);
		if (kvlist == NULL) {
			ERROR("Error parsing parameters, usage:\n"
				PMD_FAILSAFE_PARAM_STRING);
			return -1;
		}
		/* MAC addr */
		arg_count = rte_kvargs_count(kvlist,
				PMD_FAILSAFE_MAC_KVARG);
		if (arg_count > 0) {
			ret = rte_kvargs_process(kvlist,
					PMD_FAILSAFE_MAC_KVARG,
					&fs_get_mac_addr_arg,
					&dev->data->mac_addrs[0]);
			if (ret < 0)
				goto free_kvlist;
			mac_from_arg = 1;
		}
	}
free_kvlist:
	rte_kvargs_free(kvlist);
	return ret;
}

void
failsafe_args_free(struct rte_eth_dev *dev)
{
	struct sub_device *sdev;
	uint8_t i;

	FOREACH_SUBDEV(sdev, i, dev) {
		free(sdev->devargs.args);
		sdev->devargs.args = NULL;
	}
}

static int
fs_count_device(struct rte_eth_dev *dev, const char *param,
		uint8_t head __rte_unused)
{
	size_t b = 0;

	while  (param[b] != '(' &&
		param[b] != '\0')
		b++;
	if (strncmp(param, "dev", b) != 0) {
		ERROR("Unrecognized device type: %.*s", (int)b, param);
		return -EINVAL;
	}
	PRIV(dev)->subs_tail += 1;
	return 0;
}

int
failsafe_args_count_subdevice(struct rte_eth_dev *dev,
			const char *params)
{
	return fs_parse_sub_devices(fs_count_device,
				    dev, params);
}
