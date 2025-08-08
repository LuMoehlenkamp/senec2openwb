"""Test the FastAPI endpoints in new_main.py"""

import pytest
from fastapi.testclient import TestClient
from sqlmodel import Session, create_engine, SQLModel
from unittest.mock import patch
from datetime import datetime

from new_main import app
from models import EnergyData, SystemStatus, ResourceAllocation


@pytest.fixture
def test_engine():
    """Create test database engine"""
    engine = create_engine("sqlite:///:memory:", echo=True)
    SQLModel.metadata.create_all(engine)
    return engine


@pytest.fixture
def test_client():
    """Create test client for FastAPI"""
    with patch('new_main.service_instance'):
        with TestClient(app) as client:
            yield client


@pytest.fixture
def test_session(test_engine):
    """Create test database session"""
    with Session(test_engine) as session:
        yield session


def test_root_endpoint(test_client):
    """Test root endpoint"""
    response = test_client.get("/")
    assert response.status_code == 200
    data = response.json()
    assert data["message"] == "SENEC Resource Management Service"
    assert data["version"] == "1.0.0"


def test_health_endpoint(test_client):
    """Test health check endpoint"""
    with patch('new_main.Session') as mock_session_class:
        mock_session = Session.__new__(Session)  # Create a mock session
        mock_session_class.return_value.__enter__.return_value = mock_session
        
        response = test_client.get("/health")
        # Should return 200 or 503 depending on database connection
        assert response.status_code in [200, 503]


def test_energy_data_endpoints(test_client):
    """Test energy data endpoints"""
    # Mock the get_session dependency
    with patch('new_main.get_session') as mock_get_session:
        # Create a mock session with test data
        mock_session = Session.__new__(Session)
        
        # Mock energy data
        test_energy_data = [
            EnergyData(
                id=1,
                power_generation=1500.0,
                power_consumption=800.0,
                battery_charge_level=75.0,
                grid_power=700.0,
                timestamp=datetime.now()
            )
        ]
        
        # Mock the session.exec().all() method
        mock_session.exec = lambda statement: type('MockResult', (), {'all': lambda: test_energy_data})()
        mock_get_session.return_value = mock_session
        
        # Test get energy data
        response = test_client.get("/energy-data")
        assert response.status_code == 200
        data = response.json()
        assert len(data) == 1
        assert data[0]["power_generation"] == 1500.0


def test_resource_allocation_creation(test_client):
    """Test creating resource allocation"""
    with patch('new_main.get_session') as mock_get_session:
        mock_session = Session.__new__(Session)
        mock_session.add = lambda obj: None
        mock_session.commit = lambda: None
        mock_session.refresh = lambda obj: setattr(obj, 'id', 1)
        mock_get_session.return_value = mock_session
        
        allocation_data = {
            "resource_type": "battery",
            "allocated_power": 1000.0,
            "priority": 1,
            "is_active": True
        }
        
        response = test_client.post("/resource-allocations", json=allocation_data)
        assert response.status_code == 200
        data = response.json()
        assert data["resource_type"] == "battery"
        assert data["allocated_power"] == 1000.0


def test_analytics_endpoint(test_client):
    """Test analytics endpoint"""
    with patch('new_main.get_session') as mock_get_session:
        mock_session = Session.__new__(Session)
        
        # Mock energy data for analytics
        test_energy_data = [
            EnergyData(
                power_generation=1500.0,
                power_consumption=800.0,
                battery_charge_level=75.0,
                grid_power=700.0,
                timestamp=datetime.now()
            ),
            EnergyData(
                power_generation=1200.0,
                power_consumption=900.0,
                battery_charge_level=70.0,
                grid_power=300.0,
                timestamp=datetime.now()
            )
        ]
        
        mock_session.exec = lambda statement: type('MockResult', (), {'all': lambda: test_energy_data})()
        mock_get_session.return_value = mock_session
        
        response = test_client.get("/analytics/energy-summary?hours=24")
        assert response.status_code == 200
        data = response.json()
        assert "total_generation_kwh" in data
        assert "average_battery_level_percent" in data
        assert data["data_points"] == 2


def test_optimize_resources_endpoint(test_client):
    """Test resource optimization endpoint"""
    with patch('new_main.get_session') as mock_get_session:
        mock_session = Session.__new__(Session)
        
        # Mock latest energy data with low battery
        test_energy_data = EnergyData(
            power_generation=1500.0,
            power_consumption=800.0,
            battery_charge_level=25.0,  # Low battery
            grid_power=700.0,
            timestamp=datetime.now()
        )
        
        # Mock database operations
        mock_session.exec = lambda statement: type('MockResult', (), {
            'first': lambda: test_energy_data,
            'all': lambda: []
        })()
        mock_session.add = lambda obj: None
        mock_session.commit = lambda: None
        mock_get_session.return_value = mock_session
        
        response = test_client.post("/actions/optimize-resources")
        assert response.status_code == 200
        data = response.json()
        assert "optimizations_applied" in data
        assert data["battery_level"] == 25.0


if __name__ == "__main__":
    pytest.main([__file__])