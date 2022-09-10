#pragma once

// from https://kernel.googlesource.com/pub/scm/utils/mmc/mmc-utils/+/refs/heads/master/lsmmc.c

struct ids_database {
	char *type;
	int id;
	char *manufacturer;
};

static struct ids_database unk_db = {
	.type = "unk",
	.manufacturer = "Unknown",
};

static struct ids_database database[] = {
	{
		.type = "sd",
		.id = 0x01,
		.manufacturer = "Panasonic",
	},
	{
		.type = "sd",
		.id = 0x02,
		.manufacturer = "Toshiba/Kingston/Viking",
	},
	{
		.type = "sd",
		.id = 0x03,
		.manufacturer = "SanDisk",
	},
	{
		.type = "sd",
		.id = 0x08,
		.manufacturer = "Silicon Power",
	},
	{
		.type = "sd",
		.id = 0x18,
		.manufacturer = "Infineon",
	},
	{
		.type = "sd",
		.id = 0x1b,
		.manufacturer = "Transcend/Samsung",
	},
	{
		.type = "sd",
		.id = 0x1c,
		.manufacturer = "Transcend",
	},
	{
		.type = "sd",
		.id = 0x1d,
		.manufacturer = "Corsair/AData",
	},
	{
		.type = "sd",
		.id = 0x1e,
		.manufacturer = "Transcend",
	},
	{
		.type = "sd",
		.id = 0x1f,
		.manufacturer = "Kingston",
	},
	{
		.type = "sd",
		.id = 0x27,
		.manufacturer = "Delkin/Phison",
	},
	{
		.type = "sd",
		.id = 0x28,
		.manufacturer = "Lexar",
	},
	{
		.type = "sd",
		.id = 0x30,
		.manufacturer = "SanDisk",
	},
	{
		.type = "sd",
		.id = 0x31,
		.manufacturer = "Silicon Power",
	},
	{
		.type = "sd",
		.id = 0x33,
		.manufacturer = "STMicroelectronics",
	},
	{
		.type = "sd",
		.id = 0x41,
		.manufacturer = "Kingston",
	},
	{
		.type = "sd",
		.id = 0x6f,
		.manufacturer = "STMicroelectronics",
	},
	{
		.type = "sd",
		.id = 0x74,
		.manufacturer = "Transcend",
	},
	{
		.type = "sd",
		.id = 0x76,
		.manufacturer = "Patriot",
	},
	{
		.type = "sd",
		.id = 0x82,
		.manufacturer = "Gobe/Sony",
	},
	{
		.type = "sd",
		.id = 0x89,
		.manufacturer = "Unknown",
	},
	{
		.type = "sd",
		.id = 0x9e,
		.manufacturer = "PNY",
	},
	{
		.type = "mmc",
		.id = 0x00,
		.manufacturer = "SanDisk",
	},
	{
		.type = "mmc",
		.id = 0x02,
		.manufacturer = "Kingston/SanDisk",
	},
	{
		.type = "mmc",
		.id = 0x03,
		.manufacturer = "Toshiba",
	},
	{
		.type = "mmc",
		.id = 0x05,
		.manufacturer = "Unknown",
	},
	{
		.type = "mmc",
		.id = 0x06,
		.manufacturer = "Unknown",
	},
	{
		.type = "mmc",
		.id = 0x11,
		.manufacturer = "Toshiba",
	},
	{
		.type = "mmc",
		.id = 0x13,
		.manufacturer = "Micron",
	},
	{
		.type = "mmc",
		.id = 0x15,
		.manufacturer = "Samsung/SanDisk/LG",
	},
	{
		.type = "mmc",
		.id = 0x37,
		.manufacturer = "KingMax",
	},
	{
		.type = "mmc",
		.id = 0x44,
		.manufacturer = "SanDisk",
	},
	{
		.type = "mmc",
		.id = 0x2c,
		.manufacturer = "Kingston",
	},
	{
		.type = "mmc",
		.id = 0x70,
		.manufacturer = "Kingston",
	},
	{
		.type = "mmc",
		.id = 0xfe,
		.manufacturer = "Micron",
	},
	{
		.type = "mmc",
		.id = 0x90,
		.manufacturer = "Hynix",
	},
};

static inline struct ids_database *find_by_id(int id)
{
    unsigned int ids_cnt = sizeof(database) / sizeof(struct ids_database);
    for (int i = 0; i < ids_cnt; ++i) {
        if (database[i].id == id) {
            return &database[i];
        }
    }

	return &unk_db;
}
