/* serial_count_mock.c */

static int mock_opengear_serial_count;
static void mock_opengear_serial_count_set(int num_ports) {
	mock_opengear_serial_count = num_ports;
}
int opengear_serial_count(void) {
	return mock_opengear_serial_count;
}

int opengear_cascaded_node_count(const scew_element *root) {
	return 0;
}
