// SPDX-License-Identifier: GPL-2.0

/*
 * Helper functions for finding the symbol in an ELF which is "nearest"
 * to a given address.
 */

#include <stdbool.h>

#include "modpost.h"

struct syminfo {
	unsigned int symbol_index;
	unsigned int section_index;
	Elf_Addr addr;
};

/*
 * Container used to hold an entire binary search table.
 * Entries in table are ascending, sorted first by section_index,
 * then by addr, and last by symbol_index.  The sorting by
 * symbol_index is used to ensure predictable behavior when
 * multiple symbols are present with the same address.
 */
struct symsearch {
	unsigned int table_size;
	struct syminfo table[];
};

static int syminfo_compare(const void *s1, const void *s2)
{
	const struct syminfo *sym1 = s1;
	const struct syminfo *sym2 = s2;

	if (sym1->section_index > sym2->section_index)
		return 1;
	if (sym1->section_index < sym2->section_index)
		return -1;
	if (sym1->addr > sym2->addr)
		return 1;
	if (sym1->addr < sym2->addr)
		return -1;
	if (sym1->symbol_index > sym2->symbol_index)
		return 1;
	if (sym1->symbol_index < sym2->symbol_index)
		return -1;
	return 0;
}

static unsigned int symbol_count(struct elf_info *elf)
{
	unsigned int result = 0;

	for (Elf_Sym *sym = elf->symtab_start; sym < elf->symtab_stop; sym++) {
		if (is_valid_name(elf, sym))
			result++;
	}
	return result;
}

/*
 * Populate the search array that we just allocated.
 * Be slightly paranoid here.  The ELF file is mmap'd and could
 * conceivably change between symbol_count() and symsearch_populate().
 * If we notice any difference, bail out rather than potentially
 * propagating errors or crashing.
 */
static void symsearch_populate(struct elf_info *elf,
			       struct syminfo *table,
			       unsigned int table_size)
{
	bool is_arm = (elf->hdr->e_machine == EM_ARM);

	for (Elf_Sym *sym = elf->symtab_start; sym < elf->symtab_stop; sym++) {
		if (is_valid_name(elf, sym)) {
			if (table_size-- == 0)
				fatal("%s: size mismatch\n", __func__);
			table->symbol_index = sym - elf->symtab_start;
			table->section_index = get_secindex(elf, sym);
			table->addr = sym->st_value;

			/*
			 * For ARM Thumb instruction, the bit 0 of st_value is
			 * set if the symbol is STT_FUNC type. Mask it to get
			 * the address.
			 */
			if (is_arm && ELF_ST_TYPE(sym->st_info) == STT_FUNC)
				table->addr &= ~1;

			table++;
		}
	}

	if (table_size != 0)
		fatal("%s: size mismatch\n", __func__);
}

void symsearch_init(struct elf_info *elf)
{
	unsigned int table_size = symbol_count(elf);

	elf->symsearch = NOFAIL(malloc(sizeof(struct symsearch) +
				       sizeof(struct syminfo) * table_size));
	elf->symsearch->table_size = table_size;

	symsearch_populate(elf, elf->symsearch->table, table_size);
	qsort(elf->symsearch->table, table_size,
	      sizeof(struct syminfo), syminfo_compare);
}

void symsearch_finish(struct elf_info *elf)
{
	free(elf->symsearch);
	elf->symsearch = NULL;
}

