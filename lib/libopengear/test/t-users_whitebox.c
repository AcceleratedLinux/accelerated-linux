/* t-users-whitebox.c - Whitebox testing of libopengear users functions
 * Whitebox testing is achieved by including the users.c directly so that all static
 * functions become available to the unit tests. This is useful for proving some
 * basic functions that aren't exposed in the API. */

#include <../src/users.c>
#include "opengear/xml.h"
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>


/*****************************************************************************
 * Mocks
 * These functions replace system and external library calls with simulations
 * that we can control
 */

#include "usergroup_mocks.c"
#include "serial_count_mock.c"

// Copied from pwgrp.c
int
group_has_mem(const struct group *gr, const char *username)
{
	char **mem;

	if (!gr->gr_mem)
		return 0;
	for (mem = gr->gr_mem; *mem; mem++) {
		if (strcmp(*mem, username) == 0)
			return 1;
	}
	return 0;
}


/*
 * Reset Mocks
 */
static void reset_mocks() {
	reset_usergroup_mocks();
	mock_opengear_serial_count_set(4);
}

/*****************************************************************************
 * Setup Functions
 * These functions do common setup tasks used by the unit tests
 */

#include "usergroup_setup.c"
#include "xml_config_setup.c"

/*
 * Include snow as late as possible and try and do setup etc before including it.
 * In particular it redfines assert
 */
#undef assert
#include <snow.h>

/*
 * This whitebox testing is used to prove the get_roles function with various
 * combinations that would be hard to setup and test if we couldn't just call
 * the function directly with mocked data.
 */

