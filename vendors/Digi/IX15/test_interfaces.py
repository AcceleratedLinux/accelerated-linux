# Copyright 2020 Digi International Inc., All Rights Reserved
#
# This software contains proprietary and confidential information of Digi
# International Inc.  By accepting transfer of this copy, Recipient agrees
# to retain this software in confidence, to prevent disclosure to others,
# and to make no use of this software other than that for which it was
# delivered.  This is an unpublished copyrighted work of Digi International
# Inc.  Except as permitted by federal law, 17 USC 117, copying is strictly
# prohibited.
#
# Restricted Rights Legend
#
# Use, duplication, or disclosure by the Government is subject to
# restrictions set forth in sub-paragraph (c)(1)(ii) of The Rights in
# Technical Data and Computer Software clause at DFARS 252.227-7031 or
# subparagraphs (c)(1) and (2) of the Commercial Computer Software -
# Restricted Rights at 48 CFR 52.227-19, as applicable.
#
# Digi International Inc., 9350 Excelsior Blvd., Suite 700, Hopkins, MN 55343

import argparse
import configparser
import logging
import logging.handlers
import os
import random
import re
import serial
import string
import subprocess
import threading
import time

from abc import ABC, abstractmethod
from digi.xbee.devices import XBeeDevice
from digi.xbee.exception import XBeeException
from digi.xbee.models.address import XBee64BitAddress
from digi.xbee.serial import FlowControl
from enum import Enum
from serial import EIGHTBITS, STOPBITS_ONE, PARITY_NONE, PARITY_EVEN, PARITY_ODD, PARITY_MARK, PARITY_SPACE, \
    SerialException
from signal import signal, SIGINT, SIGQUIT, SIGTERM
from threading import Event

# Logger names.
_LOG_CONSOLE_HANDLER_NAME = "Interface testing console handler"
_LOG_NETWORK_HANDLER_NAME = "Interface testing network handler"

# Test names.
_TEST_BLUETOOTH = "BLUETOOTH"
_TEST_ETHERNET = "ETHERNET"
_TEST_LEDS = "LEDS"
_TEST_MODEM = "MODEM"
_TEST_SERIAL = "SERIAL"
_TEST_XBEE = "XBEE"

# Configuration sections tags.
_SEC_BLUETOOTH = "bluetooth"
_SEC_ETHERNET = "ethernet"
_SEC_LEDS = "leds"
_SEC_LOG = "logging"
_SEC_MODEM = "modem"
_SEC_SERIAL = "serial"
_SEC_XBEE = "xbee"

# Configuration parameters tags.
_PARAM_BAUDRATE = "baudrate"
_PARAM_COUNT = "count"
_PARAM_DATA_BITS = "data_bits"
_PARAM_DATA_LENGTH = "data_length"
_PARAM_ENABLE = "enable"
_PARAM_FLOW_CONTROL = "flow_control"
_PARAM_HOST = "host"
_PARAM_LEDS = "leds"
_PARAM_LED_MODES = "led_modes"
_PARAM_LOG_LEVEL = "log_level"
_PARAM_LOG_LOOP_INTERVAL = "log_loop_interval"
_PARAM_LOG_MODE = "log_mode"
_PARAM_LOG_PORT = "log_port"
_PARAM_LOG_SERVER = "log_server"
_PARAM_LOG_TO_CONSOLE = "log_to_console"
_PARAM_LOG_TO_NETWORK = "log_to_network"
_PARAM_MIN_WAIT = "min_wait"
_PARAM_MAX_WAIT = "max_wait"
_PARAM_MODEM_TEST_MODE = "modem_test_mode"
_PARAM_PACKET_SIZE = "packet_size"
_PARAM_PARITY = "parity"
_PARAM_PORT = "port"
_PARAM_PORT_TIMEOUT = "port_timeout"
_PARAM_STOP_BITS = "stop_bits"
_PARAM_STOP_ON_ERROR = "stop_on_error"
_PARAM_URL = "url"
_PARAM_VALIDATE_DATA = "validate_data"
_PARAM_XBEE_TEST_MODE = "xbee_test_mode"

# Bluetooth configuration default parameters.
_DEFAULT_BLUETOOTH_BAUDRATE = 9600
_DEFAULT_BLUETOOTH_DATA_LENGTH = 32
_DEFAULT_BLUETOOTH_ENABLE = False
_DEFAULT_BLUETOOTH_MIN_WAIT = 500
_DEFAULT_BLUETOOTH_MAX_WAIT = 1500
_DEFAULT_BLUETOOTH_PORT = "/dev/ttymxc2"
_DEFAULT_BLUETOOTH_STOP_ON_ERROR = False

# Ethernet configuration default parameters.
_DEFAULT_ETHERNET_ENABLE = True
_DEFAULT_ETHERNET_HOST = "192.168.1.1"
_DEFAULT_ETHERNET_MIN_WAIT = 100
_DEFAULT_ETHERNET_MAX_WAIT = 500
_DEFAULT_ETHERNET_PACKET_SIZE = 16
_DEFAULT_ETHERNET_COUNT = 1
_DEFAULT_ETHERNET_STOP_ON_ERROR = False

# LEDs configuration default parameters.
_DEFAULT_LEDS_ENABLE = True
_DEFAULT_LEDS_LEDS = "ALL"
_DEFAULT_LEDS_MODES = "flash"
_DEFAULT_LEDS_MIN_WAIT = 500
_DEFAULT_LEDS_MAX_WAIT = 1000
_DEFAULT_LEDS_STOP_ON_ERROR = False

# Log configuration default parameters.
_DEFAULT_LOG_ENABLE = True
_DEFAULT_LOG_LEVEL = "info"
_DEFAULT_LOG_LOOP_INTERVAL = 5
_DEFAULT_LOG_MODE = "standard"
_DEFAULT_LOG_PORT = 7360
_DEFAULT_LOG_SERVER = "192.168.1.1"
_DEFAULT_LOG_TO_CONSOLE = True
_DEFAULT_LOG_TO_NETWORK = False

# LEDs configuration default parameters.
_DEFAULT_MODEM_ENABLE = False
_DEFAULT_MODEM_COUNT = 1
_DEFAULT_MODEM_MIN_WAIT = 100
_DEFAULT_MODEM_MAX_WAIT = 1000
_DEFAULT_MODEM_PACKET_SIZE = 16
_DEFAULT_MODEM_STOP_ON_ERROR = False
_DEFAULT_MODEM_TEST_MODE = "read-imei"
_DEFAULT_MODEM_URL = "www.digi.com"

# Serial configuration default parameters.
_DEFAULT_SERIAL_BAUDRATE = 9600
_DEFAULT_SERIAL_DATA_BITS = EIGHTBITS
_DEFAULT_SERIAL_DATA_LENGTH = 32
_DEFAULT_SERIAL_ENABLE = False
_DEFAULT_SERIAL_FLOW_CONTROL = "none"
_DEFAULT_SERIAL_MIN_WAIT = 100
_DEFAULT_SERIAL_MAX_WAIT = 500
_DEFAULT_SERIAL_PARITY = "none"
_DEFAULT_SERIAL_PORT = "/dev/ttymxc1"
_DEFAULT_SERIAL_PORT_TIMEOUT = 0.1  # seconds
_DEFAULT_SERIAL_STOP_BITS = STOPBITS_ONE
_DEFAULT_SERIAL_STOP_ON_ERROR = False
_DEFAULT_SERIAL_VALIDATE_DATA = False

