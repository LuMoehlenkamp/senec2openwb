"""Test the main service functionality"""

import pytest
import asyncio
from unittest.mock import Mock, patch
from main import ConfigManager, ResourceManagementService


def test_config_manager():
    """Test ConfigManager functionality"""
    config = ConfigManager()
    
    assert config.get_senec_update_time() is not None
    assert config.get_senec_timeout_time() is not None
    assert config.get_senec_connect_timeout_time() is not None
    assert config.senec_update_time == 5
    assert config.mqtt_broker == "mosquitto"


@pytest.mark.asyncio
async def test_resource_management_service_init():
    """Test ResourceManagementService initialization"""
    config = ConfigManager()
    service = ResourceManagementService(config)
    
    assert service.config_manager == config
    assert service.mqtt_client is None
    assert service.running is False


@pytest.mark.asyncio
async def test_service_mqtt_setup():
    """Test MQTT setup in service"""
    config = ConfigManager()
    service = ResourceManagementService(config)
    
    # Mock MQTT client to avoid actual connection
    with patch('main.mqtt.Client') as mock_client_class:
        mock_client = Mock()
        mock_client_class.return_value = mock_client
        
        service.setup_mqtt()
        
        assert service.mqtt_client is not None
        mock_client.connect.assert_called_once()


@pytest.mark.asyncio
async def test_energy_data_processing():
    """Test energy data processing"""
    config = ConfigManager()
    service = ResourceManagementService(config)
    
    # Mock database operations
    with patch('main.Session') as mock_session_class:
        mock_session = Mock()
        mock_session_class.return_value.__enter__.return_value = mock_session
        
        # Test processing energy data
        service.process_energy_data("test_data")
        
        # Verify session was used
        mock_session.add.assert_called_once()
        mock_session.commit.assert_called_once()


@pytest.mark.asyncio
async def test_resource_management():
    """Test resource management logic"""
    config = ConfigManager()
    service = ResourceManagementService(config)
    
    # Mock database operations
    with patch('main.Session') as mock_session_class:
        mock_session = Mock()
        mock_session_class.return_value.__enter__.return_value = mock_session
        
        # Mock energy data with low battery
        from main import EnergyData
        mock_energy_data = EnergyData(
            power_generation=1000.0,
            power_consumption=500.0,
            battery_charge_level=15.0,  # Low battery
            grid_power=500.0
        )
        mock_session.exec.return_value.first.return_value = mock_energy_data
        
        # Test resource management
        await service.manage_resources()
        
        # Verify allocation was created for low battery
        mock_session.add.assert_called()
        mock_session.commit.assert_called()


if __name__ == "__main__":
    pytest.main([__file__])