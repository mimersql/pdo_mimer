<?php
if (!extension_loaded('pdo') || !extension_loaded('pdo_mimer'))
    die('skip: not loaded');

function skipif(bool $failed, string $msg = ''): void {
    if (!$failed)
        return;
    die('skip ' . ($msg ?: 'no reason given'));
}
