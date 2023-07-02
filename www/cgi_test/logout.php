<?php
if ($_SERVER['REQUEST_METHOD'] == 'POST' && isset($_COOKIE['name']) && isset($_COOKIE['avatar']) && isset($_COOKIE['email']))
{
    unset($_COOKIE['name']);
    unset($_COOKIE['email']);
    unset($_COOKIE['avatar']);
    setcookie('name', '', time() - 3600);
    setcookie('email', '', time() - 3600);
    setcookie('avatar', '', time() - 3600);
    header("Location: cookies.php");
    exit;
}
?>