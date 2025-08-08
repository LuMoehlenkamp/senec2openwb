"""Main module for Resource Management Service - preserves original functionality patterns"""

import asyncio
import signal
import sys
import logging
from typing import Optional
from datetime import datetime

from models import EnergyData, SystemStatus, ResourceAllocation, create_db_and_tables, get_session
from sqlmodel import Session, select
import paho.mqtt.client as mqtt
from paho.mqtt.client import Client

# Configure logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

# Global signal status (mimicking the C++ original)
signal_status: Optional[int] = None


class ConfigManager:
    """Configuration manager similar to the C++ version"""
    CONFIG_PATH = "./config.json"
    
    def __init__(self):
        # Default values matching the C++ implementation pattern
        self.senec_update_time = 5  # seconds
        self.senec_timeout_time = 30  # seconds
        self.senec_connect_timeout_time = 10  # seconds
        self.mqtt_broker = "mosquitto"
        self.mqtt_port = 1883
        
    def get_senec_update_time(self) -> Optional[int]:
        return self.senec_update_time
    
    def get_senec_timeout_time(self) -> Optional[int]:
        return self.senec_timeout_time
    
    def get_senec_connect_timeout_time(self) -> Optional[int]:
        return self.senec_connect_timeout_time


class ResourceManagementService:
    """Main service class handling resource management"""
    
    def __init__(self, config_manager: ConfigManager):
        self.config_manager = config_manager
        self.mqtt_client: Optional[Client] = None
        self.running = False
        
    def setup_mqtt(self):
        """Setup MQTT client similar to C++ implementation"""
        self.mqtt_client = mqtt.Client()
        self.mqtt_client.on_connect = self.on_mqtt_connect
        self.mqtt_client.on_message = self.on_mqtt_message
        
        try:
            self.mqtt_client.connect(
                self.config_manager.mqtt_broker, 
                self.config_manager.mqtt_port, 
                60
            )
            logger.info("MQTT client connected")
        except Exception as e:
            logger.error(f"Failed to connect to MQTT broker: {e}")
            raise
            
    def on_mqtt_connect(self, client, userdata, flags, rc):
        """MQTT connection callback"""
        if rc == 0:
            logger.info("Successfully connected to MQTT broker")
            # Subscribe to SENEC data topics
            client.subscribe("senec/+/data")
            client.subscribe("senec/+/status")
        else:
            logger.error(f"Failed to connect to MQTT broker with code {rc}")
            
    def on_mqtt_message(self, client, userdata, msg):
        """MQTT message callback - stores data using SQLModel"""
        try:
            topic = msg.topic
            payload = msg.payload.decode()
            logger.info(f"Received message on topic {topic}: {payload}")
            
            # Process different message types
            if "/data" in topic:
                self.process_energy_data(payload)
            elif "/status" in topic:
                self.process_status_data(payload)
                
        except Exception as e:
            logger.error(f"Error processing MQTT message: {e}")
    
    def process_energy_data(self, data: str):
        """Process energy data and store in database"""
        try:
            # Parse data (simplified - in real implementation would parse JSON)
            # For demo purposes, using mock data
            with Session(engine) as session:
                energy_data = EnergyData(
                    power_generation=1500.0,
                    power_consumption=800.0,
                    battery_charge_level=75.0,
                    grid_power=700.0
                )
                session.add(energy_data)
                session.commit()
                logger.info("Energy data stored successfully")
                
        except Exception as e:
            logger.error(f"Error storing energy data: {e}")
    
    def process_status_data(self, data: str):
        """Process status data and store in database"""
        try:
            with Session(engine) as session:
                status_data = SystemStatus(
                    system_state="RUNNING",
                    temperature=45.0,
                    error_count=0,
                    uptime_seconds=3600
                )
                session.add(status_data)
                session.commit()
                logger.info("Status data stored successfully")
                
        except Exception as e:
            logger.error(f"Error storing status data: {e}")
    
    async def run_periodic_tasks(self):
        """Run periodic tasks similar to the C++ event loop"""
        while self.running:
            try:
                # Periodic resource management tasks
                await self.manage_resources()
                await asyncio.sleep(self.config_manager.get_senec_update_time())
                
            except Exception as e:
                logger.error(f"Error in periodic tasks: {e}")
                await asyncio.sleep(1)
    
    async def manage_resources(self):
        """Manage energy resources based on current data"""
        try:
            with Session(engine) as session:
                # Get latest energy data
                latest_energy = session.exec(
                    select(EnergyData).order_by(EnergyData.timestamp.desc()).limit(1)
                ).first()
                
                if latest_energy:
                    # Simple resource allocation logic
                    if latest_energy.battery_charge_level < 20:
                        # Prioritize battery charging
                        allocation = ResourceAllocation(
                            resource_type="battery",
                            allocated_power=1000.0,
                            priority=1,
                            is_active=True
                        )
                        session.add(allocation)
                        session.commit()
                        logger.info("Battery charging prioritized")
                        
        except Exception as e:
            logger.error(f"Error in resource management: {e}")
    
    async def start(self):
        """Start the service"""
        logger.info("Starting Resource Management Service")
        self.running = True
        
        # Setup MQTT
        self.setup_mqtt()
        
        # Start MQTT loop in background
        self.mqtt_client.loop_start()
        
        # Run periodic tasks
        await self.run_periodic_tasks()
    
    def stop(self):
        """Stop the service"""
        logger.info("Stopping Resource Management Service")
        self.running = False
        if self.mqtt_client:
            self.mqtt_client.loop_stop()
            self.mqtt_client.disconnect()


def signal_handler(signum, frame):
    """Signal handler similar to C++ version"""
    global signal_status
    signal_status = signum
    logger.info(f"Received signal: {signal_status}")
    sys.exit(0)


async def main():
    """Main function preserving the C++ structure and patterns"""
    global signal_status
    
    # Setup signal handlers (similar to C++)
    signal.signal(signal.SIGINT, signal_handler)   # SIGINT 2
    signal.signal(signal.SIGTERM, signal_handler)  # SIGTERM 15
    
    # Initialize configuration manager
    config_manager = ConfigManager()
    
    # Validate configuration (similar to C++ validation)
    senec_update_time = config_manager.get_senec_update_time()
    senec_timeout_time = config_manager.get_senec_timeout_time()
    senec_connect_timeout_time = config_manager.get_senec_connect_timeout_time()
    
    if not all([senec_update_time, senec_timeout_time, senec_connect_timeout_time]):
        logger.error("Incomplete configuration. GoodBye!")
        return -1
    
    try:
        # Create database and tables
        create_db_and_tables()
        
        # Initialize and start service
        service = ResourceManagementService(config_manager)
        await service.start()
        
    except Exception as e:
        logger.error(f"Exception: {e}")
        return -1
    
    return 0


if __name__ == "__main__":
    # Import engine here to avoid circular imports
    from models import engine
    
    try:
        exit_code = asyncio.run(main())
        sys.exit(exit_code)
    except KeyboardInterrupt:
        logger.info("Service interrupted by user")
        sys.exit(0)
    except Exception as e:
        logger.error(f"Fatal error: {e}")
        sys.exit(1)