/*
fatls usage
--fatls mmc 0
--fatls mmc 0 /android/
Marvell>> fatls mmc 0 /android/
            ./
            ../
            data/
0 file(s), 3 dir(s)

Marvell>> fatinfo mmc 1 
mv_sdh[1]: 74 clk done(no wait) 75

include/configs/ulc1_dkb.h
-----找到存储设备编号定义文件
mmc 0 SD card
mmc 1 STDIO

*/


/* code analyse */
/* 1 命令实现文件
在U-boot中添加自己的命令
http://blog.chinaunix.net/uid-22910619-id-2606858.html
*/
/* >>1>> common/cmd_fat.c */
static int do_fat_ls(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	return do_ls(cmdtp, flag, argc, argv, FS_TYPE_FAT);
}

U_BOOT_CMD(
	fatls,	4,	1,	do_fat_ls,
	"list files in a directory (default /)",
	"<interface> [<dev[:part]>] [directory]\n"
	"    - list files from 'dev' on 'interface' in a 'directory'"
);

/* U_BOOT_CMD 定义位置 include/command.h*/
struct cmd_tbl_s {
	char		*name;		/* Command Name			*/
	int		maxargs;	/* maximum number of arguments	*/
	int		repeatable;	/* autorepeat allowed?		*/
					/* Implementation function	*/
	int		(*cmd)(struct cmd_tbl_s *, int, int, char * const []);
	char		*usage;		/* Usage message	(short)	*/
#ifdef	CONFIG_SYS_LONGHELP
	char		*help;		/* Help  message	(long)	*/
#endif
#ifdef CONFIG_AUTO_COMPLETE
	/* do auto completion on the arguments */
	int		(*complete)(int argc, char * const argv[], char last_char, int maxv, char *cmdv[]);
#endif
};


/* >>2>> do_ls() 追踪
	fs/fs.c
*/
int do_ls(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[],
	int fstype)
{
	if (argc < 2)
		return CMD_RET_USAGE;
	if (argc > 4)
		return CMD_RET_USAGE;

	if (fs_set_blk_dev(argv[1], (argc >= 3) ? argv[2] : NULL, fstype))
		return 1;

	if (fs_ls(argc >= 4 ? argv[3] : "/"))
		return 1;

	return 0;
}

int fs_set_blk_dev(const char *ifname, const char *dev_part_str, int fstype)
{
	struct fstype_info *info;
	int part, i;
#ifdef CONFIG_NEEDS_MANUAL_RELOC
	static int relocated;

	if (!relocated) {
		for (i = 0, info = fstypes; i < ARRAY_SIZE(fstypes);
				i++, info++) {
			info->probe += gd->reloc_off;
			info->close += gd->reloc_off;
			info->ls += gd->reloc_off;
			info->read += gd->reloc_off;
			info->write += gd->reloc_off;
		}
		relocated = 1;
	}
#endif

	part = get_device_and_partition(ifname, dev_part_str, &fs_dev_desc,
					&fs_partition, 1);
	if (part < 0)
		return -1;

	for (i = 0, info = fstypes; i < ARRAY_SIZE(fstypes); i++, info++) {
		if (fstype != FS_TYPE_ANY && info->fstype != FS_TYPE_ANY &&
				fstype != info->fstype)
			continue;

		if (!info->probe(fs_dev_desc, &fs_partition)) {
			fs_type = info->fstype;
			return 0;
		}
	}

	return -1;
}

/* get_device_and_partition() disk/part.c */

