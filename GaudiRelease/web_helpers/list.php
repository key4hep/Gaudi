<?php
// return the list of available .zip files (without extension)
header('Content-Type: text/plain');

$versions = array();
$has_master = false;

$dh = opendir(".");
while (($file = readdir($dh)) !== false) {
  if (preg_match("/(v[0-9]+r.*)\.zip/", $file, $matches))
    $versions[] = $matches[1];
  elseif ($file == "master.zip") {
    $has_master = true;
  }
}
closedir($dh);

natsort($versions);
if ($has_master) $versions[] = "master";

$versions = array_reverse($versions);

foreach ($versions as $value) {
  echo $value . "\n";
}
?>
