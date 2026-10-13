# TWRP for the Samsung Galaxy A6 2018

TWRP device tree for the Galaxy A6 LTE (`a6lte`), based on the TWRP AOSP 12.1
manifest. Build it with the Exynos 7870 kernel's `twrp-12.1` branch so the
A6-specific defconfig and DTBs match.

## Build

```bash
mkdir twrp && cd twrp
repo init --depth=1 \
  -u https://github.com/minimal-manifest-twrp/platform_manifest_twrp_aosp.git \
  -b twrp-12.1

mkdir -p .repo/local_manifests
cat > .repo/local_manifests/a6lte.xml <<'EOF_MANIFEST'
<?xml version="1.0" encoding="UTF-8"?>
<manifest>
    <remote fetch="https://github.com/samsungexynos7870" name="exynos7870" />
    <project name="android_device_samsung_a6lte" path="device/samsung/a6lte"
             remote="exynos7870" revision="twrp-12.1" />
    <project name="android_kernel_samsung_exynos7870" path="kernel/samsung/exynos7870"
             remote="exynos7870" revision="twrp-12.1" />
</manifest>
EOF_MANIFEST

repo sync
source build/envsetup.sh
lunch twrp_a6lte-eng
mka recoveryimage
```

The recovery image is `out/target/product/a6lte/recovery.img`.

## DTBH image handling

The shared Exynos 7870 kernel builds the A6 LTE DTBs. `dtbhtoolExynos` filters
for the A6 LTE model strings and packs them in Samsung's DTBH container;
`tools/mkbootimg-dt` appends that container to the version-0 recovery image.
The kernel defconfig enables `CONFIG_RD_LZMA`, so the build also requires an
LZMA recovery ramdisk.

This tree does not set an A6 recovery-partition size or an sboot image-size
cap: neither limit has been verified here. 

## Credits

2020 @Astrako; 2026 samsungexynos7870