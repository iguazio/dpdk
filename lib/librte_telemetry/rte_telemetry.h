/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2018 Intel Corporation
 */

#include <stdint.h>
#include <rte_compat.h>

#ifndef _RTE_TELEMETRY_H_
#define _RTE_TELEMETRY_H_

/** Maximum number of telemetry callbacks. */
#define TELEMETRY_MAX_CALLBACKS 64
/** Maximum length for string used in object. */
#define RTE_TEL_MAX_STRING_LEN 64
/** Maximum length of string. */
#define RTE_TEL_MAX_SINGLE_STRING_LEN 8192
/** Maximum number of dictionary entries. */
#define RTE_TEL_MAX_DICT_ENTRIES 256
/** Maximum number of array entries. */
#define RTE_TEL_MAX_ARRAY_ENTRIES 512

/**
 * @warning
 * @b EXPERIMENTAL: all functions in this file may change without prior notice
 *
 * @file
 * RTE Telemetry
 *
 * The telemetry library provides a method to retrieve statistics from
 * DPDK by sending a request message over a socket. DPDK will send
 * a JSON encoded response containing telemetry data.
 ***/

/** opaque structure used internally for managing data from callbacks */
struct rte_tel_data;

/**
 * The types of data that can be managed in arrays or dicts.
 * For arrays, this must be specified at creation time, while for
 * dicts this is specified implicitly each time an element is added
 * via calling a type-specific function.
 */
enum rte_tel_value_type {
	RTE_TEL_STRING_VAL, /** a string value */
	RTE_TEL_INT_VAL,    /** a signed 32-bit int value */
	RTE_TEL_U64_VAL,    /** an unsigned 64-bit int value */
};

/**
 * This telemetry callback is used when registering a telemetry command.
 * It handles getting and formatting information to be returned to telemetry
 * when requested.
 *
 * @param cmd
 * The cmd that was requested by the client.
 * @param params
 * Contains data required by the callback function.
 * @param info
 * The information to be returned to the caller.
 *
 * @return
 * Length of buffer used on success.
 * @return
 * Negative integer on error.
 */
typedef int (*telemetry_cb)(const char *cmd, const char *params,
		struct rte_tel_data *info);

/**
 * Used for handling data received over a telemetry socket.
 *
 * @param sock_id
 * ID for the socket to be used by the handler.
 *
 * @return
 * Void.
 */
typedef void * (*handler)(void *sock_id);

/**
 * @warning
 * @b EXPERIMENTAL: this API may change without prior notice
 *
 * Initialize Telemetry
 *
 * @return
 *  0 on successful initialisation.
 * @return
 *  -ENOMEM on memory allocation error
 * @return
 *  -EPERM on unknown error failure
 * @return
 *  -EALREADY if Telemetry is already initialised.
 */
__rte_experimental
int32_t
rte_telemetry_init(void);

/**
 * @warning
 * @b EXPERIMENTAL: this API may change without prior notice
 *
 * Clean up and free memory.
 *
 * @return
 *  0 on success
 * @return
 *  -EPERM on failure
 */
__rte_experimental
int32_t
rte_telemetry_cleanup(void);

/**
 * @warning
 * @b EXPERIMENTAL: this API may change without prior notice
 *
 * Runs various tests to ensure telemetry initialisation and register/unregister
 * functions are working correctly.
 *
 * @return
 *  0 on success when all tests have passed
 * @return
 *  -1 on failure when the test has failed
 */
__rte_experimental
int32_t
rte_telemetry_selftest(void);

/**
 * Used when registering a command and callback function with telemetry.
 *
 * @param cmd
 * The command to register with telemetry.
 * @param fn
 * Callback function to be called when the command is requested.
 * @param help
 * Help text for the command.
 *
 * @return
 *  0 on success.
 * @return
 *  -EINVAL for invalid parameters failure.
 *  @return
 *  -ENOENT if max callbacks limit has been reached.
 */
__rte_experimental
int
rte_telemetry_register_cmd(const char *cmd, telemetry_cb fn, const char *help);

/**
 * Initialize new version of Telemetry.
 *
 * @return
 *  0 on success.
 * @return
 *  -1 on failure.
 */
__rte_experimental
int
rte_telemetry_new_init(void);
#endif
