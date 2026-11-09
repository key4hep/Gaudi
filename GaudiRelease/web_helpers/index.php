<!DOCTYPE html>
<html>
<head><title>Available Gaudi Doxygen documentations</title></head>
<body>
<h1>Gaudi Doxygen documentation available for the following versions:</h1>
<ul>
<?php
require 'versions.inc.php';

foreach (list_versions() as $name => $has_zip) {
  echo '<li><a href="' . $name . '/index.html">' . $name . '</a>';
  if ($has_zip) {
    echo ' (<a href="' . $name . '.zip">zip</a>)';
  }
  echo '</li>';
}
?>
</ul>
</body>
</html>
