<?php
header('Content-Type: text/html; charset=utf-8');
?>
<!DOCTYPE html>
<html lang="en">
<head>
   <meta charset="UTF-8">
   <title>Server Check</title>
</head>
<body>
   <h1>Server Information</h1>
   <p><strong>Server Name:</strong> <?php echo gethostname(); ?></p>
   <p><strong>Server Software:</strong> <?php echo $_SERVER['SERVER_NAME']; ?></p>
   <p><strong>PHP Version:</strong> <?php echo phpversion(); ?></p>
   <p><strong>Operating System:</strong> <?php echo PHP_OS; ?></p>
</body>
</html>