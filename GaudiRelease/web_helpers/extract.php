<?php
$zip_name = $_GET['zip'];
$path = $_GET['path'];

if ( ! $zip_name || ! file_exists($zip_name) ) {
  header($_SERVER['SERVER_PROTOCOL'] . ' 404 Not Found', true, 404);
  echo "404 - Not Found";
  return;
}

switch(pathinfo($path, PATHINFO_EXTENSION)) {
  case 'css':  $ct = 'text/css';         break;
  case 'png':  $ct = 'image/png';        break;
  case 'gif':  $ct = 'image/gif';        break;
  case 'jpg':  $ct = 'image/jpeg';       break;
  case 'js':   $ct = 'text/javascript';  break;
  case 'svg':  $ct = 'image/svg+xml';    break;
  case 'json': $ct = 'application/json'; break;
  default:     $ct = 'text/html';
}

# PHP < 5.4.5 (or < 5.3.15) has problems with zip files with more than 65535
# entries (issue in the version of libzip used, see
# https://libzip.org/news/release-0.10.html)
if (PHP_VERSION_ID >= 50405) {
  $zip = new ZipArchive();
  if ($zip->open($zip_name)) {
    $index = $zip->locateName($path);
    if ( $index === FALSE ) {
      header($_SERVER['SERVER_PROTOCOL'] . ' 404 Not Found', true, 404);
      echo "404 - Not Found";
    } else {
      header('Content-Type: ' . $ct);
      echo $zip->getFromIndex($index);
    }
    $zip->close();
  }
} else {
  exec('/usr/bin/unzip -qq -l "' . $zip_name . '" "' . $path. '"',
       $output, $return_var);
  if ( $return_var != 0 ) {
    header($_SERVER['SERVER_PROTOCOL'] . ' 404 Not Found', true, 404);
    echo "404 - Not Found";
  } else {
    header('Content-Type: ' . $ct);
    passthru('/usr/bin/unzip -p "' . $zip_name . '" "' . $path. '"');
  }
}
?>
