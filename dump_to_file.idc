/*
Copyright (c) 2025, Konrad Rzepecki <hannibal@astral.lodz.pl>

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

static main(void) {
	auto addr;
	auto len;
	auto file;
	auto fd;
	auto i;

	addr = AskAddr(ScreenEA(), "Enter start address of dump");
	len = AskLong(NextNotTail(addr) - addr, "Enter length of dump in bytes");
	file = AskFile(1, "*", "Enter dump filename");

	print(sprintf("Dumping 0x%X bytes from 0x%08X to '%s' file", len, addr, file));

	fd = fopen(file, "wb");
	if (fd == 0) {
		print("Unable to open dump file");
		return;
	}

	for (i = 0; i < (len >> 2); i++) {
		writelong(fd, Dword(addr), 0);
		addr = addr + 4;
	}
	for (i = 0; i < (len & 3); i++) {
		fputc(Byte(addr++), fd);
	}

	fclose(fd);
	print("Dump ready.");
}
