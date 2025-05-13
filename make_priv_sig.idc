/*
Copyright (c) 2025, Konrad Rzepecki <hannibal@astral.lodz.pl>

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.
*/

#include <idc.idc>

// Different ida define it with different names so do it on our own.
#define hasUname(F) ((F & FF_ANYNAME) == FF_NAME)

extern sig_bytes;
extern min_sig_bytes;
extern sig_count;
extern var_count;
extern var_array;
extern call_count;
extern call_array;
extern format_version;
extern is_debug;

static initGlobals(void) {
	sig_bytes = 16;		// default length of signature
	min_sig_bytes = 6;	// minimal length of signature in case of short function
	sig_count = 0;		// number of signatures
	var_count = 0;		// number of named variables xfer in signatured functions
	var_count = 0;		// number of named call xfer in signatured functions
	format_version = 1;	// fromat version in case of future improvements
	is_debug = 1;		// enable debug messages

	delete_array(get_array_id("VarSigArray")); // if we run again we must delete prevoius array (i case of existence)
	delete_array(get_array_id("CallSigArray")); // if we run again we must delete prevoius array (i case of existence)
	var_array = create_array("VarSigArray");
	call_array = create_array("CallSigArray");
}

static cleanup(void) {
	delete_array(get_array_id("VarSigArray")); // don't polute database
	delete_array(get_array_id("CallSigArray"));
}

static strncmp(str1, str2, n) {
	auto cmp;
	auto i;

	for (i = 0; i < n; i++) {
		cmp = ord(str1[i]) - ord(str2[i]);
		if (cmp != 0) {
			return cmp;
		}
	}

	return 0;
}

static findFirstFunction(void) {
	auto ea;
	auto ea0;

	ea = NextFunction(0);
	if (ea == -1) {
		return PrevFunction(1);
	} else {
		ea0 = PrevFunction(ea);
		if (ea0 == -1) {
			return ea;
		} else {
			return ea0;
		}
	}
}

static isUserFunction(name) {
	if (strncmp(name, "sub_", 4) == 0) {
		return 0;
	} else if (strncmp(name, "nullsub_", 8) == 0) {
		return 0;
	} else {
		return 1;
	}
}

static dumpFormatVersion(fd) {
	auto out;
	out = sprintf("v:%i::This must be first entry in file.\n", format_version);

	if (is_debug) {
		Message("Signature line: " + out);
	}
	writestr(fd, out);
}

static dumpFuncSig(fd, name, startea, maxlen) {
	auto out;
	auto i;

	out = sprintf("%c:%s:", 'f', name);
	for (i = 0; i < maxlen && i < sig_bytes; i++) {
		out = sprintf("%s%02X", out, Byte(startea + i));
	}
	out = sprintf("%s:\n", out);

	if (is_debug) {
		Message("Signature line: " + out);
	}
	writestr(fd, out);
}

// Find all data named by user referenced in recognized function.
static markFuncVars(func_name, startea, maxlen) {
	auto i;
	auto dea;
	auto nextdea;
	auto sig;

	for (i = 0; i < maxlen; i++) {
		dea = Dfirst(startea + i); // Find first interested data xfer...
		while ((dea != -1) && (XrefType() > dr_R)) {
			dea = Dnext(startea + i, dea);
		}

		nextdea = Dnext(startea + i, dea); // ... and make sure it is last...
		while ((nextdea != -1) && (XrefType() > dr_R)) {
			nextdea = Dnext(startea + i, nextdea);
		}

		if ((dea != -1) && (nextdea == -1)) { //... because of many we cannot distinguish beetween them during matching.
			if (hasUname(GetFlags(dea))) {
				if (is_debug) {
					Message(sprintf("Found data xfer: 0x%08X %s.\n", dea, NameEx(-1, dea)));
				}
				sig = sprintf("d:%s:%i:%s:\n", func_name, i, NameEx(-1, dea));
				SetArrayString(var_array, dea, sig);
			}
		}

	}
}

// Find all data named by user referenced in recognized function.
static markFuncCalls(func_name, startea, maxlen) {
	auto i;
	auto cea;
	auto nextcea;
	auto sig;

	for (i = 0; i < maxlen; i++) {
		cea = Rfirst0(startea + i); // Find first interested call xfer...
		while ((cea != -1) && (XrefType() < fl_CF) && (XrefType() > fl_JN)) {
			cea = Rnext0(startea + i, cea);
		}

		nextcea = Rnext0(startea + i, cea); // ... and make sure it is last...
		while ((nextcea != -1) && (XrefType() < fl_CF) && (XrefType() > fl_JN)) {
			nextcea = Rnext0(startea + i, nextcea);
		}

		if ((cea != -1) && (nextcea == -1)) { //... because of many we cannot distinguish beetween them during matching.
			if (hasUname(GetFlags(cea))) {
				if (is_debug) {
					Message(sprintf("Found call xfer: 0x%08X %s.\n", cea, NameEx(-1, cea)));
				}
				sig = sprintf("c:%s:%i:%s:\n", func_name, i, NameEx(-1, cea));
				SetArrayString(call_array, cea, sig);
			}
		}

	}
}

// Dump all marked vars
static dumpVarSigs(fd) {
	auto ea;
	auto sig;

	ea = GetFirstIndex(AR_STR, var_array);
	while (ea != -1) {
		sig = GetArrayElement(AR_STR, var_array, ea);

		if (is_debug) {
			Message("Signature line: " + sig);
		}
		writestr(fd, sig);
		var_count++;

		ea = GetNextIndex(AR_STR, var_array, ea);
	}
}

// Dump all marked calls
static dumpCallSigs(fd) {
	auto ea;
	auto sig;

	ea = GetFirstIndex(AR_STR, call_array);
	while (ea != -1) {
		sig = GetArrayElement(AR_STR, call_array, ea);

		if (is_debug) {
			Message("Signature line: " + sig);
		}
		writestr(fd, sig);
		call_count++;

		ea = GetNextIndex(AR_STR, call_array, ea);
	}
}

static main(void) {
	auto file;
	auto fd;
	auto func_start;
	auto func_end;
	auto func_len;
	auto func_name;
	auto i;

	initGlobals();
	file = AskFile(1, "*", "Enter signature filename");
	fd = fopen(file, "wb");
	if (fd == 0) {
		Message("Unable to open signature file.\n");
		cleanup();
		return;
	}

	Message("Starting search for signatures to write...\n");

	dumpFormatVersion(fd);

	func_start = findFirstFunction();
	for (i = 0; func_start != -1; i++ ) {
		func_name = GetFunctionName(func_start);
		if (isUserFunction(func_name)) {
			func_end = FindFuncEnd(func_start);
			func_len = func_end - func_start;
			if (func_len >= min_sig_bytes) {
				dumpFuncSig(fd, func_name, func_start, func_len);
				markFuncVars(func_name, func_start, func_len);
				markFuncCalls(func_name, func_start, func_len);

				sig_count++;
			} else {
				if (is_debug) {
					Message(sprintf("Skipping func %s - too short (%i).\n", func_name, func_len));
				}
			}
		}

		func_start = NextFunction(func_start);
	}

	dumpVarSigs(fd);
	dumpCallSigs(fd);

	fclose(fd);
	Message(sprintf("Written:\n%4i signatures\n%4i xref data\n%4i xref call\n", sig_count, var_count, call_count));
	cleanup();
}
