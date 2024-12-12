<?php
// MySQL database connection settings
$servername = "172.20.241.9/luedataa_kannasta.php";  // MySQL server IP address
$username = "dbaccess_ro";  // Replace with the actual username
$password = "vsdjkvwselkvwe234wv234vsdfas";  // Replace with the actual password
$dbname = "rawdata";  // Replace with the actual database name

// Create connection to MySQL server
$conn = new mysqli($servername, $username, $password, $dbname);

// Check connection
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

// Get the groupid (you can modify this to fetch dynamically or from a form)
$groupid = 24;  // Replace with the actual groupid you want to fetch

// Query to fetch data from the database based on the groupid
$sql = "SELECT * FROM rawdata WHERE groupid = $groupid";
$result = $conn->query($sql);

// Check if there are results
if ($result->num_rows > 0) {
    // Output the data as a table
    echo "<table border='1'>
            <tr>
                <th>Group ID</th>
                <th>From MAC</th>
                <th>To MAC</th>
                <th>Sensor Value A</th>
                <th>Sensor Value B</th>
                <th>Sensor Value C</th>
            </tr>";
    
    while($row = $result->fetch_assoc()) {
        echo "<tr>
                <td>" . $row["groupid"] . "</td>
                <td>" . $row["from_mac"] . "</td>
                <td>" . $row["to_mac"] . "</td>
                <td>" . $row["sensorvalue_a"] . "</td>
                <td>" . $row["sensorvalue_b"] . "</td>
                <td>" . $row["sensorvalue_c"] . "</td>
              </tr>";
    }
    
    echo "</table>";
} else {
    echo "0 results";
}

// Close the database connection
$conn->close();
?>
