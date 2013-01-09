import module namespace s = "http://www.zorba-xquery.com/modules/sqlite";

let $db := s:connect("")

return {
  variable $prep-stmnt1 := s:prepare-statement($db, "CREATE TABLE smalltable (id INTEGER primary key, name TEXT not null, calories INTEGER not null)");
  variable $prep-stmnt2 := s:prepare-statement($prep-stmnt1, "INSERT INTO smalltable (name, calories) VALUES ('carrot', 80)");
  $prep-stmnt2
}
