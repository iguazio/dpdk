/* SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2008-2018 Solarflare Communications Inc.
 * All rights reserved.
 */

/*
 * This file is automatically generated. DO NOT EDIT IT.
 * To make changes, edit the .yml files in sfregistry under doc/mcdi/ and
 * rebuild this file with "make -C doc mcdiheaders".
 *
 * The version of this file has MCDI strings really used in the libefx.
 */

#ifndef _SIENA_MC_DRIVER_PCOL_STRS_H
#define	_SIENA_MC_DRIVER_PCOL_STRS_H

#define	MC_CMD_SENSOR_CONTROLLER_TEMP_ENUM_STR "Controller temperature: degC"
#define	MC_CMD_SENSOR_PHY_COMMON_TEMP_ENUM_STR "Phy common temperature: degC"
#define	MC_CMD_SENSOR_CONTROLLER_COOLING_ENUM_STR "Controller cooling: bool"
#define	MC_CMD_SENSOR_PHY0_TEMP_ENUM_STR "Phy 0 temperature: degC"
#define	MC_CMD_SENSOR_PHY0_COOLING_ENUM_STR "Phy 0 cooling: bool"
#define	MC_CMD_SENSOR_PHY1_TEMP_ENUM_STR "Phy 1 temperature: degC"
#define	MC_CMD_SENSOR_PHY1_COOLING_ENUM_STR "Phy 1 cooling: bool"
#define	MC_CMD_SENSOR_IN_1V0_ENUM_STR "1.0v power: mV"
#define	MC_CMD_SENSOR_IN_1V2_ENUM_STR "1.2v power: mV"
#define	MC_CMD_SENSOR_IN_1V8_ENUM_STR "1.8v power: mV"
#define	MC_CMD_SENSOR_IN_2V5_ENUM_STR "2.5v power: mV"
#define	MC_CMD_SENSOR_IN_3V3_ENUM_STR "3.3v power: mV"
#define	MC_CMD_SENSOR_IN_12V0_ENUM_STR "12v power: mV"
#define	MC_CMD_SENSOR_IN_1V2A_ENUM_STR "1.2v analogue power: mV"
#define	MC_CMD_SENSOR_IN_VREF_ENUM_STR "reference voltage: mV"
#define	MC_CMD_SENSOR_OUT_VAOE_ENUM_STR "AOE FPGA power: mV"
#define	MC_CMD_SENSOR_AOE_TEMP_ENUM_STR "AOE FPGA temperature: degC"
#define	MC_CMD_SENSOR_PSU_AOE_TEMP_ENUM_STR "AOE FPGA PSU temperature: degC"
#define	MC_CMD_SENSOR_PSU_TEMP_ENUM_STR "AOE PSU temperature: degC"
#define	MC_CMD_SENSOR_FAN_0_ENUM_STR "Fan 0 speed: RPM"
#define	MC_CMD_SENSOR_FAN_1_ENUM_STR "Fan 1 speed: RPM"
#define	MC_CMD_SENSOR_FAN_2_ENUM_STR "Fan 2 speed: RPM"
#define	MC_CMD_SENSOR_FAN_3_ENUM_STR "Fan 3 speed: RPM"
#define	MC_CMD_SENSOR_FAN_4_ENUM_STR "Fan 4 speed: RPM"
#define	MC_CMD_SENSOR_IN_VAOE_ENUM_STR "AOE FPGA input power: mV"
#define	MC_CMD_SENSOR_OUT_IAOE_ENUM_STR "AOE FPGA current: mA"
#define	MC_CMD_SENSOR_IN_IAOE_ENUM_STR "AOE FPGA input current: mA"
#define	MC_CMD_SENSOR_NIC_POWER_ENUM_STR "NIC power consumption: W"
#define	MC_CMD_SENSOR_IN_0V9_ENUM_STR "0.9v power voltage: mV"
#define	MC_CMD_SENSOR_IN_I0V9_ENUM_STR "0.9v power current: mA"
#define	MC_CMD_SENSOR_IN_I1V2_ENUM_STR "1.2v power current: mA"
#define	MC_CMD_SENSOR_PAGE0_NEXT_ENUM_STR "Not a sensor: reserved for the next page flag"
#define	MC_CMD_SENSOR_IN_0V9_ADC_ENUM_STR "0.9v power voltage (at ADC): mV"
#define	MC_CMD_SENSOR_CONTROLLER_2_TEMP_ENUM_STR "Controller temperature 2: degC"
#define	MC_CMD_SENSOR_VREG_INTERNAL_TEMP_ENUM_STR "Voltage regulator internal temperature: degC"
#define	MC_CMD_SENSOR_VREG_0V9_TEMP_ENUM_STR "0.9V voltage regulator temperature: degC"
#define	MC_CMD_SENSOR_VREG_1V2_TEMP_ENUM_STR "1.2V voltage regulator temperature: degC"
#define	MC_CMD_SENSOR_CONTROLLER_VPTAT_ENUM_STR "controller internal temperature sensor voltage (internal ADC): mV"
#define	MC_CMD_SENSOR_CONTROLLER_INTERNAL_TEMP_ENUM_STR "controller internal temperature (internal ADC): degC"
#define	MC_CMD_SENSOR_CONTROLLER_VPTAT_EXTADC_ENUM_STR "controller internal temperature sensor voltage (external ADC): mV"
#define	MC_CMD_SENSOR_CONTROLLER_INTERNAL_TEMP_EXTADC_ENUM_STR "controller internal temperature (external ADC): degC"
#define	MC_CMD_SENSOR_AMBIENT_TEMP_ENUM_STR "ambient temperature: degC"
#define	MC_CMD_SENSOR_AIRFLOW_ENUM_STR "air flow: bool"
#define	MC_CMD_SENSOR_VDD08D_VSS08D_CSR_ENUM_STR "voltage between VSS08D and VSS08D at CSR: mV"
#define	MC_CMD_SENSOR_VDD08D_VSS08D_CSR_EXTADC_ENUM_STR "voltage between VSS08D and VSS08D at CSR (external ADC): mV"
#define	MC_CMD_SENSOR_HOTPOINT_TEMP_ENUM_STR "Hotpoint temperature: degC"
#define	MC_CMD_SENSOR_PHY_POWER_PORT0_ENUM_STR "Port 0 PHY power switch over-current: bool"
#define	MC_CMD_SENSOR_PHY_POWER_PORT1_ENUM_STR "Port 1 PHY power switch over-current: bool"
#define	MC_CMD_SENSOR_MUM_VCC_ENUM_STR "Mop-up microcontroller reference voltage: mV"
#define	MC_CMD_SENSOR_IN_0V9_A_ENUM_STR "0.9v power phase A voltage: mV"
#define	MC_CMD_SENSOR_IN_I0V9_A_ENUM_STR "0.9v power phase A current: mA"
#define	MC_CMD_SENSOR_VREG_0V9_A_TEMP_ENUM_STR "0.9V voltage regulator phase A temperature: degC"
#define	MC_CMD_SENSOR_IN_0V9_B_ENUM_STR "0.9v power phase B voltage: mV"
#define	MC_CMD_SENSOR_IN_I0V9_B_ENUM_STR "0.9v power phase B current: mA"
#define	MC_CMD_SENSOR_VREG_0V9_B_TEMP_ENUM_STR "0.9V voltage regulator phase B temperature: degC"
#define	MC_CMD_SENSOR_CCOM_AVREG_1V2_SUPPLY_ENUM_STR "CCOM AVREG 1v2 supply (interval ADC): mV"
#define	MC_CMD_SENSOR_CCOM_AVREG_1V2_SUPPLY_EXTADC_ENUM_STR "CCOM AVREG 1v2 supply (external ADC): mV"
#define	MC_CMD_SENSOR_CCOM_AVREG_1V8_SUPPLY_ENUM_STR "CCOM AVREG 1v8 supply (interval ADC): mV"
#define	MC_CMD_SENSOR_CCOM_AVREG_1V8_SUPPLY_EXTADC_ENUM_STR "CCOM AVREG 1v8 supply (external ADC): mV"
#define	MC_CMD_SENSOR_CONTROLLER_RTS_ENUM_STR "CCOM RTS temperature: degC"
#define	MC_CMD_SENSOR_PAGE1_NEXT_ENUM_STR "Not a sensor: reserved for the next page flag"
#define	MC_CMD_SENSOR_CONTROLLER_MASTER_VPTAT_ENUM_STR "controller internal temperature sensor voltage on master core (internal ADC): mV"
#define	MC_CMD_SENSOR_CONTROLLER_MASTER_INTERNAL_TEMP_ENUM_STR "controller internal temperature on master core (internal ADC): degC"
#define	MC_CMD_SENSOR_CONTROLLER_MASTER_VPTAT_EXTADC_ENUM_STR "controller internal temperature sensor voltage on master core (external ADC): mV"
#define	MC_CMD_SENSOR_CONTROLLER_MASTER_INTERNAL_TEMP_EXTADC_ENUM_STR "controller internal temperature on master core (external ADC): degC"
#define	MC_CMD_SENSOR_CONTROLLER_SLAVE_VPTAT_ENUM_STR "controller internal temperature on slave core sensor voltage (internal ADC): mV"
#define	MC_CMD_SENSOR_CONTROLLER_SLAVE_INTERNAL_TEMP_ENUM_STR "controller internal temperature on slave core (internal ADC): degC"
#define	MC_CMD_SENSOR_CONTROLLER_SLAVE_VPTAT_EXTADC_ENUM_STR "controller internal temperature on slave core sensor voltage (external ADC): mV"
#define	MC_CMD_SENSOR_CONTROLLER_SLAVE_INTERNAL_TEMP_EXTADC_ENUM_STR "controller internal temperature on slave core (external ADC): degC"
#define	MC_CMD_SENSOR_SODIMM_VOUT_ENUM_STR "Voltage supplied to the SODIMMs from their power supply: mV"
#define	MC_CMD_SENSOR_SODIMM_0_TEMP_ENUM_STR "Temperature of SODIMM 0 (if installed): degC"
#define	MC_CMD_SENSOR_SODIMM_1_TEMP_ENUM_STR "Temperature of SODIMM 1 (if installed): degC"
#define	MC_CMD_SENSOR_PHY0_VCC_ENUM_STR "Voltage supplied to the QSFP #0 from their power supply: mV"
#define	MC_CMD_SENSOR_PHY1_VCC_ENUM_STR "Voltage supplied to the QSFP #1 from their power supply: mV"
#define	MC_CMD_SENSOR_CONTROLLER_TDIODE_TEMP_ENUM_STR "Controller die temperature (TDIODE): degC"
#define	MC_CMD_SENSOR_BOARD_FRONT_TEMP_ENUM_STR "Board temperature (front): degC"
#define	MC_CMD_SENSOR_BOARD_BACK_TEMP_ENUM_STR "Board temperature (back): degC"
#define	MC_CMD_SENSOR_IN_I1V8_ENUM_STR "1.8v power current: mA"
#define	MC_CMD_SENSOR_IN_I2V5_ENUM_STR "2.5v power current: mA"
#define	MC_CMD_SENSOR_IN_I3V3_ENUM_STR "3.3v power current: mA"
#define	MC_CMD_SENSOR_IN_I12V0_ENUM_STR "12v power current: mA"
#define	MC_CMD_SENSOR_IN_1V3_ENUM_STR "1.3v power: mV"
#define	MC_CMD_SENSOR_IN_I1V3_ENUM_STR "1.3v power current: mA"

#endif /* _SIENA_MC_DRIVER_PCOL_STRS_H */
