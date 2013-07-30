import module namespace s = "http://zorba.io/modules/sqlite";

let $db := s:connect("")

return {
  variable $prep-stmnt1 := s:prepare-statement($db, "CREATE TABLE smalltable (id INTEGER primary key, name TEXT not null, calories INTEGER not null)");
  variable $res1 := s:execute-update-prepared($prep-stmnt1);
  variable $prep-stmnt2 := s:prepare-statement($db, "INSERT INTO smalltable (name, calories) VALUES ('carrot', ?)");
  variable $not := s:set-numeric($prep-stmnt2, 1, "string value");
  $prep-stmnt1
}
