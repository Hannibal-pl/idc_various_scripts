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
	src_addr = AskAddr(0x000356A0, "Enter source address");
	dest_addr = AskAddr(0x020B5A58, "Enter destination address");
	data_size = AskLong(61662, "Destination size (zero fill area)");
}

static main(void) {
	auto buf;
	auto data_count;
	auto zero_count;
	auto bytes_packed;
	auto bytes_unpacked;
	auto bytes_zerofill;
	auto i;

	if (AskYN(0, "This script modify database and can damge it.\nIt's recomended to save database before you use it.\nDo you wish to continue?") != 1) {
		return;
	}

	bytes_packed = 0;
	bytes_unpacked = 0;
	bytes_zerofill = 0;

	init_addr();

	print("Unpacking Start.");

	while(1) {
		buf = Byte(src_addr++);
		bytes_packed++;

		if (buf == 0) {
			break;
		}

		data_count = (buf & 0xF0) >> 4;
		zero_count = (buf & 0x0F);

		for (i = 0; i < data_count; i++) {
			buf = Byte(src_addr++);
			PatchByte(dest_addr++, buf);
		}

		bytes_packed = bytes_packed + data_count;
		bytes_unpacked = bytes_unpacked + data_count;

		if (zero_count == 0x0F) {
			zero_count = Byte(src_addr++);
			bytes_packed++;
		}

		for (i = 0; i < zero_count; i++) {
			PatchByte(dest_addr++, 0);
		}
		bytes_packed = bytes_packed + zero_count;
		bytes_unpacked = bytes_unpacked + zero_count;
	}

	for (i = bytes_unpacked; i < data_size; i++) {
		PatchByte(dest_addr++, 0);
		bytes_zerofill++;
	}

	print("Unpacking End.");
	print(sprintf("Packed length:   %i.", bytes_packed));
	print(sprintf("Unpacked length: %i.", bytes_unpacked));
	print(sprintf("Zerofill length: %i.", bytes_zerofill));
}