int get_device_and_partition(const char *ifname, const char *dev_part_str,
			     block_dev_desc_t **dev_desc,
			     disk_partition_t *info, int allow_whole_dev)
{
	int ret = -1;
	const char *part_str;
	char *dup_str = NULL;
	const char *dev_str;
	int dev;
	char *ep;
	int p;
	int part;
	disk_partition_t tmpinfo;

	/*
	 * For now, we have a special case for sandbox, since there is no
	 * real block device support.
	 */
	if (0 == strcmp(ifname, "host")) {
		*dev_desc = NULL;
		info->start = info->size =  info->blksz = 0;
		info->bootable = 0;
		strcpy((char *)info->type, BOOT_PART_TYPE);
		strcpy((char *)info->name, "Sandbox host");
#ifdef CONFIG_PARTITION_UUIDS
		info->uuid[0] = 0;
#endif

		return 0;
	}

	/* If no dev_part_str, use bootdevice environment variable */
	if (!dev_part_str || !strlen(dev_part_str) ||
	    !strcmp(dev_part_str, "-"))
		dev_part_str = getenv("bootdevice");

	/* If still no dev_part_str, it's an error */
	if (!dev_part_str) {
		printf("** No device specified **\n");
		goto cleanup;
	}

	/* Separate device and partition ID specification */
	part_str = strchr(dev_part_str, ':');
	if (part_str) {
		dup_str = strdup(dev_part_str);
		dup_str[part_str - dev_part_str] = 0;
		dev_str = dup_str;
		part_str++;
	} else {
		dev_str = dev_part_str;
	}

	/* Look up the device */
	dev = get_device(ifname, dev_str, dev_desc);
	if (dev < 0)
		goto cleanup;

	/* Convert partition ID string to number */
	if (!part_str || !*part_str) {
		part = PART_UNSPECIFIED;
	} else if (!strcmp(part_str, "auto")) {
		part = PART_AUTO;
	} else {
		/* Something specified -> use exactly that */
		part = (int)simple_strtoul(part_str, &ep, 16);
		/*
		 * Less than whole string converted,
		 * or request for whole device, but caller requires partition.
		 */
		if (*ep || (part == 0 && !allow_whole_dev)) {
			printf("** Bad partition specification %s %s **\n",
			    ifname, dev_part_str);
			goto cleanup;
		}
	}

	/*
	 * No partition table on device,
	 * or user requested partition 0 (entire device).
	 */
	if (((*dev_desc)->part_type == PART_TYPE_UNKNOWN) ||
	    (part == 0)) {
		if (!(*dev_desc)->lba) {
			printf("** Bad device size - %s %s **\n", ifname,
			       dev_str);
			goto cleanup;
		}

		/*
		 * If user specified a partition ID other than 0,
		 * or the calling command only accepts partitions,
		 * it's an error.
		 */
		if ((part > 0) || (!allow_whole_dev)) {
			printf("** No partition table - %s %s **\n", ifname,
			       dev_str);
			goto cleanup;
		}

		(*dev_desc)->log2blksz = LOG2((*dev_desc)->blksz);

		info->start = 0;
		info->size = (*dev_desc)->lba;
		info->blksz = (*dev_desc)->blksz;
		info->bootable = 0;
		strcpy((char *)info->type, BOOT_PART_TYPE);
		strcpy((char *)info->name, "Whole Disk");
#ifdef CONFIG_PARTITION_UUIDS
		info->uuid[0] = 0;
#endif

		ret = 0;
		goto cleanup;
	}

	/*
	 * Now there's known to be a partition table,
	 * not specifying a partition means to pick partition 1.
	 */
	if (part == PART_UNSPECIFIED)
		part = 1;

	/*
	 * If user didn't specify a partition number, or did specify something
	 * other than "auto", use that partition number directly.
	 */
	if (part != PART_AUTO) {
		ret = get_partition_info(*dev_desc, part, info);
		if (ret) {
			printf("** Invalid partition %d **\n", part);
			goto cleanup;
		}
	} else {
		/*
		 * Find the first bootable partition.
		 * If none are bootable, fall back to the first valid partition.
		 */
		part = 0;
		for (p = 1; p <= MAX_SEARCH_PARTITIONS; p++) {
			ret = get_partition_info(*dev_desc, p, info);
			if (ret)
				continue;

			/*
			 * First valid partition, or new better partition?
			 * If so, save partition ID.
			 */
			if (!part || info->bootable)
				part = p;

			/* Best possible partition? Stop searching. */
			if (info->bootable)
				break;

			/*
			 * We now need to search further for best possible.
			 * If we what we just queried was the best so far,
			 * save the info since we over-write it next loop.
			 */
			if (part == p)
				tmpinfo = *info;
		}
		/* If we found any acceptable partition */
		if (part) {
			/*
			 * If we searched all possible partition IDs,
			 * return the first valid partition we found.
			 */
			if (p == MAX_SEARCH_PARTITIONS + 1)
				*info = tmpinfo;
		} else {
			printf("** No valid partitions found **\n");
			ret = -1;
			goto cleanup;
		}
	}
	if (strncmp((char *)info->type, BOOT_PART_TYPE, sizeof(info->type)) != 0) {
		printf("** Invalid partition type \"%.32s\""
			" (expect \"" BOOT_PART_TYPE "\")\n",
			info->type);
		ret  = -1;
		goto cleanup;
	}

	(*dev_desc)->log2blksz = LOG2((*dev_desc)->blksz);

	ret = part;
	goto cleanup;

cleanup:
	free(dup_str);
	return ret;
}


