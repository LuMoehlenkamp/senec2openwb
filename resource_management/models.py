"""Database models for Resource Management Service"""

from datetime import datetime
from typing import Optional
from sqlmodel import SQLModel, Field, create_engine, Session


class SenecDataBase(SQLModel):
    """Base model for SENEC data"""
    id: Optional[int] = Field(default=None, primary_key=True)
    timestamp: datetime = Field(default_factory=datetime.now)


class EnergyData(SenecDataBase, table=True):
    """Energy data from SENEC system"""
    power_generation: float = Field(description="Current power generation in W")
    power_consumption: float = Field(description="Current power consumption in W")
    battery_charge_level: float = Field(description="Battery charge level in %")
    grid_power: float = Field(description="Grid power flow in W (positive = export)")
    
    
class SystemStatus(SenecDataBase, table=True):
    """System status information"""
    system_state: str = Field(description="Current system state")
    temperature: Optional[float] = Field(default=None, description="System temperature in °C")
    error_count: int = Field(default=0, description="Number of errors")
    uptime_seconds: int = Field(default=0, description="System uptime in seconds")


class ResourceAllocation(SenecDataBase, table=True):
    """Resource allocation for energy management"""
    resource_type: str = Field(description="Type of resource (battery, grid, solar)")
    allocated_power: float = Field(description="Allocated power in W")
    priority: int = Field(description="Priority level (1-10)")
    is_active: bool = Field(default=True, description="Whether allocation is active")


# Database connection
DATABASE_URL = "sqlite:///./resource_management.db"
engine = create_engine(DATABASE_URL, echo=True)


def create_db_and_tables():
    """Create database and tables"""
    SQLModel.metadata.create_all(engine)


def get_session():
    """Get database session"""
    with Session(engine) as session:
        yield session