# XBee configuration default parameters.
_DEFAULT_XBEE_BAUDRATE = 9600
_DEFAULT_XBEE_ENABLE = False
_DEFAULT_XBEE_FLOW_CONTROL = "none"
_DEFAULT_XBEE_MIN_WAIT = 1000
_DEFAULT_XBEE_MAX_WAIT = 5000
_DEFAULT_XBEE_TEST_MODE = "read-mac"
_DEFAULT_XBEE_PORT = "/dev/ttyXBee"
_DEFAULT_XBEE_PORT_TIMEOUT = 2.0  # seconds
_DEFAULT_XBEE_STOP_ON_ERROR = False

# XBee commands
_XBEE_COMMAND_MAC_HIGH = "SH"
_XBEE_COMMAND_MAC_LOW = "SL"

# Other constants.
_LEDS_CMD = "ledcmd %s %s > /dev/null 2>&1"
_LEDS_OPTIONS = ["-o", "-O", "-f", "-s"]
_LOG_LOOP_FORMAT = "%-15s| %-9s| %-7s| %-10s| %-32s"
_PATTERN_IMEI = "^response: '([0-9]{15})'$"
_PATTERN_MODEM_NUMBER = ".*/Modem/([0-9]+) .*"
_PATTERN_WWAN_INTERFACE = "^(wwan0\\.[0-9]+): .*$"
_PING_COMMAND = "ping -c %s -s %s %s > /dev/null 2>&1"
_PING_COMMAND_MODEM = "ping -I %s -c %s -s %s %s > /dev/null 2>&1"
_XBEE_BROADCAST_DATA = "This is just a broadcast test message, you can ignore it."


class _LEDMode(Enum):
    """
    This class represents all available LED modes.
    """
    SWITCH_ON = (0, "switch-on", "-o")
    SWITCH_OFF = (1, "switch-off", "-O")
    FLASH = (2, "flash", "-f")
    SET = (3, "set-briefly", "-s")

    def __init__(self, identifier, name, argument):
        self.__identifier = identifier
        self.__name = name
        self.__argument = argument

    @property
    def identifier(self):
        """
        Returns the identifier of the _LEDMode element.

        Returns:
            Integer: the identifier of the _LEDMode element.
        """
        return self.__identifier

    @property
    def name(self):
        """
        Returns the name of the _LEDMode element.

        Returns:
            String: the name of the _LEDMode element.
        """
        return self.__name

    @property
    def argument(self):
        """
        Returns the argument of the _LEDMode element.

        Returns:
            String: the argument of the _LEDMode element.
        """
        return self.__argument

    @classmethod
    def get(cls, name):
        """
        Returns the _LEDMode for the given name.

        Args:
            name (String): the name of the _LEDMode to get.

        Returns:
            :class:`._LEDMode`: the _LEDMode with the given name, `None` if
                                there is not a _LEDMode with that name.
        """
        for led_mode in list(cls):
            if led_mode.name == name:
                return led_mode

        return None


class _Parity(Enum):
    """
    This class represents all available parity.
    """
    NONE = (0, "none", PARITY_NONE)
    ODD = (1, "odd", PARITY_ODD)
    EVEN = (2, "even", PARITY_EVEN)
    MARK = (3, "mark", PARITY_MARK)
    SPACE = (4, "space", PARITY_SPACE)
    UNKNOWN = (99, "unknown", None)

    def __init__(self, identifier, description, serial_value):
        self.__identifier = identifier
        self.__description = description
        self.__serial_value = serial_value

    @property
    def identifier(self):
        """
        Returns the identifier of the _Parity element.

        Returns:
            Integer: the identifier of the _Parity element.
        """
        return self.__identifier

    @property
    def description(self):
        """
        Returns the description of the _Parity element.

        Returns:
            String: the description of the _Parity element.
        """
        return self.__description

    @property
    def serial_value(self):
        """
        Returns the serial value of the _Parity element.

        Returns:
            String: the serial value of the _Parity element.
        """
        return self.__serial_value

    @classmethod
    def get(cls, identifier):
        """
        Returns the _Parity for the given identifier.

        Args:
            identifier (Integer): the identifier of the _Parity to get.

        Returns:
            :class:`._Parity`: the _Parity with the given identifier, ``None`` if
                               there is not a _Parity with that name.
        """
        for parity in list(cls):
            if parity.identifier == identifier:
                return parity

        return None

    @classmethod
    def get_by_description(cls, description):
        """
        Returns the _Parity for the given description.

        Args:
            description (String): the description of the _Parity to get.

        Returns:
            :class:`._FlowControl`: the _Parity with the given description, ``None`` if
                                    there is not a _Parity with that description.
        """
        for parity in list(cls):
            if parity.description == description:
                return parity

        return None


class _XBeeTestMode(Enum):
    """
    This class represents all available XBee test modes.
    """
    READ_MAC = (0, "read-mac")
    BROADCAST = (1, "broadcast")

    def __init__(self, identifier, name):
        self.__identifier = identifier
        self.__name = name

    @property
    def identifier(self):
        """
        Returns the identifier of the _XBeeTestMode element.

        Returns:
            Integer: the identifier of the _XBeeTestMode element.
        """
        return self.__identifier

    @property
    def name(self):
        """
        Returns the name of the _XBeeTestMode element.

        Returns:
            String: the name of the _XBeeTestMode element.
        """
        return self.__name

    @classmethod
    def get(cls, name):
        """
        Returns the _XBeeTestMode for the given name.

        Args:
            name (String): the name of the _XBeeTestMode to get.

        Returns:
            :class:`._XBeeTestMode`: the _XBeeTestMode with the given name, `None` if
                                     there is not a _XBeeTestMode with that name.
        """
        for xbee_test_mode in list(cls):
            if xbee_test_mode.name == name:
                return xbee_test_mode

        return None


class _ModemTestMode(Enum):
    """
    This class represents all available modem test modes.
    """
    READ_IMEI = (0, "read-imei")
    PING = (1, "ping")

    def __init__(self, identifier, name):
        self.__identifier = identifier
        self.__name = name

    @property
    def identifier(self):
        """
        Returns the identifier of the _ModemTestMode element.

        Returns:
            Integer: the identifier of the _ModemTestMode element.
        """
        return self.__identifier

    @property
    def name(self):
        """
        Returns the name of the _ModemTestMode element.

        Returns:
            String: the name of the _ModemTestMode element.
        """
        return self.__name

    @classmethod
    def get(cls, name):
        """
        Returns the _ModemTestMode for the given name.

        Args:
            name (String): the name of the _ModemTestMode to get.

        Returns:
            :class:`._ModemTestMode`: the _ModemTestMode with the given name, `None` if
                                      there is not a _ModemTestMode with that name.
        """
        for xbee_test_mode in list(cls):
            if xbee_test_mode.name == name:
                return xbee_test_mode

        return None


