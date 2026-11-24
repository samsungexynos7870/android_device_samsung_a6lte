# DPI
PRODUCT_PROPERTY_OVERRIDES += \
    ro.sf.lcd_density=320

# Bluetooth
PRODUCT_PROPERTY_OVERRIDES += \
    ro.bt.bdaddr_path=/efs/bluetooth/bt_addr \
    bluetooth.device.class_of_device=90,1,16 \
    bluetooth.profile.asha.central.enabled=true \
    bluetooth.profile.a2dp.source.enabled=true \
    bluetooth.profile.avrcp.target.enabled=true \
    bluetooth.profile.bas.client.enabled=true \
    bluetooth.profile.gatt.enabled=true \
    bluetooth.profile.hfp.ag.enabled=true \
    bluetooth.profile.hid.device.enabled=true \
    bluetooth.profile.hid.host.enabled=true \
    bluetooth.profile.map.server.enabled=true \
    bluetooth.profile.opp.enabled=true \
    bluetooth.profile.pan.nap.enabled=true \
    bluetooth.profile.pan.panu.enabled=true \
    bluetooth.profile.pbap.server.enabled=true \
    bluetooth.profile.sap.server.enabled=true \
    bluetooth.le.disable_apcf_extended_features=1

# FM Radio
PRODUCT_PROPERTY_OVERRIDES += \
    ro.fm.chip=0x4356 \
    ro.fm.band=1 \
    ro.fm.low_band=875 \
    ro.fm.high_band=1080 \
    ro.fm.seek_space=1 \
    ro.fm.max_scan_num=30 \
    ro.fm.seek_lev=4 \
    ro.fm.scan_sort=1

# Fix for echo in calls
PRODUCT_PROPERTY_OVERRIDES += \
    audio_hal.disable_two_mic=false

# Choreographer
PRODUCT_PROPERTY_OVERRIDES += \
    persist.log.tag.Choreographer=E

# Dual SIM
PRODUCT_PROPERTY_OVERRIDES += \
    ro.vendor.multisim.simslotcount=2
