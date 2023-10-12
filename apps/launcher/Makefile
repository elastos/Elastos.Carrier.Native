all: meerkat-deb-package

meerkat-deb-package:
	cd $(BINARY_DIR) && { \
		rm -rf debian; \
		rm -rf carrier-meerkat.deb; \
	}

	cd $(BINARY_DIR) && { \
		mkdir -p debian/usr/bin; \
		cp ${DIST_DIR}/bin/carrier-launcher debian/usr/bin; \
		strip debian/usr/bin/carrier-launcher; \
		mkdir -p debian/etc/carrier-meerkat; \
		cp ${DIST_DIR}/etc/carrier-launcher/default.conf debian/etc/carrier-meerkat; \
		mkdir -p debian/lib/systemd/system; \
		cp $(SOURCE_DIR)/debian/scripts/carrier-meerkat.service debian/lib/systemd/system; \
		mkdir -p debian/etc/init.d; \
		cp $(SOURCE_DIR)/debian/scripts/carrier-meerkat debian/etc/init.d/carrier-meerkat; \
		mkdir -p debian/var/lib/carrier-meerkat; \
		mkdir -p debian/var/lib/carrier-meerkat/db; \
		mkdir -p debian/var/log/carrier-meerkat; \
		mkdir -p debian/DEBIAN; \
		cp $(SOURCE_DIR)/debian/control/* debian/DEBIAN; \
		dpkg-deb --build debian carrier-meerkat.deb; \
	}

