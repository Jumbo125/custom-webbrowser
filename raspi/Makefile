CMAKE ?= cmake
CONFIG ?= Release
JOBS ?= $(shell nproc)
QT_PREFIX ?=
TOOLCHAIN_FILE ?=

CMAKE_PREFIX_ARG := $(if $(QT_PREFIX),-DCMAKE_PREFIX_PATH=$(QT_PREFIX),)
TOOLCHAIN_ARG := $(if $(TOOLCHAIN_FILE),-DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN_FILE),)

.PHONY: linux-amd64 linux-arm64 linux-aarch64 clean

linux-amd64:
	@if [ "$$(uname -m)" != "x86_64" ] && [ -z "$(TOOLCHAIN_FILE)" ]; then \
		echo "linux-amd64 bitte auf x86_64 bauen oder TOOLCHAIN_FILE setzen."; exit 1; \
	fi
	$(CMAKE) -S . -B build/linux-amd64 \
		-DCMAKE_BUILD_TYPE=$(CONFIG) \
		-DCMAKE_INSTALL_PREFIX=$(CURDIR)/dist/linux-amd64 \
		$(CMAKE_PREFIX_ARG) \
		$(TOOLCHAIN_ARG)
	$(CMAKE) --build build/linux-amd64 --parallel $(JOBS)
	$(CMAKE) --install build/linux-amd64

linux-arm64 linux-aarch64:
	@if [ "$$(uname -m)" != "aarch64" ] && [ "$$(uname -m)" != "arm64" ] && [ -z "$(TOOLCHAIN_FILE)" ]; then \
		echo "linux-arm64 bitte nativ auf aarch64 bauen oder TOOLCHAIN_FILE + QT_PREFIX setzen."; exit 1; \
	fi
	$(CMAKE) -S . -B build/linux-arm64 \
		-DCMAKE_BUILD_TYPE=$(CONFIG) \
		-DCMAKE_INSTALL_PREFIX=$(CURDIR)/dist/linux-arm64 \
		$(CMAKE_PREFIX_ARG) \
		$(TOOLCHAIN_ARG)
	$(CMAKE) --build build/linux-arm64 --parallel $(JOBS)
	$(CMAKE) --install build/linux-arm64

clean:
	rm -rf build dist
