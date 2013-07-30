import module namespace s = "http://zorba.io/modules/sqlite";

let $db := s:connect("")

return {
  variable $prep-stmnt1 := s:prepare-statement($db, "CREATE TABLE smalltable (id INTEGER primary key, name TEXT not null, calories INTEGER not null)");
  variable $result := s:execute-update-prepared($db);
  {"Affected Rows" : $result}
}
