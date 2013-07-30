import module namespace s = "http://zorba.io/modules/sqlite";
import module namespace f = "http://expath.org/ns/file";

let $path := f:path-to-native(resolve-uri("./"))
let $db := s:connect(concat($path, "small2.db"))

return {
  variable $prep-statement := s:prepare-statement($db, "SELECT * FROM smalltable WHERE calories > ?");
  variable $meta := s:metadata($prep-statement);
  s:set-value($prep-statement, 1, 100);
  variable $result := s:execute-query-prepared($prep-statement);

  for $e in $result
  return $e
}
