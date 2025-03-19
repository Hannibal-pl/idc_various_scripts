/*
Copyright (c) 2024, Konrad Rzepecki <hannibal@astral.lodz.pl>

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

extern start_addr;
extern filename;

static main(void) {
	auto start_addr;
	auto filename;
	auto file;
	auto count;
	auto i;

	print("SCRIPT START");
	filename = AskFile(0, "*", "Choose patch file");
	start_addr = AskAddr(0, "Choose start address");

	print("patch start");

	file = fopen(filename, "r");
	if (file == 0) {
		print("Error opening patch file");
	}
	count = filelength(file);
	print(sprintf("patch size: %i bytes", count));
	for (i = 0; i < count; i = i + 1) {
		PatchByte(start_addr++, fgetc(file));
	}

	fclose(file);
	print("patch end");
}
