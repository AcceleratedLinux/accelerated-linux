/* usergroup-setup.c - Common setup functions for unit tests */

/*
 * User database setup (mocked data)
 */

static struct passwd user1 = {
	.pw_name = "user1",
        .pw_passwd = "x",
        .pw_uid = 1,
        .pw_gid = 1,
        .pw_gecos = NULL,
        .pw_dir = "/home/user1",
        .pw_shell = "/bin/bash"
};

static struct passwd root_user = {
	.pw_name = "root",
        .pw_passwd = "x",
        .pw_uid = 0,
        .pw_gid = 0,
        .pw_gecos = NULL,
        .pw_dir = "/root",
        .pw_shell = "/bin/bash"
};

/* Creates user named user1 in group group1 */
static void setup_basic_user1(void) {
	static char *members[2] = {"user1", NULL};
	static struct group usergroup1[] = {
		{
			.gr_name = "user1",
			.gr_passwd = "x",
			.gr_gid = 1,
			.gr_mem = members
		}
	};
	mock_getpwuid_set(&user1);
	mock_getgrent_r_set(usergroup1, 1);
}

static void setup_root_user(void) {
	static char *members[2] = {"root", NULL};
	static struct group usergroup1[] = {
		{
			.gr_name = "root",
			.gr_passwd = "x",
			.gr_gid = 1,
			.gr_mem = members
		}
	};
	mock_getpwuid_set(&root_user);
	mock_getgrent_r_set(usergroup1, 1);
}

/* Creates user user1 in groups user1, admin */
static void setup_user_in_admin_group() {
	static char *members[2] = {"user1", NULL};
	static struct group usergroup1[] = {
		{
			.gr_name = "user1",
			.gr_passwd = "x",
			.gr_gid = 1,
			.gr_mem = members
		},
		{
			.gr_name = "admin",
			.gr_passwd = "x",
			.gr_gid = 2,
			.gr_mem = members
		}
	};
	mock_getpwuid_set(&user1);
	mock_getgrent_r_set(usergroup1, 2);
}

static void setup_basic_user1_with_2_groups(void) {
	static char *members[2] = {"user1", NULL};
	static struct group usergroup1[] = {
		{
			.gr_name = "user1",
			.gr_passwd = "x",
			.gr_gid = 1,
			.gr_mem = members
		},
		{
			.gr_name = "group2",
			.gr_passwd = "x",
			.gr_gid = 2,
			.gr_mem = members
		}
	};
	mock_getpwuid_set(&user1);
	mock_getgrent_r_set(usergroup1, 2);
}
