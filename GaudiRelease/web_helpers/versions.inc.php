<?php
// Documentations are published as SquashFS images, exposed as directories named
// after the version. Old documentations that have not been migrated yet are
// still .zip files (served by extract.php).
//
// Return an array mapping the name of each available documentation (most recent
// first, "master" on top) to a boolean telling if a .zip file is available.
function list_versions() {
  $versions = array();

  $dh = opendir(".");
  while (($file = readdir($dh)) !== false) {
    if (preg_match("/^(master|v[0-9]+r.*?)(\.zip)?$/", $file, $matches)) {
      $name = $matches[1];
      $versions[$name] = !empty($matches[2]) || !empty($versions[$name]);
    }
  }
  closedir($dh);

  $has_master = isset($versions["master"]);
  $master_zip = $has_master && $versions["master"];
  unset($versions["master"]);

  $names = array_keys($versions);
  natsort($names);
  $result = array();
  foreach (array_reverse($names) as $name) {
    $result[$name] = $versions[$name];
  }
  if ($has_master) {
    $result = array("master" => $master_zip) + $result;
  }
  return $result;
}
?>
