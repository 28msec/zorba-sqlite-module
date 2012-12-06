import module namespace s = "http://www.zorba-xquery.com/modules/sqlite";

let $db := s:connect("")
let $prep-stmnt1 := s:prepare-statement($db, "CREATE TABLE smalltable (id INTEGER primary key, name TEXT not null, calories INTEGER not null)")
let $result := s:execute-prepared($db)
let $db := s:disconnect($db)

return $result