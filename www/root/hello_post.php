<?php
    define('STDIN', fopen('php://stdin', 'r'));
    while (FALSE !== ($line = fgets(STDIN))) {
       echo $line;
    }
?>