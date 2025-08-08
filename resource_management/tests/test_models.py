"""Test SQLModel models and database operations"""

import pytest
from datetime import datetime
from sqlmodel import Session, create_engine, SQLModel
from models import EnergyData, SystemStatus, ResourceAllocation


@pytest.fixture
def test_engine():
    """Create test database engine"""
    engine = create_engine("sqlite:///:memory:", echo=True)
    SQLModel.metadata.create_all(engine)
    return engine


@pytest.fixture
def test_session(test_engine):
    """Create test database session"""
    with Session(test_engine) as session:
        yield session


def test_energy_data_creation(test_session):
    """Test creating EnergyData with SQLModel"""
    energy_data = EnergyData(
        power_generation=1500.0,
        power_consumption=800.0,
        battery_charge_level=75.0,
        grid_power=700.0,
        timestamp=datetime.now()
    )
    
    test_session.add(energy_data)
    test_session.commit()
    test_session.refresh(energy_data)
    
    assert energy_data.id is not None
    assert energy_data.power_generation == 1500.0
    assert energy_data.battery_charge_level == 75.0


def test_system_status_creation(test_session):
    """Test creating SystemStatus with SQLModel"""
    status = SystemStatus(
        system_state="RUNNING",
        temperature=45.0,
        error_count=0,
        uptime_seconds=3600,
        timestamp=datetime.now()
    )
    
    test_session.add(status)
    test_session.commit()
    test_session.refresh(status)
    
    assert status.id is not None
    assert status.system_state == "RUNNING"
    assert status.temperature == 45.0


def test_resource_allocation_creation(test_session):
    """Test creating ResourceAllocation with SQLModel"""
    allocation = ResourceAllocation(
        resource_type="battery",
        allocated_power=1000.0,
        priority=1,
        is_active=True,
        timestamp=datetime.now()
    )
    
    test_session.add(allocation)
    test_session.commit()
    test_session.refresh(allocation)
    
    assert allocation.id is not None
    assert allocation.resource_type == "battery"
    assert allocation.is_active is True


def test_energy_data_relationships(test_session):
    """Test querying multiple EnergyData records"""
    # Create multiple records
    for i in range(3):
        energy_data = EnergyData(
            power_generation=1000.0 + i * 100,
            power_consumption=500.0 + i * 50,
            battery_charge_level=50.0 + i * 10,
            grid_power=500.0 + i * 50
        )
        test_session.add(energy_data)
    
    test_session.commit()
    
    # Query all records
    from sqlmodel import select
    statement = select(EnergyData)
    results = test_session.exec(statement).all()
    
    assert len(results) == 3
    assert results[0].power_generation == 1000.0
    assert results[2].power_generation == 1200.0


def test_active_resource_allocations(test_session):
    """Test filtering active resource allocations"""
    # Create active and inactive allocations
    active_allocation = ResourceAllocation(
        resource_type="solar",
        allocated_power=1500.0,
        priority=1,
        is_active=True
    )
    
    inactive_allocation = ResourceAllocation(
        resource_type="grid",
        allocated_power=500.0,
        priority=2,
        is_active=False
    )
    
    test_session.add(active_allocation)
    test_session.add(inactive_allocation)
    test_session.commit()
    
    # Query only active allocations
    from sqlmodel import select
    statement = select(ResourceAllocation).where(ResourceAllocation.is_active == True)
    active_results = test_session.exec(statement).all()
    
    assert len(active_results) == 1
    assert active_results[0].resource_type == "solar"


if __name__ == "__main__":
    pytest.main([__file__])