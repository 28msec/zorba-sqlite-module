import module namespace s = "http://www.zorba-xquery.com/modules/sqlite";

let $db := s:connect("")

return {
  variable $p-stmt1 := s:prepare-statement($db, "CREATE TABLE smalltable (id INTEGER primary key asc, name TEXT not null, calories TEXT)");
  variable $inst := s:execute-update-prepared($p-stmt1);
  variable $p-stmt2 := s:prepare-statement($db, "INSERT INTO smalltable (name, calories) VALUES (?, ?)");
  s:clear-params($p-stmt2);
  s:set-string($p-stmt2, 1, 'one');
  s:set-string($p-stmt2, 2, '100');
  variable $upd1 := s:execute-update-prepared($p-stmt2);
  variable $p-stmt3 := s:prepare-statement($db, 'SELECT * FROM smalltable');
  variable $res := s:execute-query-prepared($p-stmt3);
  s:close-prepared($p-stmt1);
  s:close-prepared($p-stmt2);
  $res
}
