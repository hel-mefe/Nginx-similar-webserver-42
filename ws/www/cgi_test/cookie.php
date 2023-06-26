<?php
if ($_SERVER['REQUEST_METHOD'] == 'POST') {
    // save $_FILES['avatar'] to a inside a folder
    $avatar = $_FILES['avatar'];
    $avatar_name = $avatar['name'];
    $avatar_tmp_name = $avatar['tmp_name'];
    $avatar_size = $avatar['size'];
    $avatar_error = $avatar['error'];

    $avatar_ext = explode('.', $avatar_name);
    $avatar_actual_ext = strtolower(end($avatar_ext));

    $allowed = array('jpg', 'jpeg', 'png');

    if (in_array($avatar_actual_ext, $allowed)) {
        if ($avatar_error === 0) {
            if ($avatar_size < 1000000) {
                $avatar_name_new = uniqid('', true) . '.' . $avatar_actual_ext;
                $avatar_destination = 'uploads/' . $avatar_name_new;
                move_uploaded_file($avatar_tmp_name, $avatar_destination);
                setcookie('email', $_POST['email'], time() + 3600 * 24 * 7);
                setcookie('avatar', $avatar_destination, time() + 3600 * 24 * 7);
                setcookie('name', $_POST['name'], time() + 3600 * 24 * 7);
                $_COOKIE['name'] = $_POST['name'];
                $_COOKIE['email'] = $_POST['email'];
                $_COOKIE['avatar'] = $avatar_destination;
            } else {
                echo 'File too big';
                exit(1);
            }
        } else {
            echo 'Error';
            exit(1);
        }
    } else {
        echo 'Not allowed';
        exit(1);
    }
}
?>

<!DOCTYPE html>
<div>
    <?php if (isset($_COOKIE['name']) && isset($_COOKIE['email'])): ?>
        <p>
            <center><h1>Hello, <?= $_COOKIE['name'] ?>!</h1></center><hr>
            <center><img src="<?= $_COOKIE['avatar'] ?>" alt="avatar" width="300" height="300" style="object-fit: cover;"></center><hr><br/>
            <center><?php echo $_COOKIE['name']; ?></center>
            <center><?php echo $_COOKIE['email']; ?></center><br/><br/>
            <form action="logout.php" method="post">
                <center><input type="submit" value="Logout"></center> 
            </form>
        </p>
    <?php else: ?>
        <form method="post" enctype="multipart/form-data">
            <input type="text" name="name" placeholder="Name" />
            <input type="text" name="email" placeholder="Email" />
            <input type="file" id="avatar" name="avatar" accept="image/png, image/jpeg, image/jpg" />
            <input type="submit" value="Submit" />
        </form>
    <?php endif; ?>
</div>
