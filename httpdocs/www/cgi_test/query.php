<?php
if (isset($_GET['firstname']) && isset($_GET['lastname']))
    echo "HI MR: ".$_GET['firstname']." ".$_GET['lastname'];
else
    echo "requeried queries are not set!";
?>