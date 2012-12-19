import module namespace s = "http://www.zorba-xquery.com/modules/sqlite";
import module namespace f = "http://expath.org/ns/file";

let $path := f:path-to-native(resolve-uri("./"))
let $db := s:connect(concat($path, "small2.db"))

return {
  variable $results := s:execute-query($db, "SELECT id, name, calories FROM smalltable");
  for $e in $results
  let $id := $e("id")
  let $name := $e("name")
  let $calories := $e("calories")
  return <food><id>{$id}</id><name>{$name}</name><calories>{$calories}</calories></food>
}