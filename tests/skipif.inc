<?php
if (!getenv('PDO_MIMER_TEST_DSN')) die('skip: PDO_MIMER_TEST_DSN must be set');
if (!extension_loaded('pdo') || !extension_loaded('pdo_mimer')) die('skip: not loaded');