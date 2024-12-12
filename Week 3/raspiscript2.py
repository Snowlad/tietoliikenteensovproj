import asyncio
from bleak import BleakClient
import mysql.connector
import struct

NORDIC_MAC_ADDRESS = "D0:0F:5D:7D:BD:E8"  # Replace with your actual MAC address
SERVICE_UUID = "12345678-1234-1234-1234-1234567890ab"  # Replace with your sensor service UUID
CHARACTERISTIC_UUID = "12345678-1234-5678-1234-567812345679"  # Replace with your sensor data characteristic UUID

# MySQL connection details
MYSQL_HOST = "172.20.241.9"
MYSQL_USER = "dbaccess_rw"  # Replace with your actual credentials
MYSQL_PASSWORD = "fasdjkf2389vw2c3k234vk2f3"  # Replace with your actual credentials
MYSQL_DATABASE = "measurements"

# Function to store data in MySQL
def tallenna_data_mysql(x, y, z):
    try:
        # Connect to MySQL
        connection = mysql.connector.connect(
            host=MYSQL_HOST,
            user=MYSQL_USER,
            password=MYSQL_PASSWORD,
            database=MYSQL_DATABASE
        )
        cursor = connection.cursor()

        # SQL query to insert data into the table
        query = """
        INSERT INTO rawdata (groupid, from_mac, to_mac, sensorvalue_a, sensorvalue_b, sensorvalue_c, sensorvalue_d, sensorvalue_e, sensorvalue_f)
        VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s)
        """

        groupid = 24  # Example static group ID, change if needed
        from_mac = "D0:0F:5D:7D:BD:E8"
        to_mac = "B8:27:EB:34:22:B6"  # Replace with your Raspberry Pi MAC address if needed

        # Insert the data into the database, with the extra slots filled with 0
        cursor.execute(query, (groupid, from_mac, to_mac, x, y, z, 0, 0, 0))

        # Commit and close the connection
        connection.commit()
        cursor.close()
        connection.close()

        print(f"Data saved to MySQL: x={x}, y={y}, z={z}")

    except mysql.connector.Error as err:
        print(f"MySQL Error: {err}")

# Callback function to handle notifications
def handle_notification(sender, data):
    try:
        # Print the length of the received data to debug
        print(f"Received data length: {len(data)} bytes")
        
        # Check if the data is exactly 6 bytes (for 3 sensor values, each 2 bytes)
        if len(data) == 6:
            # Unpack the data: 3 int16 values (x, y, z)
            x, y, z = struct.unpack("<3h", data)
            print(f"Received data: x={x}, y={y}, z={z}")
            
            # Save the data to MySQL
            tallenna_data_mysql(x, y, z)
        else:
            print(f"Unexpected data length: {len(data)} bytes")
            
    except struct.error as e:
        print(f"Error unpacking data: {e}")

# Main function to connect to the Bluetooth device
async def muodosta_bluetooth_yhteys():
    try:
        async with BleakClient(NORDIC_MAC_ADDRESS) as client:
            if not client.is_connected:
                print("Connection failed.")
                return

            print(f"Connected to Nordic device: {NORDIC_MAC_ADDRESS}")

            # Start receiving notifications from the characteristic
            await client.start_notify(CHARACTERISTIC_UUID, handle_notification)
            print("Waiting for data from Nordic device...")

            # Continuous listening
            while True:
                await asyncio.sleep(1)

    except Exception as e:
        print(f"Bluetooth connection error: {e}")

# Run the program
if __name__ == "__main__":
    asyncio.run(muodosta_bluetooth_yhteys())
