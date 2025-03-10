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

extern nodestr_id;
extern leafdstr_id;
extern leafsstr_id;
extern bversion;
extern node_count;
extern node_base;
extern node_size;
extern node_off;

static streacmp(str, ea) {
	auto i;
	auto len;
	auto val;

	len = strlen(str);

	for (i = 0; i < len; i++) {
		val = get_wide_byte(ea + i) - ord(str[i]);
		if (val != 0) {
			return val;
		}
	}
	return 0;
}

static ischar(val) {
	if ((val >= 0x20) && (val <= 0x7F)) {
		return 1;
	}
	return 0;
}

static isstring(ea, len) {
	auto i;

	if (len < 2) {
		return 0;
	}

	for (i = 0; i < len - 1; i++) {
		if (ischar(get_wide_byte(ea + i)) == 0) {
			return 0;
		}
	}
//	if ((ischar(get_wide_byte(ea + len - 1)) == 0) || (get_wide_byte(ea + len - 1) != 0)) {
//		return 0;
//	}
	return 1;
}

static checkBtreeVersion(void) {
	if (streacmp("B-tree v 1.5 (C) Pol 1990", 13) == 0) {
		bversion = 15;
		node_count = 2;
		node_base = 4;
		node_size = 4;
		node_off = 2;
	} else if (streacmp("B-tree v 1.6 (C) Pol 1990", 19) == 0) {
		bversion = 16;
		node_count = 4;
		node_base = 6;
		node_size = 6;
		node_off = 4;
	} else if (streacmp("B-tree v2", 19) == 0) {
		bversion = 20;
		node_count = 4;
		node_base = 6;
		node_size = 6;
		node_off = 4;
	} else {
		bversion = -1;
	}
	print(sprintf("Binary tree version: %i", bversion));
	return bversion;
}

static getPageCount(void) {
	auto i;

	for (i = 0;; i = i + 0x2000) {
		if (!is_loaded(i)) {
			print(sprintf("Page count: %i", i / 0x2000));
			return (i / 0x2000);
		}
	}
}

static createSegments(count) {
	auto i;
	auto base;

	add_segm_ex(0x0000, 0x2000, 0x000, 0, 0, 0, ADDSEG_NOSREG);
	set_segm_name(0x0000, "HEADER");
	for (i = 0; i < count - 1; i++) {
		base = (i * 0x2000) + 0x2000;
		add_segm_ex(base, base + 0x2000, base / 16, 0, 0, 0, ADDSEG_NOSREG);
		set_segm_name(base, sprintf("PAGE%03i", i));
	}
}

static createStructs(void) {
	auto str_id;

	str_id = add_struc(0, "NODE", 0);
	if (bversion == 15) {
		add_struc_member(str_id, "type", -1, FF_BYTE | FF_DATA, -1, 1);
		add_struc_member(str_id, "count", -1, FF_BYTE | FF_DATA, -1, 1);
	} else if (bversion == 16) {
		add_struc_member(str_id, "type", -1, FF_BYTE | FF_DATA, -1, 1);
		add_struc_member(str_id, "count", -1, FF_BYTE | FF_DATA, -1, 1);
		add_struc_member(str_id, "reserved", -1, FF_WORD | FF_DATA, -1, 2);
	} else if (bversion == 20) {
		add_struc_member(str_id, "type", -1, FF_WORD | FF_DATA, -1, 2);
		add_struc_member(str_id, "count", -1, FF_WORD | FF_DATA, -1, 2);
	}
	add_struc_member(str_id, "offset", -1, FF_WORD | FF_0OFF | FF_DATA, 0xFFFFFFFFFFFFFFFF, 2, 0xFFFFFFFFFFFFFFFF, 0, 1);
	nodestr_id = str_id;
	print(sprintf("Add struct '%s' as id %i.", "NODE", str_id));

	str_id = add_struc(2, "LEAF_DATA", 0);
	add_struc_member(str_id, "len", -1, FF_WORD | FF_0NUMD | FF_DATA, -1, 2);
	add_struc_member(str_id, "data", -1, FF_DATA, -1, 0);
	leafdstr_id = str_id;
	print(sprintf("Add struct '%s' as id %i.", "LEAF_DATA", str_id));

	str_id = add_struc(3, "LEAF_STR", 0);
	add_struc_member(str_id, "len", -1, FF_WORD | FF_0NUMD | FF_DATA, -1, 2);
	add_struc_member(str_id, "data", -1, FF_STRLIT | FF_DATA, 0, 0);
	leafsstr_id = str_id;
	print(sprintf("Add struct '%s' as id %i.", "LEAF_STR", str_id));
}

static addNodes(count) {
	auto i;
	auto j;
	auto base;
	auto val;
	auto loff;
	auto len;

	for (i = 0; i < count - 1; i++) {
		base = (i * 0x2000) + 0x2000;
		val = get_wide_word(base + node_count);
		print(sprintf("Add %i nodes at PAGE%03i", val, i));
		for (j = 0; j < val; j++) {
			create_struct(base + j * node_size + node_base, -1, "NODE");
			set_array_params(base + j * node_size + node_base, 0, 16, 2);
			loff = get_wide_word(base + j * node_size + node_base + node_off);
			if (bversion <= 16) {
				loff++;
			}
			len = get_wide_word(base + loff);
			if (isstring(base + loff + 2, len) == 1) {
				create_struct(base + loff, len + 2, "LEAF_STR");
			} else {
				create_struct(base + loff, len + 2, "LEAF_DATA");
			}
			set_array_params(base + loff, 0, 16, 4);

			loff = loff + len + 2;
			len = get_wide_word(base + loff);
			if (isstring(base + loff + 2, len) == 1) {
				create_struct(base + loff, len + 2, "LEAF_STR");
			} else {
				create_struct(base + loff, len + 2, "LEAF_DATA");
			}
			set_array_params(base + loff, 0, 16, 4);
		}
	}
}

static main(void) {
	auto page_count;

	print("SCRIPT START");

	if (checkBtreeVersion() < 0) {
		print("Unsupported Btree version");
		return;
	}

	createStructs();
	page_count = getPageCount();
	createSegments(page_count);
	addNodes(page_count);

	print("SCRIPT END");
}
