import module namespace s = "http://www.zorba-xquery.com/modules/sqlite";

let $db := s:connect("")
let $prep-stmnt1 := s:prepare-statement($db, "CREATE TABLE smalltable (id INTEGER primary key, name TEXT not null, calories INTEGER not null)")
let $not := s:set-numeric($prep-stmnt1, 1, 0)
let $db := s:disconnect($db)

return $prep-stmnt1