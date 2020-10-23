# main project for qemu-riscv32
MODULES += \
	app/shell
SUBARCH := 32

include project/target/nuclei-hbird.mk

