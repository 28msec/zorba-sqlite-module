import module namespace s = "http://zorba.io/modules/sqlite";
import module namespace f = "http://expath.org/ns/file";

let $path := f:path-to-native(resolve-uri("./"))
let $db := s:connect(concat($path, "small2.db"))

return {
  variable $result := s:execute-query($db, "SELECT * FROM smalltable");
  for $e in $result
  let $cal := $e("calories")
  order by $cal
  return $e
}
