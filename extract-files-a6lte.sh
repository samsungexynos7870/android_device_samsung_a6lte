MY_DIR="${BASH_SOURCE%/*}"
if [[ ! -d "${MY_DIR}" ]]; then MY_DIR="${PWD}"; fi

ORIGINAL_DIR="${PWD}"

ANDROID_ROOT="${MY_DIR}/../../.."
DEVICE_COMMON=universal7870-common
VENDOR=samsung
VENDOR_MK_ROOT="${ANDROID_ROOT}"/vendor/"${VENDOR}"
DEVICE_COMMON_ROOT="${ANDROID_ROOT}"/device/"${VENDOR}"/"${DEVICE_COMMON}"

TARGET_SOURCES_DIR="${VENDOR_MK_ROOT}/tmp/sources"
mkdir -p "$TARGET_SOURCES_DIR"

REPO_URLS=(
    "https://github.com/Exynos7870-labs/samsung_a6eltemtr_dump.git -b a6eltemtr-user-10-QP1A.190711.020-A600T1UVS8CUA1-release-keys A600T1UVS8CUA1"
    "https://github.com/Exynos7870-labs/samsung_a6lte_dump.git -b a6ltexx-user-8.0.0-R16NW-A600FNXXU3ARL4-release-keys A600FNXXU3ARL4"
    "https://github.com/Exynos7870-labs/samsung_a6lte_dump.git -b a6ltejx-user-10-QP1A.190711.020-A600FJXU9CVB1-release-keys A600FJXU9CVB1"
)

cd "$TARGET_SOURCES_DIR"
for i in "${!REPO_URLS[@]}"; do
    # Extract target directory name (last word in the string)
    repo_info="${REPO_URLS[$i]}"
    target_dir="${repo_info##* }"
    
    echo "Checking: $target_dir"
    
    if [[ -d "$target_dir" ]]; then
        echo "  Directory $target_dir already exists. Skipping."
    else
        echo "  Cloning: ${REPO_URLS[$i]}"
        git clone ${REPO_URLS[$i]}
    fi
done
ls
cd "$ORIGINAL_DIR"

# Define source directories for each firmware dump
COMMON_Q_A6ELTE_PATH="${TARGET_SOURCES_DIR}/A600T1UVS8CUA1"   # a6elte (US) Android 10
COMMON_Q_A6LTE_PATH="${TARGET_SOURCES_DIR}/A600FJXU9CVB1"    # a6lte global Android 10
COMMON_O_A6LTE_PATH="${TARGET_SOURCES_DIR}/A600FNXXU3ARL4"   # a6lte global Android 8

# files
./extract-files.sh a6lte vendor-tools/proprietary-files_a6lte_q.txt -n -k $COMMON_Q_A6LTE_PATH
./extract-files.sh a6lte vendor-tools/proprietary-files_a6lte_o.txt -n -k $COMMON_Q_A6LTE_PATH
./extract-files.sh a6lte vendor-tools/proprietary-files_a6elte.txt -n -k $COMMON_Q_A6ELTE_PATH
