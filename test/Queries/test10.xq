import module namespace s = "http://www.zorba-xquery.com/modules/sqlite";

let $db := s:connect("", { "open-read-only" : false, "open-create" : false })
let $inst := s:execute-update($db, "CREATE TABLE smalltable (id INTEGER primary key asc, name TEXT not null, calories TEXT)")
let $prep-stmt := s:prepare-statement($db, "INSERT INTO smalltable (name, calories) VALUES ('one', '100')")

return s:execute-prepared($prep-stmt)