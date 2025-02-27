<?php
// welcome.php
session_start();

// Check if user is logged in
if (!isset($_SESSION['logged_in']) || $_SESSION['logged_in'] !== true) {
    header("Location: login.php");
    exit;
}
?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Welcome</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; text-align: center; }
        .logout-btn { background: #f44336; color: white; padding: 10px 15px; border: none; cursor: pointer; text-decoration: none; }
    </style>
</head>
<body>
    <h1>Welcome, <?php echo htmlspecialchars($_SESSION['username']); ?>!</h1>
    <p>You have successfully logged in.</p>
    <p>Server Name: <?php echo htmlspecialchars(gethostname()); ?></p>
    <p>PHP Version: <?php echo htmlspecialchars(phpversion()); ?></p>
    <a href="login.php?logout=true" class="logout-btn">Logout</a>
</body>
</html>