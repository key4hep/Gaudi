<!DOCTYPE html>
<html>
<head><title>Available Gaudi Doxygen documentations</title></head>
<body>
<h1>Gaudi Doxygen documentation available for the following versions:</h1>
<ul>
<?php
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
  echo '<li><a href="' . $value . '/index.html">' . $value . '</a> (<a href="' . $value . '.zip">zip</a>)</li>';
}
?>
</ul>
<body>
</html>
