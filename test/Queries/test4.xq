import module namespace s = "http://www.zorba-xquery.com/modules/sqlite";
import module namespace f = "http://expath.org/ns/file";

let $path := f:path-to-native(resolve-uri("./"))
let $db := s:connect(concat($path, "small2.db"))

return {
  variable $prep-statement := s:prepare-statement($db, "SELECT * FROM smalltable");
  variable $meta := s:metadata($prep-statement);
  variable $result := s:execute-query-prepared($prep-statement);
  variable $all-ok := s:disconnect($db);
  variable $cols := $meta("columns");
  
  (for $i in 1 to jn:size($cols)
    return concat($cols($i)("database"), ":", $cols($i)("table"), ":", $cols($i)("name"), " = ", $cols($i)("type"), "; ")
   ,
   for $e in $result
   return $e)
}
