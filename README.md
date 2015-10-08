# Module Info

This Drush command creates an alphabetically-sorted csv file including
the module name, the module version (if available), and the directory
the module was found in.

The main idea is to get a quick listing of modules + versions from a
given module directory when when auditing an existing site--especially
one with more than one modules directory that could contain duplicate
modules.

Use it like this:

`drush mi --folders=sites/all/modules/,sites/default/folders --filepath=./module-list.csv`
