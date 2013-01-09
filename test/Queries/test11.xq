import module namespace s = "http://www.zorba-xquery.com/modules/sqlite";

let $db := s:connect("")

return {
  variable $results := s:execute-query($db, "SELECT id, name, calories FROM smalltable");
  $results
}
