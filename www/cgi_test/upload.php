<?php
if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_FILES['file']))
{
    $targetDir = 'uploads/';
    $targetFile = $targetDir . basename($_FILES['file']['name']);

    // Check if file already exists
    if (file_exists($targetFile))
    {
        header("HTTP/1.1 403 Forbidden", true, 403);
        echo 'File already exists.';
    } 
    else 
    {
        // Move the uploaded file to the target directory
        if (move_uploaded_file($_FILES['file']['tmp_name'], $targetFile)) 
        {
            header("HTTP/1.1 201 Created", true, 201);
            echo 'File uploaded successfully.';
        }
        else
        {
            header("HTTP/1.1 403 Forbidden", true, 403);
            echo 'Error uploading file.';
        }
    }
}
?>

<!DOCTYPE html>
<html>
<body>
<form method="POST" enctype="multipart/form-data">
<input type="file" name="file" required>
<input type="submit" value="Upload">
</form>
</body>
</html>