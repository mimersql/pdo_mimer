--TEST--
Check if pdo_mimer is loaded
--EXTENSIONS--
pdo_mimer
--FILE--
<?php
echo 'The extension "pdo_mimer" is available';
?>
--EXPECT--
The extension "pdo_mimer" is available
