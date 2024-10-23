/*
 * Reads the named input GPIO value.
 * Hunts /dev/gpiochip* for the first line with a matching the name.
 * Returns 0 or 1 on success.
 * Returns -1 on error and sets errno (usually ENOENT).
 */
int gpio_get_input_by_name(const char *name);


/*
 * Reads an input GPIO value by its chip name and line offset.
 * The chipname can be (for example) "0", "gpiochip0" or "/dev/gpiochip0"
 * Returns 0 or 1 on success.
 * Returns -1 on error and sets errno (usually ENOENT).
 */
int gpio_get_input_by_line(const char *chip, unsigned int line);
