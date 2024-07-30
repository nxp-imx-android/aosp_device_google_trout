fastboot flash boot_a boot.img
fastboot flash boot_b boot.img
fastboot flash vbmeta_a vbmeta.img --disable-verity
fastboot flash vbmeta_b vbmeta.img --disable-verity
fastboot flash vbmeta_system_a vbmeta_system.img --disable-verity
fastboot flash vbmeta_system_b vbmeta_system.img --disable-verity
fastboot flash dtbo_a dtb.img
fastboot flash dtbo_b dtb.img
fastboot flash init_boot_a init_boot.img
fastboot flash init_boot_b init_boot.img
fastboot flash vendor_boot_a vendor_boot.img
fastboot flash vendor_boot_b vendor_boot.img
fastboot flash super super.img
