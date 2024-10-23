DefinitionBlock ("atecc508.aml", "SSDT", 5, "", "ATECC508", 1)
{
    External (\_SB_.PCI0.D025, DeviceObj)
    Scope (\_SB.PCI0.D025)
    {
        Device (ECC0) {
            Name (_HID, "PRP0001")
            Name (_DDN, "Atmel ATECC508A crypto device")
            Name (_CRS, ResourceTemplate () {
                I2cSerialBusV2 (
                    0x0060,              // I2C Slave Address
                    ControllerInitiated,
                    100000,              // Bus speed
                    AddressingMode7Bit,
                    "\\_SB.PCI0.D025",   // Link to ACPI I2C host controller
                    0
                )
            })
            Name (_DSD, Package () {
                ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
                Package () {
                    Package () { "compatible", "atsha204-i2c" },
                }
            })
        }
    }
}
