/* Nessus Attack Scripting Language 
 *
 * Copyright (C) 2002 - 2003 Michel Arboi and Renaud Deraison
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2,
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * In addition, as a special exception, Renaud Deraison and Michel Arboi
 * give permission to link the code of this program with any
 * version of the OpenSSL library which is distributed under a
 * license identical to that listed in the included COPYING.OpenSSL
 * file, and distribute linked combinations including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * this file, you may extend this exception to your version of the
 * file, but you are not obligated to do so.  If you do not wish to
 * do so, delete this exception statement from your version.
 *
 */
#ifndef NASL_NESSUSD_GLUE_H
#define NASL_NESSUSD_GLUE_H
tree_cell *script_timeout(lex_ctxt * );
tree_cell* script_id(lex_ctxt * );
tree_cell* script_cve_id(lex_ctxt* );
tree_cell* script_bugtraq_id(lex_ctxt* );
tree_cell* script_xref(lex_ctxt *);
tree_cell* script_see_also(lex_ctxt* );
tree_cell * script_name(lex_ctxt * );
tree_cell * script_version(lex_ctxt * );
tree_cell * script_description(lex_ctxt * );
tree_cell * script_copyright(lex_ctxt * );
tree_cell * script_summary(lex_ctxt * );
tree_cell * script_category(lex_ctxt * );
tree_cell * script_family(lex_ctxt * );
tree_cell * script_dependencie(lex_ctxt * );
tree_cell * script_require_keys(lex_ctxt * );
tree_cell * script_exclude_keys(lex_ctxt * );
tree_cell * script_require_ports(lex_ctxt * );
tree_cell * script_require_udp_ports(lex_ctxt * );
tree_cell * nasl_get_preference(lex_ctxt * );
tree_cell * script_add_preference(lex_ctxt * );
tree_cell * script_get_preference(lex_ctxt * );
tree_cell * script_get_preference_file_content(lex_ctxt * ); 
tree_cell * script_get_preference_file_location(lex_ctxt * ); 
tree_cell * safe_checks(lex_ctxt * );
tree_cell * get_kb_item(lex_ctxt * );
tree_cell * get_kb_fresh_item(lex_ctxt * );
tree_cell * get_kb_list(lex_ctxt * );
tree_cell * set_kb_item(lex_ctxt * );
tree_cell * replace_kb_item(lex_ctxt * );
tree_cell * security_hole(lex_ctxt * );
tree_cell * security_warning(lex_ctxt * );
tree_cell * security_note(lex_ctxt * );
tree_cell * nasl_scanner_get_port(lex_ctxt * );
tree_cell * nasl_scanner_add_port(lex_ctxt * );
tree_cell * nasl_scanner_status(lex_ctxt * );

tree_cell * nasl_shared_socket_register(lex_ctxt *);
tree_cell * nasl_shared_socket_acquire(lex_ctxt *);
tree_cell * nasl_shared_socket_release(lex_ctxt *);
tree_cell * nasl_shared_socket_destroy(lex_ctxt *);
#endif
