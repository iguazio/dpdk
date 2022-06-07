# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2016 Freescale Semiconductor, Inc. All rights reserved.
# Copyright 2017 NXP

#
# machine:
#
#   - can define ARCH variable (overridden by cmdline value)
#   - can define CROSS variable (overridden by cmdline value)
#   - define MACHINE_CFLAGS variable (overridden by cmdline value)
#   - define MACHINE_LDFLAGS variable (overridden by cmdline value)
#   - define MACHINE_ASFLAGS variable (overridden by cmdline value)
#   - can define CPU_CFLAGS variable (overridden by cmdline value) that
#     overrides the one defined in arch.
#   - can define CPU_LDFLAGS variable (overridden by cmdline value) that
#     overrides the one defined in arch.
#   - can define CPU_ASFLAGS variable (overridden by cmdline value) that
#     overrides the one defined in arch.
#   - may override any previously defined variable
#

# ARCH =
# CROSS =
# MACHINE_CFLAGS =
# MACHINE_LDFLAGS =
# MACHINE_ASFLAGS =
# CPU_CFLAGS =
# CPU_LDFLAGS =
# CPU_ASFLAGS =
MACHINE_CFLAGS += -march=armv8-a+crc

ifdef CONFIG_RTE_ARCH_ARM_TUNE
MACHINE_CFLAGS += -mtune=$(CONFIG_RTE_ARCH_ARM_TUNE:"%"=%)
endif

# To avoid TLS corruption issue.
MACHINE_CFLAGS += -mtls-dialect=trad
