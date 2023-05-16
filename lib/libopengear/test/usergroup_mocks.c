/* usergroup-mocks.c - Mock functions for getpwuid and getgrent.
 * Include this file directly from your unit test C file.
 */

/* Mock getpwuid */

static struct passwd *mock_getpwuid_result;

/* mock_getpwuid_set - Set mock data for getpwuid and getpwnam calls
 * @param pw The data that should be returned by the next call to getpwuid / getpwnam
 */
static void mock_getpwuid_set(struct passwd *pw) {
	mock_getpwuid_result = pw;
}

struct passwd *getpwuid(uid_t uid) {
	return mock_getpwuid_result;
}

struct passwd *getpwnam(const char *name) {
	return mock_getpwuid_result;
}

/* Mock getgrent */

static struct group *mock_getgrent_r_gbufs;
static size_t mock_getgrent_r_num_entries;
static int mock_getgrent_r_idx;
static bool mock_grent_db_open;

/* mock_getgrent_r_set - Set the data that should be returned by calls to getgrent.
 * @param gbufs - An array of group structs to be returned on subsequent calls to getgrent
 * @param num_entries - The size of the gbufs array.
 * Assumes all pointers in gbufs are static. Pointers are not copied so the source data
 * must remain intact while this mock is in use.
 */
static void mock_getgrent_r_set(struct group *gbufs, size_t num_entries) {
	mock_getgrent_r_gbufs = gbufs;
	mock_getgrent_r_num_entries = num_entries;
}

/* mock_is_grent_db_open - Can be used by unit tests to ensure the group db was closed correctly.
 * @return True if endgrent was called after setgrent.
 */
static bool mock_is_grent_db_open() {
	return mock_grent_db_open;
}

/* relocate_usergroup - This function mocks the functionalty of getgrgid_r which
 * allocates strings into a given buffer and sets the string pointers in struct
 * group to point into the given buffer. Assumes there is no dynamic allocations
 * to worry about because the mocks are made from static data.
 * @param usergroup - The group that is being relocated. Before calling this function
 * it is assumed that usergroup has been itialized with members pointing to the static
 * mock data. After this function has run, all the members will point to data in buf.
 * @param buf - The buffer to copy usergroup into
 * @param buf_size - The size allocated for buf
 */
static void relocate_usergroup(struct group *usergroup, char *buf, int buf_size) {
	/* Make sure the result will fit in buf */
	char **member;
	int num_members = 0;
	int sizeof_members = 0;
	for(member = usergroup->gr_mem; *member != NULL; member++) {
		num_members++;
		sizeof_members += strlen(*member) + 1;
	}
	assert(strlen(usergroup->gr_name) + 1
		+ strlen(usergroup->gr_passwd) + 1
		/* there is a null terminated array of pointers to the members strings */
		+ (num_members + 1) * sizeof(char*)
		/* members strings will be packed one after another */
		+ sizeof_members
			< buf_size);

	char *buf_cur = buf; /* buf_cur is our current write point to the storage buffer */
	/* Relocate gr_name */
	strcpy(buf_cur, usergroup->gr_name);
	usergroup->gr_name = buf_cur;
	buf_cur += strlen(usergroup->gr_name) + 1;

	/* Relocate gr_passwd */
	strcpy(buf_cur, usergroup->gr_passwd);
	usergroup->gr_passwd = buf_cur;
	buf_cur += strlen(usergroup->gr_passwd) + 1;

	/* Relocate gr_mem - this is an array of string pointers and the strings must
	 * also be allocated in buf. The strings are added into buf after the end of
	 * the gr_mem array section.
	 */

	/* Make an array of pointers in buf that will become gr_mem */
	char **member_array_start = (char **)buf_cur;
	char **member_array_cur = member_array_start;
	/* array is null terminated (+1) then string data starts */
	char **start_of_strings = member_array_start + (num_members + 1) * sizeof(char*);

	/* strings will be copied into buf after the gr_mem array. */
	buf_cur = (char *)start_of_strings;
	for(member = usergroup->gr_mem; *member != NULL; member++) {
		/* copy member into string data section */
		strcpy(buf_cur, *member);
		/* save pointer to string in the members array */
		*member_array_cur = buf_cur;
		member_array_cur++;
		/* advance position in string data */
		buf_cur += strlen(*member) + 1;
	}
	*member_array_cur = NULL; /* null terminate members array */
	usergroup->gr_mem = member_array_start;
}

void setgrent(void) {
	mock_getgrent_r_idx = 0;
	mock_grent_db_open = true;
}

void endgrent(void) {
	mock_grent_db_open = false;
}

int getgrent_r(struct group *gbuf, char *buf,
                      size_t buflen, struct group **gbufp) {
	if (mock_getgrent_r_idx < mock_getgrent_r_num_entries) {
		memcpy(gbuf, &mock_getgrent_r_gbufs[mock_getgrent_r_idx], sizeof(*gbuf));
		relocate_usergroup(gbuf, buf, buflen);
		*gbufp = gbuf;
		mock_getgrent_r_idx++;
		return 0;
	} else {
		return -1;
	}
}

struct group *getgrent(void) {
	struct group *res = NULL;
	if (mock_getgrent_r_idx < mock_getgrent_r_num_entries) {
		res = &mock_getgrent_r_gbufs[mock_getgrent_r_idx];
		mock_getgrent_r_idx++;
	}
	return res;
}

struct group *getgrgid(gid_t gid) {
	int i;
	for (i = 0; i < mock_getgrent_r_num_entries; i++) {
		if (mock_getgrent_r_gbufs[i].gr_gid == gid) {
			return &mock_getgrent_r_gbufs[i];
		}
	}
	return NULL;
}


static void reset_usergroup_mocks() {
	mock_getpwuid_set(NULL);
	mock_getgrent_r_set(NULL, 0);
}

