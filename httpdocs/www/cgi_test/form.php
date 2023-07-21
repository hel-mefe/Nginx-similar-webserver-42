<?php
    if ($_SERVER["REQUEST_METHOD"] == "POST") {
        $name = $_POST["name"];
        $email = $_POST["email"];

        echo "<h2>Your Input:</h2>";
        echo "Name: " . $name . "<br>";
        echo "Email: " . $email;
    }
?>
<!DOCTYPE html>
<html>
<head>
    <title>PHP Form Example</title>
</head>
<body>
    <h1>PHP Form Example</h1>
    <form method="post" action="form.php">
        <label for="name">Name:</label>
        <input type="text" name="name" id="name"><br><br>
        <label for="email">Email:</label>
        <input type="email" name="email" id="email"><br><br>
        <input type="submit" name="submit" value="Submit">
    </form>
</body>
</html>