class _LogMode(Enum):
    """
    This class represents all available log modes.
    """
    LOOP = (0, "loop")
    STANDARD = (1, "standard")

    def __init__(self, identifier, name):
        self.__identifier = identifier
        self.__name = name

    @property
    def identifier(self):
        """
        Returns the identifier of the _LogMode element.

        Returns:
            Integer: the identifier of the _LogMode element.
        """
        return self.__identifier

    @property
    def name(self):
        """
        Returns the name of the _LogMode element.

        Returns:
            String: the name of the _LogMode element.
        """
        return self.__name

    @classmethod
    def get(cls, name):
        """
        Returns the _LogMode for the given name.

        Args:
            name (String): the name of the _LogMode to get.

        Returns:
            :class:`._LogMode`: the _LogMode with the given name, `None` if
                                there is not a _LogMode with that name.
        """
        for log_mode in list(cls):
            if log_mode.name == name:
                return log_mode

        return None


class _TestStatus(Enum):
    """
    This class represents all available test status.
    """
    WAITING = (0, "WAITING")
    ERROR = (1, "ERROR")
    OK = (2, "OK")

    def __init__(self, identifier, name):
        self.__identifier = identifier
        self.__name = name

    @property
    def identifier(self):
        """
        Returns the identifier of the _TestStatus element.

        Returns:
            Integer: the identifier of the _TestStatus element.
        """
        return self.__identifier

    @property
    def name(self):
        """
        Returns the name of the _TestStatus element.

        Returns:
            String: the name of the _TestStatus element.
        """
        return self.__name


class _InterfaceTest(ABC):
    """
    Helper class used to control and manage an interface test.
    """

    def __init__(self, name, test_config, logger, log_mode):
        """
        Instantiates a new `_InterfaceTest` abstract class with the given parameters.

        Args:
            name (String): the test name.
            test_config (:class:`.configparser.SectionProxy`): the interface test configuration.
            logger (:class:`logging.Logger`): The logger.
            log_mode (:class:`.LogMode`): The logging mode.
        """
        self._name = name
        self._test_config = test_config
        self._logger = logger
        self._log_mode = log_mode
        self._status = _TestStatus.WAITING
        self._status_details = ""
        self._running = False
        self._count = 0
        self._lock = Event()
        self._enabled = self._test_config.getboolean(_PARAM_ENABLE)
        self._thread = None
        self._stopped = False

    @abstractmethod
    def _execute_test(self):
        """
        Executes custom interface test code.
        """
        pass

    @property
    def name(self):
        """
        Retrieves the test name.

        Returns:
            String: the test name
        """
        return self._name

    @property
    def config(self):
        """
        Retrieves the test configuration.

        Returns:
            :class:`.configparser.SectionProxy`: the test configuration.
        """
        return self._test_config

    @property
    def logger(self):
        """
        Retrieves the test logger.

        Returns:
            :class:`logging.Logger`: the test logger.
        """
        return self._logger

    @property
    def log_mode(self):
        """
        Retrieves the test log mode.

        Returns:
            :class:`.LogMode`: the test log mode.
        """
        return self._log_mode

    @property
    def status(self):
        """
        Retrieves the test status.

        Param:
            :class:`._TestStatus`: the test status.
        """
        return self._status

    @property
    def status_details(self):
        """
        Retrieves the test status details.

        Returns:
            String: the test status details.
        """
        return self._status_details

    @property
    def count(self):
        """
        Returns the test count.

        Returns:
            Integer: the test count.
        """
        return self._count

    @property
    def enabled(self):
        """
        Returns whether the test is enabled or not.

        Returns:
            Boolean: `True` if the test is enabled, `False` otherwise.
        """
        return self._enabled

    @property
    def running(self):
        """
        Returns whether the test is running or not.

        Returns:
            Boolean: `True` if the test is running, `False` otherwise.
        """
        return self._running

    def start(self):
        """
        Starts the interface test.
        """
        if not self._enabled or self._running:
            return

        self._status_details = ""
        self._lock.clear()
        self._running = True
        self._logger.info("Starting %s test..." % self._name)
        self._thread = threading.Thread(target=self._execute_test)
        self._thread.start()

    def stop(self):
        """
        Stops the test.
        """
        if not self._running:
            return

        self._stopped = True
        self._logger.info("Stopping %s test..." % self._name)
        self._lock.set()

    def _log_debug(self, message):
        """
        Logs the given message as debug.

        Params:
            message (String): the message to log.
        """
        if self._log_mode == _LogMode.STANDARD:
            self._logger.debug("[%s] %s" % (self._name, message))

    def _log_info(self, message):
        """
        Logs the given message as info.

        Params:
            message (String): the message to log.
        """
        if self._log_mode == _LogMode.STANDARD:
            self._logger.info("[%s] %s" % (self._name, message))

    def _log_warning(self, message):
        """
        Logs the given message as warning.

        Params:
            message (String): the message to log.
        """
        if self._log_mode == _LogMode.STANDARD:
            self._logger.warning("[%s] %s" % (self._name, message))

    def _log_error(self, message):
        """
        Logs the given message as error.

        Params:
            message (String): the message to log.
        """
        if self._log_mode == _LogMode.STANDARD:
            self._logger.error("[%s] %s" % (self._name, message))

    def _calculate_wait_time(self):
        """
        Calculates the wait time for the test based on a random value the test configuration
        minimum and maximum wait time.

        Return:
            Float: the calculated wait time for the given configuration section (in seconds).
        """
        min_val = self._test_config.getint(_PARAM_MIN_WAIT)
        max_val = self._test_config.getint(_PARAM_MAX_WAIT)
        # Avoid negative wait times.
        if min_val >= max_val or self._stopped:
            return 0
        wait_time = random.randint(min_val, max_val)
        # Convert time to seconds.
        return wait_time / 1000


class _BluetoothTest(_InterfaceTest):
    """
    Bluetooth interface test. Exercises the bluetooth interface by sending random data to the
    configured Bluetooth port.
    """

    def __init__(self, bluetooth_config, logger, log_mode):
        """
        Instantiates a new `_BluetoothTest` with the given parameters.

        Params:
            bluetooth_config (:class:`.configparser.SectionProxy`): the bluetooth test configuration.
            logger (:class:`logging.Logger`): The logger.
            log_mode (:class:`.LogMode`): The logging mode.
        """
        super().__init__(_TEST_BLUETOOTH, bluetooth_config, logger, log_mode)

    def _execute_test(self):
        """
        Executes the bluetooth test.
        """
        bluetooth_port = None
        try:
            self._log_debug("Opening Bluetooth port...")
            bluetooth_port = serial.Serial(
                port=self._test_config[_PARAM_PORT],
                baudrate=self._test_config.getint(_PARAM_BAUDRATE)
            )
            while not self._stopped:
                self._count += 1
                random_string = ''.join(random.choices(string.ascii_uppercase + string.digits,
                                                       k=self._test_config.getint(_PARAM_DATA_LENGTH)))
                bluetooth_port.write(random_string.encode())
                self._status = _TestStatus.OK
                self._status_details = ""
                self._log_info("Sent data: %s" % random_string)
                self._lock.wait(self._calculate_wait_time())
        except SerialException as e:
            self._status = _TestStatus.ERROR
            self._status_details = "Port error: %s" % str(e)
            self._log_error(self._status_details)
        finally:
            if bluetooth_port and bluetooth_port.isOpen():
                self._log_debug("Closing Bluetooth port...")
                bluetooth_port.close()
            self._running = False


