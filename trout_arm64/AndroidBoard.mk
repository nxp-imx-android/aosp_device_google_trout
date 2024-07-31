ALL_DEFAULT_INSTALLED_MODULES += $(PRODUCT_OUT)/disk.img
 
$(PRODUCT_OUT)/disk.img: $(PRODUCT_OUT)/super.img $(PRODUCT_OUT)/dtb.img $(PRODUCT_OUT)/init_boot.img $(PRODUCT_OUT)/vendor_boot.img $(PRODUCT_OUT)/vbmeta.img $(PRODUCT_OUT)/vbmeta_system.img $(PRODUCT_OUT)/boot.img
	$(hide) echo "Generating disk image..."
	/usr/bin/python3 $(PRODUCT_OUT)/generate_diskimage.py --input $(PRODUCT_OUT)/partition.bpt --image-dir $(PRODUCT_OUT) --simg2img-path $(HOST_OUT)/bin/simg2img --output $(PRODUCT_OUT)/disk.img
