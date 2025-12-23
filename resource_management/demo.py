#!/usr/bin/env python3
"""
Demo script showing SQLModel integration in the Resource Management Service
This script demonstrates the key features without requiring full dependencies
"""

import sqlite3
import json
from datetime import datetime


class SimpleSQLModelDemo:
    """Simplified demo of SQLModel functionality using basic sqlite3"""
    
    def __init__(self, db_path=":memory:"):
        self.db_path = db_path
        self.conn = sqlite3.connect(db_path)
        self.conn.row_factory = sqlite3.Row  # Enable dict-like access
        self.create_tables()
    
    def create_tables(self):
        """Create tables similar to our SQLModel models"""
        cursor = self.conn.cursor()
        
        # EnergyData table
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS energydata (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
                power_generation REAL NOT NULL,
                power_consumption REAL NOT NULL,
                battery_charge_level REAL NOT NULL,
                grid_power REAL NOT NULL
            )
        """)
        
        # SystemStatus table
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS systemstatus (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
                system_state TEXT NOT NULL,
                temperature REAL,
                error_count INTEGER DEFAULT 0,
                uptime_seconds INTEGER DEFAULT 0
            )
        """)
        
        # ResourceAllocation table
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS resourceallocation (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
                resource_type TEXT NOT NULL,
                allocated_power REAL NOT NULL,
                priority INTEGER NOT NULL,
                is_active BOOLEAN DEFAULT 1
            )
        """)
        
        self.conn.commit()
        print("✓ Database tables created successfully")
    
    def add_energy_data(self, power_gen, power_cons, battery_level, grid_power):
        """Add energy data record"""
        cursor = self.conn.cursor()
        cursor.execute("""
            INSERT INTO energydata (power_generation, power_consumption, battery_charge_level, grid_power)
            VALUES (?, ?, ?, ?)
        """, (power_gen, power_cons, battery_level, grid_power))
        self.conn.commit()
        return cursor.lastrowid
    
    def add_resource_allocation(self, resource_type, allocated_power, priority, is_active=True):
        """Add resource allocation record"""
        cursor = self.conn.cursor()
        cursor.execute("""
            INSERT INTO resourceallocation (resource_type, allocated_power, priority, is_active)
            VALUES (?, ?, ?, ?)
        """, (resource_type, allocated_power, priority, is_active))
        self.conn.commit()
        return cursor.lastrowid
    
    def get_latest_energy_data(self):
        """Get latest energy data (similar to SQLModel query)"""
        cursor = self.conn.cursor()
        cursor.execute("""
            SELECT * FROM energydata 
            ORDER BY timestamp DESC 
            LIMIT 1
        """)
        row = cursor.fetchone()
        return dict(row) if row else None
    
    def get_active_allocations(self):
        """Get active resource allocations"""
        cursor = self.conn.cursor()
        cursor.execute("""
            SELECT * FROM resourceallocation 
            WHERE is_active = 1 
            ORDER BY priority ASC, timestamp DESC
        """)
        return [dict(row) for row in cursor.fetchall()]
    
    def optimize_resources(self):
        """Demonstrate resource optimization logic"""
        latest_energy = self.get_latest_energy_data()
        if not latest_energy:
            print("No energy data available for optimization")
            return
        
        print(f"Current energy state:")
        print(f"  Generation: {latest_energy['power_generation']} W")
        print(f"  Consumption: {latest_energy['power_consumption']} W")
        print(f"  Battery: {latest_energy['battery_charge_level']}%")
        print(f"  Grid: {latest_energy['grid_power']} W")
        
        # Deactivate existing allocations
        cursor = self.conn.cursor()
        cursor.execute("UPDATE resourceallocation SET is_active = 0")
        
        optimizations = []
        
        # Battery optimization logic
        if latest_energy['battery_charge_level'] < 30:
            allocated_power = min(2000.0, latest_energy['power_generation'] * 0.8)
            self.add_resource_allocation("battery_charge", allocated_power, 1)
            optimizations.append(f"Battery charging: {allocated_power}W")
        
        # Grid export when battery full
        elif latest_energy['battery_charge_level'] > 80:
            excess = latest_energy['power_generation'] - latest_energy['power_consumption']
            if excess > 0:
                self.add_resource_allocation("grid_export", excess, 3)
                optimizations.append(f"Grid export: {excess}W")
        
        # Load balancing
        if latest_energy['power_consumption'] > latest_energy['power_generation']:
            deficit = latest_energy['power_consumption'] - latest_energy['power_generation']
            if latest_energy['battery_charge_level'] > 30:
                discharge_power = min(deficit, 1500.0)
                self.add_resource_allocation("battery_discharge", discharge_power, 2)
                optimizations.append(f"Battery discharge: {discharge_power}W")
        
        self.conn.commit()
        
        if optimizations:
            print(f"✓ Optimizations applied: {', '.join(optimizations)}")
        else:
            print("✓ No optimization needed - system is balanced")
    
    def show_analytics(self):
        """Show analytics similar to new_main.py endpoints"""
        cursor = self.conn.cursor()
        
        # Energy analytics
        cursor.execute("""
            SELECT 
                COUNT(*) as data_points,
                AVG(power_generation) as avg_generation,
                AVG(power_consumption) as avg_consumption,
                AVG(battery_charge_level) as avg_battery,
                AVG(grid_power) as avg_grid
            FROM energydata
        """)
        analytics = dict(cursor.fetchone())
        
        print(f"\nAnalytics Summary:")
        print(f"  Data points: {analytics['data_points']}")
        print(f"  Avg generation: {analytics['avg_generation']:.1f} W")
        print(f"  Avg consumption: {analytics['avg_consumption']:.1f} W")
        print(f"  Avg battery level: {analytics['avg_battery']:.1f}%")
        print(f"  Avg grid power: {analytics['avg_grid']:.1f} W")
        
        # Active allocations
        allocations = self.get_active_allocations()
        if allocations:
            print(f"\nActive Resource Allocations:")
            for alloc in allocations:
                print(f"  {alloc['resource_type']}: {alloc['allocated_power']}W (priority {alloc['priority']})")
        else:
            print(f"\nNo active resource allocations")
    
    def close(self):
        """Close database connection"""
        self.conn.close()


def main():
    """Run the demo"""
    print("SENEC2OpenWB Resource Management SQLModel Demo")
    print("=" * 50)
    
    # Create demo instance
    demo = SimpleSQLModelDemo()
    
    try:
        # Add sample energy data (simulating different scenarios)
        print("\n1. Adding sample energy data...")
        scenarios = [
            (2000, 800, 85, 1200),   # High generation, battery full
            (1200, 1500, 45, -300),  # Low generation, medium battery
            (800, 1200, 15, -400),   # Very low generation, low battery
            (2500, 900, 65, 1600),   # High generation, medium battery
        ]
        
        for i, (gen, cons, batt, grid) in enumerate(scenarios, 1):
            energy_id = demo.add_energy_data(gen, cons, batt, grid)
            print(f"  Scenario {i}: ID={energy_id}, Gen={gen}W, Cons={cons}W, Batt={batt}%, Grid={grid}W")
        
        # Show optimization for each scenario
        print(f"\n2. Running resource optimization...")
        demo.optimize_resources()
        
        # Show analytics
        print(f"\n3. Analytics Dashboard:")
        demo.show_analytics()
        
        print(f"\n✓ Demo completed successfully!")
        print(f"\nThis demonstrates the core SQLModel functionality:")
        print(f"  - Database table creation and management")
        print(f"  - Data insertion with automatic IDs and timestamps")
        print(f"  - Complex queries with filtering and ordering")
        print(f"  - Resource optimization logic")
        print(f"  - Analytics and reporting")
        
    finally:
        demo.close()


if __name__ == "__main__":
    main()