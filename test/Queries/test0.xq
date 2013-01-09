import module namespace s = "http://www.zorba-xquery.com/modules/sqlite";
import module namespace f = "http://expath.org/ns/file";

let $path := f:path-to-native(resolve-uri("./"))
let $db := s:connect(concat($path, "small2.db"))
return {
  variable $isconn := s:is-connected($db);
  variable $result := s:execute-query($db, "select * from smalltable");
  ($result, $isconn)
}
