<?php

function printErrorInfo(PDOException $exception): void {
    [$_, $code, $msg] = $exception->errorInfo;
    echo "[$code]: $msg";
}

function trycatch(callable $function): void {
    try { $function(); }
    catch(PDOException $e) { printErrorInfo($e); }
}

?>
