<?php
// Set the correct content-type for an image
header('Content-Type: image/jpeg');

// Fetch a random cat image from the Cataas API
$cat_image_url = 'https://cataas.com/cat'; 

// Use file_get_contents to fetch the image data
$image_data = file_get_contents($cat_image_url);

// Output the image data directly to the browser
echo $image_data;
?>
