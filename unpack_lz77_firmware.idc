#include <idc.idc>

extern packed_addr;
extern unpacked_addr_start;
extern unpacked_addr_end;

static init_addr(void) {
	packed_addr = AskAddr(0x0801EDF4, "Enter packed data source address");
	unpacked_addr_start = AskAddr(0x20000000, "Enter unpacked data destination address");
	unpacked_addr_end = unpacked_addr_start + AskLong(0x27C, "Enter upacked data size");
}

static main(void) {
	auto unpacked_addr;
	auto dict_addr;
	auto buf;
	auto cmd;
	auto cnt1;
	auto cnt2;
	auto i;

	if (AskYN(0, "This script modify database and can damge it.\nIt's recomended to save database before you use it.\nDo you wish to continue?") != 1) {
		return;
	}


	init_addr();

	print("Unpacking Start.");

	unpacked_addr = unpacked_addr_start;
	while (unpacked_addr < unpacked_addr_end) {
		cmd = Byte(packed_addr++);

		if ((cmd & 7) != 0) {
			cnt1 = (cmd & 7);
		} else {
			cnt1 = Byte(packed_addr++);
		}

		if ((cmd >> 4) != 0) {
			cnt2 = (cmd >> 4);
		} else {
			cnt2 = Byte(packed_addr++);
		}

		print(sprintf("CNT1 %3i, CNT2 %3i, ZERO %i", cnt1, cnt2, (cmd >> 3) & 1));

		for (i = 0; i < cnt1 - 1; i++) {
			buf = Byte(packed_addr++);
			PatchByte(unpacked_addr++, buf);
		}

		if ((cmd & 8) != 0) {
			cnt2 = cnt2 + 2;
			dict_addr = unpacked_addr - Byte(packed_addr++);
			for (i = 0; i < cnt2; i++) {
				buf = Byte(dict_addr++);
				PatchByte(unpacked_addr++, buf);
			}
		} else {
			for (i = 0; i < cnt2; i++) {
				PatchByte(unpacked_addr++, 0);
			}
		}
	}

	print("Unpacking End.");
}
