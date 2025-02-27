<!DOCTYPE html>
<html>
    <head>
        <title>Login</title>
    </head>
    <body>
        <center>
            <br>
            <br>
            <br>
            <br>
            <br>
            <br>
            <br>
            <br>
            <br>
            <br>

<?php
// Start the session
session_start();

// Set content type
header("Content-Type: text/html");


$contentType = $_SERVER['CONTENT_TYPE'] ?? '';
$rawData = file_get_contents('php://input');

if (strpos($contentType, 'application/x-www-form-urlencoded') !== false) {
    parse_str($rawData, $_POST);
} elseif (strpos($contentType, 'application/json') !== false) {
    $_POST = json_decode($rawData, true);
}


// Handle logout
if (isset($_GET['action']) && $_GET['action'] === 'logout') {
    session_destroy();
    echo "You have been logged out. <a href='login.php'>Login Again</a>";
    exit;
}

// Handle login
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $username = $_POST['username'] ?? '';
    $password = $_POST['password'] ?? '';

    // Check credentials
    if ($username === 'admin' && $password === 'admin') {
        $_SESSION['logged_in'] = true;
        echo "Login successful! <a href='login.php?action=logout'>Logout</a>";
    } else {
        echo "Invalid credentials. <a href='login.php'>Try Again</a>";
    }
    exit;
}

// Check if already logged in
if (!empty($_SESSION['logged_in'])) {
    echo "You are already logged in. <a href='login.php?action=logout'>Logout</a>";
    exit;
}

// Login form
?>
        <form method="POST" action="login.php">
            <label for="username">Username:</label>
            <input type="text" id="username" name="username" required>
            <br>
            <br>
            <label for="password">Password:</label>
            <input type="password" id="password" name="password" required>
            <br>
            <br>
            <button type="submit">Login</button>
        </form>
    </center>
    </body>
</html>