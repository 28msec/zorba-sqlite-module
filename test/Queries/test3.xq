import module namespace s = "http://www.zorba-xquery.com/modules/sqlite";
import module namespace f = "http://expath.org/ns/file";

let $path := f:path-to-native(resolve-uri("./"))
let $db := s:connect(concat($path, "small2.db"))

return {
  variable $prep-statement := s:prepare-statement($db, "SELECT * FROM smalltable");
  variable $meta := s:metadata($prep-statement);
  variable $old-db := s:disconnect($db);
  $meta
}