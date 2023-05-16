/* t-users-blackbox.c - Blackbox testing of libopengear. Blackbox testing using
 * snow is achieved by linking against libopengear and exercising API functions
 * only.
 */
#include "opengear/users.h"
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

static void setup_ports(bool **ports, int num_ports) {
	mock_opengear_serial_count_set(num_ports);
	*ports = calloc(num_ports, sizeof(bool));
	assert(*ports != NULL);
}

static void setup_random_ports(bool *ports, int num_ports) {
	int i;
	srand(time(0));
	for (i = 0; i < num_ports; ++i) {
		ports[i] = rand() > (RAND_MAX / 2);
	}
}

/*
 * Include snow as late as possible and try and do setup etc before including it.
 * In particular it redfines assert
 */
#undef assert
#include <snow.h>

/*
 * This blackbox testing is used to prove the opengear_users_getallowedports
 * function with various combinations without access to internal structures.
 */

/* describe and subdesc - these are for grouping the tests into contexts */
describe(opengear_users_getallowedports, {
	subdesc(empty_config, {
		/* it - for describe test items. Aim to have just one
		 * behaviour checked in each test item. */
		it("Handles empty case uid 0 (root) with 0 ports", {
			/* Setup the test. Snow doesn't support setup
			 * and teardown outside of the tests */
			reset_mocks();
			bool *ports = NULL;
			xmldb_t *db = setup_empty_config();
			defer(xmldb_close(db));

			/* Run the test */
			opengear_users_getallowedports(db, ports, 0, 0, 0);

			/* Check the result */
			assert(!mock_is_grent_db_open());
		});

		it ("Allows access to all ports for uid 0 (root) with 1 port", {
			reset_mocks();
			bool *ports = NULL;
			xmldb_t *db = setup_empty_config();
			defer(xmldb_close(db));
			setup_ports(&ports, 1);
			defer(free(ports));

			opengear_users_getallowedports(db, ports, 1, 0, 0);

			assert(ports[0]);
			assert(!mock_is_grent_db_open());
		});

		it ("Allows access to all ports for uid 0 (root) with 96 ports", {
			reset_mocks();
			bool *ports = NULL;
			xmldb_t *db = setup_empty_config();
			defer(xmldb_close(db));
			setup_ports(&ports, 96);
			defer(free(ports));

			opengear_users_getallowedports(db, ports, 96, 0, 0);

			int i;
			for (i = 0; i < 96; i++) {
				assert(ports[i]);
			}
			assert(!mock_is_grent_db_open());
		});

		it("Handles empty case uid 1 with 0 ports", {
			reset_mocks();
			bool *ports = NULL;
			xmldb_t *db = setup_empty_config();
			defer(xmldb_close(db));

			opengear_users_getallowedports(db, ports, 0, 1, 1);

			assert(!mock_is_grent_db_open());
		});

		it ("Denies access to all ports for uid 1 with 1 port", {
			reset_mocks();
			bool *ports = NULL;
			xmldb_t *db = setup_empty_config();
			defer(xmldb_close(db));
			setup_ports(&ports, 1);
			defer(free(ports));

			opengear_users_getallowedports(db, ports, 1, 1, 1);

			assert(!ports[0]);
			assert(!mock_is_grent_db_open());
		});

		it ("Denies access to all ports for uid 1 with 96 ports", {
			reset_mocks();
			bool *ports = NULL;
			xmldb_t *db = setup_empty_config();
			defer(xmldb_close(db));
			setup_ports(&ports, 96);
			defer(free(ports));

			opengear_users_getallowedports(db, ports, 96, 1, 1);

			int i;
			for (i = 0; i < 96; i++) {
				assert(!ports[i]);
			}
			assert(!mock_is_grent_db_open());
		});

	});

	subdesc(empty_config_user_in_password_database, {
		it("Handles empty case with 0 ports", {
			reset_mocks();
			bool *ports = NULL;
			xmldb_t *db = setup_empty_config();
			defer(xmldb_close(db));
			setup_basic_user1();

			opengear_users_getallowedports(db, ports, 0, 1, 1);

			assert(!mock_is_grent_db_open());
		});


		it ("Denies access to all ports with 1 port", {
			reset_mocks();
			bool *ports = NULL;
			xmldb_t *db = setup_empty_config();
			defer(xmldb_close(db));
			setup_basic_user1();
			setup_ports(&ports, 1);
			defer(free(ports));

			opengear_users_getallowedports(db, ports, 1, 1, 1);

			assert(!ports[0]);
			assert(!mock_is_grent_db_open());
		});

		it ("Denies access to all ports with 96 ports", {
			reset_mocks();
			bool *ports = NULL;
			xmldb_t *db = setup_empty_config();
			defer(xmldb_close(db));
			setup_basic_user1();
			setup_ports(&ports, 96);
			defer(free(ports));

			opengear_users_getallowedports(db, ports, 96, 1, 1);

			int i;
			for (i = 0; i < 96; i++) {
				assert(!ports[i]);
			}
			assert(!mock_is_grent_db_open());
		});
	});

	subdesc(empty_config_user_in_admin_group, {
		it("Handles empty case with 0 ports", {
			reset_mocks();
			bool *ports = NULL;
			xmldb_t *db = setup_empty_config();
			defer(xmldb_close(db));
			setup_user_in_admin_group();

			opengear_users_getallowedports(db, ports, 0, 1, 1);

			assert(!mock_is_grent_db_open());
		});

		it ("Allows access to all ports with 1 port", {
			reset_mocks();
			bool *ports = NULL;
			xmldb_t *db = setup_empty_config();
			defer(xmldb_close(db));
			setup_user_in_admin_group();
			setup_ports(&ports, 1);
			defer(free(ports));

			opengear_users_getallowedports(db, ports, 1, 1, 1);

			assert(ports[0]);
			assert(!mock_is_grent_db_open());
		});

		it ("Allows access to all ports with 96 ports", {
			reset_mocks();
			bool *ports = NULL;
			xmldb_t *db = setup_empty_config();
			defer(xmldb_close(db));
			setup_user_in_admin_group();
			setup_ports(&ports, 96);
			defer(free(ports));

			opengear_users_getallowedports(db, ports, 96, 1, 1);
			int i;
			for (i = 0; i < 96; i++) {
				assert(ports[i]);
			}
			assert(!mock_is_grent_db_open());
		});
	});

	subdesc(user_config_all_ports_user_role, {
		it ("Allows access to all ports", {
			reset_mocks();
			bool *ports = NULL;
			xmldb_t *db = setup_empty_config();
			defer(xmldb_close(db));
			setup_basic_user1();
			config_role(db, "user1", "all_ports_user");
			setup_ports(&ports, 96);
			defer(free(ports));

			opengear_users_getallowedports(db, ports, 96, 1, 1);

			int i;
			for (i = 0; i < 96; i++) {
				assert(ports[i]);
			}
			assert(!mock_is_grent_db_open());
		});
	});

	subdesc(user_config_basic_webui_user_role, {
		it ("Denies access to all ports", {
			reset_mocks();
			bool *ports = NULL;
			xmldb_t *db = setup_empty_config();
			defer(xmldb_close(db));
			setup_basic_user1();
			config_role(db, "user1", "basic_webui_user");
			setup_ports(&ports, 96);
			defer(free(ports));

			opengear_users_getallowedports(db, ports, 96, 1, 1);

			int i;
			for (i = 0; i < 96; i++) {
				assert(!ports[i]);
			}
			assert(!mock_is_grent_db_open());
		});
	});

	subdesc(user_ports_ACL, {
		it("Handles empty case with 0 ports", {
			reset_mocks();
			bool *ports = NULL;
			bool port_permissions[] = { true, false, false, true };
			xmldb_t *db = setup_empty_config();
			defer(xmldb_close(db));
			config_set_user1_ports(db, port_permissions, sizeof(port_permissions) / sizeof(bool));
			setup_basic_user1();

			opengear_users_getallowedports(db, ports, 0, 1, 1);
			assert(!mock_is_grent_db_open());
		});

		it ("Supports allow access 1 port", {
			reset_mocks();
			bool *ports = NULL;
			bool port_permissions[] = { true };
			xmldb_t *db = setup_empty_config();
			defer(xmldb_close(db));
			config_set_user1_ports(db, port_permissions, sizeof(port_permissions) / sizeof(bool));
			setup_basic_user1();
			setup_ports(&ports, 1);
			defer(free(ports));

			opengear_users_getallowedports(db, ports, 1, 1, 1);

			assert(ports[0]);
			assert(!mock_is_grent_db_open());
		});

		it ("supports deny access 1 port", {
			reset_mocks();
			bool *ports = NULL;
			bool port_permissions[] = { false };
			xmldb_t *db = setup_empty_config();
			defer(xmldb_close(db));
			config_set_user1_ports(db, port_permissions, sizeof(port_permissions) / sizeof(bool));
			setup_basic_user1();
			setup_ports(&ports, 1);
			defer(free(ports));

			opengear_users_getallowedports(db, ports, 1, 1, 1);

			assert(!ports[0]);
			assert(!mock_is_grent_db_open());
		});

		it ("Supports mixed access to 96 ports", {
			reset_mocks();
			bool *ports = NULL;
			xmldb_t *db = setup_empty_config();
			defer(xmldb_close(db));
			setup_basic_user1();
			setup_ports(&ports, 96);
			defer(free(ports));
			/* Make a random ACL list */
			bool port_permissions[96];
			setup_random_ports(&port_permissions[0], 96);
			config_set_user1_ports(db, port_permissions, sizeof(port_permissions) / sizeof(bool));

			opengear_users_getallowedports(db, ports, 96, 1, 1);

			for (i = 0; i < 96; i++) {
				assert(ports[i] == port_permissions[i]);
			}
			assert(!mock_is_grent_db_open());
		});
	});

	subdesc(user_in_group_admin_with_configured_ports, {
		it ("Allows access to all ports (ignores ACL)", {
			reset_mocks();
			xmldb_t *db = setup_empty_config();
			defer(xmldb_close(db));
			bool port_permissions[] = { false, true, true, false };
			config_set_user1_ports(db, port_permissions, sizeof(port_permissions) / sizeof(bool));
			bool *ports = NULL;
			setup_ports(&ports, 4);
			defer(free(ports));
			setup_user_in_admin_group();

			opengear_users_getallowedports(db, ports, 4, 1, 1);

			int i;
			for (i = 0; i < 4; i++) {
				assert(ports[i]);
			}
			assert(!mock_is_grent_db_open());
		});
	});

	subdesc(user_in_all_ports_user_role_with_configured_ports, {
		it ("Allows access to all ports (ignores ACL)", {
			reset_mocks();
			xmldb_t *db = setup_empty_config();
			defer(xmldb_close(db));
			bool port_permissions[] = { false, true, true, false };
			config_set_user1_ports(db, port_permissions, sizeof(port_permissions) / sizeof(bool));
			bool *ports = NULL;
			setup_ports(&ports, 4);
			defer(free(ports));
			setup_basic_user1();
			config_role(db, "user1", "all_ports_user");

			opengear_users_getallowedports(db, ports, 4, 1, 1);

			int i;
			for (i = 0; i < 4; i++) {
				assert(ports[i]);
			}
			assert(!mock_is_grent_db_open());
		});
	});

	subdesc(user_in_webui_user_role_with_configured_ports, {
		it ("Allows access to configured ports", {
			reset_mocks();
			xmldb_t *db = setup_empty_config();
			defer(xmldb_close(db));
			bool port_permissions[] = { false, true, true, false };
			config_set_user1_ports(db, port_permissions, sizeof(port_permissions) / sizeof(bool));
			bool *ports = NULL;
			setup_ports(&ports, 4);
			defer(free(ports));
			setup_basic_user1();
			config_role(db, "user1", "basic_webui_user");

			opengear_users_getallowedports(db, ports, 4, 1, 1);

			int i;
			for (i = 0; i < 4; i++) {
				assert(ports[i] == port_permissions[i]);
			}
			assert(!mock_is_grent_db_open());
		});
	});

	subdesc(user_has_ports_and_group_has_ports, {
		it ("Allows access based on combination of both lists of ports", {
			reset_mocks();
			xmldb_t *db = setup_empty_config();
			defer(xmldb_close(db));
			setup_basic_user1();
			bool port_permissions[96];
			setup_random_ports(&port_permissions[0], 96);
			config_set_user1_ports(db, port_permissions, sizeof(port_permissions) / sizeof(bool));
			bool group_port_permissions[96];
			setup_random_ports(&group_port_permissions[0], 96);
			config_set_group_ports(db, "user1", group_port_permissions,
				sizeof(group_port_permissions) / sizeof(bool));
			bool *ports = NULL;
			setup_ports(&ports, 96);
			defer(free(ports));

			opengear_users_getallowedports(db, ports, 96, 1, 1);

			int i;
			for (i = 0; i < 96; i++) {
				assert(ports[i] == port_permissions[i] || group_port_permissions[i]);
			}
			assert(!mock_is_grent_db_open());
		});
	});

	subdesc(user_multiple_groups_have_ports, {
		it ("Allows access based on combination of both lists of ports", {
			reset_mocks();
			xmldb_t *db = setup_empty_config();
			defer(xmldb_close(db));
			setup_basic_user1_with_2_groups();
			bool group_port_permissions1[96];
			setup_random_ports(&group_port_permissions1[0], 96);
			config_set_group_ports(db, "user1", group_port_permissions1, sizeof(group_port_permissions1) / sizeof(bool));
			bool group_port_permissions2[96];
			setup_random_ports(&group_port_permissions2[0], 96);
			config_set_group_ports(db, "group2", group_port_permissions2,
				sizeof(group_port_permissions2) / sizeof(bool));
			bool *ports = NULL;
			setup_ports(&ports, 96);
			defer(free(ports));

			opengear_users_getallowedports(db, ports, 96, 1, 1);

			int i;
			for (i = 0; i < 96; i++) {
				assert(ports[i] == group_port_permissions1[i] || group_port_permissions2[i]);
			}
			assert(!mock_is_grent_db_open());
		});
	});
	subdesc(dummy, {
		it("dummy", {
			/* shush compiler warnings about unused functions*/
			setup_root_user();
			reset_mocks();
		});
	});

});


snow_main();

