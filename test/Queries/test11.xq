import module namespace s = "http://zorba.io/modules/sqlite";

let $db := s:connect("")

return {
  variable $results := s:execute-query($db, "SELECT id, name, calories FROM smalltable");
  $results
}
