import module namespace s = "http://www.zorba-xquery.com/modules/sqlite";

let $db := s:connect("")

return {
  variable $result0 := s:execute-update($db, "CREATE TABLE smalltable (id INTEGER primary key, name TEXT not null, calories TEXT)");
  variable $result1 := s:execute-update($db, "INSERT INTO smalltable (name, calories) VALUES ('cholate milk regular',210)");
  variable $old-db := s:disconnect($db);
  $result1
}