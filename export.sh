#!/usr/bin/env bash

# Usage: . ./export.sh
#

# Check if virtual environment is already activated
if [ -n "$VIRTUAL_ENV" ] && [ "$VIRTUAL_ENV" = "$OPEN_SDK_ROOT/.venv" ]; then
    echo "Virtual environment is already activated."
    return 0
fi

# Function to find the project root directory
pwd_dir="$(pwd)"
script_dir=$(realpath $(dirname "$0"))
find_project_root() {
    if [ -e "$script_dir/export.sh" ] && [ -e "$script_dir/requirements.txt" ]; then
        echo "$script_dir"
        return 0
    fi

    if [ -e "$pwd_dir/export.sh" ] && [ -e "$pwd_dir/requirements.txt" ]; then
        echo "$pwd_dir"
        return 0
    fi

    return 1
}

OPEN_SDK_ROOT=$(find_project_root)

# Debug information
echo "OPEN_SDK_ROOT = $OPEN_SDK_ROOT"
echo "Current root = $(pwd)"
echo "Script name: $(basename "$0")"
echo "Script path: $0"

# Additional verification - check for expected project files
echo "Project files check:"
EXIT_FLAG=0
for file in "export.sh" "requirements.txt" "tos.py"; do
    if [ -f "$OPEN_SDK_ROOT/$file" ]; then
        echo "  ✓ Found $file"
    else
        echo "  ✗ Missing $file"
        EXIT_FLAG=1
    fi
done

if [ x"1" = x"$EXIT_FLAG" ]; then
    echo "Erorr: Can't export!"
    return 1
fi

# If we're not in the project root and export.sh exists in current directory, use current directory
if [ "$OPEN_SDK_ROOT" != "$(pwd)" ] && [ -f "./export.sh" ]; then
    echo "Found export.sh in current directory, using current directory as project root"
    OPEN_SDK_ROOT="$(pwd)"
    echo "Updated OPEN_SDK_ROOT = $OPEN_SDK_ROOT"
fi

# Function to check Python version
check_python_version() {
    local python_cmd="$1"
    if command -v "$python_cmd" >/dev/null 2>&1; then
        local version=$($python_cmd -c "import sys; print('.'.join(map(str, sys.version_info[:3])))" 2>/dev/null)
        if [ $? -eq 0 ]; then
            local major=$(echo "$version" | cut -d. -f1)
            local minor=$(echo "$version" | cut -d. -f2)
            local patch=$(echo "$version" | cut -d. -f3)
            # Check if version >= 3.6.0
            if [ "$major" -eq 3 ] && [ "$minor" -ge 6 ]; then
                echo "$python_cmd"
                return 0
            elif [ "$major" -gt 3 ]; then
                echo "$python_cmd"
                return 0
            fi
        fi
    fi
    return 1
}

# Determine which Python command to use
PYTHON_CMD=""
if check_python_version "python3" >/dev/null 2>&1; then
    PYTHON_CMD=$(check_python_version "python3")
    echo "Using python3 ($(python3 --version))"
elif check_python_version "python" >/dev/null 2>&1; then
    PYTHON_CMD=$(check_python_version "python")
    echo "Using python ($(python --version))"
else
    echo "Error: No suitable Python version found!"
    echo "Please install Python 3.6.0 or higher."
    return 1
fi

# Change to the script directory to ensure relative paths work correctly
cd "$OPEN_SDK_ROOT"

# create a virtual environment
if [ ! -d "$OPEN_SDK_ROOT/.venv" ]; then
    echo "Creating virtual environment..."
    $PYTHON_CMD -m venv "$OPEN_SDK_ROOT/.venv"
    if [ $? -ne 0 ]; then
        echo "Error: Failed to create virtual environment!"
        echo "Please check your Python installation and try again."
        return 1
    fi
    echo "Virtual environment created successfully."
else
    echo "Virtual environment already exists."
fi

# Verify that the virtual environment was created properly
if [ ! -f "$OPEN_SDK_ROOT/.venv/bin/activate" ]; then
    echo "Error: Virtual environment activation script not found at $OPEN_SDK_ROOT/.venv/bin/activate"
    return 1
fi


# Define custom exit function for TuyaOpen environment
exit() {
    # Check if we're in TuyaOpen virtual environment
    if [ -n "$OPEN_SDK_ROOT" ]; then
        echo "Exiting TuyaOpen environment..."

        # Call the original deactivate function
        if type deactivate >/dev/null 2>&1; then
            deactivate
        fi

        # Clean up TuyaOpen specific environment variables
        unset OPEN_SDK_PYTHON
        unset OPEN_SDK_PIP
        unset OPEN_SDK_ROOT

        # Remove our custom exit function
        unset -f exit

        echo "TuyaOpen environment deactivated."
    else
        # If not in TuyaOpen environment, call the original exit
        command exit "$@"
    fi
}

# activate
echo "DEBUG: Activating virtual environment from $OPEN_SDK_ROOT/.venv/bin/activate"
. ${OPEN_SDK_ROOT}/.venv/bin/activate
export PATH=$PATH:${OPEN_SDK_ROOT}
export OPEN_SDK_PYTHON=${OPEN_SDK_ROOT}/.venv/bin/python
export OPEN_SDK_PIP=${OPEN_SDK_ROOT}/.venv/bin/pip
export OPEN_SDK_ROOT=$OPEN_SDK_ROOT

# Export the exit function
export -f exit

# Verify activation worked
if [ -z "$VIRTUAL_ENV" ]; then
    echo "Error: Failed to activate virtual environment"
    return 1
fi
echo "Virtual environment activated successfully: $VIRTUAL_ENV"

# install dependencies
pip install -r ${OPEN_SDK_ROOT}/requirements.txt

# remove cache files
CACHE_PATH=${OPEN_SDK_ROOT}/.cache
mkdir -p ${CACHE_PATH}
rm -f ${CACHE_PATH}/.env.json
rm -f ${CACHE_PATH}/.dont_prompt_update_platform

# complete
eval "$(bash -c '_TOS_PY_COMPLETE=bash_source tos.py')"

# hello tuya
HELLO_TUYA='
 ______                 ____
/_  __/_ ____ _____ _  / __ \___  ___ ___
 / / / // / // / _ `/ / /_/ / _ \/ -_) _ \
/_/  \_,_/\_, /\_,_/  \____/ .__/\__/_//_/
         /___/            /_/
'
echo "****************************************"
echo $HELLO_TUYA
echo "Exit use: exit"
echo "****************************************"
