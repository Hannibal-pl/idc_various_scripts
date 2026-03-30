/*
Copyright (c) 2026, Konrad Rzepecki <hannibal@astral.lodz.pl>

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OFMERCHANTABILITY AND
FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.
*/

#include <idc.idc>

extern src_addr;
extern dest_addr;
extern data_size;

static init_addr(void) {
	src_addr = AskAddr(0x0000F000, "Enter source address");
	dest_addr = AskAddr(0x00017000, "Enter destination address");
	data_size = AskLong(0x648, "Enter data size");
}

static main(void) {
	auto buf;
	auto i;

	if (AskYN(0, "This script modify database and can damge it.\nIt's recomended to save database before you use it.\nDo you wish to continue?") != 1) {
		return;
	}


	init_addr();

	print("Copying Start.");

	for (i = 0; i < data_size; i++) {
		buf = Byte(src_addr++);
		PatchByte(dest_addr++, buf);
	}

	print("Copying End.");
}
