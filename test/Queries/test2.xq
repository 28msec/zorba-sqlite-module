import module namespace s = "http://www.zorba-xquery.com/modules/sqlite";
import module namespace f = "http://expath.org/ns/file";

let $path := f:path-to-native(resolve-uri("./"))
let $db := s:connect(concat($path, "small2.db"))
let $result := s:execute-query($db, "SELECT * FROM smalltable")
let $old-db := s:disconnect($db)

for $e in $result
let $cal := $e("calories")
order by $cal
return $e