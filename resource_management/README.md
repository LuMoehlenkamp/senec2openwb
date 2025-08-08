# Resource Management Service

A Python-based resource management service for SENEC2OpenWB that utilizes SQLModel for database operations.

## Features

- **SQLModel Integration**: Modern Python ORM with type hints and automatic API generation
- **FastAPI Web Interface**: RESTful API for monitoring and controlling resources
- **MQTT Integration**: Connects to existing MQTT infrastructure for real-time data
- **Database Management**: Persistent storage of energy data, system status, and resource allocations
- **Resource Optimization**: Intelligent resource allocation based on current energy data

## Components

### main.py
Preserves the original functionality patterns from the C++ implementation:
- Signal handling (SIGINT, SIGTERM)
- Configuration management
- MQTT client for data acquisition
- Periodic task execution
- Resource management logic

### new_main.py
Enhanced version with SQLModel integration:
- FastAPI web interface
- Advanced SQLModel queries and relationships
- RESTful API endpoints
- Real-time analytics
- Resource optimization endpoints

## Database Models

### EnergyData
- Power generation/consumption
- Battery charge level
- Grid power flow
- Timestamps

### SystemStatus
- System state
- Temperature monitoring
- Error tracking
- Uptime statistics

### ResourceAllocation
- Resource type (battery, grid, solar)
- Power allocation
- Priority levels
- Active/inactive status

## API Endpoints

- `GET /energy-data` - Retrieve energy data with filtering
- `GET /system-status` - Get system status information
- `GET /resource-allocations` - List resource allocations
- `POST /resource-allocations` - Create new allocations
- `GET /analytics/energy-summary` - Energy analytics
- `POST /actions/optimize-resources` - Trigger optimization
- `GET /health` - Health check

## Running the Service

### Development
```bash
cd resource_management
pip install -r requirements.txt
python new_main.py
```

### Production (Docker)
```bash
docker-compose up resource_management
```

## Testing

```bash
cd resource_management
pytest tests/
```

## Configuration

The service uses `config.json` for configuration or environment variables:
- `MQTT_BROKER` - MQTT broker hostname
- `MQTT_PORT` - MQTT broker port
- `DATABASE_URL` - Database connection string

## Integration

The service integrates with the existing SENEC2OpenWB infrastructure:
- Connects to the same MQTT broker (mosquitto)
- Processes SENEC data messages
- Provides additional monitoring and control capabilities
- Maintains all original C++ functionality patterns