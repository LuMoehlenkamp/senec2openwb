"""Enhanced main module with advanced SQLModel integration and FastAPI"""

import asyncio
import logging
from contextlib import asynccontextmanager
from typing import List, Optional
from datetime import datetime, timedelta

from fastapi import FastAPI, Depends, HTTPException, BackgroundTasks
from fastapi.responses import JSONResponse
from sqlmodel import Session, select, and_
import uvicorn

from models import (
    EnergyData, SystemStatus, ResourceAllocation, 
    create_db_and_tables, get_session, engine
)
from main import ResourceManagementService, ConfigManager

# Configure logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

# Global service instance
service_instance: Optional[ResourceManagementService] = None


@asynccontextmanager
async def lifespan(app: FastAPI):
    """Application lifespan manager"""
    global service_instance
    
    # Startup
    logger.info("Starting Resource Management Service with FastAPI")
    create_db_and_tables()
    
    config_manager = ConfigManager()
    service_instance = ResourceManagementService(config_manager)
    
    # Start background service
    background_task = asyncio.create_task(service_instance.start())
    
    yield
    
    # Shutdown
    logger.info("Shutting down Resource Management Service")
    if service_instance:
        service_instance.stop()
    background_task.cancel()
    try:
        await background_task
    except asyncio.CancelledError:
        pass


# FastAPI app with SQLModel integration
app = FastAPI(
    title="SENEC Resource Management API",
    description="Resource Management Service for SENEC2OpenWB with SQLModel integration",
    version="1.0.0",
    lifespan=lifespan
)


# API Routes with SQLModel integration

@app.get("/", response_model=dict)
async def root():
    """Root endpoint"""
    return {
        "message": "SENEC Resource Management Service",
        "version": "1.0.0",
        "status": "running"
    }


@app.get("/energy-data", response_model=List[EnergyData])
async def get_energy_data(
    limit: int = 100,
    hours: int = 24,
    session: Session = Depends(get_session)
):
    """Get energy data with SQLModel queries"""
    try:
        # Calculate time threshold
        time_threshold = datetime.now() - timedelta(hours=hours)
        
        # SQLModel query with filtering
        statement = select(EnergyData).where(
            EnergyData.timestamp >= time_threshold
        ).order_by(EnergyData.timestamp.desc()).limit(limit)
        
        energy_data = session.exec(statement).all()
        return energy_data
        
    except Exception as e:
        logger.error(f"Error retrieving energy data: {e}")
        raise HTTPException(status_code=500, detail="Failed to retrieve energy data")


@app.get("/energy-data/latest", response_model=Optional[EnergyData])
async def get_latest_energy_data(session: Session = Depends(get_session)):
    """Get the most recent energy data using SQLModel"""
    try:
        statement = select(EnergyData).order_by(EnergyData.timestamp.desc()).limit(1)
        latest_data = session.exec(statement).first()
        return latest_data
        
    except Exception as e:
        logger.error(f"Error retrieving latest energy data: {e}")
        raise HTTPException(status_code=500, detail="Failed to retrieve latest energy data")


@app.get("/system-status", response_model=List[SystemStatus])
async def get_system_status(
    limit: int = 50,
    session: Session = Depends(get_session)
):
    """Get system status with SQLModel"""
    try:
        statement = select(SystemStatus).order_by(
            SystemStatus.timestamp.desc()
        ).limit(limit)
        
        status_data = session.exec(statement).all()
        return status_data
        
    except Exception as e:
        logger.error(f"Error retrieving system status: {e}")
        raise HTTPException(status_code=500, detail="Failed to retrieve system status")


@app.get("/resource-allocations", response_model=List[ResourceAllocation])
async def get_resource_allocations(
    active_only: bool = True,
    session: Session = Depends(get_session)
):
    """Get resource allocations with SQLModel filtering"""
    try:
        statement = select(ResourceAllocation)
        
        if active_only:
            statement = statement.where(ResourceAllocation.is_active == True)
            
        statement = statement.order_by(
            ResourceAllocation.priority.asc(),
            ResourceAllocation.timestamp.desc()
        )
        
        allocations = session.exec(statement).all()
        return allocations
        
    except Exception as e:
        logger.error(f"Error retrieving resource allocations: {e}")
        raise HTTPException(status_code=500, detail="Failed to retrieve resource allocations")


@app.post("/resource-allocations", response_model=ResourceAllocation)
async def create_resource_allocation(
    allocation: ResourceAllocation,
    session: Session = Depends(get_session)
):
    """Create new resource allocation using SQLModel"""
    try:
        # Set timestamp if not provided
        if not allocation.timestamp:
            allocation.timestamp = datetime.now()
            
        session.add(allocation)
        session.commit()
        session.refresh(allocation)
        
        logger.info(f"Created resource allocation: {allocation.resource_type}")
        return allocation
        
    except Exception as e:
        logger.error(f"Error creating resource allocation: {e}")
        raise HTTPException(status_code=500, detail="Failed to create resource allocation")


@app.put("/resource-allocations/{allocation_id}", response_model=ResourceAllocation)
async def update_resource_allocation(
    allocation_id: int,
    allocation_update: ResourceAllocation,
    session: Session = Depends(get_session)
):
    """Update resource allocation using SQLModel"""
    try:
        # Find existing allocation
        statement = select(ResourceAllocation).where(ResourceAllocation.id == allocation_id)
        existing_allocation = session.exec(statement).first()
        
        if not existing_allocation:
            raise HTTPException(status_code=404, detail="Resource allocation not found")
        
        # Update fields
        existing_allocation.resource_type = allocation_update.resource_type
        existing_allocation.allocated_power = allocation_update.allocated_power
        existing_allocation.priority = allocation_update.priority
        existing_allocation.is_active = allocation_update.is_active
        
        session.add(existing_allocation)
        session.commit()
        session.refresh(existing_allocation)
        
        logger.info(f"Updated resource allocation {allocation_id}")
        return existing_allocation
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Error updating resource allocation: {e}")
        raise HTTPException(status_code=500, detail="Failed to update resource allocation")