class _EthernetTest(_InterfaceTest):
    """
    Ethernet interface test. Exercises the ethernet interface by performing a ping with the configured
    host IP address.
    """

    def __init__(self, ethernet_config, logger, log_mode):
        """
        Instantiates a new `_EthernetTest` with the given parameters.

        Params:
            ethernet_config (:class:`.configparser.SectionProxy`): the ethernet test configuration.
            logger (:class:`logging.Logger`): The logger.
            log_mode (:class:`.LogMode`): The logging mode.
        """
        super().__init__(_TEST_ETHERNET, ethernet_config, logger, log_mode)

    def _execute_test(self):
        """
        Executes the ethernet test.
        """
        host = self._test_config[_PARAM_HOST]
        count = self._test_config.getint(_PARAM_COUNT)
        packet_size = self._test_config.getint(_PARAM_PACKET_SIZE)
        try:
            while not self._stopped:
                self._count += 1
                response = os.system(_PING_COMMAND % (count, packet_size, host))
                if response == 0:
                    self._status = _TestStatus.OK
                    self._status_details = ""
                    self._log_info("Ping to %s succeed" % host)
                else:
                    self._status = _TestStatus.ERROR
                    self._status_details = "Ping to %s failed" % host
                    if self._test_config.getboolean(_PARAM_STOP_ON_ERROR):
                        self._log_error(self._status_details)
                        break
                    else:
                        self._log_warning(self._status_details)
                self._lock.wait(self._calculate_wait_time())
        finally:
            self._running = False


class _LEDsTest(_InterfaceTest):
    """
    LEDs interface test. Exercises the LEDs interface by randomly changing colors and blinking the
    configured LEDs.
    """

    def __init__(self, leds_config, logger, log_mode):
        """
        Instantiates a new `_LEDsTest` with the given parameters.

        Params:
            leds_config (:class:`.configparser.SectionProxy`): the LEDs test configuration.
            logger (:class:`logging.Logger`): The logger.
            log_mode (:class:`.LogMode`): The logging mode.
        """
        super().__init__(_TEST_LEDS, leds_config, logger, log_mode)

    def _execute_test(self):
        """
        Executes the LEDs test.
        """
        try:
            leds = self._test_config.get(_PARAM_LEDS).split(",")
            if not leds:
                self._status = _TestStatus.ERROR
                self._log_error("No LEDs defined")
                return
            leds = [n.strip() for n in leds]
            led_modes = self._test_config.get(_PARAM_LED_MODES).split(",")
            if not led_modes:
                self._status = _TestStatus.ERROR
                self._status_details = "No LED modes defined"
                self._log_error(self._status_details)
                return
            led_modes = [n.strip() for n in led_modes]
            while not self._stopped:
                self._count += 1
                led_name = leds[random.randint(0, len(leds) - 1)]
                led_mode_name = led_modes[random.randint(0, len(led_modes) - 1)]
                led_mode = _LEDMode.get(led_mode_name)
                if not led_mode:
                    self._status = _TestStatus.ERROR
                    self._status_details = "Invalid LED mode '%s'" % led_mode_name
                    self._log_error(self._status_details)
                    break
                response = os.system(_LEDS_CMD % (led_mode.argument, led_name))
                if response == 0:
                    self._status = _TestStatus.OK
                    self._status_details = ""
                    self._log_info("%s LED '%s' succeed" % (led_mode.name, led_name))
                else:
                    self._status = _TestStatus.ERROR
                    self._status_details = "Could not %s LED '%s'" % (led_mode.name, led_name)
                    if self._test_config.getboolean(_PARAM_STOP_ON_ERROR):
                        self._log_error(self._status_details)
                        break
                    else:
                        self._log_warning(self._status_details)
                self._lock.wait(self._calculate_wait_time())
        finally:
            self._running = False


class _ModemTest(_InterfaceTest):
    """
    Modem interface test. Exercises the modem interface by reading the modem IMEI number.
    """

    def __init__(self, modem_config, logger, log_mode):
        """
        Instantiates a new `_ModemTest` with the given parameters.

        Params:
            modem_config (:class:`.configparser.SectionProxy`): the modem test configuration.
            logger (:class:`logging.Logger`): The logger.
            log_mode (:class:`.LogMode`): The logging mode.
        """
        super().__init__(_TEST_MODEM, modem_config, logger, log_mode)

    def _execute_test(self):
        """
        Executes the modem test.
        """
        url = self._test_config[_PARAM_URL]
        count = self._test_config.getint(_PARAM_COUNT)
        packet_size = self._test_config.getint(_PARAM_PACKET_SIZE)
        modem_number = None
        wwan_interface = None
        while not self._stopped:
            try:
                self._count += 1
                if not modem_number:
                    modem_list_cmd = subprocess.run(["mmcli", "-L"], stdout=subprocess.PIPE)
                    if not modem_list_cmd.stdout or "No modems" in modem_list_cmd.stdout.decode('utf-8'):
                        self._status = _TestStatus.ERROR
                        self._status_details = "No modems found"
                        if self._test_config.getboolean(_PARAM_STOP_ON_ERROR):
                            self._log_error(self._status_details)
                            break
                        else:
                            self._log_warning(self._status_details)
                            continue
                    matcher = re.match(_PATTERN_MODEM_NUMBER, modem_list_cmd.stdout.decode('utf-8'))
                    if not matcher or not matcher.group(1):
                        self._status = _TestStatus.ERROR
                        self._status_details = "Invalid modem list answer"
                        if self._test_config.getboolean(_PARAM_STOP_ON_ERROR):
                            self._log_error(self._status_details)
                            break
                        else:
                            self._log_warning(self._status_details)
                            continue
                    modem_number = matcher.group(1)
                if self._test_config.getmodem_test_mode(_PARAM_MODEM_TEST_MODE) == _ModemTestMode.READ_IMEI:
                    modem_imei_cmd = subprocess.run(["mmcli", "-m", modem_number, "--command=AT+CGSN"],
                                                    stdout=subprocess.PIPE)
                    matcher = re.match(_PATTERN_IMEI, modem_imei_cmd.stdout.decode('utf-8'))
                    if not matcher or not matcher.group(1):
                        self._status = _TestStatus.ERROR
                        self._status_details = "Could not read IMEI number"
                        if self._test_config.getboolean(_PARAM_STOP_ON_ERROR):
                            self._log_error(self._status_details)
                            break
                        else:
                            self._log_warning(self._status_details)
                    else:
                        self._status = _TestStatus.OK
                        self._status_details = ""
                        self._log_info("IMEI number: %s" % matcher.group(1))
                elif self._test_config.getmodem_test_mode(_PARAM_MODEM_TEST_MODE) == _ModemTestMode.PING:
                    if not wwan_interface:
                        wwan_cmd = subprocess.Popen(["ifconfig"], stdout=subprocess.PIPE)
                        wwan_cmd_2 = subprocess.run(["grep", "wwan0\\."], stdin=wwan_cmd.stdout, stdout=subprocess.PIPE)
                        if not wwan_cmd_2.stdout:
                            self._status = _TestStatus.ERROR
                            self._status_details = "No WWAN0 interface found"
                            if self._test_config.getboolean(_PARAM_STOP_ON_ERROR):
                                self._log_error(self._status_details)
                                break
                            else:
                                self._log_warning(self._status_details)
                                continue
                        matcher = re.match(_PATTERN_WWAN_INTERFACE, wwan_cmd_2.stdout.decode('utf-8'))
                        if not matcher or not matcher.group(1):
                            self._status = _TestStatus.ERROR
                            self._status_details = "No WWAN0 interface found"
                            if self._test_config.getboolean(_PARAM_STOP_ON_ERROR):
                                self._log_error(self._status_details)
                                break
                            else:
                                self._log_warning(self._status_details)
                                continue
                        wwan_interface = matcher.group(1)
                    if wwan_interface:
                        response = os.system(_PING_COMMAND_MODEM % (wwan_interface, count, packet_size, url))
                        if response == 0:
                            self._status = _TestStatus.OK
                            self._status_details = ""
                            self._log_info("Ping to %s succeed" % url)
                        else:
                            self._status = _TestStatus.ERROR
                            self._status_details = "Ping to %s failed" % url
                            if self._test_config.getboolean(_PARAM_STOP_ON_ERROR):
                                self._log_error(self._status_details)
                                break
                            else:
                                self._log_warning(self._status_details)
            finally:
                # Wait for next loop.
                self._lock.wait(self._calculate_wait_time())
        self._running = False