/* describe and subdesc - these are for grouping the tests into contexts */
describe(get_roles, {
	subdesc(empty_config, {
		/* it - for describe test items. Aim to have just one
		 * behaviour checked in each test item. */
		it("loads ok with uid 0", {
			/* Setup the test. Snow doesn't support setup
			 * and teardown outside of the tests */
			reset_mocks();
			xmldb_t *db = setup_empty_config();
			 /* defer is used to teardown. It executes the
			  * provided function when the scope is exited
			  */
			defer(xmldb_close(db));

			/* Run the test */
			roles_t roles = get_roles(db, 0, 0);
			defer(roles_free(&roles));

			/* Check the result */
			assert(roles == NO_ROLES);

		});

		it("loads ok with uid 1", {
			reset_mocks();
			xmldb_t *db = setup_empty_config();
			defer(xmldb_close(db));

			roles_t roles = get_roles(db, 1, 1);
			defer(roles_free(&roles));

			assert(roles == NO_ROLES);
		});
	});

	/* The get_roles function should work in the case where users exist in /etc/passwd
	 * but don't appear in config.xml
	 */
	subdesc(empty_config_user_in_password_database, {
		it("loads ok with uid 0", {
			reset_mocks();
			xmldb_t *db = setup_empty_config();
			defer(xmldb_close(db));
			setup_basic_user1();

			roles_t roles = get_roles(db, 0, 0);
			defer(roles_free(&roles));

			assert(roles == NO_ROLES);
			assert(!mock_is_grent_db_open());
		});

		it("loads ok with uid 1", {
			reset_mocks();
			xmldb_t *db = setup_empty_config();
			defer(xmldb_close(db));
			setup_basic_user1();

			roles_t roles = get_roles(db, 1, 1);
			defer(roles_free(&roles));

			assert(roles == NO_ROLES);
			assert(!mock_is_grent_db_open());
		});
	});

	/* The get_roles function should detect the root user even with empty config.xml */
	subdesc(empty_config_root_user_in_password_database, {
		it("gives admin role uid=0 gid=0", {
			reset_mocks();
			xmldb_t *db = setup_empty_config();
			defer(xmldb_close(db));
			setup_root_user();

			roles_t roles = get_roles(db, 0, 0);
			defer(roles_free(&roles));

			assert(roles_has(roles, ROLE_ADMIN));
			assert(!mock_is_grent_db_open());
		});

		it("gives admin role uid=0 gid=NO_GID", {
			reset_mocks();
			xmldb_t *db = setup_empty_config();
			defer(xmldb_close(db));
			setup_root_user();

			roles_t roles = get_roles(db, 0, NO_GID);
			defer(roles_free(&roles));

			assert(roles_has(roles, ROLE_ADMIN));
			assert(!mock_is_grent_db_open());
		});

		it("Doesn't recongnise root group as admin uid=NO_UID gid=0", {
			reset_mocks();
			xmldb_t *db = setup_empty_config();
			defer(xmldb_close(db));
			setup_root_user();

			roles_t roles = get_roles(db, NO_UID, 0);
			defer(roles_free(&roles));

			assert(!roles_has(roles, ROLE_ADMIN));
			assert(!mock_is_grent_db_open());
		});
	});

	/* Check the all_ports_user role for different combinations of uid and gid */
	subdesc(user_config_all_ports_user_role, {
		it("finds all ports user role uid=1 gid=1", {
			reset_mocks();
			xmldb_t *db = setup_empty_config();
			defer(xmldb_close(db));
			setup_basic_user1();
			config_role(db, "user1", "all_ports_user");

			roles_t roles = get_roles(db, 1, 1);
			defer(roles_free(&roles));

			assert(roles_has(roles, ROLE_ALL_PORTS_USER));
			assert(!roles_has(roles, ROLE_ADMIN));
			assert(!mock_is_grent_db_open());
			assert(!mock_is_grent_db_open());
		});

		it("finds all ports user role uid=1 gid=NO_GID", {
			reset_mocks();
			xmldb_t *db = setup_empty_config();
			defer(xmldb_close(db));
			setup_basic_user1();
			config_role(db, "user1", "all_ports_user");

			roles_t roles = get_roles(db, 1, NO_GID);
			defer(roles_free(&roles));

			assert(roles_has(roles, ROLE_ALL_PORTS_USER));
			assert(!roles_has(roles, ROLE_ADMIN));
			assert(!mock_is_grent_db_open());
		});

		it("finds all ports user role uid=NO_UID gid=1", {
			reset_mocks();
			xmldb_t *db = setup_empty_config();
			defer(xmldb_close(db));
			setup_basic_user1();
			config_role(db, "user1", "all_ports_user");

			roles_t roles = get_roles(db, 1, NO_GID);
			defer(roles_free(&roles));

			assert(roles_has(roles, ROLE_ALL_PORTS_USER));
			assert(!roles_has(roles, ROLE_ADMIN));
			assert(!mock_is_grent_db_open());
		});

		it("Gives no roles when uid=NO_UID and gid=NO_GID", {
			reset_mocks();
			xmldb_t *db = setup_empty_config();
			defer(xmldb_close(db));
			setup_basic_user1();
			config_role(db, "user1", "all_ports_user");

			roles_t roles = get_roles(db, NO_UID, NO_GID);
			defer(roles_free(&roles));

			assert(roles == NO_ROLES);
			assert(!mock_is_grent_db_open());
		});
	});

	subdesc(user_config_basic_webui_user_role, {
		it("finds basic webui user role", {
			reset_mocks();
			xmldb_t *db = setup_empty_config();
			defer(xmldb_close(db));
			setup_basic_user1();
			config_role(db, "user1", "basic_webui_user");

			roles_t roles = get_roles(db, 1, 1);
			defer(roles_free(&roles));

			assert(roles_has(roles, ROLE_BASIC_WEBUI_USER));
			assert(!roles_has(roles, ROLE_ALL_PORTS_USER));
			assert(!roles_has(roles, ROLE_ADMIN));
			assert(!mock_is_grent_db_open());
		});
	});

	subdesc(dummy, {
		it("dummy", {
			/* shush compiler warnings about unused functions */
			setup_user_in_admin_group();
			setup_basic_user1_with_2_groups();
			xmldb_t *db = setup_empty_config();
			defer(xmldb_close(db));
			config_set_user1_ports(db, NULL, 0);
			config_set_group_ports(db, "group1", NULL, 0);
			reset_mocks();
		});
	});
});

snow_main();

