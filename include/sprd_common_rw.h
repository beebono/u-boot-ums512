#ifndef _SPRD_COMMON_RW_H_
#define _SPRD_COMMON_RW_H_
#include <common.h>
#define SPRD_DISK_GUID_STR "11111111-2222-3333-4444-000000000000"

enum block_boot_part {
	USER_PART       = 0x00,
	BOOT_PART1   = 0x01,
	BOOT_PART2      = 0x02
};

int common_raw_read(char *part_name, uint64_t size, uint64_t offset, char *buf);
int common_raw_write(char *part_name, uint64_t size, uint64_t updsize, uint64_t offset, char *buf);
int common_raw_erase(char *part_name, uint64_t size, uint64_t offset);
int common_repartition(disk_partition_t *partitions, int parts_count);
int32_t common_get_lba_size(void);
int do_fs_file_read(char *mpart, char *filenm, void *buf, int len);
int do_fs_file_write(char *mpart, char *filenm, void *buf, int len);
int fdt_fixup_iq_reserved_mem(void *fdt);
void fdt_fixup_pmic_wa(void *fdt);
 int common_query_backstage(char *part_name, uint32_t size, char *buf);
 int common_write_backstage(char *part_name, uint32_t size, uint64_t offset, char *buf);
 int write_sparse_img(uchar * partname, u8* buf, unsigned long length);
 int get_partition_info_by_name(block_dev_desc_t *dev_desc,uchar* partition_name,disk_partition_t *info);
 int fdt_fixup_ddr_size(void *fdt);
 int fdt_fixup_sysdump_magic(void *fdt);
 int _fixup_sysdump_uboot(void *fdt);
 int fdt_fixup_cp_boot(void *fdt);
 int fdt_fixup_memleakon(void *fdt);
 int fdt_fixup_verified_boot(void *fdt);
 int fdt_fixup_flash_lock_state(void *fdt);
 int fdt_fixup_serialno(void *fdt);
 int fdt_fixup_wdten(void *fdt);
 int fdt_fixup_dswdten(void *fdt);
 int fdt_fixup_chosen_bootargs_board_private(void *fdt);
 int fdt_fixup_switch_storage_probe(void *fdt);
 void fdt_fixup_all(u8 *fdt_blob);
 int fdt_initrd_norsvmem(void *fdt, ulong initrd_start, ulong initrd_end, int force);
 int boot_sprdisk(int offset, char *ramdisk_addr);

#ifdef CONFIG_NAND_BOOT
int do_raw_data_write(char *part, u32 updsz, u32 size, u32 off, char *buf);
#endif
int get_img_partition_size(char *part_name, uint64_t *size);
int get_img_partition_info(char *part_name, disk_partition_t *part_info);

#ifdef CONFIG_WR_SPARSE
#ifdef DEBUG
#define wr_dbg(fmt, args...) do {\
		printf("%s(): ", __func__);\
		printf(fmt, ##args);\
	} while (0)
#define wr_raw_dbg(fmt, args...) //wr_dbg
#define wr_fill_dbg wr_dbg
#define wr_dontcare_dbg wr_dbg
#else
#define wr_dbg(fmt, args...)
#define wr_raw_dbg(fmt, args...)
#define wr_fill_dbg(fmt, args...)
#define wr_dontcare_dbg(fmt, args...)
#endif

	int wr_sparse_raw(char *pname, u64 chunk_len, u64 offset, char *buf);
	int wr_sparse_fill(char *pname, s16 chunk_type, u64 size, u64 offset,
		u32 fill_val);
	int wr_sparse_dontcare(char *pname, u64 size, u64 offset);
	int wr_sparse_flush(void);
	void wr_sparse_rest(char *pname);
	int wr_sparse_prepare(u8 *base_address, u64 *max_size);
	
#define CONFIG_WRBG_SPARSE
#ifdef CONFIG_WRBG_SPARSE
	int wrbg_sparse_flush(char *pname);
#endif
#endif


#endif /* _LOADER_COMMON_H_ */