@app.delete("/resource-allocations/{allocation_id}")
async def delete_resource_allocation(
    allocation_id: int,
    session: Session = Depends(get_session)
):
    """Delete resource allocation using SQLModel"""
    try:
        statement = select(ResourceAllocation).where(ResourceAllocation.id == allocation_id)
        allocation = session.exec(statement).first()
        
        if not allocation:
            raise HTTPException(status_code=404, detail="Resource allocation not found")
        
        session.delete(allocation)
        session.commit()
        
        logger.info(f"Deleted resource allocation {allocation_id}")
        return {"message": "Resource allocation deleted successfully"}
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Error deleting resource allocation: {e}")
        raise HTTPException(status_code=500, detail="Failed to delete resource allocation")


@app.get("/analytics/energy-summary")
async def get_energy_summary(
    hours: int = 24,
    session: Session = Depends(get_session)
):
    """Get energy analytics summary using SQLModel aggregations"""
    try:
        time_threshold = datetime.now() - timedelta(hours=hours)
        
        # Get energy data within time range
        statement = select(EnergyData).where(EnergyData.timestamp >= time_threshold)
        energy_data = session.exec(statement).all()
        
        if not energy_data:
            return {"message": "No data available for the specified time range"}
        
        # Calculate analytics
        total_generation = sum(d.power_generation for d in energy_data)
        total_consumption = sum(d.power_consumption for d in energy_data)
        avg_battery_level = sum(d.battery_charge_level for d in energy_data) / len(energy_data)
        avg_grid_power = sum(d.grid_power for d in energy_data) / len(energy_data)
        
        return {
            "time_range_hours": hours,
            "data_points": len(energy_data),
            "total_generation_kwh": total_generation / 1000,
            "total_consumption_kwh": total_consumption / 1000,
            "average_battery_level_percent": round(avg_battery_level, 2),
            "average_grid_power_w": round(avg_grid_power, 2),
            "net_energy_kwh": round((total_generation - total_consumption) / 1000, 2)
        }
        
    except Exception as e:
        logger.error(f"Error generating energy summary: {e}")
        raise HTTPException(status_code=500, detail="Failed to generate energy summary")


@app.post("/actions/optimize-resources")
async def optimize_resources(
    background_tasks: BackgroundTasks,
    session: Session = Depends(get_session)
):
    """Trigger resource optimization using SQLModel data"""
    try:
        # Get latest energy data
        latest_energy = session.exec(
            select(EnergyData).order_by(EnergyData.timestamp.desc()).limit(1)
        ).first()
        
        if not latest_energy:
            raise HTTPException(status_code=404, detail="No energy data available")
        
        # Deactivate existing allocations
        existing_allocations = session.exec(
            select(ResourceAllocation).where(ResourceAllocation.is_active == True)
        ).all()
        
        for allocation in existing_allocations:
            allocation.is_active = False
            session.add(allocation)
        
        # Create optimized allocations based on current data
        optimizations = []
        
        # Battery optimization
        if latest_energy.battery_charge_level < 30:
            battery_allocation = ResourceAllocation(
                resource_type="battery_charge",
                allocated_power=min(2000.0, latest_energy.power_generation * 0.8),
                priority=1,
                is_active=True
            )
            session.add(battery_allocation)
            optimizations.append("battery_charge_prioritized")
        
        # Grid export optimization
        if latest_energy.battery_charge_level > 80 and latest_energy.power_generation > latest_energy.power_consumption:
            excess_power = latest_energy.power_generation - latest_energy.power_consumption
            grid_allocation = ResourceAllocation(
                resource_type="grid_export",
                allocated_power=excess_power,
                priority=2,
                is_active=True
            )
            session.add(grid_allocation)
            optimizations.append("grid_export_optimized")
        
        session.commit()
        
        logger.info(f"Resource optimization completed: {optimizations}")
        return {
            "message": "Resource optimization completed",
            "optimizations_applied": optimizations,
            "battery_level": latest_energy.battery_charge_level
        }
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Error optimizing resources: {e}")
        raise HTTPException(status_code=500, detail="Failed to optimize resources")


@app.get("/health")
async def health_check():
    """Health check endpoint"""
    try:
        # Test database connection
        with Session(engine) as session:
            session.exec(select(EnergyData).limit(1))
        
        return {
            "status": "healthy",
            "database": "connected",
            "service": "running" if service_instance and service_instance.running else "stopped",
            "timestamp": datetime.now().isoformat()
        }
        
    except Exception as e:
        logger.error(f"Health check failed: {e}")
        return JSONResponse(
            status_code=503,
            content={
                "status": "unhealthy",
                "error": str(e),
                "timestamp": datetime.now().isoformat()
            }
        )


if __name__ == "__main__":
    """Run the enhanced service with FastAPI and SQLModel"""
    logger.info("Starting Enhanced Resource Management Service with FastAPI")
    
    # Run with uvicorn
    uvicorn.run(
        "new_main:app",
        host="0.0.0.0",
        port=8000,
        reload=False,
        log_level="info"
    )