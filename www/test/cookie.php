<?php
session_start();
header("Content-Type: text/html; charset=UTF-8");

// --- Cookie Test ---
// Check if our test cookie exists.
if (!isset($_COOKIE['TestCookie'])) {
    // Set a cookie that expires in one hour.
    setcookie('TestCookie', 'Hello, World!', time() + 3600, '/');
    echo "TestCookie was not set. Setting cookie now. Refresh the page to see it!<br>";
} else {
    echo "TestCookie is set: " . htmlspecialchars($_COOKIE['TestCookie']) . "<br>";
}

// --- Session Test ---
// Initialize or update a session counter.
if (!isset($_SESSION['counter'])) {
    $_SESSION['counter'] = 1;
    echo "Session started. Counter initialized to 1.<br>";
} else {
    $_SESSION['counter']++;
    echo "Session counter: " . $_SESSION['counter'] . "<br>";
}

echo "<p>Refresh the page to see the session counter increment and the cookie persist.</p>";
?>