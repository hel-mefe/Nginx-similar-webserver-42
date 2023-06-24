<?php
    // Check if files were uploaded
    if(isset($_FILES['files'])){
        $errors= array();
        $uploadedFiles = $_FILES['files'];
        $uploadDirectory = 'uploads/';
        foreach($uploadedFiles['tmp_name'] as $key => $tmp_name ){
            $file_name = $uploadedFiles['name'][$key];
            $file_size = $uploadedFiles['size'][$key];
            $file_tmp = $uploadedFiles['tmp_name'][$key];
            $file_type= $uploadedFiles['type'][$key];
            if($file_size > 2097152){
                $errors[]='File size must be less than 2 MB';
            }
            $uploadPath = $uploadDirectory . basename($file_name);
            if(empty($errors)==true){
                if(move_uploaded_file($file_tmp,$uploadPath))
                {
                    header('HTTP/1.1 201 Created');
                    echo "Successfully uploaded";
                }else{
                    header('HTTP/1.1 403 Forbidden');
                    echo "Error uploading file";
                }
            }else{
                print_r($errors);
            }
        }
    } else {
        echo "Error";
    }
?>