static Elf_Sym *symsearch_find(struct elf_info *elf, Elf_Addr addr,
			       unsigned int secndx, bool allow_negative,
			       Elf_Addr min_distance,
			       bool (*filter)(const Elf_Sym *, const Elf_Sym *, void *),
			       void *filter_data)
{
	const struct syminfo *table = elf->symsearch->table;
	unsigned int table_size = elf->symsearch->table_size;
	unsigned int hi = table_size;
	unsigned int lo = 0;
	struct syminfo target;

	target.addr = addr;
	target.section_index = secndx;
	target.symbol_index = ~0;  /* compares greater than any actual index */
	while (hi > lo) {
		unsigned int mid = lo + (hi - lo) / 2;  /* Avoids overflow */

		if (syminfo_compare(&table[mid], &target) > 0)
			hi = mid;
		else
			lo = mid + 1;
	}

	/*
	 * table[hi], if it exists, is the first entry in the array which
	 * lies beyond target.  table[hi - 1], if it exists, is the last
	 * entry in the array which comes before target, including the
	 * case where it perfectly matches the section and the address.
	 *
	 * If there are multiple candidates, the filter() callback can be used
	 * to break a tie. filter() is provided with the current symbol and the
	 * best one so far. If it returns true, the current one is selected.
	 * Only a few iterations are expected, hence the linear search is fine.
	 */
	Elf_Addr distance;
	Elf_Sym *best = NULL;
	Elf_Sym *sym;
	int i;

	/* Search to the left. */
	for (i = hi - 1; i >= 0; i--) {
		if (table[i].section_index != secndx)
			break;

		distance = addr - table[i].addr;
		if (distance > min_distance)
			break;

		sym = &elf->symtab_start[table[i].symbol_index];
		if (filter(sym, best, filter_data)) {
			min_distance = distance;
			best = sym;
		}
	}

	if (!allow_negative)
		return best;

	/* Search to the right if allow_negative is true. */
	for (i = hi; i < table_size; i++) {
		if (table[i].section_index != secndx)
			break;

		distance = table[i].addr - addr;
		if (distance > min_distance)
			break;

		sym = &elf->symtab_start[table[i].symbol_index];
		if (filter(sym, best, filter_data)) {
			min_distance = distance;
			best = sym;
		}
	}

	return best;
}

/* Return true if sym1 is preferred over sym2. */
static bool symsearch_nearest_filter(const Elf_Sym *sym1, const Elf_Sym *sym2,
				     void *data)
{
	struct elf_info *elf = data;
	unsigned int bind1, bind2, unscores1, unscores2;

	/* If sym2 is NULL, this is the first occurrence, always take it. */
	if (sym2 == NULL)
		return true;

	/* Prefer lower address. */
	if (sym1->st_value < sym2->st_value)
		return true;
	if (sym1->st_value > sym2->st_value)
		return false;

	bind1 = ELF_ST_BIND(sym1->st_info);
	bind2 = ELF_ST_BIND(sym2->st_info);

	/* Prefer global symbol. */
	if (bind1 == STB_GLOBAL && bind2 != STB_GLOBAL)
		return true;
	if (bind1 != STB_GLOBAL && bind2 == STB_GLOBAL)
		return false;

	/* Prefer less underscores. */
	unscores1 = strspn(sym_name(elf, sym1), "_");
	unscores2 = strspn(sym_name(elf, sym2), "_");

	return unscores1 < unscores2;
}

/*
 * Find the syminfo which is in secndx and "nearest" to addr.
 * allow_negative: allow returning a symbol whose address is > addr.
 * min_distance: ignore symbols which are further away than this.
 *
 * Returns a pointer into the symbol table for success.
 * Returns NULL if no legal symbol is found within the requested range.
 */
Elf_Sym *symsearch_find_nearest(struct elf_info *elf, Elf_Addr addr,
				unsigned int secndx, bool allow_negative,
				Elf_Addr min_distance)
{
	return symsearch_find(elf, addr, secndx, allow_negative, min_distance,
			      symsearch_nearest_filter, elf);
}

struct name_filter_data {
	struct elf_info *elf;
	const char *name;
};

static bool symsearch_name_filter(const Elf_Sym *sym1, const Elf_Sym *sym2,
				  void *_data)
{
	struct name_filter_data *data = _data;
	const char *name;

	/* Check the symbol name. */
	name = sym_name(data->elf, sym1);
	if (strcmp(name, data->name))
		return false;

	/* If sym2 is NULL, this is the first occurrence, always take it. */
	if (!sym2)
		return true;

	/* Prefer lower address. */
	return sym1->st_value < sym2->st_value;
}

/*
 * Find the symbol which is in secndx and has the given name, and is located
 * close enough to the given address.
 * allow_negative: allow returning a symbol whose address is > addr.
 * min_distance: ignore symbols which are further away than this.
 * name: the name of the symbol to search for.
 *
 * Returns a pointer into the symbol table for success.
 * Returns NULL if no legal symbol is found within the requested range.
 */
Elf_Sym *symsearch_find_with_name(struct elf_info *elf, Elf_Addr addr,
				  unsigned int secndx, bool allow_negative,
				  Elf_Addr min_distance, const char *name)
{
	struct name_filter_data data = { .elf = elf, .name = name };

printf("%s(%d): find_with_name(name=%s)\n", __FILE__, __LINE__, name);
	return symsearch_find(elf, addr, secndx, allow_negative, min_distance,
			      symsearch_name_filter, &data);
}
