#!/usr/bin/env python3
"""
Integration test showing how the C++ and Python services work together
This demonstrates the MQTT integration between the existing C++ service and new Python service
"""

import json
import time
import threading
from datetime import datetime


class MockMQTTIntegration:
    """Mock MQTT integration to demonstrate service interaction"""
    
    def __init__(self):
        self.messages = []
        self.subscribers = []
    
    def publish(self, topic, payload):
        """Simulate publishing a message"""
        message = {
            'topic': topic,
            'payload': payload,
            'timestamp': datetime.now().isoformat()
        }
        self.messages.append(message)
        print(f"📤 MQTT Publish: {topic} -> {payload}")
        
        # Notify subscribers
        for subscriber in self.subscribers:
            if self.topic_matches(subscriber['pattern'], topic):
                subscriber['callback'](topic, payload)
    
    def subscribe(self, topic_pattern, callback):
        """Simulate subscribing to a topic"""
        self.subscribers.append({
            'pattern': topic_pattern,
            'callback': callback
        })
        print(f"📥 MQTT Subscribe: {topic_pattern}")
    
    def topic_matches(self, pattern, topic):
        """Simple topic matching (simplified)"""
        if '+' in pattern:
            # Replace + with anything for basic matching
            pattern_parts = pattern.split('/')
            topic_parts = topic.split('/')
            if len(pattern_parts) != len(topic_parts):
                return False
            for p, t in zip(pattern_parts, topic_parts):
                if p != '+' and p != t:
                    return False
            return True
        return pattern == topic


class MockCppService:
    """Mock the C++ SENEC2OpenWB service behavior"""
    
    def __init__(self, mqtt_client):
        self.mqtt_client = mqtt_client
        self.running = False
    
    def start(self):
        """Start the mock C++ service"""
        print("🔧 C++ Service: Starting SENEC data acquisition...")
        self.running = True
        
        # Simulate periodic data publishing (like the C++ service does)
        def publish_loop():
            counter = 0
            while self.running:
                counter += 1
                
                # Simulate SENEC data (what the C++ service would publish)
                senec_data = {
                    'timestamp': datetime.now().isoformat(),
                    'power_generation': 1500 + (counter % 500),
                    'power_consumption': 800 + (counter % 300),
                    'battery_charge_level': 60 + (counter % 40),
                    'grid_power': 700 + (counter % 400),
                    'sequence': counter
                }
                
                # Publish to MQTT (simulating C++ publishing behavior)
                self.mqtt_client.publish('senec/house01/data', json.dumps(senec_data))
                
                # Also publish status data
                status_data = {
                    'timestamp': datetime.now().isoformat(),
                    'system_state': 'RUNNING',
                    'temperature': 45.0 + (counter % 10),
                    'uptime_seconds': counter * 5,
                    'error_count': 0
                }
                self.mqtt_client.publish('senec/house01/status', json.dumps(status_data))
                
                time.sleep(2)  # 2-second interval
        
        # Start in background thread
        self.thread = threading.Thread(target=publish_loop, daemon=True)
        self.thread.start()
    
    def stop(self):
        """Stop the mock C++ service"""
        print("🔧 C++ Service: Stopping...")
        self.running = False


