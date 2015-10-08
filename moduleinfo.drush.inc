<?php

/**
 * @file
 * Drush command to return info about the modules contained in a directory.
 */

// Constants.
define('DRUSH_MODULEINFO_NO_VERSION', 'NONE');
define('DRUSH_MODULEINFO_CSV_HEADERS', 'Module Name,Version,Path');

/**
 * Implements hook_drush_command().
 */
function moduleinfo_drush_command() {
  $items = array();

  $items['moduleinfo'] = array(
    'description' => dt('Creates a csv file containing the names and versions of all the modules in one or more module directories.'),
    'options' => array(
      'dirs' => array(
        'description' => dt('A comma-separated list of paths (relative to the current directory) of folders to scan for modules.'),
        'required' => TRUE,
      ),
      'filepath' => array(
        'description' => dt('Path (relative to the current directory) of csv file to generate.'),
        'required' => TRUE,
      ),
    ),
    'examples' => array(
      'drush mi --dirs=sites/all/modules/,sites/default/folders --filepath=./module-list.csv' => dt('Creates a csv file, alphabetically ordered by module, showing all modules from `sites/all/modules/`, and `sites/default/modules/`.'),
    ),
    'aliases' => array(
      'mi',
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );

  return $items;
}

/**
 * Callback function for 'moduleinfo' command.
 */
function drush_moduleinfo() {
  // Options to verify.
  $dirs = drush_get_option_list('dirs');
  $file_path = drush_get_option('filepath', FALSE);

  if ($dirs && $file_path) {
    $results = array();

    // Loop through as many directories as were passed into --dirs argument.
    foreach ($dirs as $dir) {
      if ($handle = opendir($dir)) {
        while (FALSE !== ($module_name = readdir($handle))) {
          $module_directory = $dir . $module_name;
          if (!in_array($module_name, array('.', '..')) && is_dir($module_directory)) {
            // Find and parse info file.
            $info_file_pattern = '%s/%s.info';

            $results[] = array(
              $module_name,
              _moduleinfo_get_version($module_directory, $module_name),
              $dir,
            );
          }
        }
        closedir($handle);
      }
    }

    // Sort by module name if we had more than one directory in the first place.
    if (count($dirs) > 1) {
      usort($results, function($a, $b) {
        return strcasecmp($a[0], $b[0]);
      });
    }

    drush_log(dt('!results_count modules found.', array('!results_count' => count($results))), 'success');

    // Write the csv file.
    $csv = _moduleinfo_write_csv($results, $file_path);

    if ($csv) {
      drush_log(dt('CSV file written to !file_path.', array('!file_path' => $file_path)), 'success');
    }
    else {
      drush_log(dt('CSV file could not be written to !file_path.', array('!file_path' => $file_path)), 'error');
    }
  }
}

/**
 * Helper function for drush_moduleinfo().
 *
 * Finds the module's info file, parses it for a version string, and returns it
 * or the string defined in DRUSH_MODULEINFO_NO_VERSION.
 *
 * @param string $module_directory
 *   Path to the directory containing the module whose info file we're searching
 *   for.
 *
 * @param string $module_name
 *   Name of the module whose info file we're after.
 *
 * @return $version
 *   The version of the module as found in the info file, or the value of the
 *   DRUSH_MODULEINFO_NO_VERSION constant.
 */
function _moduleinfo_get_version($module_directory, $module_name) {
  // Prepare sprintf, regex patterns, and figure the default info file name.
  $info_file_pattern = '%s/%s.info';
  $version_pattern = '/version = "?([67]\.x-\d+\.\d+)"?/m';
  $info_file_name = sprintf($info_file_pattern, $module_directory, $module_name);

  // Most modules have a predictable info file name.
  if (file_exists($info_file_name)) {
    $info_file_contents = file_get_contents($info_file_name);
  }
  // Some modules don't.
  else {
    if ($handle = opendir($module_directory)) {
      while (FALSE !== ($file = readdir($handle))) {
        if (pathinfo($file, PATHINFO_EXTENSION) === 'info') {
          // This is it.
          $info_file_contents = file_get_contents($module_directory . '/' . $file);
          break;
        }
      }
    }
  }

  // Find the version, if any, and make sure we always return something.
  if (isset($info_file_contents) && !empty($info_file_contents)) {
    $result = preg_match($version_pattern, $info_file_contents, $matches);
    $version = $result && isset($matches[1]) ? $matches[1] : DRUSH_MODULEINFO_NO_VERSION;
  }
  else {
    $version = DRUSH_MODULEINFO_NO_VERSION;
  }

  return $version;
}

/**
 * Helper function for drush_moduleinfo().
 *
 * Creates csv file using provided path and results set.
 *
 * @param array $results
 *   Array of results from drush_moduleinfo().
 *
 * @param string $file_path
 *   Path at which to create file.
 *
 * @return bool
 *   Whether or not the file was successfully created.
 */
function _moduleinfo_write_csv($results, $file_path) {
  $file = fopen($file_path, 'w');

  fputcsv($file, explode(',', DRUSH_MODULEINFO_CSV_HEADERS));

  foreach ($results as $fields) {
    fputcsv($file, $fields);
  }

  fclose($file);

  return file_exists($file_path);
}
