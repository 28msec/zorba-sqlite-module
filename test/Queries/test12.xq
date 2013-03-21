import module namespace s = "http://www.zorba-xquery.com/modules/sqlite";

let $db := s:connect("non-existent-file.db", {"open-read-only" : false(), "open-create" : false()})

return {
  variable $results := s:execute-query($db, "SELECT id, name, calories FROM smalltable");
  $results
}
