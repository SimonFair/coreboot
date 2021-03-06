/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef AMD_PICASSO_SMU_H
#define AMD_PICASSO_SMU_H

/*
 * SMU mailbox register offsets in indirect address space accessed by an index/data pair in
 * D0F00 config space.
 */
#define REG_ADDR_MESG_ID	0x3b10528
#define REG_ADDR_MESG_RESP	0x3b10564
#define REG_ADDR_MESG_ARGS_BASE	0x3b10998

#define SMU_NUM_ARGS		6

enum smu_message_id {
	SMC_MSG_S3ENTRY = 0x0c,
};

/*
 * Request the SMU put system into S3, S4, or S5. On entry, SlpTyp determines S-State and
 * SlpTypeEn gets set by the SMU. Function does not return if successful.
 */
void smu_sx_entry(void);

#endif /* AMD_PICASSO_SMU_H */
