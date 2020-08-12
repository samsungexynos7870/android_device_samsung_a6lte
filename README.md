# OrangeFox Recovery Project for the Samsung Galaxy A6 2018

### How to build ###

```bash
# Create dirs
$ mkdir ofox ; cd ofox

# Init repo
$ repo init --depth=1 -u https://gitlab.com/OrangeFox/Manifest.git -b fox_9.0

# Clone a6lte repo
$ git clone https://gitlab.com/OrangeFox/device/a6lte.git -b fox_9.0 device/samsung/a6lte

# Sync
$ repo sync --no-repo-verify -c --force-sync --no-clone-bundle --no-tags --optimized-fetch --prune -j`nproc`

# Build
$ mv device/samsung/a6lte/build_ofox.sh .
$ . build_ofox.sh a6lte
```
## Credits
2019 @Astrako

## Contact
Telegram support group: https://t.me/joinchat/D1Jk_VbieGBXOWZt2y8O7A
