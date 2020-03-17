/* SPDX-License-Identifier: GPL-2.0-only */
/* This file is part of the coreboot project. */

#include <device/mmio.h>
#include <cpu/x86/msr.h>
#include <cpu/amd/msr.h>
#include <cbfs.h>
#include <region_file.h>
#include <timer.h>
#include <bootstate.h>
#include <rules.h>
#include <console/console.h>
#include <amdblocks/psp.h>
#include <soc/iomap.h>
#include <soc/northbridge.h>
#include "psp_def.h"

#define C2P_BUFFER_MAXSIZE 0xc00 /* Core-to-PSP buffer */
#define P2C_BUFFER_MAXSIZE 0xc00 /* PSP-to-core buffer */

struct {
	u8 buffer[C2P_BUFFER_MAXSIZE];
} __attribute__((aligned(32))) c2p_buffer;

struct {
	u8 buffer[P2C_BUFFER_MAXSIZE];
} __attribute__((aligned(32))) p2c_buffer;

static uint32_t smm_flag; /* Non-zero for SMM, clear when not */

static void set_smm_flag(void)
{
	smm_flag = 1;
}

static void clear_smm_flag(void)
{
	smm_flag = 0;
}

int psp_notify_smm(void)
{
	msr_t msr;
	int cmd_status;
	struct mbox_cmd_smm_info_buffer buffer = {
		.header = {
			.size = sizeof(buffer)
		},
		.req = {
			.psp_smm_data_region = (uintptr_t)p2c_buffer.buffer,
			.psp_smm_data_length = sizeof(p2c_buffer),
			.psp_mbox_smm_buffer_address = (uintptr_t)c2p_buffer.buffer,
			.psp_mbox_smm_flag_address = (uintptr_t)&smm_flag,
		}
	};

	msr = rdmsr(SMM_ADDR_MSR);
	buffer.req.smm_base = ((uint64_t)msr.hi << 32) | msr.lo;
	msr = rdmsr(SMM_MASK_MSR);
	msr.lo &= 0xffff0000; /* mask SMM_LOCK and SMM_TSEG_VALID and reserved bits */
	buffer.req.smm_mask = ((uint64_t)msr.hi << 32) | msr.lo;

	soc_fill_smm_trig_info(&buffer.req.smm_trig_info);
#if (CONFIG(SOC_AMD_COMMON_BLOCK_PSP_GEN2))
	soc_fill_smm_reg_info(&buffer.req.smm_reg_info);
#endif

	printk(BIOS_DEBUG, "PSP: Notify SMM info... ");

	set_smm_flag();
	cmd_status = send_psp_command(MBOX_BIOS_CMD_SMM_INFO, &buffer);
	clear_smm_flag();

	/* buffer's status shouldn't change but report it if it does */
	psp_print_cmd_status(cmd_status, (struct mbox_default_buffer *)&buffer);

	return cmd_status;
}