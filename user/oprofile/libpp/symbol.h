/**
 * @file symbol.h
 * Symbol containers
 *
 * @remark Copyright 2002, 2004 OProfile authors
 * @remark Read the file COPYING
 *
 * @author Philippe Elie
 * @author John Levon
 */

#ifndef SYMBOL_H
#define SYMBOL_H

#include "name_storage.h"
#include "growable_vector.h"
#include "format_flags.h"
#include "op_types.h"

#include <bfd.h>

#include <list>


/// for storing sample counts
typedef growable_vector<count_type> count_array_t;


/// A simple container for a fileno:linenr location.
struct file_location {
	file_location() : linenr(0) {}
	/// empty if not valid.
	debug_name_id filename;
	/// 0 means invalid or code is generated internally by the compiler
	unsigned int linenr;

	bool operator<(file_location const & rhs) const {
		// Note we sort on filename id not on string
		return filename < rhs.filename ||
		  (filename == rhs.filename && linenr < rhs.linenr);
	}
};


/// associate vma address with a file location and a samples count
struct sample_entry {
	sample_entry() : vma(0) {}
	/// From where file location comes the samples
	file_location file_loc;
	/// From where virtual memory address comes the samples
	bfd_vma vma;
	/// the samples count
	count_array_t counts;
};


/// associate a symbol with a file location, samples count and vma address
struct symbol_entry {
	symbol_entry() : size(0) {}
	/// which image this symbol belongs to
	image_name_id image_name;
	/// owning application name: identical to image name if profiling
	/// session did not separate samples for shared libs or if image_name
	/// is not a shared lib
	image_name_id app_name;
	/// file location, vma and cumulated samples count for this symbol
	sample_entry sample;
	/// name of symbol
	symbol_name_id name;
	/// symbol size as calculated by op_bfd, start of symbol is sample.vma
	size_t size;

	/**
	 * @param fl  input hint
	 *
	 * combine fl with the calculated hint. It's theoretically possible
	 * that we get a symbol where its samples pass the border line, but
	 * the start is below it, but the the hint is only used for formatting
	 */
	column_flags output_hint(column_flags fl) const;
};


/// a collection of sorted symbols
typedef std::vector<symbol_entry const *> symbol_collection;


/**
 * The public data for call-graph symbols. Each caller/callee has
 * the sample counts replaced with the relevant arc counts, whilst
 * the cg_symbol retains its self count.
 */
struct cg_symbol : public symbol_entry {
	cg_symbol(symbol_entry const & sym) : symbol_entry(sym) {}

	typedef std::vector<symbol_entry> children;

	/// all callers of this symbol
	children callers;
	/// total count of callers
	count_array_t total_caller_count;

	/// all symbols called by this symbol
	children callees;
	/// total count of callees
	count_array_t total_callee_count;
};


/// a collection of sorted callgraph symbols
typedef std::vector<cg_symbol> cg_collection;


/// for storing diff %ages
typedef growable_vector<double> diff_array_t;


/**
 * Data for a diffed symbol.
 */
struct diff_symbol : public symbol_entry  {
	diff_symbol(symbol_entry const & sym) : symbol_entry(sym) {}

	/// diff %age values for each profile class
	diff_array_t diffs;
};


/// a collection of diffed symbols
typedef std::vector<diff_symbol> diff_collection;


#endif /* !SYMBOL_H */
