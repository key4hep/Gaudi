<?php
// return the list of available documentations (without extension)
header('Content-Type: text/plain');

require 'versions.inc.php';

foreach (list_versions() as $name => $has_zip) {
  echo $name . "\n";
}
?>