class _SerialTest(_InterfaceTest):
    """
    Serial interface test. Exercises the serial interface by writing random data to the serial port and
    reading the same data back using a loopback connector.
    """

    def __init__(self, serial_config, logger, log_mode):
        """
        Instantiates a new `_SerialTest` with the given parameters.

        Params:
            serial_config (:class:`.configparser.SectionProxy`): the serial test configuration.
            logger (:class:`logging.Logger`): The logger.
            log_mode (:class:`.LogMode`): The logging mode.
        """
        super().__init__(_TEST_SERIAL, serial_config, logger, log_mode)

    def _execute_test(self):
        """
        Executes the serial test.
        """
        while not self._stopped:
            self._count += 1
            serial_port = None
            try:
                self._log_debug("Opening Serial port...")
                serial_port = serial.Serial(
                    port=self._test_config[_PARAM_PORT],
                    baudrate=self._test_config.getint(_PARAM_BAUDRATE),
                    parity=self._test_config.getparity(_PARAM_PARITY).serial_value,
                    stopbits=self._test_config.getfloat(_PARAM_STOP_BITS),
                    bytesize=self._test_config.getint(_PARAM_DATA_BITS),
                    xonxoff=self._test_config.getflow_control(_PARAM_FLOW_CONTROL) == FlowControl.SOFTWARE,
                    rtscts=self._test_config.getflow_control(_PARAM_FLOW_CONTROL) == FlowControl.HARDWARE_RTS_CTS,
                    dsrdtr=self._test_config.getflow_control(_PARAM_FLOW_CONTROL) == FlowControl.HARDWARE_DSR_DTR,
                    timeout=self._test_config.getfloat(_PARAM_PORT_TIMEOUT)
                )

                random_string = ''.join(random.choices(string.ascii_uppercase + string.digits,
                                                       k=self._test_config.getint(_PARAM_DATA_LENGTH)))
                serial_port.write(random_string.encode())
                read_data = serial_port.read(len(random_string))
                self._log_info("Read data: %s" % read_data.decode("utf-8"))
                if self._test_config.getboolean(_PARAM_VALIDATE_DATA) and read_data.decode("utf-8") != random_string:
                    self._status = _TestStatus.ERROR
                    self._status_details = "Read data does not match with sent data"
                    if self._test_config.getboolean(_PARAM_STOP_ON_ERROR):
                        self._log_error(self._status_details)
                        break
                    else:
                        self._log_warning(self._status_details)
                else:
                    self._status = _TestStatus.OK
                    self._status_details = ""
            except SerialException as e:
                self._status = _TestStatus.ERROR
                self._status_details = "Port error: %s" % str(e)
                if self._test_config.getboolean(_PARAM_STOP_ON_ERROR):
                    self._log_error(self._status_details)
                    break
                else:
                    self._log_warning(self._status_details)
            finally:
                # Close port.
                if serial_port and serial_port.isOpen():
                    self._log_debug("Closing Serial port...")
                    serial_port.close()
                # Wait for next loop.
                self._lock.wait(self._calculate_wait_time())

        self._running = False


class _XBeeTest(_InterfaceTest):
    """
    XBee interface test. Exercises the XBee interface by reading the MAC address of the XBee device
    connected to the configured port.
    """

    def __init__(self, xbee_config, logger, log_mode):
        """
        Instantiates a new `_XBeeTest` with the given parameters.

        Params:
            xbee_config (:class:`.configparser.SectionProxy`): the XBee test configuration.
            logger (:class:`logging.Logger`): The logger.
            log_mode (:class:`.LogMode`): The logging mode.
        """
        super().__init__(_TEST_XBEE, xbee_config, logger, log_mode)

    def _execute_test(self):
        """
        Executes the XBee test.
        """
        xbee_device = XBeeDevice(
            port=self._test_config[_PARAM_PORT],
            baud_rate=self._test_config.getint(_PARAM_BAUDRATE),
            flow_control=self._test_config.getflow_control(_PARAM_FLOW_CONTROL),
            _sync_ops_timeout=self._test_config.getfloat(_PARAM_PORT_TIMEOUT))

        while not self._stopped:
            self._count += 1
            try:
                self._log_debug("Opening XBee port...")
                xbee_device.open(force_settings=True)
                if self._test_config.getxbee_test_mode(_PARAM_XBEE_TEST_MODE) == _XBeeTestMode.READ_MAC:
                    self._log_debug("Reading XBee MAC...")
                    mac_high = xbee_device.get_parameter(_XBEE_COMMAND_MAC_HIGH)
                    mac_low = xbee_device.get_parameter(_XBEE_COMMAND_MAC_LOW)
                    self._log_debug("XBee MAC read!")
                    if not mac_high or not mac_low:
                        self._status = _TestStatus.ERROR
                        self._status_details = "Could not read MAC address."
                        if self._test_config.getboolean(_PARAM_STOP_ON_ERROR):
                            self._log_error(self._status_details)
                            break
                        else:
                            self._log_warning(self._status_details)
                    else:
                        self._status = _TestStatus.OK
                        self._status_details = ""
                        mac_address = XBee64BitAddress(mac_high + mac_low)
                        self._log_info("MAC address: %s" % mac_address)
                elif self._test_config.getxbee_test_mode(_PARAM_XBEE_TEST_MODE) == _XBeeTestMode.BROADCAST:
                    xbee_device.send_data_broadcast(_XBEE_BROADCAST_DATA)
                    self._status = _TestStatus.OK
                    self._status_details = ""
                    self._log_info("Broadcast data sent.")
            except XBeeException as e:
                self._status = _TestStatus.ERROR
                self._status_details = "%s" % str(e)
                if self._test_config.getboolean(_PARAM_STOP_ON_ERROR):
                    self._log_error(self._status_details)
                    break
                else:
                    self._log_warning(self._status_details)
            finally:
                if xbee_device and xbee_device.is_open():
                    self._log_debug("Closing XBee port...")
                    xbee_device.close()
                # Wait for next loop.
                self._lock.wait(self._calculate_wait_time())
        self._running = False


