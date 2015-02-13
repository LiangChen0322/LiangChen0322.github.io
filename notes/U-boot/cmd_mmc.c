/*
烧写信息
> flash:pbootloader
command::mmc dev 2 0; mmc write 0xafff800 0xb400 0x468; mmc dev0
mmc2(part 0) is current device

MMC write: dev # 2, block # 46080, count 1128 ... 1128 blocks wK
mmc2(part 0) is current device

> getvar:partition-type:rbootloader

> getvar:max-download-size

> download:0008cee8
recv data addr=afff800 size=8cee8

> flash:rbootloader
command::mmc dev 2 0; mmc write 0xafff800 0x2400 0x468; mmc dev0
mmc2(part 0) is current device

MMC write: dev # 2, block # 9216, count 1128 ... 1128 blocks wrK
mmc2(part 0) is current device
*/
