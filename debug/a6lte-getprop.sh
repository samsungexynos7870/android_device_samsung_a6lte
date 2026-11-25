#!/system/bin/sh
# Eng-only boot property dump for debugging boot_completed and services.
[ "$(getprop ro.build.type)" = "eng" ] || exit 0

out=/cache/a6lte-getprop.txt
mkdir -p /cache 2>/dev/null
if ! touch "$out" 2>/dev/null; then
    out=/data/local/tmp/a6lte-getprop.txt
    mkdir -p /data/local/tmp 2>/dev/null
fi

{
    echo "=== a6lte getprop $(date) ==="
    getprop
    echo "=== sys.boot_completed ==="
    getprop sys.boot_completed
} > "$out" 2>&1
echo "a6lte-getprop: initial sys.boot_completed=$(getprop sys.boot_completed)" > /dev/kmsg 2>/dev/null

i=0
while [ "$i" -lt 30 ]; do
    sleep 10
    echo "=== $(date) sys.boot_completed=$(getprop sys.boot_completed) ===" >> "$out"
    echo "a6lte-getprop: sys.boot_completed=$(getprop sys.boot_completed)" > /dev/kmsg 2>/dev/null
    i=$((i + 1))
done