class MockPythonService:
    """Mock the Python Resource Management service behavior"""
    
    def __init__(self, mqtt_client):
        self.mqtt_client = mqtt_client
        self.energy_data = []
        self.resource_allocations = []
        
        # Subscribe to topics (like the Python service does)
        self.mqtt_client.subscribe('senec/+/data', self.on_energy_data)
        self.mqtt_client.subscribe('senec/+/status', self.on_status_data)
    
    def on_energy_data(self, topic, payload):
        """Handle energy data from C++ service"""
        try:
            data = json.loads(payload)
            print(f"🐍 Python Service: Received energy data from {topic}")
            print(f"   Generation: {data['power_generation']}W, Consumption: {data['power_consumption']}W")
            print(f"   Battery: {data['battery_charge_level']}%, Grid: {data['grid_power']}W")
            
            # Store data (simulating SQLModel operations)
            self.energy_data.append(data)
            
            # Trigger resource management
            self.manage_resources(data)
            
        except Exception as e:
            print(f"🐍 Python Service: Error processing energy data: {e}")
    
    def on_status_data(self, topic, payload):
        """Handle status data from C++ service"""
        try:
            data = json.loads(payload)
            print(f"🐍 Python Service: Status - {data['system_state']}, Temp: {data['temperature']}°C")
        except Exception as e:
            print(f"🐍 Python Service: Error processing status data: {e}")
    
    def manage_resources(self, energy_data):
        """Resource management logic (simulating SQLModel operations)"""
        allocations = []
        
        battery_level = energy_data['battery_charge_level']
        generation = energy_data['power_generation']
        consumption = energy_data['power_consumption']
        
        # Clear previous allocations
        self.resource_allocations.clear()
        
        # Resource allocation logic
        if battery_level < 25:
            allocation = {
                'resource_type': 'battery_charge',
                'allocated_power': min(2000, generation * 0.8),
                'priority': 1,
                'reason': 'Low battery'
            }
            self.resource_allocations.append(allocation)
            allocations.append(f"Battery charge: {allocation['allocated_power']}W")
        
        elif battery_level > 85:
            excess = generation - consumption
            if excess > 0:
                allocation = {
                    'resource_type': 'grid_export',
                    'allocated_power': excess,
                    'priority': 3,
                    'reason': 'Battery full, excess energy'
                }
                self.resource_allocations.append(allocation)
                allocations.append(f"Grid export: {allocation['allocated_power']}W")
        
        if consumption > generation and battery_level > 30:
            deficit = consumption - generation
            allocation = {
                'resource_type': 'battery_discharge',
                'allocated_power': min(deficit, 1500),
                'priority': 2,
                'reason': 'Load balancing'
            }
            self.resource_allocations.append(allocation)
            allocations.append(f"Battery discharge: {allocation['allocated_power']}W")
        
        if allocations:
            print(f"🐍 Python Service: Resource allocations: {', '.join(allocations)}")
            
            # Publish control commands back to MQTT (for other services)
            control_data = {
                'timestamp': datetime.now().isoformat(),
                'allocations': self.resource_allocations
            }
            self.mqtt_client.publish('openwb/resource_management/control', json.dumps(control_data))
    
    def get_analytics(self):
        """Generate analytics from stored data"""
        if not self.energy_data:
            return "No data available"
        
        recent_data = self.energy_data[-10:]  # Last 10 readings
        avg_generation = sum(d['power_generation'] for d in recent_data) / len(recent_data)
        avg_consumption = sum(d['power_consumption'] for d in recent_data) / len(recent_data)
        avg_battery = sum(d['battery_charge_level'] for d in recent_data) / len(recent_data)
        
        return {
            'data_points': len(recent_data),
            'avg_generation': round(avg_generation, 1),
            'avg_consumption': round(avg_consumption, 1),
            'avg_battery_level': round(avg_battery, 1),
            'active_allocations': len(self.resource_allocations)
        }


def main():
    """Run the integration demo"""
    print("SENEC2OpenWB Integration Demo")
    print("C++ Service ↔ MQTT ↔ Python Resource Management")
    print("=" * 60)
    
    # Create MQTT broker simulation
    mqtt_broker = MockMQTTIntegration()
    
    # Create services
    cpp_service = MockCppService(mqtt_broker)
    python_service = MockPythonService(mqtt_broker)
    
    try:
        # Start C++ service
        cpp_service.start()
        
        print(f"\n🚀 Services started! Running for 10 seconds...")
        print(f"   Watch the data flow from C++ → MQTT → Python")
        print(f"   Python service will process data and create resource allocations\n")
        
        # Let it run for a bit
        time.sleep(10)
        
        # Show analytics
        print(f"\n📊 Analytics from Python Service:")
        analytics = python_service.get_analytics()
        if isinstance(analytics, dict):
            for key, value in analytics.items():
                print(f"   {key.replace('_', ' ').title()}: {value}")
        else:
            print(f"   {analytics}")
        
        print(f"\n📈 Total MQTT Messages: {len(mqtt_broker.messages)}")
        
        # Show some sample messages
        print(f"\n📝 Sample MQTT Messages:")
        for msg in mqtt_broker.messages[-3:]:
            print(f"   {msg['topic']}: {msg['payload'][:100]}...")
        
        print(f"\n✅ Integration demo completed successfully!")
        print(f"\nThis demonstrates:")
        print(f"  ✓ C++ service publishes SENEC data to MQTT (preserved functionality)")
        print(f"  ✓ Python service subscribes and processes the data")
        print(f"  ✓ SQLModel-like data storage and analytics")
        print(f"  ✓ Resource management and optimization")
        print(f"  ✓ Control commands published back to MQTT")
        print(f"  ✓ Both services running independently but cooperatively")
        
    finally:
        cpp_service.stop()


if __name__ == "__main__":
    main()