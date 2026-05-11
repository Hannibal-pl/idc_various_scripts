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
extern data_size;
extern is_encrypted;
extern random_bytes;

// decryption values
extern c1;
extern c2;
extern r;

static init_addr(void) {
	src_addr = AskAddr(ScreenEA(), "Enter Type 1/2 char string start address");
	data_size = AskLong(0, "Enter Type 1/2 char string length");
	is_encrypted = AskYN(1, "Is char string encrypted?");
	if (is_encrypted) {
		random_bytes = AskLong(4, "Random bytes at begining (lenIV).");
	}
}

static dByte(addr) {
	auto buf;
	auto plain;

	buf = Byte(addr);
	if (is_encrypted == 1) {
		plain = (buf ^ (r >> 8)) & 0xFF;
		r = ((buf + r) * c1 + c2) & 0xFFFF;
		return plain;
	} else {
		return buf;
	}
}

static init_decrypt(void) {
	auto i;

	c1 = 52845;
	c2 = 22719;
	r = 4330; /* eexec 55665 */

	// skip random bytes;
	for (i = 0; i < random_bytes; i++) {
		dByte(src_addr++);
	}
}

static main(void) {
	auto buf;
	auto i;
	auto param;
	auto command;
	auto comment_addr;

	if (AskYN(0, "This script modify database and can damge it.\nIt's recomended to save database before you use it.\nDo you wish to continue?") != 1) {
		return;
	}


	init_addr();

	print("Decoding Start.");

	if (is_encrypted == 1) {
		if (random_bytes > 0) {
			MakeComm(src_addr, sprintf("skip %i random bytes (lenIV)", random_bytes));
			i = random_bytes;
		}
		init_decrypt();
	} else {
		i = 0;
	}

	for (; i < data_size; i++) {
		comment_addr = src_addr;
		buf = dByte(src_addr++);

		if ((buf < 32) && (buf != 28)) { /* command */
			if (buf == 0) {
				command = "error";
			} else if (buf == 1) {
				command = "hstem";
			} else if (buf == 3) {
				command = "vstem";
			} else if (buf == 4) {
				command = "vmoveto";
			} else if (buf == 5) {
				command = "rlineto";
			} else if (buf == 6) {
				command = "hlineto";
			} else if (buf == 7) {
				command = "vlineto";
			} else if (buf == 8) {
				command = "rrcurveto";
			} else if (buf == 9) {
				command = "closepath";
			} else if (buf == 10) {
				command = "callsubr";
			} else if (buf == 11) {
				command = "return";
			} else if (buf == 12) {
				buf = dByte(src_addr++);

				if (buf == 0) {
					command = "dotsection";
				} else if (buf == 1) {
					command = "vstem3";
				} else if (buf == 2) {
					command = "hstem3";
				} else if (buf == 3) {
					command = "and";
				} else if (buf == 4) {
					command = "or";
				} else if (buf == 5) {
					command = "not";
				} else if (buf == 6) {
					command = "seac";
				} else if (buf == 7) {
					command = "sbw";
				} else if (buf == 8) {
					command = "store";
				} else if (buf == 9) {
					command = "abs";
				} else if (buf == 10) {
					command = "add";
				} else if (buf == 11) {
					command = "sub";
				} else if (buf == 12) {
					command = "div";
				} else if (buf == 13) {
					command = "load";
				} else if (buf == 14) {
					command = "neg";
				} else if (buf == 15) {
					command = "eq";
				} else if (buf == 16) {
					command = "callothersubr";
				} else if (buf == 17) {
					command = "pop";
				} else if (buf == 18) {
					command = "drop";
				} else if (buf == 20) {
					command = "put";
				} else if (buf == 21) {
					command = "get";
				} else if (buf == 22) {
					command = "ifelse";
				} else if (buf == 23) {
					command = "random";
				} else if (buf == 24) {
					command = "mul";
				} else if (buf == 26) {
					command = "sqrt";
				} else if (buf == 27) {
					command = "dup";
				} else if (buf == 28) {
					command = "exch";
				} else if (buf == 29) {
					command = "index";
				} else if (buf == 30) {
					command = "roll";
				} else if (buf == 33) {
					command = "setcurrentpoint";
				} else if (buf == 34) {
					command = "hflex";
				} else if (buf == 35) {
					command = "flex";
				} else if (buf == 36) {
					command = "hflex1";
				} else if (buf == 37) {
					command = "flex1";
				} else {
					command = sprintf("unknown long command (12 %i)", buf);
				}

				i++;
			} else if (buf == 13) {
				command = "hsbw";
			} else if (buf == 14) {
				command = "endchar";
			} else if (buf == 16) {
				command = "blend";
			} else if (buf == 18) {
				command = "hstemhm";
			} else if (buf == 19) {
				command = "hintmask";
			} else if (buf == 20) {
				command = "cntrmask";
			} else if (buf == 21) {
				command = "rmoveto";
			} else if (buf == 22) {
				command = "hmoveto";
			} else if (buf == 23) {
				command = "vstemhm";
			} else if (buf == 24) {
				command = "rcurveline";
			} else if (buf == 25) {
				command = "rlinecurve";
			} else if (buf == 26) {
				command = "vvcurveto";
			} else if (buf == 27) {
				command = "hhcurveto";
			} else if (buf == 29) {
				command = "callgsubr";
			} else if (buf == 30) {
				command = "vhcurveto";
			} else if (buf == 31) {
				command = "hvcurveto";
			} else {
				command = sprintf("unknown command (%i)", buf);
			}

			MakeComm(comment_addr, command);
		} else {
			param = 0;
			if (buf == 28) { /* 3 byte encoded int */
				buf = dByte(src_addr++);
				param = param + buf;
				param = param << 8;
				buf = dByte(src_addr++);
				param = param + buf;

				if (param & 0x8000) {
					param = param | ~0x7FFF;
				}

				i = i + 2;
			} else if (buf < 247) { /* 1 byte encoded int */
				param = buf - 139;
			} else if (buf < 251) { /* 2 byte encoded positive int */
				param = buf;
				buf = dByte(src_addr++);
				param = ((param - 247) * 256) + buf + 108;

				i++;
			} else if (buf < 255) { /* 2 byte encoded negative int */
				param = buf;
				buf = dByte(src_addr++);
				param = -((param - 251) * 256) - buf - 108;

				i++;
			} else { /* 5 byte encoded int */
				buf = dByte(src_addr++);
				param = param + buf;
				param = param << 8;
				buf = dByte(src_addr++);
				param = param + buf;
				param = param << 8;
				buf = dByte(src_addr++);
				param = param + buf;
				param = param << 8;
				buf = dByte(src_addr++);
				param = param + buf;

				i = i + 4;
			}

			MakeComm(comment_addr, sprintf("int param %i", param));
		}

	}

	print("Decoding End.");
}
