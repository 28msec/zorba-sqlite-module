import module namespace s = "http://www.zorba-xquery.com/modules/sqlite";

let $db := s:connect("")
let $result := s:execute-update($db, "CREATE TABLE smalltable (id INTEGER primary key, name TEXT not null, calories TEXT)")
let $result := s:execute-update($db, "INSERT INTO smalltable (name, calories) VALUES ('cholate milk regular',210)")
let $old-db := s:disconnect($db)

return $result