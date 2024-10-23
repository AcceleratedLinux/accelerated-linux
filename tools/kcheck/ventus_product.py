groups = {}
products = {}

groups['disable_ventus'] = dict(
    user = dict(
        USER_PPPD_WITH_TACACS='n',
        USER_PAM_TACACS='n',
        USER_PPPD_WITH_RADIUS='n',
        USER_PAM_RADIUS='n',
        USER_PAM_LDAP='n',
    ),
    linux = dict(
        USB_PRINTER='n',
    )
)

# THESE ARE VENTUS FEATURES. THIS SHOULD NOT BE MODIFIED UNLESS APPROVED FROM VENTUS
groups['config_skinny_features'] = dict(
    include=[
        'config_cli',
        'config_cloud',
        'config_aview',
        'config_vrrp',
        'config_ddns',
        'config_netflow',
        'config_intelliflow',
        'config_multicast',
        'config_openvpn',
        'config_wireguard',
        'config_qos',
        'config_iptunnel',
        'config_pppol2tp',
        'config_custdefcfg',
        'gnss_software',
        'config_apparmor',
        'varlog_mount',
    ],
)

groups['config_ex12_skinny'] = dict(
    include=[
        'config',
        'config_skinny_features',
        'config_cellular',
        'config_stdcpp',
        'config_serial_small',
        'config_duplicate_flash_image',
        'config_modbus_gateway',
        'config_usb_serial_ch341',
        'config_ledcmd_wrapper',
        'config_reset_button',
        'disable_ventus',
    ],
    user = dict(
        USER_BUSYBOX_DIFF='y',
    )
)

groups['config_ix_skinny'] = dict(
    include=[
        'config',
        'config_skinny_features',
        'config_stdcpp',
        'config_python',
        'config_ledcmd_wrapper',
        'config_cellular',
        'config_duplicate_flash_image',
        'config_modbus_gateway',
        'config_serial_small',
        'config_usb_serial_ch341',
        'config_power_management',
        'config_gov_powersave',
        'config_gov_ondemand',
        'config_gov_userspace',
        'config_reset_button',
        'disable_ventus',
    ],
    user = dict(
        USER_BUSYBOX_DIFF='y',
    )
)

groups['config_ix10_skinny'] = dict(
    include=[
        'config_ix_skinny',
        'config_imx_temp_scaling',
        'gnss_modem',
    ],
)

groups['config_ix20_skinny'] = dict(
    include=[
        'config_ix10_skinny',
    ]
)

groups['config_ix20w_skinny'] = dict(
    include = [
        'config_ix20_skinny',
        'config_wireless',
        'config_restrict_5g_channels_ap_only',
    ],
)

groups['config_ix30_skinny'] = dict(
    include = [
        'config_ix20_skinny',
    ],
    user=dict(
        PROP_CONFIG_IOD='y',
        PROP_CONFIG_IOD_AIN='y',
    )
)

groups['config_ex15_skinny'] = dict(
    include=[
        'config',
        'config_skinny_features',
        'config_cellular',
        'config_stdcpp',
        'config_serial_small',
        'config_duplicate_flash_image',
        'config_modbus_gateway',
        'config_usb_serial_ch341',
        'config_reset_button',
        'gnss_modem',
        'config_ledcmd_wrapper',
        'disable_ventus',
    ],
    user = dict(
        USER_BUSYBOX_DIFF='y',
    )
)

groups['config_ex15w_skinny'] = dict(
    include=[
        'config_ex15_skinny',
        'config_wireless',
        'config_restrict_5g_channels_ap_only',
    ],
    linux=dict(
        OVERLAY_FS='n',
        OVERLAY_FS_REDIRECT_ALWAYS_FOLLOW='n',
    ),
    user=dict(
        USER_LINUX_FIRMWARE_QCA988X_XOS='y',
    ),
)

products['Digi EX12-V'] = dict(
    vendor='Digi',
    product='EX12-V',
    arch='arm',
    tools='arm-linux-musleabi-20220202',
    libc='musl',
    include=[
        'hardware_ex12',
        'config_ex12_skinny',
        'strongswan_netkey',
        'linux_crypto_arm',
        'linux_i2c',
        'linux_gpio_keys',
    ],
    user=dict(
        PROP_BOOTLOADER_UPDATE='y',
        PROP_BOOTLOADER_UPDATE_IMAGE="http://eng.digi.com/builds/DAL/release/bootloaders/EX12-21.5.56.54-202105191152-bloader-signed.bin",
        PROP_BOOTLOADER_UPDATE_IMAGE_HASH="f1098f290c807beb92f72bf7862c8484a515ef384cb50123213d542582a0923e",
    )
)

products['Digi IX10-V'] = dict(
    vendor='Digi',
    product='IX10-V',
    arch='arm',
    tools='arm-linux-musleabi-20220202',
    libc='musl',
    include=[
        'hardware_ix10',
        'config_ix10_skinny',
        'config_all_firmware',
        'strongswan_netkey',
        'linux_crypto_arm',
        'linux_i2c',
        'linux_gpio_keys',
        'config_realport_imx6',
    ],
    user=dict(
        PROP_FIBOCOM='y',
    )
)

products['Digi IX20-V'] = dict(
    vendor='Digi',
    product='IX20-V',
    arch='arm',
    tools='arm-linux-musleabi-20220202',
    libc='musl',
    include=[
        'hardware_ix20',
        'config_ix20_skinny',
        'config_all_firmware',
        'strongswan_netkey',
        'linux_crypto_arm',
        'linux_i2c',
        'linux_gpio_keys',
        'config_realport_imx6',
    ]
)

products['Digi IX20W-V'] = dict(
    vendor = 'Digi',
    product = 'IX20W-V',
    arch = 'arm',
    tools = 'arm-linux-musleabi-20220202',
    libc = 'musl',
    include = [
        'hardware_ix20w',
        'config_ix20w_skinny',
        'strongswan_netkey',
        'linux_crypto_arm',
        'linux_i2c',
        'linux_gpio_keys',
        'config_realport_imx6',
        'config_hotspot',
    ],
    user = dict(
        USER_LINUX_FIRMWARE_SDMA_IMX6Q='y',
    )
)

products['Digi IX30-V'] = dict(
    vendor = 'Digi',
    product = 'IX30-V',
    arch = 'arm',
    tools = 'arm-linux-musleabi-20220202',
    libc = 'musl',
    include = [
        'hardware_ix30',
        'config_ix30_skinny',
        'config_rtc',
        'config_all_firmware',
        'strongswan_netkey',
        'linux_crypto_arm',
        'linux_i2c',
        'config_realport_imx6',
    ],
	user = dict(
		PROP_CONFIG_USB_SERIAL='y'
	)
)

products['Digi EX15-V'] = dict(
    vendor='Digi',
    product='EX15-V',
    arch='mips',
    tools='mips-linux-musl-20220202',
    libc='musl',
    include=[
        'hardware_ex15',
        'config_ex15_skinny',
        'config_all_firmware',
        'linux_gpio_keys',
        'strongswan_netkey',
    ],
)

products['Digi EX15W-V'] = dict(
    vendor='Digi',
    product='EX15W-V',
    arch='mips',
    tools='mips-linux-musl-20220202',
    libc='musl',
    include=[
        'hardware_ex15w',
        'config_ex15w_skinny',
        'linux_gpio_keys',
        'strongswan_netkey',
    ]
)
