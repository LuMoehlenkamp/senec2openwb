#!/bin/bash

# Startup script for Resource Management Service

echo "SENEC2OpenWB Resource Management Service Startup"
echo "================================================"

# Check if we're in the right directory
if [ ! -f "requirements.txt" ]; then
    echo "Error: requirements.txt not found. Please run this script from the resource_management directory."
    exit 1
fi

# Check Python availability
python_version=$(python3 --version 2>/dev/null)
if [ $? -ne 0 ]; then
    echo "Error: Python 3 is not available. Please install Python 3.11 or later."
    exit 1
fi
echo "Found: $python_version"

# Check if virtual environment should be created
if [ ! -d "venv" ]; then
    echo "Creating virtual environment..."
    python3 -m venv venv
    if [ $? -ne 0 ]; then
        echo "Error: Failed to create virtual environment."
        exit 1
    fi
fi

# Activate virtual environment
echo "Activating virtual environment..."
source venv/bin/activate

# Install dependencies
echo "Installing dependencies..."
pip install --upgrade pip
pip install -r requirements.txt

if [ $? -ne 0 ]; then
    echo "Warning: Some dependencies failed to install. The service may not work correctly."
    echo "You can try installing manually with: pip install sqlmodel fastapi uvicorn paho-mqtt"
fi

# Check if database directory exists
mkdir -p data

# Run the service based on argument
if [ "$1" = "simple" ]; then
    echo "Starting simple service (main.py)..."
    python main.py
elif [ "$1" = "api" ]; then
    echo "Starting API service (new_main.py)..."
    python new_main.py
elif [ "$1" = "test" ]; then
    echo "Running tests..."
    pytest tests/ -v
else
    echo ""
    echo "Usage: $0 [simple|api|test]"
    echo "  simple  - Run the basic service (main.py)"
    echo "  api     - Run the FastAPI service (new_main.py)"
    echo "  test    - Run the test suite"
    echo ""
    echo "Environment variables:"
    echo "  MQTT_BROKER - MQTT broker hostname (default: mosquitto)"
    echo "  MQTT_PORT   - MQTT broker port (default: 1883)"
    echo "  DATABASE_URL - Database URL (default: sqlite:///./data/resource_management.db)"
fi