class TestSession:
    """
    Helper class used to execute interface tests in the device. Supported tests:

        - Bluetooth interface
        - Ethernet interface
        - LEDs interface
        - Modem interface
        - Serial port interface
        - XBee interface
    """

    def __init__(self, config_file=None):
        """
        Class constructor. Instantiates a new :class`TestSession` with the given parameters.

        Params:
            config_file (String, optional): path of the configuration file to use, defaults to `None`.
        """
        # Variables.
        self._stopped = False
        self._tests = []
        self._log_thread = None
        self._log_lock = Event()
        self._logger = None

        # Load configuration.
        self._init_default_config()
        if config_file:
            self._load_file(config_file, self._config)

    def start(self):
        """
        Starts the interface tests.
        """
        # Register signal handlers.
        signal(SIGINT, self._signal_handler)
        signal(SIGTERM, self._signal_handler)
        signal(SIGQUIT, self._signal_handler)

        # Configure loggers.
        log_config = self._config[_SEC_LOG]
        log_mode = log_config.getlog_mode(_PARAM_LOG_MODE)
        self._setup_loggers(log_config)
        self._logger.info("Starting interface tests...")
        # Start tests.
        self._tests.append(_BluetoothTest(self._config[_SEC_BLUETOOTH], self._logger, log_mode))
        self._tests.append(_EthernetTest(self._config[_SEC_ETHERNET], self._logger, log_mode))
        self._tests.append(_LEDsTest(self._config[_SEC_LEDS], self._logger, log_mode))
        self._tests.append(_ModemTest(self._config[_SEC_MODEM], self._logger, log_mode))
        self._tests.append(_SerialTest(self._config[_SEC_SERIAL], self._logger, log_mode))
        self._tests.append(_XBeeTest(self._config[_SEC_XBEE], self._logger, log_mode))
        for test in self._tests:
            test.start()

        if log_mode == _LogMode.LOOP:
            self._log_thread = threading.Thread(target=self._log_tests_table,
                                                kwargs={'interval': log_config.getint(_PARAM_LOG_LOOP_INTERVAL)})
            self._log_thread.start()

        # Wait until all tests are stopped.
        while any(test.running for test in self._tests):
            if not self._stopped:
                time.sleep(0.2)
        self._stopped = True

        self._logger.info("Tests finished!")

    def _setup_loggers(self, log_config):
        """
        Configures the loggers for the test session.
        """
        # Create the logger.
        self._logger = logging.getLogger('test_interfaces')
        # Clear handlers.
        for h in self._logger.handlers:
            self._logger.removeHandler(h)
        # Do not add handlers if logging is disabled.
        if not log_config.getboolean(_PARAM_ENABLE):
            return
        # Determine log level.
        log_level = log_config.getlog_level(_PARAM_LOG_LEVEL) if \
            log_config.getlog_mode(_PARAM_LOG_MODE) == _LogMode.STANDARD else logging.INFO
        self._logger.setLevel(log_level)
        # Determine log format.
        if log_config.getlog_mode(_PARAM_LOG_MODE) == _LogMode.STANDARD:
            log_format = logging.Formatter("%(levelname)-7s - %(message)s")
        else:
            log_format = logging.Formatter("%(message)s")
        # Configure console handler.
        if log_config.getboolean(_PARAM_LOG_TO_CONSOLE):
            console_handler = logging.StreamHandler()
            console_handler.name = _LOG_CONSOLE_HANDLER_NAME
            console_handler.setLevel(log_level)
            console_handler.setFormatter(log_format)
            self._logger.addHandler(console_handler)
        # Configure network handler.
        if log_config.getboolean(_PARAM_LOG_TO_NETWORK):
            network_handler = logging.handlers.SocketHandler(log_config.get(_PARAM_LOG_SERVER),
                                                             log_config.getint(_PARAM_LOG_PORT))
            network_handler.name = _LOG_NETWORK_HANDLER_NAME
            network_handler.setLevel(log_level)
            network_handler.setFormatter(log_format)
            self._logger.addHandler(network_handler)

    def _log_tests_table(self, interval):
        """
        Logs the tests status in a table

        Args:
            interval (Integer): time to wait until next print.
        """
        while not self._stopped:
            self._logger.info(_LOG_LOOP_FORMAT % ("Test name", "Running", "Count", "Last run", "Details"))
            self._logger.info("-" * 77)
            for test in self._tests:
                if not test.enabled:
                    continue
                self._logger.info(_LOG_LOOP_FORMAT % (test.name, test.running, test.count,
                                                      test.status.name, test.status_details))
            self._logger.info("")
            self._log_lock.wait(interval)

    def _signal_handler(self, signal_number, _frame):
        """
        Handler executed when an interrupt or terminate signal is received.

        Args:
            signal_number: the received signal.
            _frame: current stack frame
        """
        if self._logger:
            self._logger.info("Received signal %s, stopping..." % signal_number)
        self._terminate_gracefully()

    def _terminate_gracefully(self):
        """
        Terminates the process gracefully by shutting down the acquired resources.
        """
        self._stopped = True
        # Stop all tests.
        for test in self._tests:
            test.stop()
        # Notify log thread.
        if self._log_thread:
            self._log_lock.set()

    def _init_default_config(self):
        """
        Initializes the default configuration.
        """
        # Create an initial configuration with default values
        self._config = configparser.ConfigParser(
            converters={_PARAM_PARITY: self._parse_parity,
                        _PARAM_FLOW_CONTROL: self._parse_flow_control,
                        _PARAM_LOG_LEVEL: self._parse_loglevel,
                        _PARAM_LOG_MODE: self._parse_log_mode,
                        _PARAM_XBEE_TEST_MODE: self._parse_xbee_test_mode,
                        _PARAM_MODEM_TEST_MODE: self._parse_modem_test_mode})

        self._config.read_dict({
            _SEC_BLUETOOTH: {
                _PARAM_ENABLE: _DEFAULT_BLUETOOTH_ENABLE,
                _PARAM_PORT: _DEFAULT_BLUETOOTH_PORT,
                _PARAM_BAUDRATE: _DEFAULT_BLUETOOTH_BAUDRATE,
                _PARAM_DATA_LENGTH: _DEFAULT_BLUETOOTH_DATA_LENGTH,
                _PARAM_MIN_WAIT: _DEFAULT_BLUETOOTH_MIN_WAIT,
                _PARAM_MAX_WAIT: _DEFAULT_BLUETOOTH_MAX_WAIT,
                _PARAM_STOP_ON_ERROR: _DEFAULT_BLUETOOTH_STOP_ON_ERROR
            },

            _SEC_ETHERNET: {
                _PARAM_ENABLE: _DEFAULT_ETHERNET_ENABLE,
                _PARAM_HOST: _DEFAULT_ETHERNET_HOST,
                _PARAM_PACKET_SIZE: _DEFAULT_ETHERNET_PACKET_SIZE,
                _PARAM_COUNT: _DEFAULT_ETHERNET_COUNT,
                _PARAM_MIN_WAIT: _DEFAULT_ETHERNET_MIN_WAIT,
                _PARAM_MAX_WAIT: _DEFAULT_ETHERNET_MAX_WAIT,
                _PARAM_STOP_ON_ERROR: _DEFAULT_ETHERNET_STOP_ON_ERROR
            },

            _SEC_LEDS: {
                _PARAM_ENABLE: _DEFAULT_LEDS_ENABLE,
                _PARAM_LEDS: _DEFAULT_LEDS_LEDS,
                _PARAM_LED_MODES: _DEFAULT_LEDS_MODES,
                _PARAM_MIN_WAIT: _DEFAULT_LEDS_MIN_WAIT,
                _PARAM_MAX_WAIT: _DEFAULT_LEDS_MAX_WAIT,
                _PARAM_STOP_ON_ERROR: _DEFAULT_LEDS_STOP_ON_ERROR
            },

            _SEC_LOG: {
                _PARAM_ENABLE: _DEFAULT_LOG_ENABLE,
                _PARAM_LOG_LEVEL: _DEFAULT_LOG_LEVEL,
                _PARAM_LOG_MODE: _DEFAULT_LOG_MODE,
                _PARAM_LOG_TO_CONSOLE: _DEFAULT_LOG_TO_CONSOLE,
                _PARAM_LOG_TO_NETWORK: _DEFAULT_LOG_TO_NETWORK,
                _PARAM_LOG_PORT: _DEFAULT_LOG_PORT,
                _PARAM_LOG_SERVER: _DEFAULT_LOG_SERVER
            },

            _SEC_MODEM: {
                _PARAM_ENABLE: _DEFAULT_MODEM_ENABLE,
                _PARAM_MODEM_TEST_MODE: _DEFAULT_MODEM_TEST_MODE,
                _PARAM_URL: _DEFAULT_MODEM_URL,
                _PARAM_PACKET_SIZE: _DEFAULT_MODEM_PACKET_SIZE,
                _PARAM_COUNT: _DEFAULT_MODEM_COUNT,
                _PARAM_MIN_WAIT: _DEFAULT_MODEM_MIN_WAIT,
                _PARAM_MAX_WAIT: _DEFAULT_MODEM_MAX_WAIT,
                _PARAM_STOP_ON_ERROR: _DEFAULT_MODEM_STOP_ON_ERROR
            },

            _SEC_SERIAL: {
                _PARAM_ENABLE: _DEFAULT_SERIAL_ENABLE,
                _PARAM_PORT: _DEFAULT_SERIAL_PORT,
                _PARAM_BAUDRATE: _DEFAULT_SERIAL_BAUDRATE,
                _PARAM_PORT_TIMEOUT: _DEFAULT_SERIAL_PORT_TIMEOUT,
                _PARAM_DATA_BITS: _DEFAULT_SERIAL_DATA_BITS,
                _PARAM_STOP_BITS: _DEFAULT_SERIAL_STOP_BITS,
                _PARAM_PARITY: _DEFAULT_SERIAL_PARITY,
                _PARAM_FLOW_CONTROL: _DEFAULT_SERIAL_FLOW_CONTROL,
                _PARAM_DATA_LENGTH: _DEFAULT_SERIAL_DATA_LENGTH,
                _PARAM_VALIDATE_DATA: _DEFAULT_SERIAL_VALIDATE_DATA,
                _PARAM_MIN_WAIT: _DEFAULT_SERIAL_MIN_WAIT,
                _PARAM_MAX_WAIT: _DEFAULT_SERIAL_MAX_WAIT,
                _PARAM_STOP_ON_ERROR: _DEFAULT_SERIAL_STOP_ON_ERROR
            },

            _SEC_XBEE: {
                _PARAM_ENABLE: _DEFAULT_XBEE_ENABLE,
                _PARAM_PORT: _DEFAULT_XBEE_PORT,
                _PARAM_BAUDRATE: _DEFAULT_XBEE_BAUDRATE,
                _PARAM_FLOW_CONTROL: _DEFAULT_XBEE_FLOW_CONTROL,
                _PARAM_PORT_TIMEOUT: _DEFAULT_XBEE_PORT_TIMEOUT,
                _PARAM_MIN_WAIT: _DEFAULT_XBEE_MIN_WAIT,
                _PARAM_MAX_WAIT: _DEFAULT_XBEE_MAX_WAIT,
                _PARAM_XBEE_TEST_MODE: _DEFAULT_XBEE_TEST_MODE,
                _PARAM_STOP_ON_ERROR: _DEFAULT_XBEE_STOP_ON_ERROR
            }
        })

    @staticmethod
    def _load_file(file_path, config):
        """
        Processes the provided configuration file and returns.

        Args:
             file_path (String, optional, default=`None`): Absolute path of the file
                with the configuration to parse.

        Raises:
            ValueError: If any of the variables does not match the expected format.
        """
        # Verify that the configuration file is readable
        if not os.access(file_path, os.R_OK):
            raise ValueError("[ERROR] Cannot read file: %s" % os.path.abspath(file_path))

        # Read the configuration file.
        config.read(file_path)

        # Check that the values match the format.
        # If any of the check fails, it will trigger a ValueError exception.
        bluetooth_section = config[_SEC_BLUETOOTH]
        bluetooth_section.getboolean(_PARAM_ENABLE)
        bluetooth_section.get(_PARAM_PORT)
        bluetooth_section.getint(_PARAM_BAUDRATE)
        bluetooth_section.getint(_PARAM_DATA_LENGTH)
        bluetooth_section.getint(_PARAM_MIN_WAIT)
        bluetooth_section.getint(_PARAM_MAX_WAIT)
        bluetooth_section.getboolean(_PARAM_STOP_ON_ERROR)

        eth_section = config[_SEC_ETHERNET]
        eth_section.getboolean(_PARAM_ENABLE)
        eth_section.get(_PARAM_HOST)
        eth_section.getint(_PARAM_PACKET_SIZE)
        eth_section.getint(_PARAM_COUNT)
        eth_section.getint(_PARAM_MIN_WAIT)
        eth_section.getint(_PARAM_MAX_WAIT)
        eth_section.getboolean(_PARAM_STOP_ON_ERROR)

        leds_section = config[_SEC_LEDS]
        leds_section.getboolean(_PARAM_ENABLE)
        leds_section.get(_PARAM_LEDS)
        leds_section.get(_PARAM_LED_MODES)
        leds_section.getint(_PARAM_MIN_WAIT)
        leds_section.getint(_PARAM_MAX_WAIT)
        leds_section.getboolean(_PARAM_STOP_ON_ERROR)

        log_section = config[_SEC_LOG]
        log_section.getboolean(_PARAM_ENABLE)
        log_section.getlog_level(_PARAM_LOG_LEVEL)
        log_section.getint(_PARAM_LOG_LOOP_INTERVAL)
        log_section.getlog_mode(_PARAM_LOG_MODE)
        log_section.getboolean(_PARAM_LOG_TO_CONSOLE)
        log_section.getboolean(_PARAM_LOG_TO_NETWORK)
        log_section.getint(_PARAM_LOG_PORT)
        log_section.get(_PARAM_LOG_SERVER)

        modem_section = config[_SEC_MODEM]
        modem_section.getboolean(_PARAM_ENABLE)
        modem_section.getmodem_test_mode(_PARAM_MODEM_TEST_MODE)
        modem_section.get(_PARAM_URL)
        modem_section.getint(_PARAM_PACKET_SIZE)
        modem_section.getint(_PARAM_COUNT)
        modem_section.getint(_PARAM_MIN_WAIT)
        modem_section.getint(_PARAM_MAX_WAIT)
        modem_section.getboolean(_PARAM_STOP_ON_ERROR)

        serial_section = config[_SEC_SERIAL]
        serial_section.getboolean(_PARAM_ENABLE)
        serial_section.get(_PARAM_PORT)
        serial_section.getint(_PARAM_BAUDRATE)
        serial_section.getfloat(_PARAM_PORT_TIMEOUT)
        serial_section.getint(_PARAM_DATA_BITS)
        serial_section.getint(_PARAM_STOP_BITS)
        serial_section.getparity(_PARAM_PARITY)
        serial_section.getflow_control(_PARAM_FLOW_CONTROL)
        serial_section.getint(_PARAM_DATA_LENGTH)
        serial_section.getboolean(_PARAM_VALIDATE_DATA)
        serial_section.getint(_PARAM_MIN_WAIT)
        serial_section.getint(_PARAM_MAX_WAIT)
        serial_section.getboolean(_PARAM_STOP_ON_ERROR)

        xbee_section = config[_SEC_XBEE]
        xbee_section.getboolean(_PARAM_ENABLE)
        xbee_section.get(_PARAM_PORT)
        xbee_section.getxbee_test_mode(_PARAM_XBEE_TEST_MODE)
        xbee_section.getint(_PARAM_BAUDRATE)
        xbee_section.getfloat(_PARAM_PORT_TIMEOUT)
        xbee_section.getflow_control(_PARAM_FLOW_CONTROL)
        xbee_section.getint(_PARAM_MIN_WAIT)
        xbee_section.getint(_PARAM_MAX_WAIT)
        xbee_section.getboolean(_PARAM_STOP_ON_ERROR)

    @staticmethod
    def _parse_flow_control(flow_control):
        """
        Transforms configuration file flow control values to 'serial' values.

        Args:
            flow_control (String): The value to transform.

        Returns:
            :class: `FlowControl`: The equivalent 'flow control' value.

        Raises:
            ValueError: If `flow_control` is different from `none`, `software`, `rts-cts`,
                        and `dtr-dsr`.
        """
        if flow_control.lower() == "none":
            return FlowControl.NONE
        elif flow_control.lower() == "rts-cts":
            return FlowControl.HARDWARE_RTS_CTS
        elif flow_control.lower() == "dsr-dtr":
            return FlowControl.HARDWARE_DSR_DTR
        elif flow_control.lower() == "software":
            return FlowControl.SOFTWARE
        else:
            raise ValueError("[ERROR] Incorrect flow control %s" % repr(flow_control))

    @staticmethod
    def _parse_parity(parity):
        """
        Transforms configuration file parity values to 'serial' values.

        Args:
            parity (String): The value to transform.

        Returns:
            :class: `_Parity`: The equivalent 'parity' value.

        Raises:
            ValueError: If `parity` is different from `none`, `odd`, `even`, `mark` and `space`.
        """
        parity_value = _Parity.get_by_description(parity)
        if not parity_value:
            raise ValueError("[ERROR] Incorrect parity %s" % repr(parity))

        return parity_value

    @staticmethod
    def _parse_xbee_test_mode(test_mode):
        """
        Transforms configuration file XBee test mode values to '_XBeeTestMode' values.

        Args:
            test_mode (String): The value to transform.

        Returns:
            :class: `_XBeeTestMode`: The equivalent '_XBeeTestMode' value.

        Raises:
            ValueError: If `test_mode` is different from `read-mac` and `broadcast`.
        """
        test_mode_value = _XBeeTestMode.get(test_mode)
        if not test_mode_value:
            raise ValueError("[ERROR] Incorrect XBee test mode %s" % repr(test_mode))

        return test_mode_value

    @staticmethod
    def _parse_modem_test_mode(test_mode):
        """
        Transforms configuration file modem test mode values to '_ModemTestMode' values.

        Args:
            test_mode (String): The value to transform.

        Returns:
            :class: `_ModemTestMode`: The equivalent '_ModemTestMode' value.

        Raises:
            ValueError: If `test_mode` is different from `read-imei` and `ping`.
        """
        test_mode_value = _ModemTestMode.get(test_mode)
        if not test_mode_value:
            raise ValueError("[ERROR] Incorrect modem test mode %s" % repr(test_mode))

        return test_mode_value

    @staticmethod
    def _parse_loglevel(log_level):
        """
        Transforms configuration file log levels to 'logging' levels.

        Args:
            log_level (String): The level to transform.

        Returns:
            Integer: The equivalent 'logging' level.

        Raises:
            ValueError: If `level` is different from `debug`, `info`, `warning`,
                        `warn` and `error`.
        """
        if log_level == "debug":
            return logging.DEBUG
        if log_level == "info":
            return logging.INFO
        if log_level in ["warn", "warning"]:
            return logging.WARNING
        if log_level == "error":
            return logging.ERROR

        raise ValueError("[ERROR] Incorrect log level %s" % repr(log_level))

    @staticmethod
    def _parse_log_mode(log_mode):
        """
        Transforms configuration file log mode values to '_LogMode' values.

        Args:
            log_mode (String): The value to transform.

        Returns:
            :class: `_LogMode`: The equivalent '_LogMode' value.

        Raises:
            ValueError: If `log_mode` is not a valid value.
        """
        log_mode_value = _LogMode.get(log_mode)
        if not log_mode_value:
            raise ValueError("[ERROR] Incorrect log mode %s" % repr(log_mode))

        return log_mode_value


def main():
    # Parse arguments.
    parser = argparse.ArgumentParser()
    parser.add_argument("--config", default="", help="configuration file path")
    args = parser.parse_args()
    # Instantiate and starts the test session.
    test_session = TestSession(args.config)
    test_session.start()


if __name__ == "__main__":
    main()
