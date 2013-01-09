import module namespace s = "http://www.zorba-xquery.com/modules/sqlite";

let $db := s:connect("")

return {
  variable $prep-stmnt1 := s:prepare-statement($db, "CREATE TABLE smalltable (id INTEGER primary key, name TEXT not null, calories INTEGER not null)");
  s:set-numeric($prep-stmnt1, 1, 0);
  $prep-stmnt1
}
