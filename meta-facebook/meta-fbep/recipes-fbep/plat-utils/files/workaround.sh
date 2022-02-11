#!/bin/sh

<<<<<<< HEAD
# TODO:
# 	Remove this workwround at PVT entry

echo "Overwrite the register 0x1 of AUX HSC with 0x2"
i2cset -f -y 18 0x43 0x1 0x2

# Set the OCW of P12V_1/P12V_2 HSC to ~150 Amps then enable it
# TODO:
#	If necessary, shall we add the feature in driver probe for DTS?

i2cset -f -y 17 0x40 0xD 0x60
i2cset -f -y 16 0x53 0xD 0x60
i2cset -f -y 17 0x40 0x3 0x20
i2cset -f -y 16 0x53 0x3 0x20
=======
echo "Overwrite the register 0x1 of AUX HSC with 0x2"
i2cset -f -y 18 0x43 0x1 0x2

# Set the OCW of P12V_1/P12V_2 HSC to ~290 Amps then enable it

i2cset -f -y 17 0x40 0xD 0xB9
i2cset -f -y 16 0x53 0xD 0xB9

val=$(i2cget -f -y 17 0x40 0x3)
alert_reg=$(($val|0x20))
i2cset -f -y 17 0x40 0x3 $alert_reg
val=$(i2cget -f -y 16 0x53 0x3)
alert_reg=$(($val|0x20))
i2cset -f -y 16 0x53 0x3 $alert_reg
>>>>>>> facebook/helium
