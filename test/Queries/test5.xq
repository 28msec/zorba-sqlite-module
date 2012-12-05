import module namespace s = "http://www.zorba-xquery.com/modules/sqlite";
import module namespace f = "http://expath.org/ns/file";

let $path := f:path-to-native(resolve-uri("./"))
let $db := s:connect(concat($path, "small2.db"))
let $prep-statement := s:prepare-statement($db, "SELECT * FROM smalltable WHERE calories > ?")
let $meta := s:metadata($prep-statement)
let $nothing := s:set-value($prep-statement, 1, 100)
let $result := s:execute-query-prepared($prep-statement)
let $db := s:disconnect($db)

for $e in $result
return $e